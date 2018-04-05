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
//		System Shock - ©1994-1995 Looking Glass Technologies, Inc.
//
//		Shock.c	-	Mac-specific initialization and main event loop.
//
//====================================================================================


//--------------------
//  Includes
//--------------------
#include <Balloons.h>

#include "Shock.h"
#include "InitMac.h"
#include "Prefs.h"
#include "AEHandlers.h"
#include "ShockBitmap.h"
#include "ShockDialogs.h"
#include "DialogHelpers.h"
#include "ShockHelp.h"
#include "MoviePlay.h"
#include "MacTune.h"
#ifdef TESTING
#include "Tests.h"
#endif

#include "amaploop.h"
#include "hkeyfunc.h"
#include "input.h"
#include "mainloop.h"
#include "setup.h"
#include "screen.h"
#include "fullscrn.h"
#include "status.h"
#include "gamewrap.h"
#include "faketime.h"
#include "map.h"

extern bool game_paused;		// I've learned such bad lessons from LG.
extern bool objdata_loaded;
extern bool music_on;
extern bool startup_music;

//--------------------
//  Globals
//--------------------
WindowPtr			gMainWindow;
MenuHandle		gMainMenus[kNumMenus];
RgnHandle			gCursorRgn;
short					gCursorSet;
Boolean				gDone = false;
Boolean				gInForeground = true;
Boolean				gPlayingGame;		//¥¥¥ Temp
Boolean				gIsNewGame;
FSSpec				gSavedGameFile;
long					gGameSavedTime;
Boolean				gDeadPlayerQuit;
Boolean				gGameCompletedQuit;

//--------------------
//  Prototypes
//--------------------
extern void init_all(void);
extern void inv_change_fullscreen(bool on);
extern void object_data_flush(void);
extern Boolean IsFullscreenWareOn(void);
extern errtype load_da_palette(void);

void SetupTitleMenus(void);
void HandleNewGame(void);
void HandleOpenGame(void);
void ShockGameLoop(void);
void HandlePausedEvents(void);
void SetupPauseMenus(void);
void RestoreTitleScreen(void);
errtype CheckFreeSpace(short	checkRefNum);


//------------------------------------------------------------------------------------
//		Main function.
//------------------------------------------------------------------------------------
void main(void)
{  
	InitMac();																// init mac managers
				
	FlushEvents(autoKey+keyDown+mouseDown,0L);		// get rid of any extra mouse/key clicks
 
	GetFolders();															// get refs to data, sound, etc folders.

 	CheckConfig();														// make sure we can run in this environment

	SetDefaultPrefs();													// Initialize the preferences file.
	LoadPrefs(kPrefsResID);
		
	DoAEInstallation();
	SetupWindows(&gMainWindow);								// setup everything
	SetupOffscreenBitmaps();			
 	SetUpMenus(gMainMenus, kNumMenus);
	AddHelpMenu();
	
#ifdef TESTING
	SetupTests();
#endif

	// Initialize
	
	HideCursor();
	HideMenuBar();

	init_all();
	if (gShockPrefs.prefPlayIntro)
	{
		PlayIntroCutScene();
		gShockPrefs.prefPlayIntro = 0;
		SavePrefs(kPrefsResID);
	}
	
	SetupTitleScreen();
	SetupTitleMenus();
	ShowMenuBar();
	ShowCursor();

	// Look for any saved game files to launch with

//	CountAppFiles(&msg, &count);

//	if (count!=0)
//	{
//		GetAppFiles(1, &theFile);
//		OpenGame(&theFile, 0L);
//	 }
  	do
	{
		HandleEvents();
//		HandleGolfStuff();
//		HandleCursorStuff();
	}
	while (!gDone);
	 
	// black screen so it doesn't flash when we quit
	SetPort(gMainWindow);
	PaintRect(&gMainWindow->portRect);
	
//	CleanupAndExit();			ETS patch does this now.
}

//------------------------------------------------------------------------------------
//		Main function.
//------------------------------------------------------------------------------------
void HandleEvents(void)
{
	WindowPtr 	whichWindow;
	EventRecord 	theEvent; 
	char				theKey;
	GrafPtr			savePort;
//	long				temp;
 	
	if (WaitNextEvent(everyEvent, &theEvent, 10L, 0L)) 
	{
		switch (theEvent.what) 
		{
			case osEvt:
			  	if (theEvent.message & 0x01000000)						// suspend/resume event?
				{
				  	if (theEvent.message & 1)  									// Resume
					{	
						InitCursor();
//						CursorSet = kNoCursor;
				  	 				  	 	
				  	 	gInForeground = true;
						if ((CurScreenDepth() == 8) && (gStartupDepth == 8))
				  		 	FixPalette();
				  		 
				  	 	CheckBitDepth();
	
						// update the main window in case in needs it
						// (do this now instead of waiting for the next updateEvt
						//  so it doesn't interfere with some of the other monitor functions)
						{
							GetPort(&savePort);
							SetPort(gMainWindow);
							BeginUpdate(gMainWindow);
							UpdateWindow(gMainWindow);
							EndUpdate(gMainWindow);
							SetPort(savePort);
						}
					}
					else																		// Suspend
					{
				  	 	// save off the screen ctSeed so we can see if the palette changes
				  	 	if ((CurScreenDepth()==8) && (gStartupDepth==8))
				  			RememberSeed();
						gInForeground = false;
					}
				}
		  		break;
		  
			case mouseDown:
				switch (FindWindow(theEvent.where, &whichWindow)) 
				{
					case inMenuBar:
//						SetMenus();
						DoCommand(MenuSelect(theEvent.where)); 
						break;
						
					case inSysWindow:
						SystemClick(&theEvent, whichWindow);
						break;
							
					case inContent:
						GlobalToLocal(&theEvent.where);
#ifdef TESTING
DoTestClick(theEvent.where);
#endif
						switch(DoShockTitleButtons(theEvent.where))
						{
							case 0:
								HandleNewGame();
			 					break;
							case 1:
								HandleOpenGame();
								break;
							case 2:
								PlayIntroCutScene();
								break;
							case 3:
								DoQuit();
						}
						break;
							
					default: ;
				}
				break;
			
			case keyDown:
			case autoKey: 
				theKey = theEvent.message & charCodeMask;
				if ((theEvent.modifiers & cmdKey) != 0) 
				{
					DoCommand(MenuKey(theKey));
				}
				else if (theKey == 5)						// Help key
					ShowShockHelp();
			  	break;
			   
			case updateEvt: 
				whichWindow=(WindowPtr)theEvent.message;
				GetPort(&savePort);
				SetPort(whichWindow);
				BeginUpdate(whichWindow);
				UpdateWindow(whichWindow);
  				EndUpdate(whichWindow);
				SetPort(savePort);
				break;
			
			case kHighLevelEvent:
				AEProcessAppleEvent(&theEvent);
				break;
				
			default: ;
		} /* end of case theEvent.what */		
	} /* if */

	CheckBitDepth();
}
 
//------------------------------------------------------------------------------------
//		Draw from the main offscreen pixmap to the screen.
//------------------------------------------------------------------------------------
void UpdateWindow(WindowPtr wind)
 {
	RGBColor	black = {0, 0, 0};

	if (wind != gMainWindow)
	 	return;
	
 	// erase everything outside of the active area
 	if ((gMainWindow->portRect.right - gMainWindow->portRect.left != gActiveWide) || 
 	    (gMainWindow->portRect.bottom - gMainWindow->portRect.top != gActiveHigh))
 	{
 	 	Rect	r;

 	 	r = gMainWindow->portRect;
 	 	r.bottom = gActiveArea.top;
 	 	PaintRect(&r);
 	 	
 	 	r = gMainWindow->portRect;
 	 	r.top = gActiveArea.bottom;
 	 	PaintRect(&r);

 	 	r = gMainWindow->portRect;
 	 	r.right = gActiveArea.left;
 	 	PaintRect(&r);
 	 	
 	 	r = gMainWindow->portRect;
 	 	r.left = gActiveArea.right;
 	 	PaintRect(&r);
	}

	//¥¥¥ For now, just copy from main offscreen bitmap
	
 	ResetCTSeed();		// make sure all color table seeds match
	RGBForeColor(&black);
  	CopyBits(&gMainOffScreen.bits->portBits, &gMainWindow->portBits, &gOffActiveArea, &gActiveArea, srcCopy, 0L);

#ifdef TESTING
	DoTestUpdate(wind);	
#endif

 }

//------------------------------------------------------------------------------------
//		Perform the menu commands.
//------------------------------------------------------------------------------------
void DoCommand(unsigned long mResult)
{
	short					theItem;
	Str255				name;
	Boolean				savedOK;
	ModalFilterUPP	stdFilterProcPtr;

	theItem = LoWord(mResult);

	switch (HiWord(mResult)) 
	{			
		case mApple:
			switch(theItem)
			 {
				case 1:
					ScrollCredits();
					break;
				case 2:
					ShowShockHelp();
					break;
				default:
					GetItem(gMainMenus[0], theItem, name);
					OpenDeskAcc(name);
					break;
			 }
			break;
		
		case kHMHelpMenuID:
			ShowShockHelp();
			break;
		
		case mFile:
			switch(theItem)
			 {
			 	case fileNewGame:
					HandleNewGame();
			 		break;
			 	
			 	case fileOpenGame:
			 		if (gPlayingGame)								// If currently playing a game (not at menu screen)
			 		{
				 		if (*tmd_ticks > (gGameSavedTime + (5 * CIT_CYCLE)))		// If the current game needs saving...
				 		{
				 			short		btn;
							stdFilterProcPtr = NewModalFilterProc(ShockAlertFilterProc);
					 		btn = Alert((global_fullmap->cyber) ? 1010 :1009, stdFilterProcPtr);
							DisposeRoutineDescriptor(stdFilterProcPtr);
							
							if (global_fullmap->cyber)						// In cyberspace, all you can do is end the game
							{															// or just keep playing.
								if (btn == 1)
						 			HandleOpenGame();
							}
							else														// If in normal space, save the game first.
							{
						 		switch (btn)
						 		{
						 			case 1:											// Yeah, save it
								 		if (gIsNewGame)
										 	savedOK = DoSaveGameAs();
										 else
										 	savedOK = DoSaveGame();
										 if (!savedOK)
										 	break;
						 			case 2:											// No, don't save it
						 				HandleOpenGame();
						 				break;
						 		}
						 	}
					 	}
					 	else
					 		HandleOpenGame();
					 }
					 else
					 	HandleOpenGame();
			 		break;
			 	
			 	case fileSaveGame:
			 		if (gIsNewGame)
					 	savedOK = DoSaveGameAs();
					 else
					 	savedOK = DoSaveGame();
					 if (savedOK)
						game_paused = FALSE;
			 		break;
			 	
			 	case fileSaveGameAs:
				 	if (DoSaveGameAs())
						game_paused = FALSE;
			 		break;
			 	
				case filePlayIntro:
					PlayIntroCutScene();
					break;
				
				case fileResumeGame:
					{
						long		keys[4];
						
						do
							GetKeys((UInt32 *)keys);
						while ((keys[0] | keys[1]) != 0);
						game_paused = FALSE;
					}
					break;
								
				case fileEndGame:
				case fileQuit:
			 		if (gPlayingGame)								// If currently playing a game (not at menu screen)
			 		{
				 		if (*tmd_ticks > (gGameSavedTime + (5 * CIT_CYCLE)))		// If the current game needs saving...
				 		{
							short		btn;
							stdFilterProcPtr = NewModalFilterProc(ShockAlertFilterProc);
					 		btn = Alert((global_fullmap->cyber) ? 1010 :1009, stdFilterProcPtr);
							DisposeRoutineDescriptor(stdFilterProcPtr);

							if (global_fullmap->cyber)				// In cyberspace, all you can do is end the game
							{													// or just keep playing.
								if (btn == 1)
								{
									gPlayingGame = FALSE;
									game_paused = FALSE;
									if (theItem == fileQuit)
										DoQuit();
								}
							}
							else												// If in normal space...
							{
								switch(btn)
						 		{
						 			case 1:									// Yeah, save it
								 		if (gIsNewGame)
										 	savedOK = DoSaveGameAs();
										 else
										 	savedOK = DoSaveGame();
										 if (!savedOK)
										 	break;
						 			case 2:									// No, don't save it
										gPlayingGame = FALSE;
										game_paused = FALSE;
										if (theItem == fileQuit)
											DoQuit();
						 				break;
						 		}
						 	}
					 	}
					 	else
					 	{
							gPlayingGame = FALSE;
							game_paused = FALSE;
							if (theItem == fileQuit)
								DoQuit();
						}
					 }
					 else
					 {
						if (theItem == fileQuit)
							DoQuit();
					}
					break;
				 
				default:;
			 }
			break;
			
		case mOptions:
			switch(theItem)
			 {
				case optGameOptions:
					DoGameOptionsDlg();
					break;
				 
				case optSoundOptions:
					DoSoundOptionsDlg();
					break;
				 
				case optGraphicsOptions:
					DoGraphicsOptionsDlg();
					break;
				 
				default:;
			 }
			break;

#ifdef TESTING
		case mTests:
			switch(theItem)
			{
				case testBrowseImages:
					DoTestBrowseImages();
					break;
				case testBrowseFonts:
					DoTestBrowseFonts();
					break;
				case testLoadPalette:
					DoTestLoadPalette();
					break;
				case testMoveKeys:
					DoTestMoveKeys();
					break;
				case testMouse:
					DoTestMouse();
					break;
				case testPlayMovie:
				case testPlayMovie2x:
				case testPlayMovieDblSpd:
				case testPlayMovieHalfSpd:
					DoPlayMovie(theItem);
					break;
				case testPlayIntro:
				case testPlayDeath:
				case testPlayEndGame:
				case testPlayCitadelVM:
				case testPlayDetachVM	:
				case testPlayJettisonVM:
				case testPlayLaserMalVM:
				case testPlayShieldsVM:
				case testPlayAutoDesVM:
					DoPlayCutScene(theItem);
					break;
				case testPlayBark1:
				case testPlayBark2:
				case testPlayAlog1:
				case testPlayAlog2:
					DoPlayAudioLog(theItem);
					break;
			}
			break;
		
		case mTests2:
			switch(theItem)
			{
				case testLoadLevelR:
				case testLoadLevel1:
				case testLoadLevel2:
				case testLoadLevel3:
					DoLoadLevelMap(theItem);
					break;
				case testZoomIn:
				case testZoomOut:
					DoZoomCurrMap(theItem);
					break;
				case testRender:
					RenderTest();
					break;
			}
			break;
#endif

	}
	
	HiliteMenu(0);
}

//------------------------------------------------------------------------------------
//		Handle Quit menu command/apple event.
//------------------------------------------------------------------------------------
void DoQuit(void)
{
//	if (AskToSave(1))
//	{
//		if (modeflag!=-1)
//			EndGame(false);
		gDone = true;
//	}
}

//------------------------------------------------------------------------------------
//  Enable/disable appropriate menu items for the title screen.
//------------------------------------------------------------------------------------
void SetupTitleMenus(void)
{
	EnableItem(gMainMenus[mFile-128], fileNewGame);
	EnableItem(gMainMenus[mFile-128], fileOpenGame);
	EnableItem(gMainMenus[mFile-128], filePlayIntro);
	EnableItem(gMainMenus[mFile-128], fileQuit);
	DisableItem(gMainMenus[mFile-128], fileSaveGame);
	DisableItem(gMainMenus[mFile-128], fileSaveGameAs);
	DisableItem(gMainMenus[mFile-128], fileEndGame);
	DisableItem(gMainMenus[mFile-128], fileResumeGame);

	EnableItem(gMainMenus[mOptions-128], optGameOptions);
	EnableItem(gMainMenus[mOptions-128], optGraphicsOptions);
	EnableItem(gMainMenus[mOptions-128], optSoundOptions);
}


//------------------------------------------------------------------------------------
//  Handle "New Game" command.
//------------------------------------------------------------------------------------
void HandleNewGame()
{
	if (DoNewGameDlg())										// Put up New Game dialog
	{
		Str255		titleStr;
		
		SetPort(gMainWindow);								// Update window before loading
		BeginUpdate(gMainWindow);
		UpdateWindow(gMainWindow);
		EndUpdate(gMainWindow);
		
		gIsNewGame = TRUE;									// It's a whole new ballgame.
		gGameSavedTime = *tmd_ticks;
		
		GetIndString(titleStr, kProgressTitles, 1);	// Get appropriate title string
		StartProgressDlg(titleStr, objdata_loaded ? 330 : 780);		// Doesn't take as long if object data already loaded
		go_and_start_the_game_already();				// Load up everything for a new game
		if (startup_music)
		{
			if (music_on)
				MacTuneStartCurrentTheme();
			else if (gShockPrefs.soBackMusic)
				start_music();
		}
		EndProgressDlg();
		
		HideCursor();
		HideMenuBar();
		
		ShockGameLoop();										// Play the game!!!
		
		RestoreTitleScreen();
		SetupTitleMenus();
		ShowMenuBar();
		ShowCursor();
		InvalRect(&gMainWindow->portRect); 
	}
}


//------------------------------------------------------------------------------------
//  Handle "Open Game..." command.
//------------------------------------------------------------------------------------
void HandleOpenGame(void)
{
	StandardFileReply	reply;
	SFTypeList				typeList;
	Str255					titleStr, temp;
	
	typeList[0] = 'Sgam';
	StandardGetFile(nil, 1, typeList, &reply);			// Get the file to load.
	if (reply.sfGood)												// If they actually chose a file, then
	{
		if (gPlayingGame)										// If we were in the middle of playing a game,
		{																// shutdown the game before loading new one.
			loopmode_exit(_current_loop);
	     	closedown_game(TRUE);
			StopShockTimer();
		}

		SetPort(gMainWindow);								// Update window before loading
		BeginUpdate(gMainWindow);
		UpdateWindow(gMainWindow);
		EndUpdate(gMainWindow);
		
		GetIndString(titleStr, kProgressTitles, 2);	// Get string that says "Opening "
		Pstrcat(titleStr, reply.sfFile.name);			// and append the file name.
		GetIndString(temp, kProgressTitles, 3);		// Get string that says "...
		Pstrcat(titleStr, temp);								// and append it.

		StartProgressDlg(titleStr, objdata_loaded ? 328 : 778);		// Doesn't take as long if object data already loaded
		load_that_thar_game(&reply.sfFile);			// Open the game
		if (startup_music)
		{
			if (music_on)
				MacTuneStartCurrentTheme();
			else if (gShockPrefs.soBackMusic)
				start_music();
		}
		EndProgressDlg();

		gIsNewGame = FALSE;									// Nope, it's not a new game.
		gSavedGameFile = reply.sfFile;
		gGameSavedTime = *tmd_ticks;
		
		HideCursor();
		HideMenuBar();
		game_paused = FALSE;
		
		ShockGameLoop();										// Play the game!!!

		RestoreTitleScreen();
		SetupTitleMenus();
		ShowMenuBar();
		ShowCursor();
		InvalRect(&gMainWindow->portRect); 
	}
}


//------------------------------------------------------------------------------------
//  Open a game from an ODOC AppleEvent.
//------------------------------------------------------------------------------------
void HandleAEOpenGame(FSSpec *openSpec)
{
	Str255					titleStr, temp;

	GetIndString(titleStr, kProgressTitles, 2);	// Get string that says "Opening "
	Pstrcat(titleStr, openSpec->name);				// and append the file name.
	GetIndString(temp, kProgressTitles, 3);		// Get string that says "...
	Pstrcat(titleStr, temp);								// and append it.

	StartProgressDlg(titleStr, objdata_loaded ? 328 : 778);		// Doesn't take as long if object data already loaded
	load_that_thar_game(openSpec);					// Open the game
	if (startup_music)
	{
		if (music_on)
			MacTuneStartCurrentTheme();
		else if (gShockPrefs.soBackMusic)
			start_music();
	}
	EndProgressDlg();

	gIsNewGame = FALSE;									// Nope, it's not a new game.
	gSavedGameFile = *openSpec;
	gGameSavedTime = *tmd_ticks;
	
	HideCursor();
	HideMenuBar();
	game_paused = FALSE;
	
	ShockGameLoop();										// Play the game!!!

	RestoreTitleScreen();
	SetupTitleMenus();
	ShowMenuBar();
	ShowCursor();
	InvalRect(&gMainWindow->portRect); 
}


//--------------------------------------------------------------------
//  The main game loop for System Shock.
//--------------------------------------------------------------------
void ShockGameLoop(void)
{
	Size		dummy;
	FSSpec	fSpec;

	gPlayingGame = TRUE;
	gDeadPlayerQuit = FALSE;
	gGameCompletedQuit = FALSE;
	
	MaxMem(&dummy);							// Compact heap before starting the game.

	gr_clear(0xFF);
	load_da_palette();		// KLC - added here.  Used to be in setup_start().

	if (IsFullscreenWareOn())
	{
		fullscreen_start();
		_new_mode = _current_loop = FULLSCREEN_LOOP;
	}
	else
	{
		screen_start();											// Initialize the screen for slot view.
		_new_mode = _current_loop = GAME_LOOP;
	}

	StartShockTimer();									// Startup the game timer.
	
	while (gPlayingGame)
	{
		if (!(_change_flag&(ML_CHG_BASE<<1)))
			input_chk();
		if (globalChanges)
		{
			if (_change_flag&(ML_CHG_BASE<<3))
				loopmode_switch(&_current_loop);
			chg_unset_flg(ML_CHG_BASE<<3);
		}
		
		if (_current_loop == AUTOMAP_LOOP)
			automap_loop();									// Do the fullscreen map loop.
		else
			game_loop();										// Run the game!
		
		if (game_paused)									// If the game is paused, go to the "paused" Mac
		{															// event handling loop.
			if (music_on)
				MacTuneKillCurrentTheme();
			status_bio_end();
			uiHideMouse(NULL);							// Setup the environment for the Mac loop.
			CopyBits(&gMainWindow->portBits, &gMainOffScreen.bits->portBits, &gActiveArea, &gOffActiveArea, srcCopy, 0L);
			SetupPauseMenus();
			ShowMenuBar();
			ShowCursor();
			
			MaxMem(&dummy);							// Compact heap during a pause.

		 	do														// The loop itself.
				HandlePausedEvents();
			while (game_paused);
			
			HideCursor();
			HideMenuBar();
			FlushEvents(everyEvent, 0);
			status_bio_start();
			uiShowMouse(NULL);
			if (music_on)
				MacTuneStartCurrentTheme();
		}
		
		chg_set_flg(_static_change);
	}

	// We're through playing now.
	uiHideMouse(NULL);
	loopmode_exit(_current_loop);
	status_bio_end();
     stop_music();											//KLC - add here to stop music at end game
	
	if (gDeadPlayerQuit)									// If we quit because the player was killed, show
	{																// the death movie.
		FSMakeFSSpec(gCDDataVref, gCDDataDirID, "\pDeath", &fSpec);
		PlayCutScene(&fSpec, TRUE, TRUE);		
		gDeadPlayerQuit = FALSE;
	}

	if (gGameCompletedQuit)								// If we quit because the game was completed, show
	{																// the endgame movie.
		FSMakeFSSpec(gCDDataVref, gCDDataDirID, "\pEndgame", &fSpec);		
		PlayCutScene(&fSpec, TRUE, TRUE);
		gGameCompletedQuit = FALSE;

		PaintRect(&gMainWindow->portRect);
		ShowCursor();
		DoEndgameDlg();
	}
		
     closedown_game(TRUE);

	MaxMem(&dummy);									// Compact heap after quitting the game.

	StopShockTimer();										// Startup the game timer.
}

//--------------------------------------------------------------------
//  Handle Mac events while the game is paused.
//--------------------------------------------------------------------
void HandlePausedEvents(void)
{
	WindowPtr 	whichWindow;
	EventRecord 	theEvent; 
	char				theKey;
	GrafPtr			savePort;
 	
	if (WaitNextEvent(everyEvent, &theEvent, 10L, 0L)) 
	{
		switch (theEvent.what) 
		{
			case osEvt:
			  	if (theEvent.message & 0x01000000)						// suspend/resume event?
				{
				  	if (theEvent.message & 1)  									// Resume
					{	
						InitCursor();
//						CursorSet = kNoCursor;
				  	 				  	 	
				  	 	gInForeground = true;
						if ((CurScreenDepth() == 8) && (gStartupDepth == 8))
				  		 	FixPalette();
				  		 
				  	 	CheckBitDepth();
	
						// update the main window in case in needs it
						// (do this now instead of waiting for the next updateEvt
						//  so it doesn't interfere with some of the other monitor functions)
						{
							GetPort(&savePort);
							SetPort(gMainWindow);
							BeginUpdate(gMainWindow);
							UpdateWindow(gMainWindow);
							EndUpdate(gMainWindow);
							SetPort(savePort);
						}
					}
					else																		// Suspend
					{
				  	 	// save off the screen ctSeed so we can see if the palette changes
				  	 	if ((CurScreenDepth()==8) && (gStartupDepth==8))
				  			RememberSeed();
						gInForeground = false;
					}
				}
		  		break;
		  
			case mouseDown:
				switch (FindWindow(theEvent.where, &whichWindow)) 
				{
					case inMenuBar:
//						SetMenus();
						DoCommand(MenuSelect(theEvent.where)); 
						break;
						
					case inSysWindow:
						SystemClick(&theEvent, whichWindow);
						break;
				}
				break;
			
			case keyDown:
			case autoKey: 
				theKey = theEvent.message & charCodeMask;
				if ((theEvent.modifiers & cmdKey) != 0) 
				{
					DoCommand(MenuKey(theKey));
				}
				else if (theKey == 5)						// Help key
					ShowShockHelp();
				else if (theKey == 27)					// Esc key (resume if paused)
					game_paused = FALSE;
			  	break;
			   
			case updateEvt: 
				whichWindow=(WindowPtr)theEvent.message;
				GetPort(&savePort);
				SetPort(whichWindow);
				BeginUpdate(whichWindow);
				UpdateWindow(whichWindow);
  				EndUpdate(whichWindow);
				SetPort(savePort);
				break;
			
			case kHighLevelEvent:
				AEProcessAppleEvent(&theEvent);
				break;
				
			default: ;
		} // end of case theEvent.what	
	} // if

	CheckBitDepth();
}

//------------------------------------------------------------------------------------
//  Enable/disable appropriate menu items for the pause loop.
//------------------------------------------------------------------------------------
void SetupPauseMenus(void)
{
	DisableItem(gMainMenus[mFile-128], fileNewGame);
	DisableItem(gMainMenus[mFile-128], filePlayIntro);
	EnableItem(gMainMenus[mFile-128], fileOpenGame);
	if (global_fullmap->cyber)														// Can't save the game while in cyberspace.
	{
		DisableItem(gMainMenus[mFile-128], fileSaveGame);
		DisableItem(gMainMenus[mFile-128], fileSaveGameAs);
	}
	else
	{
		EnableItem(gMainMenus[mFile-128], fileSaveGame);
		EnableItem(gMainMenus[mFile-128], fileSaveGameAs);
	}
	EnableItem(gMainMenus[mFile-128], fileResumeGame);
	EnableItem(gMainMenus[mFile-128], fileQuit);
	EnableItem(gMainMenus[mFile-128], fileEndGame);

	EnableItem(gMainMenus[mOptions-128], optGameOptions);
	EnableItem(gMainMenus[mOptions-128], optGraphicsOptions);
	EnableItem(gMainMenus[mOptions-128], optSoundOptions);
}


//------------------------------------------------------------------------------------
//  Restores the title screen after the game has finished playing.
//------------------------------------------------------------------------------------
void RestoreTitleScreen(void)
{
	CTabHandle		ctab;
	gr_clear(0xff);
	ctab = GetCTable(9003);							// Get the title screen CLUT
	if (ctab)	
	{
		BlockMove((**(ctab)).ctTable, (**(gMainColorHand)).ctTable, 256 * sizeof(ColorSpec));
		SetEntries(0, 255, (**(gMainColorHand)).ctTable);
		ResetCTSeed();
		DisposCTable(ctab);
	}
	
	LoadPictShockBitmap(&gMainOffScreen, 9003);	// Load the title screen back into offscreen space.
	SetupTitleScreen();
}

//------------------------------------------------------------------------------------
//  "Save As" the current game (always ask for file name).
//------------------------------------------------------------------------------------
Boolean DoSaveGameAs(void)
{
	StandardFileReply	reply;
	Str255					titleStr, temp;
	
	if (gIsNewGame)															// If it's a new game
		GetIndString(temp, kSaveStrings, 1);						// get default name (Untitled Game)
	else
		BlockMove(gSavedGameFile.name, temp, 64);			// else use existing name
	GetIndString(titleStr, kSaveStrings, 2);
	StandardPutFile(titleStr, temp, &reply);

	SetPort(gMainWindow);								// Update window before continuing
	BeginUpdate(gMainWindow);
	UpdateWindow(gMainWindow);
	EndUpdate(gMainWindow);

	if (!reply.sfGood)
		return (FALSE);

	if (CheckFreeSpace(reply.sfFile.vRefNum) == ERR_NOMEM)
	{
		// ¥¥¥Put up alert saying "not enough disk space".
		return (FALSE);
	}
		
	GetIndString(titleStr, kProgressTitles, 4);	// Get string that says "Saving "
	Pstrcat(titleStr, reply.sfFile.name);			// and append the file name.
	GetIndString(temp, kProgressTitles, 3);		// Get string that says "...
	Pstrcat(titleStr, temp);								// and append it.

	// Put up wait cursor.
	StartProgressDlg(titleStr, 80);
	save_game(&reply.sfFile);							// Save the game
	EndProgressDlg();
	// Show arrow cursor.
	
	gIsNewGame = FALSE;									// It's no longer a new game.
	gSavedGameFile = reply.sfFile;
	gGameSavedTime = *tmd_ticks;		

	return (TRUE);
}


//------------------------------------------------------------------------------------
//  Save the current game (don't prompt for file name, just do it).
//------------------------------------------------------------------------------------
Boolean DoSaveGame(void)
{
	Str255					titleStr, temp;
	
	if (CheckFreeSpace(gSavedGameFile.vRefNum) == ERR_NOMEM)
	{
		// ¥¥¥Put up alert saying "not enough disk space".
		return (FALSE);
	}
		
	GetIndString(titleStr, kProgressTitles, 4);	// Get string that says "Saving "
	Pstrcat(titleStr, gSavedGameFile.name);		// and append the file name.
	GetIndString(temp, kProgressTitles, 3);		// Get string that says "...
	Pstrcat(titleStr, temp);								// and append it.

	// Put up wait cursor.
	StartProgressDlg(titleStr, 80);
	save_game(&gSavedGameFile);						// Save the game
	EndProgressDlg();
	// Show arrow cursor.
	
	gGameSavedTime = *tmd_ticks;		

	return (TRUE);
}


#define NEEDED_DISKSPACE   700000
//------------------------------------------------------------------------------------
//  See if we have enough free space to save the file.
//------------------------------------------------------------------------------------
errtype CheckFreeSpace(short	checkRefNum)
{
	HParamBlockRec	pbRec;
	OSErr				err;

	pbRec.volumeParam.ioCompletion = NULL;	
	pbRec.volumeParam.ioVolIndex = 0;							// 0 here means use the vRefNum alone to specify volume.
	pbRec.volumeParam.ioNamePtr = NULL;	
	pbRec.volumeParam.ioVRefNum = checkRefNum;	
	
	err = PBHGetVInfo(&pbRec, FALSE);							// Get the volume info.
	if (err == noErr)
	{
		if ((ulong)(pbRec.volumeParam.ioVAlBlkSiz * pbRec.volumeParam.ioVFrBlk) < NEEDED_DISKSPACE)
			return (ERR_NOMEM);
	}
	return (OK);
}
