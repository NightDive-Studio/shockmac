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
//===========================================================================
//
//	PlayETheme.c -	Play some elevator music!
//
//===========================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Timer.h>
#include <Movies.h>
#include <QuickTimeComponents.h>

#include "lg.h"
#include "kb.h"
#include "midi.h"


//------------
// Prototypes
//------------
void main(void);
void EndSequenceCallBack(long seq_id);
void KillCurrentTheme(void);
void MyMusicAI(void);


//------------------
//  Structures
//------------------
typedef struct
{
	TMTask			task;					// The actual TimeManager task structure
	long				appA5;					// We need this silly thing for 68K programs
}
TriggerTask, *TriggerTaskPtr;

//------------
// Globals
//------------
TimerUPP			gTriggerProcPtr;
TriggerTask			gTriggerTask;
void (*seq_miditrig)(void);

short				gVRef;
long					gDirID;

TunePlayer			thePlayer;
short				gCurTheme;
long					gVolume;

Boolean				gTuneDone;				// True when a sequence has finished playing.
Boolean				gReadyToQueue;			// True when it's time to queue up a new sequence.
long					gNextSeq;				// The next sequence to queue up.
long					gLevelState;				// Walking, combat, etc. for the level theme.

#define	kWalking	0
#define	kCombat		1
#define	kDecon		2


//---------------------------------------------------------------
//  The following section is the time manager task for triggering the next sequence queue.
//---------------------------------------------------------------
#pragma require_prototypes off

#ifndef __powerc
TriggerTaskPtr GetTriggerTask(void) = 0x2049;				// MOVE.L A1,A0
#endif

//---------------------------------------------------------------
#ifdef __powerc
pascal void TriggerProc(TMTaskPtr tmTaskPtr)
#else
pascal void TriggerProc(void)
#endif
{
#ifndef __powerc
	 TriggerTaskPtr	tmTaskPtr = GetTriggerTask();			// get address of task record
	long				curA5 = SetA5(tmTaskPtr->appA5);		// save and set value of A5
#endif
	
	// Call the MusicAI routine to determine the next sequence to queue.
	if (seq_miditrig)
		(*seq_miditrig)();
	
#ifndef __powerc
	SetA5(curA5);											// restore A5
#endif
}
#pragma require_prototypes on


//--------------------------------------------------------------------------
//	main -	Open up the theme file and play some elevator music.
//--------------------------------------------------------------------------
void main(void)
{
	WDPBRec	wdpb;
	OSErr		err;
 	kbs_event	event;
	bool 		inloop = TRUE;
	long			statusCount;
	int			seq;
	FSSpec		themeSpec;
	
	printf("\nMidi Theme Player Test Program.   Use the following keys...\n");
	printf("+,-:   Change volume\n");
	printf("1:     Go to level 1\n");
	printf("2:     Go to level 2\n");
	printf("e:     Go to elevator\n");
	printf("m:     Go to menu screen\n");
	printf("c:     Combat mode\n");
	printf("d:     \"Deconstruct\" mode\n");
	printf("w:     Walking mode\n");
	printf("q:     Quit\n\n");

	// Initialize everything.
	EnterMovies();
	kb_startup(NULL);
	snd_start_midi();
	seq_finish = &EndSequenceCallBack;
	seq_miditrig = &MyMusicAI;
	gVolume = 0x10000;

	gTriggerProcPtr = NewTimerProc(TriggerProc);			// Make a UPP for the TM task
	gTriggerTask.task.tmAddr = gTriggerProcPtr;				// Insert the trigger TM task
	gTriggerTask.task.tmWakeUp = 0;
	gTriggerTask.task.tmReserved = 0;
#ifndef __powerc
	gTriggerTask.appA5 = SetCurrentA5();
#endif
	InsTime((QElemPtr)&gTriggerTask);
	
	memset(&wdpb, 0x00, sizeof(wdpb));					// Find out about the current directory
	PBHGetVol(&wdpb, false);
	gVRef = wdpb.ioWDVRefNum;
	gDirID = wdpb.ioWDDirID;

	gCurTheme = -1;
	
	// Load up a player to play the tunes.
	if (snd_set_midi_sequences(8) != SND_OK)
	{
		printf("Error allocating tune players.\n");
		ExitMovies();
		return;
	}
	seq = snd_find_free_sequence();
	thePlayer = snd_get_sequence(seq);

	// Start by playing the title screen theme.
	FSMakeFSSpec(gVRef, gDirID, "\pTheme0", &themeSpec);	
	if (snd_load_theme(&themeSpec, thePlayer) != noErr)
	{
		printf("Error:  Could not open the file Theme0.\n");
		ExitMovies();
		return;
	}
	gCurTheme = 0;
	
	// Setup the queueing globals.
	gTuneDone = FALSE;
	gReadyToQueue = FALSE;
	gNextSeq = 0;
	
	// Start off by playing the intro loop sequence (#2).
	err = TuneQueue(thePlayer, (unsigned long *)(*gTuneHdl + gOffsets[2]), 0x10000,
					  0, 0x7FFFFFFF, kTuneStartNow, gTuneCBProcPtr, 0);
	PrimeTime((QElemPtr)&gTriggerTask, 6000);				// Trigger in 6 seconds

	// The key event loop.
	statusCount = 0;
	do
	{
		// Check the tune queue status.
/*		TuneStatus	tpStatus;
		
		if (statusCount > 50000)
		{
			TuneGetStatus(thePlayer, &tpStatus);
			printf("Tune time:%d,  Queued time:%d, Queue count:%d, Queue spots:%d\n", 
					tpStatus.time, tpStatus.queueTime, tpStatus.queueCount, tpStatus.queueSpots);
			statusCount = 0;
		}
		statusCount++;
*/
		// Queue up the next sequence.
		if (gReadyToQueue)
		{
			printf("Queueing sequence %d.\n", gNextSeq);
			err = TuneQueue(thePlayer, (unsigned long *)(*gTuneHdl + gOffsets[gNextSeq]), 0x10000,
							  0, 0x7FFFFFFF, 0, gTuneCBProcPtr, 0);
			gReadyToQueue = FALSE;
		}
		
		// If gTuneDone is TRUE, then a sequence just finished playing, which means a new sequence
		// just started playing, so prime the timer task for the next sequence trigger.
		if (gTuneDone)
		{
			printf("Priming timer to fire in 6 seconds.\n");
			PrimeTime((QElemPtr)&gTriggerTask, 6000);				// Trigger in 6 seconds
			gTuneDone = FALSE;
		}
		
		// Check for keypresses.
		event = kb_next();
		if (event.code!=0xff)
			switch (event.ascii)
			{
				case 'q':
					printf("Quitting.\n");
					inloop=FALSE;
					break;
					
				case 'e':
					printf("Switching to elevator theme.\n");
					KillCurrentTheme();
					FSMakeFSSpec(gVRef, gDirID, "\pTheme7", &themeSpec);	
					if (snd_load_theme(&themeSpec, thePlayer) != noErr)
					{
						printf("Error:  Could not open the elevator theme.\n");
						ExitMovies();
						return;
					}
					gTuneDone = FALSE;
					gCurTheme = 7;
					gNextSeq = 0;
					TuneQueue(thePlayer, (unsigned long *)(*gTuneHdl), 0x10000,
									  0, 0x7FFFFFFF, kTuneStartNow, gTuneCBProcPtr, 0);
					PrimeTime((QElemPtr)&gTriggerTask, 6000);				// Trigger in 6 seconds
					break;

				case 'm':
					printf("Switching to menu screen theme.\n");
					KillCurrentTheme();
					FSMakeFSSpec(gVRef, gDirID, "\pTheme0", &themeSpec);	
					if (snd_load_theme(&themeSpec, thePlayer) != noErr)
					{
						printf("Error:  Could not open the menu theme.\n");
						ExitMovies();
						return;
					}
					gTuneDone = FALSE;
					gCurTheme = 7;
					gNextSeq = 2;
					TuneQueue(thePlayer, (unsigned long *)(*gTuneHdl + gOffsets[2]), 0x10000,
									  0, 0x7FFFFFFF, kTuneStartNow, gTuneCBProcPtr, 0);
					PrimeTime((QElemPtr)&gTriggerTask, 6000);				// Trigger in 6 seconds
					break;

				case '1':
					printf("Switching to level1 theme.\n");
					KillCurrentTheme();
					FSMakeFSSpec(gVRef, gDirID, "\pTheme1", &themeSpec);	
					if (snd_load_theme(&themeSpec, thePlayer) != noErr)
					{
						printf("Error:  Could not open the level1 theme.\n");
						ExitMovies();
						return;
					}
					gOffsets = (long *)*gOfsHdl;
					gTuneDone = FALSE;
					gCurTheme = 1;
					gNextSeq = 7;
					gLevelState = kWalking;
					// Start off with the intro theme (#7)
					TuneQueue(thePlayer, (unsigned long *)(*gTuneHdl + gOffsets[7]), 0x10000,
									  0, 0x7FFFFFFF, kTuneStartNow, gTuneCBProcPtr, 0);
					PrimeTime((QElemPtr)&gTriggerTask, 6000);				// Trigger in 6 seconds
					break;

				case '2':
					printf("Switching to level2 theme.\n");
					KillCurrentTheme();
					FSMakeFSSpec(gVRef, gDirID, "\pTheme2", &themeSpec);	
					if (snd_load_theme(&themeSpec, thePlayer) != noErr)
					{
						printf("Error:  Could not open the level1 theme.\n");
						ExitMovies();
						return;
					}
					gOffsets = (long *)*gOfsHdl;
					gTuneDone = FALSE;
					gCurTheme = 2;
					gNextSeq = 7;
					gLevelState = kWalking;
					// Start off with the intro theme (#7).
					TuneQueue(thePlayer, (unsigned long *)(*gTuneHdl + gOffsets[7]), 0x10000,
									  0, 0x7FFFFFFF, kTuneStartNow, gTuneCBProcPtr, 0);
					PrimeTime((QElemPtr)&gTriggerTask, 6000);				// Trigger in 6 seconds
					break;
					
				case 'c':
					printf("Transitioning to combat mode.\n");
					gLevelState = kCombat;
					break;
					
				case 'w':
					printf("Transitioning to walking.\n");
					gLevelState = kWalking;
					break;
					
				case 'd':
					printf("Transitioning to deconstruct mode.\n");
					gLevelState = kDecon;
					break;

		            case '-':
			            	if (gVolume > 0)
			            		gVolume -= 0x2000;
					TuneSetVolume(thePlayer, gVolume);
			            	printf("New volume: %X\n", gVolume);
			            	break;

		            case '+':
			            	if (gVolume < 0x10000)
			            		gVolume += 0x2000;
					TuneSetVolume(thePlayer, gVolume);
			            	printf("New volume: %X\n", gVolume);
			            	break;
			}
	} while (inloop);
	KillCurrentTheme();
	
	snd_stop_midi();
	kb_shutdown();

	ExitMovies();

	RmvTime((QElemPtr)&gTriggerTask);						// Stop the trigger task
	DisposeRoutineDescriptor(gTriggerProcPtr);					// Dispose its UPP

	printf("\nDone.\n");
}


//--------------------------------------------------------------------------
//	Call-back routine.  Get's called when tune is finished.
//--------------------------------------------------------------------------
void EndSequenceCallBack(long seq_id)
{
	gTuneDone = TRUE;
}

//--------------------------------------------------------------------------
//  Stop playing the current theme and free up all resources.
//--------------------------------------------------------------------------
void KillCurrentTheme(void)
{
	if (gCurTheme < 0)
		return;

	RmvTime((QElemPtr)&gTriggerTask);			// Remove the TimeManager task that queues up
	InsTime((QElemPtr)&gTriggerTask);				// next seq, and re-insert to prevent any tasks from 
													// triggering.
	TuneStop(thePlayer, kStopTuneFade);
	TuneFlush(thePlayer);
	
	snd_release_current_theme();
}

//---------------------------------------------------------------
//  Routine that figures out the next sequence to queue up.
//---------------------------------------------------------------
void MyMusicAI(void)
{
	switch (gCurTheme)
	{
		case 0:						// Theme 0 - intro loop
			gNextSeq = 2;			// Just play the same sequence over and over
			break;
			
		case 1:						// Theme 1 - level music (sensitive to combat, peril, etc.)
			if (gLevelState == kWalking)
				switch (gNextSeq)
				{
					case 26:						// If was walking A
						gNextSeq = 27;			// then move to walking B.
						break;
					case 27:						// If was walking B
						gNextSeq = 28;			// then move to walking C.
						break;
					case 28:						// If was walking C
						gNextSeq = 29;			// then move to walking D.
						break;
					case 29:						// If was walking D
						gNextSeq = 26;			// then move to walking A.
						break;
					default:						// If any non-walking music was playing
						gNextSeq = 26;			// then move to walking A.
						break;
				}
			if (gLevelState == kCombat)
				switch (gNextSeq)
				{
					case 12:						// If was peril A
						gNextSeq = 14;			// then move to peril C.
						break;
					case 14:						// If was peril C
						gNextSeq = 0;			// then move to combat A.
						break;
					case 0:						// If was combat A
						gNextSeq = 1;			// then move to combat B.
						break;
					case 1:						// If was combat B or
					default:						// If any non-combat music was playing
						gNextSeq = 12;			// then move to peril A.
						break;
				}
			if (gLevelState == kDecon)
				gNextSeq = 5;					// Play "deconstruct" music over and over
			break;
					
		case 2:						// Theme 2 - level music (sensitive to combat, peril, etc.)
			if (gLevelState == kWalking)
				switch (gNextSeq)
				{
					case 25:						// If was walking A
						gNextSeq = 26;			// then move to walking B.
						break;
					case 26:						// If was walking B
						gNextSeq = 27;			// then move to walking C.
						break;
					case 27:						// If was walking C
						gNextSeq = 28;			// then move to walking D.
						break;
					case 28:						// If was walking D
						gNextSeq = 25;			// then move to walking A.
						break;
					default:						// If any non-walking music was playing
						gNextSeq = 25;			// then move to walking A.
						break;
				}
			if (gLevelState == kCombat)
				switch (gNextSeq)
				{
					case 0:						// If was combat A
						gNextSeq = 1;			// then move to combat B.
						break;
					case 1:						// If was combat B
						gNextSeq = 0;			// then move to combat C.
						break;
					default:						// If any non-combat music was playing
						gNextSeq = 0;			// then move to peril A.
						break;
				}
			if (gLevelState == kDecon)
				gNextSeq = 5;					// Play "deconstruct" music over and over
			break;

		case 7:
			if (gNextSeq >= 7)
				gNextSeq = 0;
			else
				gNextSeq++;
			break;
	}
	
	gReadyToQueue = TRUE;
}
