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
//====================================================================
//
//		System Shock - Macintosh.
//
//		High-Level AppleEvent handlers.
//
//====================================================================


#include "Shock.h"
#include "AEHandlers.h"

//--------------------------------------------------------------------
//  Install the Apple Event handlers.
//--------------------------------------------------------------------
void DoAEInstallation(void )
{
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, NewAEEventHandlerProc(HandleODOC), 0, false);
	AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerProc(HandleQUIT), 0, false);
	AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, NewAEEventHandlerProc(HandlePDOC), 0, false);
	AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, NewAEEventHandlerProc(HandleOAPP), 0, false);
}

//--------------------------------------------------------------------
//  Make sure all the info is in for an AppleEvent.
//--------------------------------------------------------------------
OSErr RequiredCheck(AppleEvent *theAppleEvent )
{
	OSErr		anErr;
	DescType	typeCode;
	Size			actualSize;
	
	anErr = AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr, typeWildCard, &typeCode, NULL, 0, &actualSize);
	if (anErr == errAEDescNotFound)
		return(noErr);
	if (anErr == noErr)
		return(errAEEventNotHandled);
	return(anErr);
}

//--------------------------------------------------------------------
//  Handle the Open Application AppleEvent - do nothing for Shock.
//--------------------------------------------------------------------
pascal OSErr HandleOAPP(AppleEvent *theAppleEvent, AppleEvent *, long  )
{
	OSErr		anErr;
	
	anErr = RequiredCheck(theAppleEvent);
	return(anErr);
}
	
//--------------------------------------------------------------------
//  Handle the OpenDocument AppleEvent - extract the file specs and open document windows for them.
//--------------------------------------------------------------------
pascal OSErr HandleODOC(AppleEvent *theAppleEvent, AppleEvent *, long  )
{
	OSErr			anErr;
	AEDescList		docList;
	FSSpec			anFSS;
	long				itemsInList;
	AEKeyword		theKeyword;
	DescType		typeCode;
	Size				actualSize;
	FInfo				theFInfo;
	Boolean			isStationery;
	
	anErr = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &docList);
	if (anErr)return(anErr);
	
	anErr = RequiredCheck(theAppleEvent);
	if (anErr)return(anErr);

	anErr = AECountItems(&docList, &itemsInList);	
	if (anErr)return(anErr);
	
	// If there is a list of files, just get the first one (can't open more than one game at a time).
	if (itemsInList > 0)
	{
		anErr = AEGetNthPtr(&docList, 1, typeFSS, &theKeyword, &typeCode, (Ptr) &anFSS, sizeof(FSSpec ), &actualSize);
		if (anErr)
			return(anErr);
		
		FSpGetFInfo(&anFSS, &theFInfo);
		isStationery =((theFInfo.fdFlags & 0x0800) != 0);

		// If this is actually a game file, then set flags to assume an "Open" has occurred.
		if (theFInfo.fdType == 'Sgam')
		{
			HandleAEOpenGame(&anFSS);
		}
	}
	
	AEDisposeDesc(&docList);
	return(noErr);
}

//--------------------------------------------------------------------
//  Handle the Print Document AppleEvent - don't do nuthin.
//--------------------------------------------------------------------
pascal OSErr HandlePDOC(AppleEvent *theAppleEvent, AppleEvent *, long  )
{
	OSErr	anErr;

	anErr = RequiredCheck(theAppleEvent);
	return(anErr);
}

//--------------------------------------------------------------------
//  Handle the Quit AppleEvent - handle quitting.
//--------------------------------------------------------------------
pascal OSErr HandleQUIT(AppleEvent *theAppleEvent, AppleEvent *, long  )
{
	OSErr	anErr;
	
	anErr = RequiredCheck(theAppleEvent);
	if (anErr)
		return(anErr);
	
	DoQuit();
	return(noErr);
}
