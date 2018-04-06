/*

Copyright (C) 2015-2018 Night Dive Studios, LLC.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
*/
//====================================================================================
//
//		System Shock - й1994-1995 Looking Glass Technologies, Inc.
//
//		InitMac.c	-	Initialize Mac toolbox managers and setup the application's globals.
//
//====================================================================================


//--------------------
//  Includes
//--------------------
#include <Palettes.h>
#include <GestaltEqu.h>
#include <Movies.h>

#include "Shock.h"
#include "InitMac.h"
#include "ShockBitmap.h"
#include "MacTune.h"

//--------------------
//  Globals
//--------------------
#ifndef __MWERKS__
QDGlobals	qd;
#endif
Handle			gExtraMemory = nil;
ColorSpec 		*gOriginalColors;
unsigned long	gRandSeed;
short				gMainVRef;
CursHandle		gWatchCurs;
short				gOriginalDepth = -1;
short				gLastAlertDepth = -1;
short				gStartupDepth;
Ptr				gScreenAddress;
long				gScreenRowbytes;
short				gScreenWide, gScreenHigh;
short				gActiveWide, gActiveHigh;
short				gActiveLeft, gActiveTop;
Rect				gActiveArea, gOffActiveArea;
Boolean			gIsPowerPC = false;
long				gDataDirID;
short				gDataVref;
long				gCDDataDirID;
short				gCDDataVref;
long				gAlogDirID;
short				gAlogVref;
long				gBarkDirID;
short				gBarkVref;
Boolean			gMenusHid;

//---------------------------
//  Externs
//---------------------------
void status_bio_update(void);
extern bool gBioInited;
pascal void MousePollProc(void);


//---------------------------
//  Internal Prototypes
//---------------------------
void Cleanup(void);
pascal void ETSPatch(void);
void InstallETSPatch(void);

//---------------------------
//  Time Manager routines and globals
//---------------------------
TimerUPP		pShockTicksPtr;				// Globals for the Shock "tickcount" TM task.
ShockTask		pShockTicksTask;			// It increments gShockTicks 280 times per second.
long				gShockTicks;
long 				*tmd_ticks;
//Handle			gTaskHdl;

#if __profile__
#pragma profile off
#endif
//---------------------------------------------------------------
//  The following section is the time manager task for incrementing the Shock timer.
//---------------------------------------------------------------
#pragma require_prototypes off

#ifndef __powerc
ShockTaskPtr GetShockTask(void) = 0x2049;							// MOVE.L A1,A0
#endif

//---------------------------------------------------------------
#ifdef __powerc
pascal void ShockTicksProc(TMTaskPtr tmTaskPtr)
#else
pascal void ShockTicksProc(void)
#endif
{
#ifndef __powerc
	 ShockTaskPtr	tmTaskPtr = GetShockTask();				// get address of task record
#endif
	
	(*(((ShockTaskPtr)tmTaskPtr)->ticksPtr)) += 4;		// increment by 4 to fake 280 ticks/sec counter.

	if ((*(((ShockTaskPtr)tmTaskPtr)->ticksPtr)) % 8 == 0)
		MousePollProc();													// update the cursor 35 times/sec
	status_bio_update();													// draw the biometer

	PrimeTime((QElemPtr)tmTaskPtr, kShockTicksFreq);	// Do this 70 times a second.
}
#pragma require_prototypes on

#if __profile__
#pragma profile on
#endif

//------------------------------------------------------------------------------------
//		Initialize the Macintosh managers.
//------------------------------------------------------------------------------------
void InitMac(void)
{
	OSErr	err;
	long		resp;
	
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
	
	MaxApplZone();
	for (int i=0; i<10; i++)										// Get some room for more handles
		MoreMasters();
	
	// Allocate memory for various things, initialize others
		
	gExtraMemory = NewHandle(16384L);				// Some extra room in case we have to die
	
	FailNIL(gOriginalColors = (ColorSpec *)NewPtr(256 * sizeof(ColorSpec)));		// Original palette
	
	GetDateTime(&gRandSeed);								// Start off with a random seed
	gRandSeed += TickCount()<<8;
	
	GetVol(nil,	&gMainVRef);									// Where was I launched from?
	
	gWatchCurs = GetCursor(watchCursor);
	HNoPurge((Handle)gWatchCurs);
	
	InstallETSPatch();											// Ensure that we always cleanup at quit time

	// Check for QuickTime 2.1
	err = Gestalt(gestaltQuickTime, &resp);
	if (err || (!err && resp < 0x02100000))
	 	ErrorDie(6);
	
	EnterMovies();
	InstallShockTimers();
	
	gMenusHid = FALSE;
 }

//------------------------------------------------------------------------------------
//		Make sure the game can run on this mo-sheen.
//------------------------------------------------------------------------------------
void CheckConfig(void)
{
	OSErr				err;
	long					resp;
	int					depth;
	GDHandle     		devhandle;
	PixMapHandle 	pmhan;
	
	// Check for System 7
	err = Gestalt(gestaltSystemVersion, &resp);
	if (err || (!err && resp < 0x0700))
	 	ErrorDie(2);
	
	// Check for 32-bit mode
	err = Gestalt(gestaltAddressingModeAttr, &resp);
	if (err || (!err && (resp & (1 << gestalt32BitAddressing) == 0)))
	 	ErrorDie(8);
	
	// Check for Color QD
	err = Gestalt(gestaltQuickdrawFeatures, &resp);
	if (!err && (resp & (1 << gestaltHasColor)))
	{
		devhandle = GetMainDevice();
		pmhan = (*devhandle)->gdPMap;
		depth = (*pmhan)->pixelSize;
		
		// if we're in 8 bit, save off the color table.  If not, check to see if it is available
		// and switch to it if we can.  Also save the original color depth.
		
		if (depth == 8)
			BlockMove((**((*pmhan)->pmTable)).ctTable, gOriginalColors, 256*sizeof(ColorSpec));
		else
		{
			if (HasDepth(devhandle,8,0,0))
			{
				InitCursor();
				if (StopAlert(1002, nil) == 1)
				{
					gOriginalDepth = depth;								// Save original depth so we can switch back
					SetDepth(devhandle,8,0,0);
					devhandle = GetMainDevice();
					pmhan = (*devhandle)->gdPMap;
					depth = (*pmhan)->pixelSize;
				}
				else
					CleanupAndExit();
			}
			else
				ErrorDie(4);
		}
	}
	else
		ErrorDie(4);
	
	// Check for Sound Manager 3.0.  We do this by checking for multiple channel support, which is only
	// available with SM 3.0.  If it returns an error, then die.
	err = Gestalt(gestaltSoundAttr, &resp);
	if (err || (!err && (resp & (1 << gestaltMultiChannels) == 0)))
	 	ErrorDie(7);
	
	// Record info about the main monitor size.
	gStartupDepth = depth;
	gScreenRowbytes = (long)((*pmhan)->rowBytes & 0x7FFF);
	gScreenAddress = (*pmhan)->baseAddr;
	gScreenWide = (*pmhan)->bounds.right - (*pmhan)->bounds.left;
	gScreenHigh = (*pmhan)->bounds.bottom - (*pmhan)->bounds.top;
	
	// If the screen is larger than 640x480, then center the "active" area in the screen.
	if (gScreenWide >= 640 && gScreenHigh >= 480)
	{
		gActiveWide = screenMaxX;
		gActiveHigh = screenMaxY;
	}
	else
		ErrorDie(11);
	 
	gActiveLeft = ((gScreenWide>>1) - (gActiveWide>>1)) & 0x7FFE;		// put it on even byte
	gActiveTop = (gScreenHigh >> 1) - (gActiveHigh>>1);
  	
	SetRect(&gActiveArea, gActiveLeft, gActiveTop, gActiveWide+gActiveLeft, gActiveHigh+gActiveTop);
	SetRect(&gOffActiveArea, 0, 0, gActiveWide, gActiveHigh);
	
 	// Fix up ScreenAddress (so it really points to the first address of the active area)
  	gScreenAddress += (gScreenRowbytes * (long)gActiveTop);
  	gScreenAddress += gActiveLeft;
 
	// Check to see if we're running on a PowerPC
	err = Gestalt(gestaltSysArchitecture, &resp);
	if (!err && (resp & (1 << gestaltPowerPC)))
		gIsPowerPC = true;
}

//------------------------------------------------------------------------------------
//		Make a color window the size of the main screen, and black it out.
//------------------------------------------------------------------------------------
void SetupWindows(WindowPtr *mainWind)
{	
	FailNIL(*mainWind = GetNewCWindow(1000, 0L, (WindowPtr)-1L));
	
	SizeWindow(*mainWind, gScreenWide, gScreenHigh, false);
	MoveWindow(*mainWind, 0, 0, true);
	
	SetPort(*mainWind);
	SetOrigin(-gActiveLeft, -gActiveTop);								// Set the main window's origin
	OffsetRect(&gActiveArea, -gActiveLeft, -gActiveTop);
	
	ShowWindow(*mainWind);
	PaintRect(&(*mainWind)->portRect);		// black it out
 }

//------------------------------------------------------------------------------------
//  Load and install the standard menus.
//------------------------------------------------------------------------------------
void SetUpMenus(MenuHandle *theMenus, short numMenus)
{
	short		i;
	
	for (i=0; i<numMenus; i++)
		FailNIL(theMenus[i] = GetMenu(128+i));		// get menu resources
	
	AddResMenu(theMenus[0],'DRVR'); 						// add the apple menu items
	
	for (i=0; i<numMenus; i++)
		InsertMenu(theMenus[i], 0);							// Insert apple, file, edit, etc.
	
	DrawMenuBar();
}

//------------------------------------------------------------------------------------
//  Get the dirID and Vref for any folders Shock uses.
//------------------------------------------------------------------------------------
//#define PLAY_FROM_CD				//еее Make sure this is defined for shipping version.
void GetFolders(void)
{
	long						temp;
	HParamBlockRec 	hpb;
	OSErr					err, verr;
	Str255					volName;

	// Get the location of our current working directory.
	
	hpb.ioParam.ioCompletion = 0L;
	hpb.fileParam.ioFDirIndex = 0;
 	GetWDInfo(gMainVRef, &hpb.fileParam.ioVRefNum, &hpb.fileParam.ioDirID, &temp);
 	
 	// Now get info on the "Data" directory.
 	
	hpb.fileParam.ioNamePtr = "\pData";
	err = PBGetCatInfo((CInfoPBPtr)&hpb, false);
	if (err == noErr)
	{
		gDataVref = hpb.fileParam.ioVRefNum;
		gDataDirID = hpb.fileParam.ioDirID;
	}
	else
		ErrorDie(12);		// No "Data" folder.

	// Check for the CD-ROM volume, and look for sub-folders there.

	Boolean	foundCD = FALSE;
	int		volIndex = 1;
	do
	{
		hpb.volumeParam.ioCompletion = NULL;	
		hpb.volumeParam.ioVolIndex = volIndex;		// look for volume based on volume index.
		hpb.volumeParam.ioNamePtr = volName;
		verr = PBHGetVInfo(&hpb, FALSE);				// Get the volume info.

		if (verr == noErr)										// See if there's a folder called "System Shock Data"
		{
			hpb.fileParam.ioDirID = 2;						// Root is always 2
			hpb.fileParam.ioFDirIndex = 0;
			hpb.fileParam.ioNamePtr = "\pSystem Shock Data";
			err = PBGetCatInfo((CInfoPBPtr)&hpb, false);
			if (err == noErr)												// If no error, this is our drive!
			{
				gCDDataVref = hpb.fileParam.ioVRefNum;
				gCDDataDirID = hpb.fileParam.ioDirID;
				foundCD = TRUE;
				break;
			}
		}
		volIndex++;												// Check the next volume.
	} while (verr == noErr);

	if (!foundCD)
		ErrorDie(15);											// Can't find "System Shock" CD volume.
	
	// Now go into the CD data folder and find the "Alogs" and "Barks" folders.
	
	hpb.fileParam.ioNamePtr = "\pAlogs";
	err = PBGetCatInfo((CInfoPBPtr)&hpb, false);
	if (err == noErr)
	{
		gAlogVref = hpb.fileParam.ioVRefNum;
		gAlogDirID = hpb.fileParam.ioDirID;
	}
	else
		ErrorDie(13);		// No "Alogs" folder.

	hpb.fileParam.ioVRefNum = gCDDataVref;
	hpb.fileParam.ioDirID = gCDDataDirID;
	hpb.fileParam.ioNamePtr = "\pBarks";
	err = PBGetCatInfo((CInfoPBPtr)&hpb, false);
	if (err == noErr)
	{
		gBarkVref = hpb.fileParam.ioVRefNum;
		gBarkDirID = hpb.fileParam.ioDirID;
	}
	else
		ErrorDie(14);		// No "Barks" folder.		
}

//------------------------------------------------------------------------------------
//		Check a memory address (handle or pointer) to see if its NIL, and wasn't allocated.
// 	If it was, we have to fail out of the program.
//------------------------------------------------------------------------------------
void FailNIL(void *memory)
{	
	if (!memory)
	{
		if (gExtraMemory)
			DisposHandle(gExtraMemory);
		
		ErrorDie(1);
	} 
}

//------------------------------------------------------------------------------------
//		Get a resource and fail correctly if it can't be loaded.
//------------------------------------------------------------------------------------
Handle GetResourceFail(long id, short num)
{
	Handle 	h;
	
	h = GetResource(id, num);
	if (h) return(h);
	
	// At this point GetResource failed, figure out why.
	SetResLoad(false);
	h = GetResource(id, num);
	SetResLoad(true);
	
	if (gExtraMemory)
		DisposHandle(gExtraMemory);
	
	if (h) 
		ErrorDie(1);		// resource is there, must be a memory problem
	else
		ErrorDie(3);		// resource not there, somethings bad
		
	return (nil);
}


//------------------------------------------------------------------------------------
//  Startup the SystemShock timer.
//------------------------------------------------------------------------------------
void InstallShockTimers(void)
{
	gShockTicks = 0;
	tmd_ticks = &gShockTicks;

	pShockTicksPtr = NewTimerProc(ShockTicksProc);		// Make a UPP for the TM task
	pShockTicksTask.task.tmAddr = pShockTicksPtr;				// Insert the Shock ticks TM task
	pShockTicksTask.task.tmWakeUp = 0;
	pShockTicksTask.task.tmReserved = 0;
	pShockTicksTask.ticksPtr = &gShockTicks;
	
	// Start it when starting the game up.
}

//------------------------------------------------------------------------------------
//  Remove the SystemShock timer.
//------------------------------------------------------------------------------------
void RemoveShockTimers(void)
{
	RmvTime((QElemPtr)&pShockTicksTask);					// Stop the Shock ticks task
	DisposeRoutineDescriptor(pShockTicksPtr);					// Dispose its UPP
}

//------------------------------------------------------------------------------------
//  Prime the System Shock timer.
//------------------------------------------------------------------------------------
void StartShockTimer(void)
{
	pShockTicksTask.task.tmWakeUp = 0;
	pShockTicksTask.task.tmReserved = 0;
	InsXTime((QElemPtr)&pShockTicksTask);
	PrimeTime((QElemPtr)&pShockTicksTask, kShockTicksFreq);	// Increment 280 times a second
}

//------------------------------------------------------------------------------------
//  Stop the System Shock timer.
//------------------------------------------------------------------------------------
void StopShockTimer(void)
{
	RmvTime((QElemPtr)&pShockTicksTask);								// Stop the Shock ticks task
}

//------------------------------------------------------------------------------------
//  Hide the menu bar, sucka.
//------------------------------------------------------------------------------------
void HideMenuBar(void)
 {
 	Rect		r;
 	
 	if (!gMenusHid)
 	{
 		// Make visible region include menu bar area
	 	r = gMainWindow->portRect;
 		RectRgn(gMainWindow->visRgn, &r);
		r.bottom = r.top + LMGetMBarHeight();
 		
 		// If running on a 640 x 480 monitor and the game is playing, restore from the off-screen bitmap.
 		if (gMainWindow->portRect.top == 0)
 		{
			CopyBits(&gMainOffScreen.bits->portBits, &gMainWindow->portBits, &r, &r, srcCopy, 0L);
 		}
 		else 		// Blank the menu bar area
 		{
	 		PaintRect(&r);
	 	}
	 	gMenusHid = TRUE;
	 }
 }
 
//------------------------------------------------------------------------------------
//  Show the menu bar.
//------------------------------------------------------------------------------------
void ShowMenuBar(void)
 {
 	Rect		r;

	if (gMenusHid)
	 {
	 	// Adjust the visible region to exclude the menu bar.
	 	r = gMainWindow->portRect;
		r.top += LMGetMBarHeight();
	 	RectRgn(gMainWindow->visRgn, &r);

		// Show the menu bar.
	 	gMenusHid = FALSE;
	 	DrawMenuBar();
	 }
 }
 
//------------------------------------------------------------------------------------
//  Display an alert using the str# resource with index strignum, then die.
//------------------------------------------------------------------------------------
void ErrorDie(short stringnum)
{
	if (gExtraMemory)
		DisposHandle(gExtraMemory);	// free our extra space
 
 	StringAlert(stringnum);
	CleanupAndExit();
}

//------------------------------------------------------------------------------------
// 	Display an alert using the str# resource with index strignum
//------------------------------------------------------------------------------------
void StringAlert(short stringnum)
{
	Str255		message, explain;
	
	InitCursor();
	GetIndString(message, 1000, stringnum);
	GetIndString(explain, 1001, stringnum);
	ParamText(message, explain, "\p", "\p");
	
	if (*explain)
		StopAlert(1001, nil);
	else
		StopAlert(1000, nil);
}

#pragma mark -
//------------------------------------------------------------------------------------
//  Close all our resources, then quit.
//------------------------------------------------------------------------------------
void Cleanup(void)
{
	GDHandle	devhandle;

	MacTuneShutdown();
	RemoveShockTimers();

	snd_kill_all_samples();
	snd_shutdown();
	
	if (gOriginalDepth != -1)											// If color depth was changed at beginning of app,
	{																				// then switch it back to the original.
		devhandle = GetMainDevice();
		if (devhandle)
			if (HasDepth(devhandle, gOriginalDepth, 0, 0))
				SetDepth(devhandle, gOriginalDepth, 0, 0);
	}
	else
		CleanupPalette();													// Else switch back to original 8-bit palette.
	
	gr_close();
	mouse_shutdown();
	kb_shutdown();
	ResTerm();
}

//------------------------------------------------------------------------------------
//  Normal cleanup when the program quits.
//------------------------------------------------------------------------------------
void CleanupAndExit(void)
{
	Cleanup();
	ExitToShell();
}


#pragma mark -
//------------------------------------------------------------------------------------
// ExitToShell patch
// We patch ExitToShell to ensure that the application cleans up even
// when the user aborts via Command-Option-Escape or the "es" command
// of MacsBug.
//------------------------------------------------------------------------------------

static UniversalProcPtr	sOldETSRoutine;
static UniversalProcPtr	sNewETSRoutine;

enum { kExitToShellProcInfo = kPascalStackBased };

pascal void ETSPatch ()
{
	Cleanup();

	CallUniversalProc(sOldETSRoutine, kExitToShellProcInfo);
}

//------------------------------------------------------------------------------------
// InstallETSPatch
//------------------------------------------------------------------------------------
void InstallETSPatch ()
{
	sOldETSRoutine = ::NGetTrapAddress(_ExitToShell, ToolTrap);
	sNewETSRoutine = NewRoutineDescriptor((ProcPtr) &ETSPatch,
					kExitToShellProcInfo, GetCurrentArchitecture());
	::NSetTrapAddress(sNewETSRoutine, _ExitToShell, ToolTrap);
}
