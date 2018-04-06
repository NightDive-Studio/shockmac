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
//		Prefs.h	-	Handles saving and loading preferences.
//
//====================================================================================


//--------------------
//  Types
//--------------------
typedef struct 
{
	short		prefVer;				// Version - set to 0 for now.
	short		prefPlayIntro;		// Play intro at startup if non-zero.
	
	// Game Options
	short		goMsgLength;			// 0 - normal, 1 - brief
	Boolean	goPopupLabels;
	Boolean	goOnScreenHelp;
	short		goLanguage;			// 0 - English
	
	// Sound Options
	Boolean	soBackMusic;
	Boolean	soSoundFX;
	short		soMusicVolume;
	
	// Display Options
	short		doResolution;			// 0 - High, 1 - Low
	short		doDetail;				// 0 - Min, 1-Low, 2-High, 3-Max
	short		doGamma;
	Boolean	doUseQD;
	
} ShockPrefs;

#define kPrefsFileType 		'Sprf'
#define	kPrefsFileName		"\pSystem Shock Prefs"
#define kPrefsResID			'Pref'

//--------------------
//  Globals
//--------------------
extern ShockPrefs		gShockPrefs;

//--------------------
//  Prototypes
//--------------------
void SetDefaultPrefs(void);
OSErr LoadPrefs(ResType resID);
OSErr SavePrefs(ResType resID);
OSErr GetPrefsDir(short *vRef, long *parID);
