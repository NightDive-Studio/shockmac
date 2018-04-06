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
//		ShockDialogs.c	-	All the dialog boxes used in System Shock.
//
//====================================================================================


//--------------------
//  Includes
//--------------------
#include <string.h>

#include "Prefs.h"
#include "DialogHelpers.h"
#include "ShockDialogs.h"
#include "MacTune.h"
#include "InitMac.h"

#include "player.h"
#include "popups.h"
#include "olhext.h"
#include "faketime.h"
#include "frtypes.h"
#include "frtypesx.h"
#include "tools.h"

//--------------------
//  Externs
//--------------------
extern Boolean	gPlayingGame;
extern char		which_lang;
extern int 			_fr_global_detail;
extern frc 			*svga_render_context;
extern bool 		sfx_on;
extern Boolean	DoubleSize;
extern Boolean	SkipLines;

void start_music(void);
void stop_music(void);
void change_svga_screen_mode(void);
void gamma_dealfunc(ushort gamma_qvar);


//------------------------------------------------------------------------------------
//  Show and handle the Game Options dialog.
//------------------------------------------------------------------------------------
void DoGameOptionsDlg(void)
{
	ShockPrefs			localPrefs;
	GrafPtr				savePort;
	DialogPtr			dlog;
	UserItemUPP 		btnOutlineProcPtr;
	Handle				hand;
	Rect					r;
	short					itype,itemhit;
	ModalFilterUPP	stdFilterProcPtr;
	
	localPrefs = gShockPrefs;															// Use a local copy of the prefs while in dlg.
	
	GetPort(&savePort);																	// Save current port
	
	dlog = GetNewDialog(kGameOptionsDlg, nil, (WindowPtr)-1L);		// Load the dialog window
	SetPort(dlog);
	
	gIgnoreGray = true;																		// Setup the OK button outline
	btnOutlineProcPtr = NewUserItemProc(OKButtonUser);
	GetDItem(dlog, kUsrOKOutline, &itype, &hand, &r);
	SetDItem(dlog, kUsrOKOutline, itype, (Handle)btnOutlineProcPtr, &r);
	
	GetDItem(dlog, kChkPopupLabels, &itype, &hand, &r);					// Set "Popup Button Labels" checkbox
	SetCtlValue((ControlHandle)hand, (short)localPrefs.goPopupLabels);
	
	GetDItem(dlog, kChkOnScreenHelp, &itype, &hand, &r);					// Set "On-Screen Help" checkbox
	SetCtlValue((ControlHandle)hand, (short)localPrefs.goOnScreenHelp);
	
	GetDItem(dlog, localPrefs.goMsgLength + kRadNormal, &itype, &hand, &r);	// Set "Message Length" radio buttons
	SetCtlValue((ControlHandle)hand, (short)true);

	stdFilterProcPtr = NewModalFilterProc(ShockFilterProc);

	ShowWindow(dlog);																	// Show the dialog
	 	
	do																								// Handle dialog events
	{
		ModalDialog(stdFilterProcPtr, &itemhit);
		
		if (itemhit == kChkPopupLabels)												// For checkboxes, just flip the state
		{
			localPrefs.goPopupLabels ^= true;
			GetDItem(dlog, kChkPopupLabels, &itype, &hand, &r);
			SetCtlValue((ControlHandle)hand, (short)localPrefs.goPopupLabels);
		}
		else if (itemhit == kChkOnScreenHelp)
		{
			localPrefs.goOnScreenHelp ^= true;
			GetDItem(dlog, kChkOnScreenHelp, &itype, &hand, &r);
			SetCtlValue((ControlHandle)hand, (short)localPrefs.goOnScreenHelp);
		}
		else if (itemhit == kRadNormal || itemhit == kRadBrief)				// For radio buttons,
		{
			GetDItem(dlog, localPrefs.goMsgLength + kRadNormal, &itype, &hand, &r);	// Turn off previous button
			SetCtlValue((ControlHandle)hand, (short)false);
			
			localPrefs.goMsgLength = itemhit - kRadNormal;										// Record new button
			
			GetDItem(dlog, localPrefs.goMsgLength + kRadNormal, &itype, &hand, &r);	// Turn on new button
			SetCtlValue((ControlHandle)hand, (short)true);
		}
	}
	while (itemhit != kBtnOK && itemhit != kBtnCancel);						// Until OK or Cancel is clicked
	
	if (itemhit == kBtnOK)																// If user clicked OK, then
	{
		gShockPrefs = localPrefs;														// Save changes in global prefs struct
		SavePrefs(kPrefsResID);														// and save to prefs file
		
		popup_cursors = gShockPrefs.goPopupLabels;
		olh_active = gShockPrefs.goOnScreenHelp;
//		which_lang = (char)gShockPrefs.goLanguage;
	}
	
	DisposeRoutineDescriptor(stdFilterProcPtr);								// Dispose of user item UPPs
	DisposeRoutineDescriptor(btnOutlineProcPtr);
		
	SetPort(savePort);																		// Restore port back to main window
	DisposDialog(dlog);																		// Free this dialog
}

//------------------------------------------------------------------------------------
//  Show and handle the Sound Options dialog.
//------------------------------------------------------------------------------------
void DoSoundOptionsDlg(void)
{
	ShockPrefs			localPrefs;
	GrafPtr				savePort;
	DialogPtr			dlog;
	UserItemUPP 		btnOutlineProcPtr;
//	UserItemUPP		statDimProcPtr;
//	UserItemUPP 		sliderProcPtr;
	Handle				hand;
	Rect					r;
	short					itype, itemhit;
	ModalFilterUPP	stdFilterProcPtr;
	
	localPrefs = gShockPrefs;															// Use a local copy of the prefs while in dlg.
	
	GetPort(&savePort);																	// Save current port
	
	dlog = GetNewDialog(kSoundOptionsDlg, nil, (WindowPtr)-1L);		// Load the dialog window
	SetPort(dlog);
	
	gIgnoreGray = true;																		// Setup the OK button outline
	btnOutlineProcPtr = NewUserItemProc(OKButtonUser);
	GetDItem(dlog, kUsrOKOutline, &itype, &hand, &r);
	SetDItem(dlog, kUsrOKOutline, itype, (Handle)btnOutlineProcPtr, &r);
	
/*	gSliderDimmed = gDimmed = !localPrefs.soBackMusic;
	BlockMove("\pVolume:", gDimTitle, 8);
	statDimProcPtr = NewUserItemProc(DimStaticUser);					// Setup the dimmable static "Volume"
	GetDItem(dlog, kStatVolume, &itype, &hand, &r);
	SetDItem(dlog, kStatVolume, itype, (Handle)statDimProcPtr, &r);
*/
	GetDItem(dlog, kChkBackMusic, &itype, &hand, &r);						// Set "Background Music" checkbox
	SetCtlValue((ControlHandle)hand, (short)localPrefs.soBackMusic);
	
/*	gSliderLastPos = localPrefs.soMusicVolume;
	SetupSlider();
	sliderProcPtr = NewUserItemProc(DrawSlider);							// Set up the "volume" slider
	GetDItem(dlog, kSldVolume, &itype, &hand, &r);
	SetDItem(dlog, kSldVolume, itype, (Handle)sliderProcPtr, &r);
*/
	GetDItem(dlog, kChkSoundFX, &itype, &hand, &r);							// Set "Sound FX" checkbox
	SetCtlValue((ControlHandle)hand, (short)localPrefs.soSoundFX);
	
	stdFilterProcPtr = NewModalFilterProc(ShockFilterProc);
	
	ShowWindow(dlog);																	// Show the dialog
	 	
	do																								// Handle dialog events
	{
		ModalDialog(stdFilterProcPtr, &itemhit);
		
/*		if (itemhit == kSldVolume)														// For the slider, let the slider
		{																							// tracker handle it.
			if (!gSliderDimmed)
				localPrefs.soMusicVolume = DoSliderTracking(dlog, itemhit, NULL);
		}
*/
		if (itemhit == kChkBackMusic)												// For checkboxes, just flip the state
		{
			localPrefs.soBackMusic ^= true;
			GetDItem(dlog, kChkBackMusic, &itype, &hand, &r);
			SetCtlValue((ControlHandle)hand, (short)localPrefs.soBackMusic);
/*
			GetDItem(dlog, kStatVolume, &itype, &hand, &r);					// Activate/dim the "volume" static
			gDimmed = !localPrefs.soBackMusic;
			InvalRect(&r);
			
			GetDItem(dlog, kSldVolume, &itype, &hand, &r);					// Activate/dim the "volume" slider
			gSliderDimmed = !localPrefs.soBackMusic;
			SetSliderBitmaps();
			InvalRect(&r);
*/
		}
		else if (itemhit == kChkSoundFX)
		{
			localPrefs.soSoundFX ^= true;
			GetDItem(dlog, kChkSoundFX, &itype, &hand, &r);
			SetCtlValue((ControlHandle)hand, (short)localPrefs.soSoundFX);
		}
	}
	while (itemhit != kBtnOK && itemhit != kBtnCancel);						// Until OK or Cancel is clicked
	
	if (itemhit == kBtnOK)																// If user clicked OK, then
	{
		Boolean	musicChanged = (gShockPrefs.soBackMusic != localPrefs.soBackMusic);
		
		gShockPrefs = localPrefs;														// Save changes in global prefs struct

		sfx_on = gShockPrefs.soSoundFX;
		
		if (musicChanged)
		{
			if (gPlayingGame)									// If I'm currently playing a game...
			{
				if (gShockPrefs.soBackMusic)												// If music is being turned on,
				{
					start_music();																// load the theme for this level
					if (mlimbs_status == 0)													// If not enough memory
					{
						StringAlert(17);														// tell the user
						gShockPrefs.soBackMusic = FALSE;								// and reset the music back off
					}
					else
						MacTuneKillCurrentTheme();										// but don't start playing just yet.
				}
				else
					stop_music();																// Else unload all music stuff.
			}
			else														// If I'm at menu screen...
			{
				if (MacTuneInit() != 0)														// If not enough memory to start music
				{
					StringAlert(17);															// tell the user
					gShockPrefs.soBackMusic = FALSE;									// and reset the music back off
				}
				MacTuneShutdown();
			}
		}
		
		SavePrefs(kPrefsResID);														// Save to prefs file
	}

//	FreeSlider();
	DisposeRoutineDescriptor(stdFilterProcPtr);								// Dispose of user item UPPs
//	DisposeRoutineDescriptor(sliderProcPtr);
//	DisposeRoutineDescriptor(statDimProcPtr);
	DisposeRoutineDescriptor(btnOutlineProcPtr);
	
	SetPort(savePort);																		// Restore port back to main window
	DisposDialog(dlog);																		// Free this dialog
}

//------------------------------------------------------------------------------------
//  Show and handle the Game Options dialog.
//------------------------------------------------------------------------------------
void DoGraphicsOptionsDlg(void)
{
	ShockPrefs			localPrefs;
	GrafPtr				savePort;
	DialogPtr			dlog;
	UserItemUPP 		btnOutlineProcPtr;
	UserItemUPP		popMenuProcPtr;
	UserItemUPP		sliderProcPtr;
	Handle				hand;
	Rect					r;
	short					itype,itemhit;
	Point					tempP;
	short					sel;
	ModalFilterUPP	stdFilterProcPtr;
	
	localPrefs = gShockPrefs;															// Use a local copy of the prefs while in dlg.
	
	GetPort(&savePort);																	// Save current port
	
	dlog = GetNewDialog(kGraphicsOptionsDlg, nil, (WindowPtr)-1L);	// Load the dialog window
	SetPort(dlog);
	
	gIgnoreGray = true;																		// Setup the OK button outline
	btnOutlineProcPtr = NewUserItemProc(OKButtonUser);
	GetDItem(dlog, kUsrOKOutline, &itype, &hand, &r);
	SetDItem(dlog, kUsrOKOutline, itype, (Handle)btnOutlineProcPtr, &r);
	
	GetDItem(dlog, kChkSkipLines, &itype, &hand, &r);						// Set "Skip even lines" checkbox
	SetCtlValue((ControlHandle)hand, (short)localPrefs.doUseQD);
	HiliteControl((ControlHandle)hand, 
						(localPrefs.doResolution == 0) ? 255 : 0);
	
	GetDItem(dlog, localPrefs.doResolution + kRadHighRes, &itype, &hand, &r);	// Set "Resolution" radio buttons
	SetCtlValue((ControlHandle)hand, (short)true);

	gPopupMenuHdl = GetMenu(8201);												// Setup the popup menu drawing item
	InsertMenu(gPopupMenuHdl, -1);
	gPopupSel = localPrefs.doDetail + 1;
	popMenuProcPtr = NewUserItemProc(PopupMenuUser);
	GetDItem(dlog, kPopDetail, &itype, &hand, &r);
	SetDItem(dlog, kPopDetail, itype, (Handle)popMenuProcPtr, &r);
	
	gSliderDimmed = FALSE;
	gSliderLastPos = localPrefs.doGamma;
	SetupSlider();
	sliderProcPtr = NewUserItemProc(DrawSlider);							// Set up the "Gamma" slider
	GetDItem(dlog, kSldGamma, &itype, &hand, &r);
	SetDItem(dlog, kSldGamma, itype, (Handle)sliderProcPtr, &r);
		
	stdFilterProcPtr = NewModalFilterProc(ShockFilterProc);
	
	ShowWindow(dlog);																	// Show the dialog
	 	
	do																								// Handle dialog events
	{
		ModalDialog(stdFilterProcPtr, &itemhit);
		
		if (itemhit == kSldGamma)														// For the slider, let the slider
		{																							// tracker handle it.
			localPrefs.doGamma = DoSliderTracking(dlog, itemhit, 
												(gPlayingGame) ?  (SliderCallbackProcPtr)gamma_dealfunc : NULL);
		}
		if (itemhit == kChkSkipLines)													// For checkboxes, just flip the state
		{
			localPrefs.doUseQD ^= true;
			GetDItem(dlog, kChkSkipLines, &itype, &hand, &r);
			SetCtlValue((ControlHandle)hand, (short)localPrefs.doUseQD);
		}
		else if (itemhit == kRadHighRes || itemhit == kRadLowRes)				// For radio buttons,
		{
			GetDItem(dlog, localPrefs.doResolution + kRadHighRes, &itype, &hand, &r);	// Turn off previous button
			SetCtlValue((ControlHandle)hand, (short)false);
			
			localPrefs.doResolution = itemhit - kRadHighRes;										// Record new button
			
			GetDItem(dlog, localPrefs.doResolution + kRadHighRes, &itype, &hand, &r);	// Turn on new button
			SetCtlValue((ControlHandle)hand, (short)true);
			
			GetDItem(dlog, kChkSkipLines, &itype, &hand, &r);					// Adjust "Skip even lines" checkbox
			if (localPrefs.doResolution == 0 && localPrefs.doUseQD)				// If switched to Hires and skip was on,
			{
				localPrefs.doUseQD = FALSE;												// turn skipping off,
				SetCtlValue((ControlHandle)hand, 0);									// and update the checkbox.
			}
			HiliteControl((ControlHandle)hand, 											// Enable/disable the checkbox
								(localPrefs.doResolution == 0) ? 255 : 0);
		}
		else if (itemhit == kPopDetail)												// Show the Detail popup menu
		{
			GetDItem(dlog, itemhit, &itype, &hand, &r);
			
			tempP.h = r.left+1;
			tempP.v = r.top+1;
			LocalToGlobal(&tempP);
			
			sel = gPopupSel;
			CheckItem(gPopupMenuHdl, gPopupSel, true);										// check the current item
			sel = PopUpMenuSelect(gPopupMenuHdl, tempP.v, tempP.h, gPopupSel);	// Do the popup thang
			CheckItem(gPopupMenuHdl, gPopupSel, false);										// Uncheck when done
			if ((sel!=0) && (sel != gPopupSel))
			{
				gPopupSel = sel;
				InvalRect(&r);
				localPrefs.doDetail = sel - 1;
			}
		}
	}
	while (itemhit != kBtnOK && itemhit != kBtnCancel);						// Until OK or Cancel is clicked

	if (itemhit == kBtnCancel)															// If cancel was clicked and the gamma
	{																								// was changed, reset it to the original.
		if ((gShockPrefs.doGamma != localPrefs.doGamma) && (gPlayingGame))
			gamma_dealfunc(gShockPrefs.doGamma);
	}
		
	if (itemhit == kBtnOK)																// If user clicked OK, then
	{
		Boolean	resChanged = (gShockPrefs.doResolution != localPrefs.doResolution);

		gShockPrefs = localPrefs;														// Save changes in global prefs struct
		SavePrefs(kPrefsResID);														// and save to prefs file
		
		if (resChanged)																		// Update the game's globals.
		{
			DoubleSize = (gShockPrefs.doResolution == 1);
			if (gPlayingGame)
				change_svga_screen_mode();
		}

		SkipLines = gShockPrefs.doUseQD;

		_fr_global_detail = gShockPrefs.doDetail;
		if (gPlayingGame)
		{
			fauxrend_context *frc = (fauxrend_context *)svga_render_context;
			frc->detail = gShockPrefs.doDetail;
		}
	}

	DisposeRoutineDescriptor(stdFilterProcPtr);								// Dispose of user item UPPs
	DisposeRoutineDescriptor(sliderProcPtr);
	DisposeRoutineDescriptor(popMenuProcPtr);
	DisposeRoutineDescriptor(btnOutlineProcPtr);

	DeleteMenu(8201);																	// Dispose of popup menu handle
	ReleaseResource((Handle)gPopupMenuHdl);

	SetPort(savePort);																		// Restore port back to main window
	DisposDialog(dlog);																		// Free this dialog
}


//------------------------------------------------------------------------------------
//  Show and handle the New Game dialog.
//------------------------------------------------------------------------------------
Boolean DoNewGameDlg(void)
{
	GrafPtr				savePort;
	DialogPtr			dlog;
	UserItemUPP 		btnOutlineProcPtr;
	UserItemUPP		popMenuProcPtr;
	UserItemUPP		grpBoxProcPtr;
	Handle				hand;
	Rect					r;
	short					itype, itemhit;
	Point					tempP;
	short					sel, cursel;
	Boolean				ret = FALSE;
	ModalFilterUPP	stdFilterProcPtr;
	Str255				name;
	int					 i;
	
	GetPort(&savePort);																	// Save current port
	
	dlog = GetNewDialog(kNewGameDlg, nil, (WindowPtr)-1L);			// Load the dialog window
	SetPort(dlog);
	
	gIgnoreGray = true;																		// Setup the OK button outline
	btnOutlineProcPtr = NewUserItemProc(OKButtonUser);
	GetDItem(dlog, kUsrOKOutline, &itype, &hand, &r);
	SetDItem(dlog, kUsrOKOutline, itype, (Handle)btnOutlineProcPtr, &r);
	
	gPopupMenuHdl = GetMenu(8301);												// Get the menu used in popups
	InsertMenu(gPopupMenuHdl, -1);
	for (i = 0; i < 4; i++)
		gNewGameSel[i] = 3;																// Default all to "Normal" еее right?

	popMenuProcPtr = NewUserItemProc(PopupMenuUserNG);
	GetDItem(dlog, kPopCombat, &itype, &hand, &r);
	SetDItem(dlog, kPopCombat, itype, (Handle)popMenuProcPtr, &r);
	GetDItem(dlog, kPopMission, &itype, &hand, &r);
	SetDItem(dlog, kPopMission, itype, (Handle)popMenuProcPtr, &r);
	GetDItem(dlog, kPopPuzzles, &itype, &hand, &r);
	SetDItem(dlog, kPopPuzzles, itype, (Handle)popMenuProcPtr, &r);
	GetDItem(dlog, kPopCyberspace, &itype, &hand, &r);
	SetDItem(dlog, kPopCyberspace, itype, (Handle)popMenuProcPtr, &r);
		
	grpBoxProcPtr = NewUserItemProc(GroupBoxUser);						// Setup the "Difficulty" group box
	GetDItem(dlog, kGrpDifficulty, &itype, &hand, &r);
	SetDItem(dlog, kGrpDifficulty, itype, (Handle)grpBoxProcPtr, &r);
	
	stdFilterProcPtr = NewModalFilterProc(ShockFilterProc);

	ShowWindow(dlog);																	// Show the dialog
	 	
	do																								// Handle dialog events
	{
		ModalDialog(stdFilterProcPtr, &itemhit);
		
		if (itemhit >= kPopCombat && itemhit <= kPopCyberspace)
		{
			GetDItem(dlog, itemhit, &itype, &hand, &r);
			
			tempP.h = r.left+1;
			tempP.v = r.top+5;
			LocalToGlobal(&tempP);
			
			cursel = gNewGameSel[itemhit - kPopCombat];
			CheckItem(gPopupMenuHdl, cursel, true);											// check the current item
			sel = PopUpMenuSelect(gPopupMenuHdl, tempP.v, tempP.h, cursel);	// Do the popup thang
			CheckItem(gPopupMenuHdl, cursel, false);										// Uncheck when done
			if ((sel!=0) && (sel != cursel))
			{
				gNewGameSel[itemhit - kPopCombat] = sel;
				InvalRect(&r);
			}
		}
		else if (itemhit == kBtnOK)
		{			
			GetDItem(dlog, kEdtPlayerName, &itype, &hand, &r);
	 		GetIText(hand, name);
	 		
	 		if (name[0] == 0)
	 		{
				Alert(1003, nil);
				itemhit = -1;
	 		}
	 		if (name[0] > 19)
	 		{
				Alert(1004, nil);
	 			itemhit = -1;
	 		}
		}
	}
	while (itemhit != kBtnOK && itemhit != kBtnCancel);						// Until OK or Cancel is clicked
	
	if (itemhit == kBtnOK)																// If user clicked OK, then
	{
		for (i = 0; i < 4; i++)																// save the difficulty settings,
			player_struct.difficulty[i] = gNewGameSel[i]-1;
		strcpy(player_struct.name, p2cstr(name));								// and the player name,
		ret = TRUE;																			// and tell it's ok to go on.
	}
		
	DisposeRoutineDescriptor(stdFilterProcPtr);								// Dispose of user item UPPs
	DisposeRoutineDescriptor(grpBoxProcPtr);
	DisposeRoutineDescriptor(popMenuProcPtr);
	DisposeRoutineDescriptor(btnOutlineProcPtr);

	DeleteMenu(8301);																	// Dispose of popup menu handle
	ReleaseResource((Handle)gPopupMenuHdl);

	SetPort(savePort);																		// Restore port back to main window
	DisposDialog(dlog);																		// Free this dialog
	
	return (ret);
}

//------------------------------------------------------------------------------------
//  Show and handle the Endgame statistics dialog.
//------------------------------------------------------------------------------------
void DoEndgameDlg(void)
{
	GrafPtr				savePort;
	DialogPtr			dlog;
	UserItemUPP 		btnOutlineProcPtr;
	Handle				hand;
	Rect					r;
	short					itype,itemhit;
	ModalFilterUPP	stdFilterProcPtr;
	char 					buf[256];
	char					stupid;
	int					i, score;

	GetPort(&savePort);																	// Save current port
	
	dlog = GetNewDialog(kEndGameDlg, nil, (WindowPtr)-1L);				// Load the dialog window
	SetPort(dlog);
	
	gIgnoreGray = true;																		// Setup the OK button outline
	btnOutlineProcPtr = NewUserItemProc(OKButtonUser);
	GetDItem(dlog, kUsrOKOutline, &itype, &hand, &r);
	SetDItem(dlog, kUsrOKOutline, itype, (Handle)btnOutlineProcPtr, &r);
	
	// Figure out the elapsed time
	second_format(player_struct.game_time / CIT_CYCLE, buf);
	GetDItem(dlog, kStatTime, &itype, &hand, &r);
	SetIText(hand, c2pstr(buf));
	
	// Figure out the kills
	numtostring(player_struct.num_victories, buf);
	GetDItem(dlog, kStatKills, &itype, &hand, &r);
	SetIText(hand, c2pstr(buf));
	
	// Figure out the regens
	numtostring(player_struct.num_deaths, buf);
	GetDItem(dlog, kStatRegens, &itype, &hand, &r);
	SetIText(hand, c2pstr(buf));

	// Figure out the difficulty index
	stupid = 0;
	for (i=0; i < 4; i++)
		stupid += (player_struct.difficulty[i] * player_struct.difficulty[i]);
	numtostring(stupid,buf);
	GetDItem(dlog, kStatDiff, &itype, &hand, &r);
	SetIText(hand, c2pstr(buf));

	// Figure out the score
	stupid = 0;
	for (i=0; i < 4; i++)
		stupid += (player_struct.difficulty[i] * player_struct.difficulty[i]);
	// death is 10 anti-kills, but you always keep at least a third of your kills.
	score = player_struct.num_victories - min(player_struct.num_deaths * 10, player_struct.num_victories * 2 / 3);
	score = score * 10000;
	score = score - min(score * 2 / 3, ((player_struct.game_time / (CIT_CYCLE * 36)) * 100));
	score = score * (stupid + 1) / 37;  // 9 * 4 + 1 is best difficulty factor
	if (stupid == 36)
		score += 2222222; // secret kevin bonus
	numtostring(score, buf);
	GetDItem(dlog, kStatScore, &itype, &hand, &r);
	SetIText(hand, c2pstr(buf));

	stdFilterProcPtr = NewModalFilterProc(ShockFilterProc);

	ShowWindow(dlog);																	// Show the dialog
	 	
	do																								// Handle dialog events
	{
		ModalDialog(stdFilterProcPtr, &itemhit);	
	}
	while (itemhit != kBtnOK);															// Until OK is clicked
		
	DisposeRoutineDescriptor(stdFilterProcPtr);								// Dispose of user item UPPs
	DisposeRoutineDescriptor(btnOutlineProcPtr);
		
	SetPort(savePort);																		// Restore port back to main window
	DisposDialog(dlog);																		// Free this dialog
}


//===========================================================================
//  PROGRESS DIALOG ROUTINES
//===========================================================================
DialogPtr	gProgressDlg = NULL;
int			gNumSteps;
int			gProgress;
GrafPtr		gSavePort;

//---------------------------------------------------------------------------
//  Setup and show the progress dialog.
//---------------------------------------------------------------------------
void StartProgressDlg(const Str255& title, int numSteps)
{
 	DialogPtr	dlog;
 	short			itype;
 	Handle		hand;
 	Rect			r;
	RGBColor	clr;

 	GetPort(&gSavePort);												// Save for later

     ParamText(title, NULL, NULL, NULL);						// Set the dialog's title
     
     dlog = GetNewDialog(2100,0L,(WindowPtr) -1L);	// Show the progress dialog.
 	SetPort(dlog);
 	DrawDialog(dlog);
 	
 	GetDItem(dlog, 2, &itype, &hand ,&r);						// Draw a border around the progress bar.
 	FrameRect(&r);
 	
	clr.red = 0x8000;													// Set color to draw progress bar in.
	clr.green = 0x8000;
	clr.blue = 0xB000;
	RGBForeColor(&clr);

 	gProgressDlg = dlog;												// Setup globals for subsequent calls.
 	gNumSteps = numSteps;
 	gProgress = 0;
}

//---------------------------------------------------------------------------
//  Advance the progress bar in the dialog.
//---------------------------------------------------------------------------
void AdvanceProgress(void)
{
 	short			itype;
 	Handle		hand;
 	Rect			r;
 	float			percent;
 	short			prog;

	if (gProgressDlg == NULL)
		return;

 	GetDItem(gProgressDlg, 2, &itype, &hand, &r);			// Get the progress bar's item rect.
 	InsetRect(&r, 1, 1);

	gProgress++;															// Advance the progress amount
	percent = (float)gProgress / (float)gNumSteps;			// Calculate the progress bar width
	prog = r.left + (short)(percent * (float)(r.right - r.left));
	r.right = min(r.right, prog);

	PaintRect(&r);														// Draw the progress bar
}

//---------------------------------------------------------------------------
//  Finish drawing progress bar and remove the dialog.
//---------------------------------------------------------------------------
void EndProgressDlg(void)
{
	RGBColor	black = {0,0,0};
	
	if (gProgressDlg)
	{
		if (gProgress < gNumSteps)											// Make sure we actually finish the progress.
		{
		 	short			itype;
		 	Handle		hand;
		 	Rect			r;
		 	long			temp;
		 	
		 	GetDItem(gProgressDlg, 2, &itype, &hand, &r);			// Get the progress bar's item rect.
		 	InsetRect(&r, 1, 1);		
			PaintRect(&r);														// Draw the progress bar as complete.
 			Delay(20L, &temp);												// Let user see that we're done.
		}
		
		RGBForeColor(&black);
 		DisposDialog(gProgressDlg);
		gProgressDlg = NULL;

 		SetPort(gSavePort);
	}
}

//--------------------------------------------------------------------
//	  Append src to dest - Pascal strings!!!
//--------------------------------------------------------------------
void Pstrcat(StringPtr dest, ConstStr255Param src)
{
	long	sLen = min(*src, 255 - *dest);
	
	BlockMove(src+1, dest + *dest + 1, sLen);
	*dest += sLen;
}
