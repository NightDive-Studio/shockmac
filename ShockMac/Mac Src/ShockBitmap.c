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
//		ShockBitmap.c	-	Manages off-screen bitmaps and palettes.
//
//====================================================================================


//--------------------
//  Includes
//--------------------
#include <Palettes.h>
#include "Shock.h"
#include "InitMac.h"
#include "ShockBitmap.h"

//--------------------
//  Prototypes
//--------------------
int TrackTitleButton(int btn);

//--------------------
//  Globals
//--------------------
PixMapHandle 	gScreenPixMap;
CTabHandle			gMainColorHand;
Boolean				gChangedColors = false;
ShockBitmap		gMainOffScreen;


//------------------------------------------------------------------------------------
//		Setup the main offscreen bitmaps.
//------------------------------------------------------------------------------------
void SetupOffscreenBitmaps(void)
{
	GDHandle     	devhandle;
	
	devhandle = GetMainDevice();
	gScreenPixMap = (*devhandle)->gdPMap;
	
	gMainColorHand = GetCTable(1000);								// Get the Shock CLUT
	HLockHi((Handle)gMainColorHand);
	SetEntries(0, 255, (**(gMainColorHand)).ctTable);
	gChangedColors = true;
	DrawMenuBar();		// redraw with new colors
	
	ResetCTSeed();
	
	// allocate main offscreen buffer
	NewShockBitmap(&gMainOffScreen, gActiveWide, gActiveHigh, true);
}

//------------------------------------------------------------------------------------
//		Reset the CTseeds of the screen & pixmap to be the same
//------------------------------------------------------------------------------------
void ResetCTSeed(void)
{
	(*gMainColorHand)->ctSeed = (*(*gScreenPixMap)->pmTable)->ctSeed; 
	(*(**(*(CWindowPtr)gMainWindow).portPixMap).pmTable)->ctSeed = (*gMainColorHand)->ctSeed;
}

//------------------------------------------------------------------------------------
//		Save the screen pixmap's ctSeed before a suspend.
//------------------------------------------------------------------------------------
long oldctSeed;
void RememberSeed(void)
{
	oldctSeed =	(*(*gScreenPixMap)->pmTable)->ctSeed;
}

//------------------------------------------------------------------------------------
//		If the palette changed while switched out, then reset it.
//------------------------------------------------------------------------------------
void FixPalette(void)
{
	if ((*(*gScreenPixMap)->pmTable)->ctSeed != oldctSeed)
	{
		if (CurScreenDepth() == 8)
			SetEntries(0, 255, (**(gMainColorHand)).ctTable);
		ResetCTSeed();
		DrawMenuBar();
		UpdateWindow(gMainWindow);
	}
}

//------------------------------------------------------------------------------------
// 	Setup a new ShockBitmap structure.
//------------------------------------------------------------------------------------
void NewShockBitmap(ShockBitmap *theMap, short width, short height, Boolean color)
{
	GrafPtr 			savePort;
	PixMapPtr		pmaptr;
	
	GetPort(&savePort);
	
	theMap->Color = color;
	theMap->bits = 0L;
	SetRect(&theMap->bounds, 0, 0, width, height);
	
	if (theMap->Color)																// Setup color pixmaps.
	{
		theMap->OrigBits = Build8PixMap(&theMap->CPort, width, height);
		
		SetPort((GrafPtr)&theMap->CPort);
		EraseRect(&theMap->bounds);
		
		pmaptr = *(theMap->CPort.portPixMap);
		theMap->Address = StripAddress(pmaptr->baseAddr);
		theMap->RowBytes = (long)(pmaptr->rowBytes & 0x7FFF);
		theMap->bits = (GrafPtr) &theMap->CPort;
	}
	else																					// setup B&W maps
	{
		theMap->BWBits.bounds = theMap->bounds;
		theMap->BWBits.rowBytes = ((width+15) >> 4)<<1; 		// round to even
		theMap->BWBits.baseAddr = NewPtr(((long) height * (long) theMap->BWBits.rowBytes));
		FailNIL(theMap->BWBits.baseAddr);
		
		theMap->BWBits.baseAddr = StripAddress(theMap->BWBits.baseAddr);
		
		OpenPort(&theMap->BWPort);
		SetPort(&theMap->BWPort);
		SetPortBits(&theMap->BWBits);
		
		// make sure the clip & vis regions are big enough
		SetRectRgn(theMap->BWPort.visRgn,theMap->bounds.left,theMap->bounds.top,theMap->bounds.right,theMap->bounds.bottom);
		SetRectRgn(theMap->BWPort.clipRgn,theMap->bounds.left,theMap->bounds.top,theMap->bounds.right,theMap->bounds.bottom);
		EraseRect(&theMap->bounds);
		  
		theMap->Address = theMap->BWBits.baseAddr;
		theMap->RowBytes = (long) theMap->BWBits.rowBytes;
		theMap->bits = (GrafPtr) &theMap->BWPort;
	}

	SetPort(savePort);
}

//------------------------------------------------------------------------------------
//  Deallocate a Shock bitmap.
//------------------------------------------------------------------------------------
void FreeShockBitmap(ShockBitmap *theMap)
{
	if (theMap->Color)
	{
		if (theMap->OrigBits != 0L)
			DisposeHandle(theMap->OrigBits);
		CloseCPort(&theMap->CPort);
	}
	else
	{
		ClosePort(&theMap->BWPort);
		DisposPtr(theMap->Address);
	}
}

//------------------------------------------------------------------------------------
// 	Code to build an offscreen pixmap (8bit) of a given size, using the current pStd clut
//		store pixmap in ColorBack cGrafPort.
//------------------------------------------------------------------------------------
Handle Build8PixMap(CGrafPtr theCGrafPtr, short width, short height) 
{
	Rect        			bRect;
	PixMapHandle		pmap;
	PixMapPtr			pmaptr;
	long         			bytes;
	Handle				hand;
	
	SetRect(&bRect, 0, 0, width, height);
	
	OpenCPort(theCGrafPtr);   							// open a new color port Ñ this calls InitCPort
	
	pmap = NewPixMap();
	MoveHHi((Handle)pmap);
	HLock((Handle)pmap);
	
	theCGrafPtr->portPixMap = pmap;
	pmaptr = *(theCGrafPtr->portPixMap);
	pmaptr->bounds=bRect;
	
	pmaptr->rowBytes = ((width+1)>>1)<<1;
	bytes = (long) height * (long)pmaptr->rowBytes;
	pmaptr->rowBytes |= 0x8000;
	
	hand = NewHandle(bytes + 32);
	FailNIL((Ptr) hand);
	MoveHHi(hand);
	HLock(hand);
	pmaptr->baseAddr = (Ptr)((unsigned long)(*hand+31) & 0xFFFFFFE0);
	
	pmaptr->pmTable = gMainColorHand;
	
	// make sure the clip & vis regions are big enough
	SetRectRgn(theCGrafPtr->visRgn,bRect.left,bRect.top,bRect.right,bRect.bottom);
	SetRectRgn(theCGrafPtr->clipRgn,bRect.left,bRect.top,bRect.right,bRect.bottom);
	
	PenNormal();
	
	return (hand);
}

//------------------------------------------------------------------------------------
//		Draw a pict into a ShockBitmap structure
//------------------------------------------------------------------------------------
void LoadPictShockBitmap(ShockBitmap *theMap, short PictID)
{
	PicHandle 	pic;
	Rect			r;
	GrafPtr		savePort;
	
	pic = (PicHandle)GetResourceFail('PICT', PictID);
	if (pic!=0L)
	{
		GetPort(&savePort);
		SetPort(theMap->bits);
		PaintRect(&theMap->bounds);
		
		r = (*pic)->picFrame;
		OffsetRect(&r, -r.left, -r.top);
		OffsetRect(&r, 0, (PictID == 9003) ? 20 : 0);		// The title screen needs to come down
		DrawPicture(pic,&r);										// so title bar won't cover it.
		
		ReleaseResource((Handle)pic);
		SetPort(savePort);
	}
}

//------------------------------------------------------------------------------------
// 	Return the main monitor's current screen depth.
//------------------------------------------------------------------------------------
short CurScreenDepth(void)
{
	short					depth;
	GDHandle     		devhandle;
	PixMapHandle 	pmhan;
	
	devhandle = GetMainDevice();
	pmhan = (*devhandle)->gdPMap;
	depth = (*pmhan)->pixelSize;
	
	return(depth);
}

//------------------------------------------------------------------------------------
// 	This routine checks to make sure the bit depth wasn't changed while the player switched out to
//		the Finder or another program.  If it was, then it disables all the menu items until the bit depth
//		is correctly set.
//------------------------------------------------------------------------------------
void CheckBitDepth(void)
{
 	short		newDepth;

	if (gInForeground)
	{
		newDepth = CurScreenDepth();
		if ((newDepth != gStartupDepth) && (gLastAlertDepth != newDepth))
		{
		 	gLastAlertDepth = newDepth;
		 	StringAlert(5);

			// dim all menus until they change back
			DisableItem(gMainMenus[mFile-128], fileNewGame);
			DisableItem(gMainMenus[mFile-128], fileOpenGame);
			DisableItem(gMainMenus[mFile-128], fileSaveGame);
			DisableItem(gMainMenus[mFile-128], fileEndGame);
			DisableItem(gMainMenus[mOptions-128], 0);
			DrawMenuBar();
		}
		else if ((newDepth == gStartupDepth) && (gLastAlertDepth != -1))
		{
			gLastAlertDepth = -1;

			// restore the dimmed menus
			EnableItem(gMainMenus[mFile-128], fileNewGame);
			EnableItem(gMainMenus[mFile-128], fileOpenGame);
			EnableItem(gMainMenus[mFile-128], fileSaveGame);
			EnableItem(gMainMenus[mFile-128], fileEndGame);
			EnableItem(gMainMenus[mOptions-128], 0);
			// SetMenus();
			DrawMenuBar();
			
			if (gStartupDepth == 8)
				FixPalette();
		}
	}
}

//------------------------------------------------------------------------------------
// 	Switch back to the original palette (saved when the game was started).
//------------------------------------------------------------------------------------
void CleanupPalette(void)
{
	if (gChangedColors)										// reset palette
	{	
		if (CurScreenDepth() == 8)
			SetEntries(0, 255, gOriginalColors);
		DrawMenuBar();									// redraw with new colors
	}
}


Rect	pBtnRect[4];

//------------------------------------------------------------------------------------
//  Copy the picture buttons to the title screen (which is already in the offscreen buffer).
//------------------------------------------------------------------------------------
void SetupTitleScreen(void)
{
	PicHandle 	pic;
	Rect			r;
	GrafPtr		savePort;
	
	GetPort(&savePort);
	SetPort(gMainOffScreen.bits);
	
	pic = (PicHandle)GetResourceFail('PICT', 9010);
	if (pic)
	{
		r = (*pic)->picFrame;
		OffsetRect(&r, -r.left + 32, -r.top + 262);
		DrawPicture(pic, &r);
		ReleaseResource((Handle)pic);
		pBtnRect[0] = r;
	}
	pic = (PicHandle)GetResourceFail('PICT', 9011);
	if (pic)
	{
		r = (*pic)->picFrame;
		OffsetRect(&r, -r.left + 32, -r.top + 298);
		DrawPicture(pic, &r);
		ReleaseResource((Handle)pic);
		pBtnRect[1] = r;
	}
	pic = (PicHandle)GetResourceFail('PICT', 9012);
	if (pic)
	{
		r = (*pic)->picFrame;
		OffsetRect(&r, -r.left + 32, -r.top + 334);
		DrawPicture(pic, &r);
		ReleaseResource((Handle)pic);
		pBtnRect[2] = r;
	}
	pic = (PicHandle)GetResourceFail('PICT', 9013);
	if (pic)
	{
		r = (*pic)->picFrame;
		OffsetRect(&r, -r.left + 32, -r.top + 390);
		DrawPicture(pic, &r);
		ReleaseResource((Handle)pic);
		pBtnRect[3] = r;
	}
	
	SetPort(savePort);
}

//------------------------------------------------------------------------------------
//  Handle clicking in title screen buttons.  Returns:
//  	0 - New Game,   1 - Open Game,   2 - Play Intro,  3 - Quit,   -1 - No button
//------------------------------------------------------------------------------------
int DoShockTitleButtons(Point mousePt)
{
	for (int i = 0; i < 4; i++)
	{
		if (PtInRect(mousePt, &pBtnRect[i]))
			return (TrackTitleButton(i));
	}
	return (-1);
}

//------------------------------------------------------------------------------------
//  Handle mouse tracking for the button.  Returns button if clicked, -1 if not.
//------------------------------------------------------------------------------------
int TrackTitleButton(int btn)
{
	PicHandle	phNorm, phClick;
	Point			currPt;
	Boolean		oldState = TRUE;
	Boolean		newState;
	Rect			r = pBtnRect[btn];
		
	// Get handles to the regular and pressed buttons.
	phNorm = (PicHandle)GetResourceFail('PICT', 9010 + btn);
	phClick = (PicHandle)GetResourceFail('PICT', 9020 + btn);
	
	// Start by drawing the button in the clicked position.
	DrawPicture(phClick, &r);

	// While the mouse button is down, draw button in different states, as needed.
	while (StillDown())
	{
		GetMouse(&currPt);
		newState = PtInRect(currPt, &r);
		if (newState != oldState)
		{
			DrawPicture((newState) ? phClick : phNorm, &r);
			oldState = newState;
		}
	}

	// Draw the button in normal state after release of mouse button.
	DrawPicture(phNorm, &r);
	
	// Free up the picture resources.	
	ReleaseResource((Handle)phNorm);
	ReleaseResource((Handle)phClick);
	
	// If in rect when released, return the button number, else return -1.
	return ((newState) ? btn : -1);
}
