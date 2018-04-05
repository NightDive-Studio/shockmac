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
//		ShockDialogs.h	-	All the dialog boxes used in System Shock.
//
//====================================================================================


//--------------------
//  Constants
//--------------------
#define kGameOptionsDlg				8000			// Dialog ID and items for the Game Options dialog.
#define kRadNormal					4
#define	kRadBrief						5
#define	kChkPopupLabels			6
#define	kChkOnScreenHelp			7
#define	kPopLanguage					12

#define kSoundOptionsDlg			8100			// Dialog ID and items for the Sound Options dialog.
#define	kChkBackMusic				4
#define	kChkSoundFX					5
#define	kStatVolume					8
#define	kSldVolume					9

#define kGraphicsOptionsDlg		8200			// Dialog ID and items for the Graphics Options dialog.
#define kRadHighRes					4
#define	kRadLowRes					5
#define	kChkSkipLines				6
#define	kPopDetail						11
#define kSldGamma						13

#define kNewGameDlg					8300			// Dialog ID and items for the New Game dialog.
#define kEdtPlayerName				6
#define kGrpDifficulty				15
#define	kPopCombat					11
#define	kPopMission					12
#define	kPopPuzzles					13
#define	kPopCyberspace				14

#define kEndGameDlg					8400			// Dialog ID and items for the Endgame dialog.
#define kStatTime						9
#define kStatKills						10
#define kStatRegens					11
#define kStatDiff						12
#define kStatScore						13

#define kProgressTitles				2100			// String for progress dialog titles.
#define kSaveStrings					2110			// Strings used for save dialog.

//--------------------
//  Prototypes
//--------------------
void DoGameOptionsDlg(void);
void DoSoundOptionsDlg(void);
void DoGraphicsOptionsDlg(void);
Boolean DoNewGameDlg(void);
void DoEndgameDlg(void);

void StartProgressDlg(const Str255& title, int numSteps);
void AdvanceProgress(void);
void EndProgressDlg(void);

void Pstrcat(StringPtr dest, ConstStr255Param src);
