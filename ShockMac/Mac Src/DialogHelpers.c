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
//		DialogHelpers.c	-	Utility routines and user items for dialog boxes.
//
//====================================================================================


//--------------------
//  Includes
//--------------------
#include <Icons.h>
#include "InitMac.h"
#include "ShockBitmap.h"
#include "DialogHelpers.h"

//--------------------
//  Globals
//--------------------
Boolean			gGrayOK = false;
Boolean			gIgnoreGray = false;

Str255			gDimTitle;
Boolean			gDimmed;

MenuHandle	gPopupMenuHdl;
short				gPopupSel;
short				gNewGameSel[4];


//------------------------------------------------------------------------------------
//  Flash a button.
//------------------------------------------------------------------------------------
void FlashButton(WindowPtr dlog, short itemN)
{
	short		itype;
	Rect		r;
	Handle	hand;
	long		temp;
	
	GetDItem(dlog, itemN, &itype, &hand, &r);
	HiliteControl((ControlHandle)hand, 1);
	Delay(8, &temp);
	HiliteControl((ControlHandle)hand, 0);
}

//------------------------------------------------------------------------------------
//  Modal dialog filter proc to handle standard events.  So far, just handle "Esc" key.
//------------------------------------------------------------------------------------
pascal Boolean ShockFilterProc(DialogPtr dlog, EventRecord *evt, short *itemHit)
 {
	char	key;
	
	if ((evt->what==keyDown) || (evt->what==autoKey))
	{
		key = evt->message & charCodeMask;
		
		if (((key==3) || (key==13)) && (!gGrayOK || gIgnoreGray))
		{
			FlashButton(dlog,1);
			*itemHit = 1;
			return(true);
		}
		else if (((key==27) || ((key=='.') && (evt->modifiers & cmdKey))))
		{
			FlashButton(dlog,2);
			*itemHit = 2;
			return(true);
		}
	}
	
	return(false);
}

//------------------------------------------------------------------------------------
//  Modal alert filter proc to handle the "Save prompt" alert.
//------------------------------------------------------------------------------------
pascal Boolean ShockAlertFilterProc(DialogPtr dlog, EventRecord *evt, short *itemHit)
 {
	char	key;
	
	if ((evt->what==keyDown) || (evt->what==autoKey))
	{
		key = evt->message & charCodeMask;
		
		// Enter or Return or 'S' is save as pressing "Save"
		if (key==3 || key==13 || key=='s' || key=='S')
		{
			FlashButton(dlog,1);
			*itemHit = 1;
			return(true);
		}
		
		// Esc or cmd-period or 'C' same as "Cancel".
		else if (((key==27) || (key=='c') || (key=='C') || ((key=='.') && (evt->modifiers & cmdKey))))
		{
			FlashButton(dlog,3);
			*itemHit = 3;
			return(true);
		}
		
		// 'D' for "Don't Save"
		else if (key=='d' || key=='D')
		{
			FlashButton(dlog,2);
			*itemHit = 2;
			return(true);
		}
	}
	
	return(false);
}

//------------------------------------------------------------------------------------
//  User item to draw a round rectangle around an OK button.
//------------------------------------------------------------------------------------
pascal void OKButtonUser(WindowPtr dlog, short itemN)
{
	short		itype;
	Rect		r;
	Handle	hand;
	
	GetDItem(dlog, itemN, &itype ,&hand, &r);
	
	PenNormal();
	PenSize(3,3);
	if (gGrayOK && !gIgnoreGray)
		PenPat(&qd.gray);
	FrameRoundRect(&r, 16, 16);
	PenNormal();
}

//------------------------------------------------------------------------------------
//  User item to draw static text dimmed, if needed.
//------------------------------------------------------------------------------------
pascal void DimStaticUser(WindowPtr dlog, short itemN)
{
	short					itype;
	Rect					r;
	Handle				hand;
	RGBColor			rgbBlack = {0, 0, 0};
	RGBColor			rgbGray = {0x8888, 0x8888, 0x8888};
	
	GetDItem(dlog, itemN, &itype ,&hand, &r);
	
	PenNormal();
	
	if (gDimmed)
		RGBForeColor(&rgbGray);
	MoveTo(r.left + 1, r.top + 12);
	DrawString(gDimTitle);
	
	RGBForeColor(&rgbBlack);
}

//------------------------------------------------------------------------------------
//  User item to draw a popup menu control.
//------------------------------------------------------------------------------------
pascal void PopupMenuUser(WindowPtr dlog, short itemN)
{
	Rect			r, r2;
	short			itype;
	Handle		hand;
	Handle		ArrowRes;
	BitMap		ArrowIcon;
	Str255		str;
	
	GetDItem(dlog, itemN, &itype, &hand, &r);						// Get item rect
	r2 = r;																			// and save a copy
	
	r.right -= 18;																// Blank out interior of popup menu area
	InsetRect(&r, 0, 2);
	EraseRect(&r);

	r = r2;																			// Draw the frame and shadow of the popup menu
	r.bottom -= 2;
	FrameRect(&r);
	MoveTo(r.right,r.top+2);
	LineTo(r.right,r.bottom);
	LineTo(r.left+2,r.bottom);
	
	MoveTo(r.left+15, r.bottom-5);										// Draw the current menu item text
	GetItem(gPopupMenuHdl, gPopupSel, str);
//	FitStringToWidth(str,(r.right-10)-(r.left+15));
	DrawString(str);
	
	ArrowRes = GetResource('SICN', 4000);							// Draw the down arrow at right of popup
	if (ArrowRes)
	{
		HLock(ArrowRes);
		SetRect(&ArrowIcon.bounds, 0, 0, 16, 16);
		ArrowIcon.rowBytes = 2;
		ArrowIcon.baseAddr = *ArrowRes;
		
		InsetRect(&r, 1, 1);
		r.right -= 2;
		r.left = r.right - 16;
		r.bottom = r.top + 16;
		CopyBits(&ArrowIcon, &dlog->portBits, &ArrowIcon.bounds, &r, srcCopy, 0L);
		
		HUnlock(ArrowRes);
		ReleaseResource(ArrowRes);
	}
/*	
	if (MainSelect==-1)														// Dim everything if disabled
	{
		GetDItem(dlog,itemN,&itype,&hand,&r);
		PenPat(gray);
		PenMode(srcBic);
		InsetRect(&r,-1,-1);
		PaintRect(&r);
	}
*/
	PenNormal();
}


//------------------------------------------------------------------------------------
//  Draw popups for the "New Game" dialog.  This seems so wasteful.
//------------------------------------------------------------------------------------
pascal void PopupMenuUserNG(WindowPtr dlog, short itemN)
{
	Rect			r, r2;
	short			itype;
	Handle		hand;
	Handle		ArrowRes;
	BitMap		ArrowIcon;
	Str255		str;
	
	GetDItem(dlog, itemN, &itype, &hand, &r);						// Get item rect
	r2 = r;																			// and save a copy
	
	// First, draw the popup menu.
	
	r.right = r.left + 102;													// Adjust rectangle for drawing the popup
	r.top += 2;
	r.bottom -= 2;
	
	r.right -= 18;																// Blank out interior of popup menu area
	InsetRect(&r, 0, 2);
	EraseRect(&r);

	r.right += 18;																// Draw the frame and shadow of the popup menu
	r.bottom -= 2;
	FrameRect(&r);
	MoveTo(r.right,r.top+2);
	LineTo(r.right,r.bottom);
	LineTo(r.left+2,r.bottom);
	
	MoveTo(r.left+15, r.bottom-5);										// Draw the current menu item text
	GetItem(gPopupMenuHdl, gNewGameSel[itemN - 11], str);
	DrawString(str);
	
	ArrowRes = GetResource('SICN', 4000);							// Draw the down arrow at right of popup
	if (ArrowRes)
	{
		HLock(ArrowRes);
		SetRect(&ArrowIcon.bounds, 0, 0, 16, 16);
		ArrowIcon.rowBytes = 2;
		ArrowIcon.baseAddr = *ArrowRes;
		
		InsetRect(&r, 1, 1);
		r.right -= 2;
		r.left = r.right - 16;
		r.bottom = r.top + 16;
		CopyBits(&ArrowIcon, &dlog->portBits, &ArrowIcon.bounds, &r, srcCopy, 0L);
		
		HUnlock(ArrowRes);
		ReleaseResource(ArrowRes);
	}
	
	// Draw the explanation text to the right of the popup
	
	TextFont(geneva);
	TextSize(9);
	
	GetIndString(str, 8300, ((itemN - 11) * 4) + gNewGameSel[itemN - 11]);
	r2.left += 115;
	r2.top += 1;
	if (StringWidth(str) <= (r2.right - r2.left))
	{
		EraseRect(&r2);
		MoveTo(r2.left, r2.top + 16);
		DrawString(str);
	}
	else
		TextBox(str+1, *str, &r2, teForceLeft);
	
	TextFont(systemFont);
	TextSize(12);
	PenNormal();
}

//------------------------------------------------------------------------------------
//  Draw a GroupBox user item.
//------------------------------------------------------------------------------------
pascal void GroupBoxUser(WindowPtr dlog, short itemNum)
{
	short		itype;
	Rect		r;
	Handle	hand;
	
	GetDItem(dlog, itemNum, &itype, &hand, &r);
	
	PenPat(&qd.gray);
	FrameRect(&r);
	PenNormal();
}


//====================================================================================
//  The following globals and functions are for the Slider dialog control.
//====================================================================================
short				gSliderLastPos;
Handle			gSliderThumbHdl = nil;		// Handle to small icon suite for the slider
ShockBitmap	gSliderBack;						// Bitmap containing slider background
ShockBitmap	gSliderAsm;						// Bitmap for assembling slider before blitting to screen
Boolean			gSliderDimmed;					// TRUE if the slider should be drawn dimmed (and disabled)

//------------------------------------------------------------------------------------
//  Allocate all bitmaps and do other initialization for the slider.
//------------------------------------------------------------------------------------
void SetupSlider(void)
{
	// Allocate bitmaps for the background and the assembly area
	NewShockBitmap(&gSliderBack, kSliderWidth, kSliderHeight, true);
	NewShockBitmap(&gSliderAsm, kSliderWidth, kSliderHeight, true);
	
	// Setup the slider background and thumb bitmaps
	SetSliderBitmaps();
}
	
//------------------------------------------------------------------------------------
//  Setup the slider background bitmap, the thumb icon bitmap, and draw them into the assembly bitmap.
//------------------------------------------------------------------------------------
void SetSliderBitmaps(void)
{
	PicHandle 	pic;
	Rect			r;
	GrafPtr		savePort;
	
	// Load the slider background picture into the background bitmap
	pic = (PicHandle)GetResourceFail('PICT', (gSliderDimmed) ? kSliderBackPictDim : kSliderBackPict);
	if (pic != nil)
	{
		GetPort(&savePort);
		SetPort(gSliderBack.bits);
		PenPat(&qd.white);
		PaintRect(&gSliderBack.bounds);
		
		r = (*pic)->picFrame;
		OffsetRect(&r, -r.left, -r.top);
		DrawPicture(pic, &r);
		
		ReleaseResource((Handle)pic);
		SetPort(savePort);
	}
	
	// Make a handle suite for the thumb icon.
	if (gSliderThumbHdl)
		DisposeIconSuite(gSliderThumbHdl, TRUE);
	GetIconSuite(&gSliderThumbHdl, (gSliderDimmed) ? kThumbIconDim : kThumbIcon, svAllAvailableData);

	// To initialize the assembly bitmap, copy the background and place the
	// thumb at the last position.	
	SetRect(&r, 0, 0, kSliderWidth, kSliderHeight);
  	CopyBits(&gSliderBack.bits->portBits, &gSliderAsm.bits->portBits, &r, &r, srcCopy, 0L);
  	
  	r.top = -2;
  	r.bottom = r.top + 16;
  	r.left = gSliderLastPos - 2;
  	r.right = r.left + 16;
	GetPort(&savePort);
	SetPort(gSliderAsm.bits);
	PlotIconSuite(&r, atNone, ttNone, gSliderThumbHdl);
	SetPort(savePort);
}

//------------------------------------------------------------------------------------
//  Draw the slider user item by blitting the assembly bitmap to the screen.
//------------------------------------------------------------------------------------
pascal void DrawSlider(WindowPtr dlog, short itemNum)
{
	short			itype;
	Rect			itemr, offr;
	Handle		hand;
	
	SetRect(&offr, 0, 0, kSliderWidth, kSliderHeight);

	GetDItem(dlog, itemNum, &itype, &hand, &itemr);
	itemr.right = itemr.left + kSliderWidth;
	itemr.bottom = itemr.top + kSliderHeight;

  	CopyBits(&gSliderAsm.bits->portBits, &dlog->portBits, &offr, &itemr, srcCopy, 0L);
}

//------------------------------------------------------------------------------------
//  Handle the slider tracking.  Return the new slider value (0-100).
//------------------------------------------------------------------------------------
short DoSliderTracking(WindowPtr dlog, short itemNum, SliderCallbackProcPtr cb)
{
	short			itype;
	Rect			itemr, offr, r, newr;
	Handle		hand;
	Point			mousePt;
	short			newPos, origPos;
	GrafPtr		savePort;
	
	GetDItem(dlog, itemNum, &itype, &hand, &itemr);
	origPos = gSliderLastPos;													// Remember what the slider pos was
																							// before this tracking.
	do
	{
		GetMouse(&mousePt);														// Get current mouse location (local)
		
		if (mousePt.v < itemr.top-3 || mousePt.v > itemr.bottom+3)	// If we're out of the slider (plus slack),
			newPos = origPos;															// snap back to the original position
		else
		{
			newPos = mousePt.h - itemr.left - 6;							// Figure the new position
		
			if (newPos > kSliderWidth-12)									// Do a little range checking for width
				newPos = kSliderWidth-12;
			if (newPos < 0)
				newPos = 0;
		}

		if (newPos != gSliderLastPos)
		{
			// Copy a rectangle from the background pict to the assembly pict that will
			// redraw over the old slider position.
			SetRect(&offr, gSliderLastPos, 0, gSliderLastPos + 12, kSliderHeight);
	  		CopyBits(&gSliderBack.bits->portBits, &gSliderAsm.bits->portBits, &offr, &offr, srcCopy, 0L);
			
			// Copy a rectangle from the background pict to the assembly pict that will
			// redraw over the new slider position.  Then, plot the thumb
			SetRect(&newr, newPos, 0, newPos + 12, kSliderHeight);
	  		CopyBits(&gSliderBack.bits->portBits, &gSliderAsm.bits->portBits, &newr, &newr, srcCopy, 0L);

		  	r.top = -2;
		  	r.bottom = r.top + 16;
		  	r.left = newPos - 2;
		  	r.right = r.left + 16;
			GetPort(&savePort);
			SetPort(gSliderAsm.bits);
			PlotIconSuite(&r, atNone, ttNone, gSliderThumbHdl);
			SetPort(savePort);
			
			// Lastly, copy the union of the two rectangles to the screen.
			UnionRect(&offr, &newr, &r);
			offr = r;
			OffsetRect(&offr, itemr.left, itemr.top);
  			CopyBits(&gSliderAsm.bits->portBits, &dlog->portBits, &r, &offr, srcCopy, 0L);
			
			gSliderLastPos = newPos;												// Save off as last pos
			
			if (cb)																		// If there is a callback proc, notify it.
				(*cb)(newPos);
		}
	} while (StillDown());
	
	return (gSliderLastPos);
}

//------------------------------------------------------------------------------------
//  De-allocate all bitmaps.
//------------------------------------------------------------------------------------
void FreeSlider(void)
{
	DisposeIconSuite(gSliderThumbHdl, TRUE);
	gSliderThumbHdl = NULL;
	FreeShockBitmap(&gSliderAsm);
	FreeShockBitmap(&gSliderBack);
}
