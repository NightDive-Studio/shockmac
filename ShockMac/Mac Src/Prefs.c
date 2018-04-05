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
//		Prefs.c	-	Handles saving and loading preferences.
//
//====================================================================================


//--------------------
//  Includes
//--------------------
#include <GestaltEqu.h>
#include <Folders.h>
#include "Shock.h"
#include "Prefs.h"

#include "popups.h"
#include "olhext.h"

void SetShockGlobals(void);

//--------------------
//  Globals
//--------------------
ShockPrefs		gShockPrefs;

//--------------------
//  Externs
//--------------------
extern char		which_lang;
extern bool 		sfx_on;
extern int 			_fr_global_detail;
extern Boolean	DoubleSize;
extern Boolean	SkipLines;


//--------------------------------------------------------------------
//	  Initialize the preferences to their default settings.
//--------------------------------------------------------------------
void SetDefaultPrefs(void)
{
	gShockPrefs.prefVer = 0;
	gShockPrefs.prefPlayIntro = 1;				// First time through, play the intro
	
	gShockPrefs.goMsgLength = 0;					// Normal
	gShockPrefs.goPopupLabels = true;
	gShockPrefs.goOnScreenHelp = true;
	gShockPrefs.goLanguage = 0;					// English

	gShockPrefs.soBackMusic = true;
	gShockPrefs.soSoundFX = true;
	gShockPrefs.soMusicVolume = 33;			// еее Figure out when sound is put in.

	gShockPrefs.doResolution = 0;					// High-res.
	gShockPrefs.doDetail = 2;						// High detail.
	gShockPrefs.doGamma = 29;						// Default gamma (29 out of 100).
	gShockPrefs.doUseQD = false;

	SetShockGlobals();
}

//--------------------------------------------------------------------
//	  Locate the preferences file and load them to set our global pref settings.
//--------------------------------------------------------------------
OSErr LoadPrefs(ResType resID)
{
	OSErr					err;
	Handle					prefHdl;
	short						prefVRef;
	long						prefParID;
	short						fRef;
	HParamBlockRec		info;
	
	err = GetPrefsDir(&prefVRef, &prefParID);
	if (err == noErr)
	{
		fRef = HOpenResFile(prefVRef, prefParID, kPrefsFileName, fsRdPerm);		// Open the prefs file.
		if (fRef == -1)																						// If not there, then create it
		{																											// with default values.
			HCreateResFile(prefVRef, prefParID, kPrefsFileName);
			err = ResError();
			if (err == noErr)
			{
				info.ioParam.ioCompletion = 0L;
				info.ioParam.ioVRefNum = prefVRef;
				info.ioParam.ioNamePtr = kPrefsFileName;
				info.fileParam.ioDirID = prefParID;
				info.fileParam.ioFDirIndex = -1;
				err = PBHGetFInfo(&info, FALSE);
				if (err == noErr)
				{
					info.fileParam.ioDirID = prefParID;
					info.fileParam.ioFDirIndex = -1;
					info.fileParam.ioFlFndrInfo.fdCreator = kAppFileType;
					info.fileParam.ioFlFndrInfo.fdType = kPrefsFileType;
					PBHSetFInfoSync(&info);
				}
				SavePrefs(resID);
			}
		}
		else																				// Else set the global preferences struct
		{																					// from the prefs resource.
			prefHdl = GetResource(resID, 128);
			if (prefHdl)
			{
				BlockMove(*prefHdl, &gShockPrefs, sizeof(ShockPrefs));				
				ReleaseResource(prefHdl);
				SetShockGlobals();
			}
			CloseResFile(fRef);
		}
	}
	return(err);
}

//--------------------------------------------------------------------
//	  Save global settings in the preferences file.
//--------------------------------------------------------------------
OSErr SavePrefs(ResType resID)
{
	OSErr		err;
	Handle		prefHdl;
	short			prefVRef;
	long			prefParID;
	short			fRef;
	
	err = GetPrefsDir(&prefVRef, &prefParID);
	if (err == noErr)
	{
		fRef = HOpenResFile(prefVRef, prefParID, kPrefsFileName, fsRdWrPerm);
		if (fRef != -1)
		{
			prefHdl = GetResource(resID, 128);										// Get the prefs resource
			if (!prefHdl)																		// If there is no resource in the file, then
			{																						// add one.
				prefHdl = NewHandle(sizeof(ShockPrefs));
				if (prefHdl)
					AddResource(prefHdl, resID, 128, "\p");
			}
			
			BlockMove(&gShockPrefs, *prefHdl, sizeof(ShockPrefs));		// Set prefs handle from our global.
			
			ChangedResource(prefHdl);													// Write out the changed resource.
			WriteResource(prefHdl);
			ReleaseResource(prefHdl);
			UpdateResFile(fRef);
			CloseResFile(fRef);
		}
	}
	return (err);
}

//--------------------------------------------------------------------
//	  Get a reference to the Preferences folder.
//--------------------------------------------------------------------
OSErr GetPrefsDir(short *vRef, long *parID)
{
	OSErr	err;
	long		fm;
	
	err = Gestalt(gestaltFindFolderAttr, &fm);
	err |= (fm & (1 << gestaltFindFolderPresent)) == 0;
	
	if (err == noErr)												// If folder mgr present, then find
	{																		// the 'Preferences' folder
		err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, vRef, parID);
	}
	return (err);
}

//--------------------------------------------------------------------
//  Set the corresponding Shock globals from the prefs structure.
//--------------------------------------------------------------------
void SetShockGlobals(void)
{
	popup_cursors = gShockPrefs.goPopupLabels;
	olh_active = gShockPrefs.goOnScreenHelp;
	which_lang = 0;													// always English

	sfx_on = gShockPrefs.soSoundFX;
	
	DoubleSize = (gShockPrefs.doResolution == 1);		// Set this True for low-res.
	SkipLines = gShockPrefs.doUseQD;
	_fr_global_detail = gShockPrefs.doDetail;
}
