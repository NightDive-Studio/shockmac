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
//		System Shock - �1994-1995 Looking Glass Technologies, Inc.
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

//--------------------
//  Globals
//--------------------
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
long				gAlogDirID;
short				gAlogVref;
long				gBarkDirID;
short				gBarkVref;

//------------------------------------------------------------------------------------
//		Initialize the Macintosh managers.
//------------------------------------------------------------------------------------
void InitMac(void)
{
	short		i;
	
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
	
	MaxApplZone();
	for (i=0; i<10; i++)										// Get some room for more handles
		MoreMasters();
	
	// Allocate memory for various things, initialize others
		
	gExtraMemory = NewHandle(16384L);				// Some extra room in case we have to die
	
	FailNIL(gOriginalColors = (ColorSpec *)NewPtr(256 * sizeof(ColorSpec)));		// Original palette
	
	GetDateTime(&gRandSeed);								// Start off with a random seed
	gRandSeed += TickCount()<<8;
	
	GetVol(nil,	&gMainVRef);									// Where was I launched from?
	
	gWatchCurs = GetCursor(watchCursor);
	HNoPurge((Handle)gWatchCurs);
	
//	EnterMovies();
//	InstallTimer();
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
	
	// Check for QuickTime 2.0
	err = Gestalt(gestaltQuickTime, &resp);
	if (err || (!err && resp < 0x02000000))
	 	ErrorDie(6);
	
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
	gActiveTop = ((gScreenHigh - GetMBarHeight()) >> 1) - (gActiveHigh>>1);
  	
	if (gActiveTop < GetMBarHeight())
		gActiveTop = 0;
	gActiveTop += GetMBarHeight();
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
void GetFolders(void)
{
	long						temp;
	HParamBlockRec 	hpb;
	OSErr					err;
	
	// Get the location of our current working directory.
	
	hpb.ioParam.ioCompletion = 0L;
	hpb.fileParam.ioFDirIndex = 0;
 	GetWDInfo(gMainVRef, &hpb.fileParam.ioVRefNum, &hpb.fileParam.ioDirID, &temp);
 	
 	// Now get info on the "Data" directory.
 	
	hpb.fileParam.ioNamePtr = "\pData";
	err = PBGetCatInfo((CInfoPBPtr)&hpb, false);
	
	// If we found it, then set our globals, otherwise die.
	
	if (err == noErr)
	{
		gDataVref = hpb.fileParam.ioVRefNum;
		gDataDirID = hpb.fileParam.ioDirID;
	}
	else
		ErrorDie(12);		// No "Data" folder.
	
	// Now go into the data folder and get the "Alogs" and "Barks" folders.
	
	hpb.fileParam.ioNamePtr = "\pAlogs";
	err = PBGetCatInfo((CInfoPBPtr)&hpb, false);
	if (err == noErr)
	{
		gAlogVref = hpb.fileParam.ioVRefNum;
		gAlogDirID = hpb.fileParam.ioDirID;
	}
	else
		ErrorDie(13);		// No "Alogs" folder.

	hpb.fileParam.ioVRefNum = gDataVref;
	hpb.fileParam.ioDirID = gDataDirID;
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

//------------------------------------------------------------------------------------
//  Close all our resources, then quit.
//------------------------------------------------------------------------------------
void CleanupAndExit(void)
{
	GDHandle	devhandle;

/*
	DeleteTempRecFiles();
	FreeSoundStuff();
	RemoveTimer();
	RemoveAEHandlers();
*/
	if (gOriginalDepth != -1)											// If color depth was changed at beginning of app,
	{																				// then switch it back to the original.
		devhandle = GetMainDevice();
		if (devhandle)
			if (HasDepth(devhandle, gOriginalDepth, 0, 0))
				SetDepth(devhandle, gOriginalDepth, 0, 0);
	} 
	else
		CleanupPalette();													// Else switch back to original 8-bit palette.
	
/*	ExitMovies();
	
	gr_close();
	kb_shutdown();
	ResTerm();*/
	
	ExitToShell();
}
