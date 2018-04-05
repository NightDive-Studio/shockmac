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
//==============================================================================
//
//		System Shock - й1994-1995 Looking Glass Technologies, Inc.
//
//		MacTune.c	-	Rewrite of Shock's MLIMBS.C file to use QuickTime MIDI rather than AIL.
//
//==============================================================================


#include "MacTune.h"
#include "musicai.h"


//-----------------
//  GLOBALS
//-----------------
bool		mlimbs_on = FALSE;
char		mlimbs_status = 0;

struct	mlimbs_request_info current_request[MLIMBS_MAX_SEQUENCES - 1]; // Request information

ulong		mlimbs_counter = 0;
long		mlimbs_error;
bool		mlimbs_semaphore = FALSE;

Handle			gHeaderHdl, gTuneHdl, gOfsHdl;			// Holds the tune-related data for the current theme file.
long				*gOffsets;										// Array of offsets for the beginning of each tune.
TunePlayer	gPlayer;										// The Tune Player.
Boolean			gTuneDone;										// True when a sequence has finished playing (set by CB proc).
Boolean			gReadyToQueue;								// True when it's time to queue up a new sequence.
int				gOverlayTime;								// Amount of time (in millisecs) to wait for overlays.
int				gQueueTime;									// Amount of time (in millisecs) to wait to queue next tune.

TuneCallBackUPP	gTuneCBProc;							// Pointer to tune-finished callback proc.

CalcTuneTask	gCalcTuneTask;								// Global to hold task info.
TimerUPP		gCalcTuneProcPtr;							// UPP for the 6-second time manager tune determiner task.


//-------------------
//  INTERNAL PROTOTYPES
//-------------------
pascal void TuneEndCB(const TuneStatus *status, long refCon);


//---------------------------------------------------------------
//  The following section is the time manager task for determining the next tune to play.
//---------------------------------------------------------------
#pragma require_prototypes off

#ifndef __powerc
CalcTuneTaskPtr GetCalcTuneTask(void) = 0x2049;				// MOVE.L A1,A0
#endif

//---------------------------------------------------------------
#ifdef __powerc
pascal void CalcTuneProc(TMTaskPtr)
#else
pascal void CalcTuneProc(void)
#endif
{
#ifndef __powerc
	CalcTuneTaskPtr	tmTaskPtr = GetCalcTuneTask();			// get address of task record
	long					curA5 = SetA5(tmTaskPtr->appA5);		// save and set value of A5
#endif
	
	gReadyToQueue = TRUE;								// It's time to queue up another tune.
	
#ifndef __powerc
	SetA5(curA5);											// restore A5
#endif
}
#pragma require_prototypes on


//------------------------------------------------------------------------------
//  Initializes the MacTune system.
//------------------------------------------------------------------------------
int MacTuneInit(void)
{
	if (mlimbs_status != 0)								// If already inited, return
		return 0;

//еее   if (!music_card) return -1;
// Put something here to check for the existence of the QuickTime Musical Instruments.  Or maybe
// in music_init.

	// See if there is enough memory to load the QuickTime Music stuff.  If not, return an error.
	Handle	syshdl = NewHandleSys(819200);		// Try to allocate an 800K handle in system heap.
	if (!syshdl)													// If it couldn't allocate the memory,
		return 1;													// return an error.
	else
		DisposeHandle(syshdl);								// We don't need this, it was just a test.

	// Load up a player to play the tunes.
	gPlayer = OpenDefaultComponent(kTunePlayerType, 0);
	if (!gPlayer)
	{
		DebugStr("\pError:  Could not open a tune player.");		//еее Handle this!
		return 2;
	}
	
	// Setup the end-of-tune callback proc.
	gTuneCBProc = NewTuneCallBackProc(TuneEndCB);
	
	// Setup a time-manager task for queueing up new tunes.
	gCalcTuneProcPtr = NewTimerProc(CalcTuneProc);			// Make a UPP for the TM task
	gCalcTuneTask.task.tmAddr = gCalcTuneProcPtr;					// Insert the calc tune TM task
	gCalcTuneTask.task.tmWakeUp = 0;
	gCalcTuneTask.task.tmReserved = 0;
#ifndef __powerc
	gCalcTuneTask.appA5 = SetCurrentA5();
#endif
	InsTime((QElemPtr)&gCalcTuneTask);

	// Initialize some globals.
 	gHeaderHdl = NULL;
 	gTuneHdl = NULL;
 	gOfsHdl = NULL;
	mlimbs_status = 1;

	return 0;
}

//------------------------------------------------------------------------------
//  Shuts down the system so it stops playing music and releases all resources.
//------------------------------------------------------------------------------
void MacTuneShutdown(void)
{
	if (mlimbs_status == 0)									// If already shut down, do nothing
   		return;

	MacTunePurgeCurrentTheme();						// Kill the current theme (unloading theme data)

	RmvTime((QElemPtr)&gCalcTuneTask);			// Stop the calc tune task
	DisposeRoutineDescriptor(gCalcTuneProcPtr);	// Dispose its UPP
	
	DisposeRoutineDescriptor(gTuneCBProc);			// Cleanup callback and player
	CloseComponent(gPlayer);

	mlimbs_status = 0;
}

//--------------------------------------------------------------------------
//	Call-back routine.  Get's called when tune is finished.
//--------------------------------------------------------------------------
pascal void TuneEndCB(const TuneStatus *, long)
{
	gTuneDone = TRUE;
}

//------------------------------------------------------------------------------
//  Loads all resources associated with a theme file.  Stops and purges and currently loaded theme.  Returns
//  a 1 if successful, < 0 if an error.
//------------------------------------------------------------------------------
int MacTuneLoadTheme(FSSpec *themeSpec, int themeID)
{
	short  	filenum;
	Handle	binHdl;
	Ptr		p;
	
	extern uchar track_table[NUM_SCORES][SUPERCHUNKS_PER_SCORE]; 
	extern uchar transition_table[NUM_TRANSITIONS];
	extern uchar layering_table[NUM_LAYERS][MAX_KEYS];
	extern uchar key_table[NUM_LAYERABLE_SUPERCHUNKS][KEY_BAR_RESOLUTION];

	if (mlimbs_status == 0)											// Only do this if MacTune is inited.
		return (-1);
	
	MacTunePurgeCurrentTheme();								// Purge the current theme.
		
	// Open the theme file.
	filenum = FSpOpenResFile(themeSpec, fsRdPerm);
	if (filenum == -1)
		return (-2);

	// First, get the 'tbin' resource and copy its data to the MusicAI global arrays.
	binHdl = GetResource('tbin', 128);
	if (binHdl == NULL)
	{
		CloseResFile(filenum);
Debugger();	//еее
		return (-3);
	}
	HLock(binHdl);
	p = *binHdl;
	BlockMoveData(p, track_table, NUM_SCORES * SUPERCHUNKS_PER_SCORE);
	p += NUM_SCORES * SUPERCHUNKS_PER_SCORE;
	BlockMoveData(p, transition_table, NUM_TRANSITIONS);
	p += NUM_TRANSITIONS;
	BlockMoveData(p, layering_table, NUM_LAYERS * MAX_KEYS);
	p += NUM_LAYERS * MAX_KEYS;	
	BlockMoveData(p, key_table, NUM_LAYERABLE_SUPERCHUNKS * KEY_BAR_RESOLUTION);
	p += NUM_LAYERABLE_SUPERCHUNKS * KEY_BAR_RESOLUTION;
	gOverlayTime = *(int *)p;
	p += 4;
	gQueueTime = *(int *)p;
	HUnlock(binHdl);
	ReleaseResource(binHdl);
	
	// Next, get the theme-related resources.
	gHeaderHdl = GetResource('thdr', 128);
	if (gHeaderHdl == NULL)
	{
		CloseResFile(filenum);
		return (-4);
	}
	gTuneHdl = GetResource('them', 128);
	if (gTuneHdl == NULL)
	{
		CloseResFile(filenum);
		return (-5);
	}
	gOfsHdl = GetResource('tofs', 128);
	if (gOfsHdl == NULL)
	{
		CloseResFile(filenum);
		return (-6);
	}
	
	DetachResource(gHeaderHdl);						// Turn these into normal handles.
	HLockHi(gHeaderHdl);
	DetachResource(gTuneHdl);	
	HLockHi(gTuneHdl);
	DetachResource(gOfsHdl);
	HLockHi(gOfsHdl);
	
	CloseResFile(filenum);
	
	// Set the tune header (load instruments, etc, can take a second or two).
	TuneSetHeader(gPlayer, (unsigned long *)*gHeaderHdl);
	TunePreroll(gPlayer);
	
	// Setup the tune offset pointer.
	gOffsets = (long *)*gOfsHdl;
	
	// Initialize our playtime globals.
	gTuneDone = FALSE;
	gReadyToQueue = FALSE;

	// Here's a big hack.  If we're loading theme 0 (machine sounds only), then don't do an
	// intro transition.
	if (themeID == 0)
	   current_mode = NORMAL_MODE;

	return(1);
}

//------------------------------------------------------------------------------
//  If there is a theme loaded, start playing it.
//------------------------------------------------------------------------------
void MacTuneStartCurrentTheme(void)
{	
	if (mlimbs_status && gTuneHdl)					// If MacTune is inited and there is a theme loaded,
	{
		int	pid = current_request[0].pieceID;
		if (pid != 255)										// If there is a tune requested,
		{
			MacTunePlayTune(pid);						// play it right now.
		}
		else
			gTuneDone = TRUE;								// else make sure we check again soon.

	}
}

//------------------------------------------------------------------------------
//  Stop the current tune playing, stop the tune queue timer task.
//------------------------------------------------------------------------------
void MacTuneKillCurrentTheme(void)
{
	if (mlimbs_status && gTuneHdl)						// Only do this if MacTune is inited and there is a 
	{																	// current theme.
		RmvTime((QElemPtr)&gCalcTuneTask);		// Remove the TimeManager task that queues up
		InsTime((QElemPtr)&gCalcTuneTask);			// next tune, and re-insert to prevent any tasks from 
																		// triggering.
		TuneStop(gPlayer, kStopTuneFade);				// Stop the current tune (if any).
		TuneFlush(gPlayer);									// Flush the queue.
		
		gReadyToQueue = FALSE;
	}
}

//------------------------------------------------------------------------------
//  Stop the current tune playing, stop the tune queue timer task.
//------------------------------------------------------------------------------
void MacTunePurgeCurrentTheme()
{
	MacTuneKillCurrentTheme();							// Kill the current theme.
	
	// Dispose of all the theme's data.
	if (gHeaderHdl)
	{
		HUnlock(gHeaderHdl);
		DisposeHandle(gHeaderHdl);
		gHeaderHdl = NULL;
	}
	if (gTuneHdl)
	{
		HUnlock(gTuneHdl);
		DisposeHandle(gTuneHdl);
		gTuneHdl = NULL;
	}
	if (gOfsHdl)
	{
		HUnlock(gOfsHdl);
		DisposeHandle(gOfsHdl);
		gOfsHdl = NULL;
	}
	
	// Free the tune player component, then open it back up again.  This should make the
	// game run much faster.
	CloseComponent(gPlayer);
	gPlayer = OpenDefaultComponent(kTunePlayerType, 0);
	if (!gPlayer)
		DebugStr("\pError:  Could not open a tune player.");		//еее Handle this!

	// Clear our the current request array.
	for (int i = 0; i < MLIMBS_MAX_SEQUENCES -1; i++)
	{
		current_request[i].pieceID = 255;
	}
	mlimbs_counter = 0;
}

//------------------------------------------------------------------------------
//  Play a tune right now (prime the TM task).  Usually this is called when music is first started.
//------------------------------------------------------------------------------
void MacTunePlayTune(int tune)
{
if (tune == 255 || tune == -1)
	DebugStr("\pEep Eep Invalid tune!");  //еее
	
	if (gOffsets[tune] != -1)							// If there really is a tune there, play it now.
	{
		TuneQueue(gPlayer, (unsigned long *)(*gTuneHdl + gOffsets[tune]), 0x10000,
						0, 0x7FFFFFFF, kTuneStartNow, gTuneCBProc, 0);
		PrimeTime((QElemPtr)&gCalcTuneTask, gOverlayTime + gQueueTime);
//еее temp
// the above amount for PrimeTime is temporary because we're not doing overlays yet.
//  so just queue up the next tune at queue time.
	}
	
	// If there was no tune to play this time, set a flag so it will prime the timer again.
	else
		gTuneDone = TRUE;
}

//------------------------------------------------------------------------------
//  Add a tune to the tune queue.
//------------------------------------------------------------------------------
void MacTuneQueueTune(int tune)
{
if (tune == 255 || tune == -1)
	DebugStr("\pEep Eep Invalid tune!");  //еее
	
	if (gOffsets[tune] != -1)							// If there really is a tune there, queue it up.
	{
		TuneStatus	tpStatus;
		TuneGetStatus(gPlayer, &tpStatus);		// We'll need this later.

		TuneQueue(gPlayer, (unsigned long *)(*gTuneHdl + gOffsets[tune]), 0x10000,
						0, 0x7FFFFFFF, 0, gTuneCBProc, 0);

		// Normally we don't prime it yet; we want to wait until the currently playing tune finishes.
		// However, if tune was playing before we queued this one, go ahead and prime away.
		if (tpStatus.queueTime == 0)
			PrimeTime((QElemPtr)&gCalcTuneTask, gOverlayTime + gQueueTime);
	}
	
	// If there was no tune to queue this time, set a flag so it will prime the timer again.
	else
		gTuneDone = TRUE;
}

//------------------------------------------------------------------------------
//  Prime the TM task to trigger the next tune queueing.
//------------------------------------------------------------------------------
void MacTunePrimeTimer(void)
{
	PrimeTime((QElemPtr)&gCalcTuneTask, gOverlayTime + gQueueTime);
//еее temp
// the above amount for PrimeTime is temporary because we're not doing overlays yet.
//  so just queue up the next tune at queue time.
}
