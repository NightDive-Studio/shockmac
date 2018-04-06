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
//=======================================================================
//
//		System Shock - й1994-1995 Looking Glass Technologies, Inc.
//
//		Tests.c	-	Code to do test commands.
//
//=======================================================================


//--------------------
//  Includes
//--------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Movies.h>

#include "amap.h"
#include "lvldata.h"
#include "saveload.h"
#include "audiolog.h"

#include "Shock.h"
#include "InitMac.h"
#include "ShockBitmap.h"
#include "MoviePlay.h"
#include "Tests.h"
 #include "FrUtils.h"

// MLA - stuff for TestRender
#include "frtypes.h"
#include "frflags.h"
#include "frworld.h"
#include "map.h"
#include "tilename.h"
#include "frprotox.h"
#include "Rendtool.h"
#include "faketime.h"
#include "Game_Screen.h"
#include "fullscrn.h"

extern errtype object_data_load(void);

//--------------------
//  Typedefs
//--------------------
/*typedef struct {
   grs_bitmap bm;       // embedded bitmap, bm.bits set to NULL
   union {
      LGRect updateArea;  // update area (for anims)
      LGRect anchorArea;  // area to anchor sub-bitmap
      LGPoint anchorPt;   // point to anchor from
      } u;
   long pallOff;        // offset to pallette
                        // bitmap's bits follow immediately
} FrameDesc; */

//--------------------
//  Prototypes
//--------------------
void DoUpdateBrowseImages(WindowPtr wnd);
void SetImage(Ptr p);
void DoClickBrowseImages(Point localPt);
void DoUpdateBrowseFonts(WindowPtr wnd);
void SetFont(Ptr p);
void DoClickBrowseFonts(Point localPt);
void DoClickLoadPalette(Point localPt);
void MoveSquare(short x, short y);
Boolean IsOptKeyDown(void);
Boolean IsCmdKeyDown(void);

//--------------------
//  Globals
//--------------------
short		gCurrTest;

// defines
#define QuickTickCount() (* (long *) 0x16A)

Boolean IsOptKeyDown(void)
 {
	long		keys[4];

#ifdef __MWERKS__
		GetKeys((UInt32 *)keys);
#else
		GetKeys(keys);
#endif
	return((keys[1] & 0x00000004) != 0L);
 }

Boolean IsCmdKeyDown(void)
 {
	long		keys[4];

#ifdef __MWERKS__
		GetKeys((UInt32 *)keys);
#else
		GetKeys(keys);
#endif
	return((keys[1] & 0x00008000) != 0L);
 }


//------------------------------------------------------------------------
//  Initialize globals for tests.
//------------------------------------------------------------------------
void SetupTests(void)
{
	gCurrTest = -1;			// No test currently running
}

//------------------------------------------------------------------------
//  Handle mouse-downs for tests.
//------------------------------------------------------------------------
void DoTestClick(Point localPt)
{
	switch(gCurrTest)
	{
		case 0:
			DoClickBrowseImages(localPt);
			break;
		case 1:
			DoClickBrowseFonts(localPt);
			break;
		case 2:
			DoClickLoadPalette(localPt);
			break;
	}
}

//------------------------------------------------------------------------
//  Handle update events for tests.
//------------------------------------------------------------------------
void DoTestUpdate(WindowPtr wnd)
{
	switch(gCurrTest)
	{
		case 0:
			DoUpdateBrowseImages(wnd);
			break;
		case 1:
		case 2:
			DoUpdateBrowseFonts(wnd);
			break;
	}
}


//=======================================================================
//  Private Globals used for Testing.
//=======================================================================
Rect			pResourceBtns;
Rect			pItemBtns;
Rect			pDoneBtn;
short			pResNum;
RefTable	*pRefTbl;
Ref			pRefId;
Id				pResId;


//=======================================================================
//  BROWSE IMAGES TEST
//=======================================================================
//------------------------------------------------------------------------
//  Initialize globals for tests.
//------------------------------------------------------------------------
void DoTestBrowseImages(void)
{
	StandardFileReply	reply;
	SFTypeList				typeList;
	Ptr						p = NULL;

	typeList[0] = 'Sdat';
	StandardGetFile(nil, 1, typeList, &reply);
	if (!reply.sfGood)
		return;

	pResNum = ResOpenFile(&reply.sfFile);
	if (pResNum < 0)
	{
		ParamText("\pCan't open res file.", "\p", "\p", "\p");
		StopAlert(1000, nil);
		return;
	}
	
	{
		Id 				id;
		ResDesc		*prd;
		long			rfs;
		
		for (id = ID_MIN; id <= resDescMax; id++)
		{
			prd = RESDESC(id);
			if (prd->filenum == pResNum && prd->type == RTYPE_IMAGE)
			{
				pRefId = MKREF(id, 0);
   				pRefTbl = ResReadRefTable(REFID(pRefId));
	   			rfs = RefSize(pRefTbl,REFINDEX(pRefId));
	   			p = NewPtrClear(rfs);
	  			RefExtract(pRefTbl, pRefId, p);
//	   			ResFreeRefTable(rt);
	   			break;
			}
		}
	}
	if (p == NULL)
	{
		ParamText("\pCouldn't get a ref to an image.", "\p", "\p", "\p");
		StopAlert(1000, nil);
		return;
	}

	MenuHandle	mh = GetMenu(mTests);
	DisableItem(mh, 0);

	GrafPtr	savePort;	
	GetPort(&savePort);									// Clear out the offscreen bitmap and write
	SetPort(gMainOffScreen.bits);						// font stats.
	PaintRect(&gMainOffScreen.bounds);
	SetPort(savePort);

	SetImage(p);												// Set the initial bitmap
	DisposePtr(p);
	
	gCurrTest = 0;
	InvalRect(&gMainWindow->portRect);
}

//------------------------------------------------------------------------
//  Handle update events for tests.
//------------------------------------------------------------------------
void DoUpdateBrowseImages(WindowPtr)
{
	Rect			r;
	short			x, y;
	PicHandle	pic;

 	r = gMainWindow->portRect;
 	r.bottom = r.top + 74;
 	EraseRect(&r);
 	
 	x = (gMainWindow->portRect.right - gMainWindow->portRect.left - 230) / 2;
 	y = r.top+50;
 	TextFont(systemFont);
 	TextSize(12);
 	TextFace(0);
 	
 	MoveTo(x, y);
 	DrawString("\pResource");

 	MoveTo(x+117, y);
 	DrawString("\pItem");

	pic = (PicHandle)GetResourceFail('PICT', 19000);		// Draw up/down buttons
	if (pic!=0L)
	{
		r = (*pic)->picFrame;
		OffsetRect(&r, -r.left, -r.top);
		OffsetRect(&r, x+69, y-22);
		DrawPicture(pic, &r);
		pResourceBtns = r;
		
		OffsetRect(&r, 89, 0);
		DrawPicture(pic, &r);
		pItemBtns = r;
	}

	pic = (PicHandle)GetResourceFail('PICT', 19001);		// Draw Done button
	if (pic!=0L)
	{
		r = (*pic)->picFrame;
		OffsetRect(&r, -r.left, -r.top);
		OffsetRect(&r, x+209, y-11);
		DrawPicture(pic, &r);
		pDoneBtn = r;
	}
}

//------------------------------------------------------------------------
//  Copy the bitmap from the resource to the offscreen buffer.
//------------------------------------------------------------------------
void SetImage(Ptr p)
{
	FrameDesc	*fd = (FrameDesc *)p;
	char			buff[256];
	RGBColor	black = {0, 0, 0};
	RGBColor	white = {0xffff, 0xffff, 0xffff};
	grs_bitmap	bm2;
	
	gr_set_fcolor(0xff);
	gr_rect(0, 0, 640, 480);

	sprintf(buff, "RES: %d, ITEM: %d of %d        IMAGE: %dx%d, r:%d, t:%d, a:%d",
				REFID(pRefId), REFINDEX(pRefId)+1, pRefTbl->numRefs,
				fd->bm.w, fd->bm.h, fd->bm.row, fd->bm.type, fd->bm.align);
				
	TextFont(geneva);
	TextSize(9);
	TextMode(srcOr);

	MoveTo(1, 12);
	RGBForeColor(&white);
	DrawText(buff, 0, strlen(buff));
	RGBForeColor(&black);

	fd->bm.bits = (uchar *)(fd+1);
	if (fd->bm.type == 4)
	{
		gr_rsd8_convert(&fd->bm, &bm2);
//		gr_bitmap(&bm2, 0, 40);
		gr_scale_ubitmap(&bm2, 0, 40, bm2.w * 2, bm2.h * 2);
//		gr_scale_ubitmap(&bm2, 0, 0, 640, 480);
	}
	else
//		gr_bitmap(&fd->bm, 0, 40);
		gr_scale_ubitmap(&fd->bm, 0, 40, fd->bm.w * 2, fd->bm.h * 2);
//		gr_scale_ubitmap(&fd->bm, 0, 0, 640, 480);
}

//------------------------------------------------------------------------
//  Copy the bitmap from the resource to the offscreen buffer.
//------------------------------------------------------------------------
void DoClickBrowseImages(Point localPt)
{
	Ptr		p;
	short		id = REFID(pRefId);
	short		ind = REFINDEX(pRefId);
	
	if (PtInRect(localPt, &pDoneBtn))					// We're done.
	{
	   	ResFreeRefTable(pRefTbl);							// Free the existing refTable
		ResCloseFile(pResNum);								// Close the res file

		gCurrTest = -1;											// No longer testing
		InvalRect(&gMainWindow->portRect);

		MenuHandle	mh = GetMenu(mTests);
		EnableItem(mh, 0);
		DrawMenuBar();
		return;
	}
	
	if (PtInRect(localPt, &pResourceBtns))			// Get the prev/next resource.
	{
		Id 				n;
		ResDesc		*prd;
		bool			found = FALSE;
		
		if (localPt.v <= pResourceBtns.top+19)		// If in upper button:
		{
			for (n = id-1; n >= 0; n--)
			{
				prd = RESDESC(n);
				if (prd->filenum == pResNum && prd->type == RTYPE_IMAGE)
				{
					found = TRUE;
					break;
				}
			}
		}
		else															// Else in lower button:
		{
			for (n = id+1; n <= resDescMax; n++)
			{
				prd = RESDESC(n);
				if (prd->filenum == pResNum && prd->type == RTYPE_IMAGE)
				{
					found = TRUE;
					break;
				}
			}
		}
		if (found)
		{
   			ResFreeRefTable(pRefTbl);					// Free the existing refTable

			pRefId = MKREF(n, 0);
 			pRefTbl = ResReadRefTable(REFID(pRefId));
			p = NewPtrClear(RefSize(pRefTbl,REFINDEX(pRefId)));
  			RefExtract(pRefTbl, pRefId, p);
			SetImage(p);
			DisposePtr(p);
		}
		else SysBeep(0);
		return;
	}
	
	if (PtInRect(localPt, &pItemBtns))					// Get the next image in the resource.
	{
		if (localPt.v <= pResourceBtns.top+19)		// If in upper button:
		{
			if (IsOptKeyDown())
				ind -= 100;
			else
				ind--;
			if (ind >= 0)
			{
				pRefId = MKREF(id, ind);
		
				p = NewPtrClear(RefSize(pRefTbl,REFINDEX(pRefId)));
				RefExtract(pRefTbl, pRefId, p);
				SetImage(p);
				DisposePtr(p);
			}
			else
				SysBeep(0);
		}
		else															// If in lower button:
		{
			if (IsOptKeyDown())
				ind += 100;
			else
				ind++;
			if (ind < pRefTbl->numRefs)
			{
				pRefId = MKREF(id, ind);
		
				p = NewPtrClear(RefSize(pRefTbl,REFINDEX(pRefId)));
				RefExtract(pRefTbl, pRefId, p);
				SetImage(p);
				DisposePtr(p);
			}
			else
				SysBeep(0);
		}
		return;
	}
}




//=======================================================================
//  BROWSE FONTS TEST
//=======================================================================
//------------------------------------------------------------------------
//  Initialize globals for tests.
//------------------------------------------------------------------------
void DoTestBrowseFonts(void)
{
	StandardFileReply	reply;
	SFTypeList				typeList;
	Ptr						p = NULL;

	typeList[0] = 'Sres';
	StandardGetFile(nil, 1, typeList, &reply);
	if (!reply.sfGood)
		return;

	pResNum = ResOpenFile(&reply.sfFile);
	if (pResNum < 0)
	{
		ParamText("\pCan't open res file.", "\p", "\p", "\p");
		StopAlert(1000, nil);
		return;
	}
	
	{
		Id 				id;
		ResDesc		*prd;
		long			rfs;
		
		for (id = ID_MIN; id <= resDescMax; id++)
		{
			prd = RESDESC(id);
			if (prd->filenum == pResNum && prd->type == RTYPE_FONT)
			{
				ResLock(id);
				rfs = ResSize(id);
	   			p = NewPtrClear(rfs);
				ResExtract(id, p);
				ResUnlock(id);
				pResId = id;
	   			break;
			}
		}
	}
	if (p == NULL)
	{
		ParamText("\pCouldn't get a ref to an image.", "\p", "\p", "\p");
		StopAlert(1000, nil);
		return;
	}

	MenuHandle	mh = GetMenu(mTests);
	DisableItem(mh, 0);

	SetFont(p);												// Set the initial bitmap
	DisposePtr(p);
	
	gCurrTest = 1;
	InvalRect(&gMainWindow->portRect);
}

//------------------------------------------------------------------------
//  Handle update events for font browsing.
//------------------------------------------------------------------------
void DoUpdateBrowseFonts(WindowPtr)
{
	Rect			r;
	short			x, y;
	PicHandle	pic;

 	r = gMainWindow->portRect;
 	r.bottom = r.top + 74;
 	EraseRect(&r);
 	
 	x = (gMainWindow->portRect.right - gMainWindow->portRect.left - 230) / 2;
 	y = r.top+50;
 	TextFont(systemFont);
 	TextSize(12);
 	TextFace(0);
 	
 	MoveTo(x, y);
 	DrawString("\pResource");

	pic = (PicHandle)GetResourceFail('PICT', 19000);		// Draw up/down buttons
	if (pic!=0L)
	{
		r = (*pic)->picFrame;
		OffsetRect(&r, -r.left, -r.top);
		OffsetRect(&r, x+69, y-22);
		DrawPicture(pic, &r);
		pResourceBtns = r;
	}

	pic = (PicHandle)GetResourceFail('PICT', 19001);		// Draw Done button
	if (pic!=0L)
	{
		r = (*pic)->picFrame;
		OffsetRect(&r, -r.left, -r.top);
		OffsetRect(&r, x+117, y-11);
		DrawPicture(pic, &r);
		pDoneBtn = r;
	}
}

//------------------------------------------------------------------------
//  Copy the bitmap from the resource to the offscreen buffer.
//------------------------------------------------------------------------
void SetFont(Ptr p)
{
	GrafPtr		savePort;
	grs_font	*fd = (grs_font *)p;
	Ptr			imgp, offp;
	short			i;
	char			buff[256];
	RGBColor	black = {0, 0, 0};
	RGBColor	white = {0xffff, 0xffff, 0xffff};
	
	GetPort(&savePort);									// Clear out the offscreen bitmap and write
	SetPort(gMainOffScreen.bits);						// font stats.
	PaintRect(&gMainOffScreen.bounds);

	sprintf(buff, "RES: %d       FONT: %d, min:%d, max:%d, %dx%d",
				pResId,
				fd->id, fd->min, fd->max, fd->w, fd->h);
	
	TextFont(geneva);
	TextSize(9);
	TextMode(srcOr);

	MoveTo(1, 12);
	RGBForeColor(&white);
	DrawText(buff, 0, strlen(buff));
	RGBForeColor(&black);
	SetPort(savePort);
	
	if (fd->id == 0xCCCC)			// If it's a color font.
	{
		imgp = (Ptr)fd;
		imgp += fd->buf;
		offp = gMainOffScreen.Address + (gMainOffScreen.RowBytes * 17);
		for (i=0; i < fd->h; i++)						// Copy image data to the offscreen bitmap
		{
			BlockMove(imgp, offp, (fd->w > gMainOffScreen.RowBytes) ? gMainOffScreen.RowBytes : fd->w);
			imgp += fd->w;
			offp += gMainOffScreen.RowBytes;
		}
	}
/*
	else
	{
		ShockBitmap	monoMap;
		Rect	r;

		NewShockBitmap(&monoMap, fd->w, fd->h, FALSE);
		imgp = (Ptr)fd;
		imgp += fd->buf;
		offp = monoMap.Address + (monoMap.RowBytes * 17);
		for (i=0; i < fd->h; i++)						// Copy image data to the offscreen bitmap
		{
			BlockMove(imgp, offp, monoMap.RowBytes);
			imgp += fd->w;
			offp += monoMap.RowBytes;
		}
		SetRect(&r, 0, 0, fd->w, fd->h);
	  	CopyBits(&monoMap.bits->portBits, &gMainOffScreen.bits->portBits,
	  				  &r, &r, srcCopy, 0L);
	}
*/
  	CopyBits(&gMainOffScreen.bits->portBits, &gMainWindow->portBits,
  				  &gOffActiveArea, &gActiveArea, srcCopy, 0L);
}

//------------------------------------------------------------------------
//  Handle button clicks for Font browsing.
//------------------------------------------------------------------------
void DoClickBrowseFonts(Point localPt)
{
	Ptr		p;
	short		id = pResId;
	
	if (PtInRect(localPt, &pDoneBtn))					// We're done.
	{
		ResCloseFile(pResNum);								// Close the res file

		gCurrTest = -1;											// No longer testing
		InvalRect(&gMainWindow->portRect);

		MenuHandle	mh = GetMenu(mTests);
		EnableItem(mh, 0);
		DrawMenuBar();
		return;
	}
	
	if (PtInRect(localPt, &pResourceBtns))			// Get the prev/next resource.
	{
		Id 				n;
		ResDesc		*prd;
		bool			found = FALSE;
		
		if (localPt.v <= pResourceBtns.top+19)		// If in upper button:
		{
			for (n = id-1; n >= 0; n--)
			{
				prd = RESDESC(n);
				if (prd->filenum == pResNum && prd->type == RTYPE_FONT)
				{
					found = TRUE;
					break;
				}
			}
		}
		else															// Else in lower button:
		{
			for (n = id+1; n <= resDescMax; n++)
			{
				prd = RESDESC(n);
				if (prd->filenum == pResNum && prd->type == RTYPE_FONT)
				{
					found = TRUE;
					break;
				}
			}
		}
		if (found)
		{
			pResId = n;
			ResLock(n);
   			p = NewPtrClear(ResSize(n));
			ResExtract(n, p);
			ResUnlock(n);
			SetFont(p);
			DisposePtr(p);
		}
		else SysBeep(0);
		return;
	}
}




//=======================================================================
//  LOAD PALETTE TEST
//=======================================================================
//------------------------------------------------------------------------
//  Open a file containing palette resources.
//------------------------------------------------------------------------
void DoTestLoadPalette(void)
{
	StandardFileReply	reply;
	SFTypeList				typeList;
	Ptr						p = NULL;

	typeList[0] = 'Sres';
	StandardGetFile(nil, 1, typeList, &reply);
	if (!reply.sfGood)
		return;

	pResNum = ResOpenFile(&reply.sfFile);
	if (pResNum < 0)
	{
		ParamText("\pCan't open res file.", "\p", "\p", "\p");
		StopAlert(1000, nil);
		return;
	}
	
	{
		Id 				id;
		ResDesc		*prd;
		long			rfs;
		
		for (id = ID_MIN; id <= resDescMax; id++)
		{
			prd = RESDESC(id);
			if (prd->filenum == pResNum && prd->type == RTYPE_UNKNOWN)
			{
				ResLock(id);
				rfs = ResSize(id);
	   			p = NewPtrClear(rfs);
				ResExtract(id, p);
				ResUnlock(id);
				pResId = id;
	   			break;
			}
		}
	}
	if (p == NULL)
	{
		ParamText("\pCouldn't get a palette resource.", "\p", "\p", "\p");
		StopAlert(1000, nil);
		return;
	}

	MenuHandle	mh = GetMenu(mTests);
	DisableItem(mh, 0);

 	gr_set_pal(0, 256, (uchar *)p);
	DisposePtr(p);
	
	gCurrTest = 2;
	InvalRect(&gMainWindow->portRect);
}

//------------------------------------------------------------------------
//  Handle button clicks for Font browsing.
//------------------------------------------------------------------------
void DoClickLoadPalette(Point localPt)
{
	Ptr		p;
	short		id = pResId;
	
	if (PtInRect(localPt, &pDoneBtn))					// We're done.
	{
		ResCloseFile(pResNum);								// Close the res file

		gCurrTest = -1;											// No longer testing
		InvalRect(&gMainWindow->portRect);

		MenuHandle	mh = GetMenu(mTests);
		EnableItem(mh, 0);
		DrawMenuBar();
		return;
	}
	
	if (PtInRect(localPt, &pResourceBtns))			// Get the prev/next resource.
	{
		Id 				n;
		ResDesc		*prd;
		bool			found = FALSE;
		
		if (localPt.v <= pResourceBtns.top+19)		// If in upper button:
		{
			for (n = id-1; n >= 0; n--)
			{
				prd = RESDESC(n);
				if (prd->filenum == pResNum && prd->type == RTYPE_UNKNOWN)
				{
					found = TRUE;
					break;
				}
			}
		}
		else															// Else in lower button:
		{
			for (n = id+1; n <= resDescMax; n++)
			{
				prd = RESDESC(n);
				if (prd->filenum == pResNum && prd->type == RTYPE_UNKNOWN)
				{
					found = TRUE;
					break;
				}
			}
		}
		if (found)
		{
			pResId = n;
			ResLock(n);
   			p = NewPtrClear(ResSize(n));
			ResExtract(n, p);
			ResUnlock(n);
 			gr_set_pal(0, 256, (uchar *)p);
			DisposePtr(p);
		}
		else SysBeep(0);
		return;
	}
}




//=======================================================================
//  MOVEMENT KEY TEST
//=======================================================================
Rect			gSquare;
RGBColor	gSqColor;
static RNDSTREAM_STD(rs);

//------------------------------------------------------------------------
void DoTestMoveKeys(void)
{
	GrafPtr		savePort;
	RGBColor	black = {0, 0, 0};
	RGBColor	white = {0xffff, 0xffff, 0xffff};
	Boolean		done;
	
	GetPort(&savePort);									// Clear out the offscreen bitmap
	SetPort(gMainOffScreen.bits);
	PaintRect(&gMainOffScreen.bounds);

	SetRect(&gSquare, 0, 0, 16, 16);				// Set up initial rectangle to draw
	OffsetRect(&gSquare, gMainOffScreen.bounds.right / 2, gMainOffScreen.bounds.bottom / 2);
	gSqColor = white;
	RGBForeColor(&gSqColor);
	PaintRect(&gSquare);									// Draw it
	RGBForeColor(&black);

	SetPort(savePort);										// Blit to the screen.
  	CopyBits(&gMainOffScreen.bits->portBits, &gMainWindow->portBits,
  				  &gOffActiveArea, &gActiveArea, srcCopy, 0L);
	
	TextFont(geneva);										// Draw some instructions.
	TextSize(9);
	TextMode(srcOr);

	MoveTo(10, -20);
	RGBForeColor(&white);
	DrawString("\pPress arrow keys to move, <space> to change colors, 'Q' to quit the test.");
	RGBForeColor(&black);

	RndSeed(&rs, 22);										// Start off the random number generator.

//	kb_startup(NULL);
	done = FALSE;
	while (!done)												// Poll the keyboard.
	{
		if (kb_state(0x31))								// Spacebar - change color
		{
			gSqColor.red = RndRange(&rs, 0, 0xFFFF);
			gSqColor.green = RndRange(&rs, 0, 0xFFFF);
			gSqColor.blue = RndRange(&rs, 0, 0xFFFF);
			MoveSquare(0, 0);
		}
		if (kb_state(0x7B))
			MoveSquare(-1, 0);
		if (kb_state(0x7C))
			MoveSquare(1, 0);
		if (kb_state(0x7D))
			MoveSquare(0, 1);
		if (kb_state(0x7E))
			MoveSquare(0, -1);
		if (kb_state(0x0C))								// 'Q'
			done = TRUE;
	}
//	kb_shutdown();

	PaintRect(&gMainWindow->portRect);
}

//------------------------------------------------------------------------
void MoveSquare(short x, short y)
{
	GrafPtr		savePort;
	RGBColor	black = {0, 0, 0};
	RGBColor	white = {0xffff, 0xffff, 0xffff};

	GetPort(&savePort);									// Erase the old rect
	SetPort(gMainOffScreen.bits);
	PaintRect(&gSquare);
	
	OffsetRect(&gSquare, x, y);
	RGBForeColor(&gSqColor);
	PaintRect(&gSquare);									// Draw the new rect
	RGBForeColor(&black);

	SetPort(savePort);										// Blit to the screen just what has changed.
  	CopyBits(&gMainOffScreen.bits->portBits, &gMainWindow->portBits,
  				  &gOffActiveArea, &gActiveArea, srcCopy, 0L);
}




//=======================================================================
//  MOUSE TEST
//=======================================================================
void show_coords(mouse_event* e, void* data);

void DoTestMouse(void)
{
	GrafPtr		savePort;
	RGBColor	black = {0, 0, 0};
	RGBColor	white = {0xffff, 0xffff, 0xffff};
	Boolean		done;
	int			callbackid = -1;
	
	GetPort(&savePort);									// Clear out the offscreen bitmap
	SetPort(gMainOffScreen.bits);
	PaintRect(&gMainOffScreen.bounds);

	// Write initial mouse pos

	SetPort(savePort);										// Blit to the screen.
  	CopyBits(&gMainOffScreen.bits->portBits, &gMainWindow->portBits,
  				  &gOffActiveArea, &gActiveArea, srcCopy, 0L);
	
	TextFont(geneva);										// Draw some instructions.
	TextSize(9);
	TextMode(srcOr);

	MoveTo(1, -20);
	RGBForeColor(&white);
	DrawString("\pMove mouse and click button.  Option-click to quit the test.");
	RGBForeColor(&black);

	mouse_init(16,16);
	mouse_set_callback(show_coords, (void*)0x600F, &callbackid);
	done = FALSE;
	while (!done)												// Check for mouse clicks.
	{
		mouse_event e;
		errtype err = mouse_next(&e);
		char	buff[256];
		Rect	r;
		
		if (err == OK)
		{
			if (e.type ==2 && (e.modifiers & 0x08))		// If option-click, quit
				done = TRUE;
			else															// If normal click, draw stats.
			{
				GetPort(&savePort);
				SetPort(gMainOffScreen.bits);
				SetRect(&r, 0, 16, gMainOffScreen.bounds.right, 28);
				PaintRect(&r);
			
				if (e.type == 2)
					sprintf(buff, "CLICK: (%d, %d)  t:%d, ts:%d",
								e.x, e.y, e.type, e.timestamp);
				if (e.type == 4)
					sprintf(buff, "UP: (%d, %d)  t:%d, ts:%d",
								e.x, e.y, e.type, e.timestamp);
				else if (e.type == 1)
					sprintf(buff, "MOVE: (%d, %d)  t:%d",
								e.x, e.y, e.type);
							
				TextFont(geneva);
				TextSize(9);
				TextMode(srcOr);
			
				MoveTo(1, 26);
				RGBForeColor(&white);
				DrawText(buff, 0, strlen(buff));
				RGBForeColor(&black);
				SetPort(savePort);
			  	CopyBits(&gMainOffScreen.bits->portBits, &gMainWindow->portBits,
			  				  &r, &r, srcCopy, 0L);
			}
		}
	}
	mouse_shutdown();

	PaintRect(&gMainWindow->portRect);
}

//------------------------------------------------------------------------
void show_coords(mouse_event* e, void* )
{
	GrafPtr		savePort;
	RGBColor	black = {0, 0, 0};
	RGBColor	col = {0xFFFF, 0x3333, 0x3333};
	Rect			r;
	char			buff[256];

	GetPort(&savePort);													// Erase text area
	SetPort(gMainOffScreen.bits);
	SetRect(&r, 0, 0, gMainOffScreen.bounds.right, 15);
	PaintRect(&r);

	sprintf(buff, "POS: (%d, %d)", e->x, e->y);
				
	TextFont(geneva);
	TextSize(9);
	TextMode(srcOr);

	MoveTo(1, 12);
	RGBForeColor(&col);
	DrawText(buff, 0, strlen(buff));
	RGBForeColor(&black);
	SetPort(savePort);
  	CopyBits(&gMainOffScreen.bits->portBits, &gMainWindow->portBits,
  				  &r, &r, srcCopy, 0L);
}



#define	PAL_CHG_TEST	1

//=======================================================================
//  PLAY MOVIE TEST
//=======================================================================
pascal void TestCallbackProc(QTCallBack cb, long refCon);

void DoPlayMovie(short cmd)
{
	StandardFileReply	reply;
	SFTypeList				typeList;
	Ptr						p = NULL;
	Movie 					aMovie = nil;
	OSErr					err;
	RGBColor				black = {0, 0, 0};
	RGBColor				white = {0xffff, 0xffff, 0xffff};
#ifdef PAL_CHG_TEST
	Boolean					chgPal = FALSE;
#endif

	EnterMovies();
	
	typeList[0] = 'MooV';
	StandardGetFilePreview(nil, 1, typeList, &reply);
	if (!reply.sfGood)
	{
		ExitMovies();
		return;
	}

	// Get the movie.	
	{
		short 	movieResFile;
		
		err = OpenMovieFile(&reply.sfFile, &movieResFile, fsRdPerm);
		if (err == noErr) 
		{
			short 		movieResID = 0;		// get first movie
			Str255 		movieName;
			Boolean 		wasChanged;
		
			err = NewMovieFromFile(&aMovie, movieResFile, &movieResID,
							movieName, newMovieActive, &wasChanged);
			CloseMovieFile (movieResFile);
		}
		else
		{
			SysBeep(0);
			ExitMovies();
			return;
		}
	}

//	SetEntries(0, 255, gOriginalColors);
	RGBForeColor(&black);
	PaintRect(&gMainWindow->portRect);
	
	{
		Rect 					movieBox, r;
		EventRecord		theEvent;
		WindowPtr			whichWindow;
		Boolean				done;
		TimeValue			tv;
		TimeScale			ts;
		short					mv;
		Fixed					mr;
		long					tc;
		long					mt;
		Str255				mtStr;
		long					ms;
		TimeBase			tb;
		QTCallBack			qtb;
		QTCallBackUPP	callbackProc;
		char					buff[256];
		CTabHandle			ctab[16];
		Track				vidTrack;
		Media				vidMedia;

		
		GetMovieBox (aMovie, &movieBox);		
		OffsetRect (&movieBox, -movieBox.left, -movieBox.top);
		if (cmd == testPlayMovie2x)
		{
			movieBox.right *= 2;
			movieBox.bottom *= 2;
		}
		OffsetRect(&movieBox, (gActiveArea.right - movieBox.right) / 2,
										  (gActiveArea.bottom - movieBox.bottom) / 2);
		SetMovieBox (aMovie, &movieBox);
		SetMovieGWorld (aMovie, (CGrafPtr)gMainWindow, nil);

		// Get the movie's color table and set the palette with it.
		GetMovieColorTable(aMovie, &ctab[0]);
		SetEntries(1, 253, &(**ctab[0]).ctTable[1]);
		ResetCTSeed();

		//еее Load movie into RAM
//		err = LoadMovieIntoRam(aMovie, 0, GetMovieDuration(aMovie), keepInRam);
		GoToBeginningOfMovie(aMovie);
		
		// Get movie info.
		
		mr = GetMoviePreferredRate(aMovie);
		if (cmd == testPlayMovieDblSpd)
		{
			mr *= 2;
			SetMoviePreferredRate(aMovie, mr);
		}
		else if (cmd == testPlayMovieHalfSpd)
		{
			mr /= 2;
			SetMoviePreferredRate(aMovie, mr);
		}
		tv = GetMovieDuration(aMovie);
		ts = GetMovieTimeScale(aMovie);
		mv = GetMoviePreferredVolume(aMovie);
		tc = GetMovieTrackCount(aMovie);
		mt = GetMovieCreationTime(aMovie);
		LG_memset(mtStr, 0, 255);
		IUDateString(mt, shortDate, mtStr);
		ms = GetMovieDataSize(aMovie, 0, tv);
		
		// Get track and media information.
		{
			Track	track;
			Media	media;
			
			long	tc = GetMovieTrackCount(aMovie);
			for (long t = 1; t <= tc; t++)
			{
				track = GetMovieIndTrack(aMovie, t);
				if (track)
				{
					media = GetTrackMedia(track);
					if (media)
					{
						OSType	medType;
						
						GetMediaHandlerDescription(media, &medType, nil, nil);
						if (medType == VideoMediaType)
						{
							vidTrack = track;
							vidMedia = media;
							
							ImageDescriptionHandle	idh = (ImageDescriptionHandle)NewHandle(sizeof(ImageDescription));
							
							long	sdc = GetMediaSampleDescriptionCount(media);
							for (long s = 1; s <= sdc; s++)
							{
								GetMediaSampleDescription(media, s, (SampleDescriptionHandle)idh);
								GetImageDescriptionCTable(idh, &ctab[s]);
							}
							
							DisposeHandle((Handle)idh);
						}
					}
				}
			}
		}
		
		// Draw movie information.
			
		TextFont(geneva);
		TextSize(9);
		TextMode(srcOr);
		
		sprintf(buff, "MOVIE:  box(%d, %d, %d, %d), size:%dK, dur:%d sec, rate:0x%x, vol:0x%x, tracks:%d, created:%s", 
							movieBox.left, movieBox.top, movieBox.right, movieBox.bottom,
							ms/1024, tv/ts, mr, mv, tc, &mtStr[1]);

		MoveTo(1, -20);
		RGBForeColor(&white);
		DrawText(buff, 0, strlen(buff));
		RGBForeColor(&black);
		SetRect(&r, 50, -17, 100, 0);
		
		// Setup a time base call-back routine.
		
		tb = GetMovieTimeBase(aMovie);
		qtb = NewCallBack(tb, callBackAtTime);
		if (qtb)
		{
			callbackProc = NewQTCallBackProc(TestCallbackProc);		// Make a UPP for callback
			CallMeWhen(qtb, callbackProc, 0, triggerTimeFwd, 7000, ts);
		}
		else
			SysBeep(0);

		// Preroll the movie to make it play mo' better.
		
		PrerollMovie(aMovie, 0, mr);
		
		// Movie playing event loop.
		
		StartMovie (aMovie);	
		HideCursor();
		done = FALSE;
		while(!IsMovieDone(aMovie) && !done) 
		{
			if (WaitNextEvent (everyEvent, &theEvent, 0, nil)) 
			{
				switch ( theEvent.what ) 
				{
					case updateEvt:	
						whichWindow = (WindowPtr)theEvent.message;
						if (whichWindow == gMainWindow) 
						{
							BeginUpdate (whichWindow);
							UpdateMovie(aMovie);
							SetPort (whichWindow);
							PaintRect (&whichWindow->portRect);
							EndUpdate (whichWindow);
						}
						break;
					case keyDown:
					case autoKey:
					case mouseDown:
						done = TRUE;
						break;
				}
			}
			MoviesTask (aMovie, 0);
			
			tv = GetMovieTime(aMovie, NULL);
//tv = TrackTimeToMediaTime(tv, vidTrack);
			sprintf(buff, "Movie Time: %d", tv);
			PaintRect (&r);
			MoveTo(1, -6);
			RGBForeColor(&white);
			DrawText(buff, 0, strlen(buff));
			RGBForeColor(&black);

#ifdef PAL_CHG_TEST
			if (tv > 3100 && !chgPal)
			{
				SetEntries(1, 253, &(**ctab[2]).ctTable[1]);
				chgPal = TRUE;
			}
#endif
		}
		DisposeCallBack(qtb);
		DisposeMovie (aMovie);
		PaintRect(&gMainWindow->portRect);
		ShowCursor();
	}

	ExitMovies();
}

//------------------------------------------------------------------------
pascal void TestCallbackProc(QTCallBack cb, long refCon)
{
	char			buff[256];
	RGBColor	black = {0, 0, 0};
	RGBColor	purple = {0xFFFF, 0x3333, 0x3333};
	TimeBase	cbtb;
	TimeValue	tv;
	
	cbtb = GetCallBackTimeBase(cb);
	tv = GetTimeBaseTime(cbtb, 600, NULL);

	sprintf(buff, "It called me!!   time:%d, refCon:%d", tv, refCon);

	MoveTo(1, 12);
	RGBForeColor(&purple);
	DrawText(buff, 0, strlen(buff));
	RGBForeColor(&black);
}


//=======================================================================
//  PLAY CUT SCENE TEST
//=======================================================================
void DoPlayCutScene(short cmd)
{
	FSSpec	fSpec;

	switch (cmd)
	{
		case testPlayIntro:
			FSMakeFSSpec(gDataVref, gDataDirID, "\pIntro", &fSpec);
			PlayCutScene(&fSpec, FALSE, TRUE);
			break;
		case testPlayDeath:
			FSMakeFSSpec(gDataVref, gDataDirID, "\pDeath", &fSpec);
			PlayCutScene(&fSpec, TRUE, TRUE);
			break;
		case testPlayEndGame:
			FSMakeFSSpec(gDataVref, gDataDirID, "\pEndgame", &fSpec);
			PlayCutScene(&fSpec, TRUE, FALSE);
			break;
		
		//------ VMAIL ------
		case testPlayCitadelVM:
			FSMakeFSSpec(gDataVref, gDataDirID, "\pCitadel Status", &fSpec);
			PaintRect(&gMainWindow->portRect);
			PlayVMail(&fSpec, 100, 100);
			break;

		case testPlayDetachVM	:
			FSMakeFSSpec(gDataVref, gDataDirID, "\pDetach", &fSpec);
			PaintRect(&gMainWindow->portRect);
			PlayVMail(&fSpec, 100, 100);
			break;
		
		case testPlayJettisonVM:
			FSMakeFSSpec(gDataVref, gDataDirID, "\pJettison Pod", &fSpec);
			PaintRect(&gMainWindow->portRect);
			PlayVMail(&fSpec, 100, 100);
			break;
		
		case testPlayLaserMalVM:
			FSMakeFSSpec(gDataVref, gDataDirID, "\pLaser Malfunction", &fSpec);
			PaintRect(&gMainWindow->portRect);
			PlayVMail(&fSpec, 100, 100);
			break;
		
		case testPlayShieldsVM:
			FSMakeFSSpec(gDataVref, gDataDirID, "\pShields On", &fSpec);
			PaintRect(&gMainWindow->portRect);
			PlayVMail(&fSpec, 100, 100);
			break;
		
		case testPlayAutoDesVM:
			FSMakeFSSpec(gDataVref, gDataDirID, "\pAuto-destruct", &fSpec);
			PaintRect(&gMainWindow->portRect);
			PlayVMail(&fSpec, 100, 100);
			break;		
	}
}


//=======================================================================
//  PLAY AUDIOLOG AND BARK TEST
//=======================================================================
void DoPlayAudioLog(short cmd)
{
	switch (cmd)
	{
		case testPlayBark1:
			audiolog_bark_play(0x01);
			break;
		case testPlayBark2:
			audiolog_bark_play(0x66);
			break;
		case testPlayAlog1:
			audiolog_play(0x0B);
			break;
		case testPlayAlog2:
			audiolog_play(0xA0);
			break;
	}
	while (audiolog_playing(-1))
		audiolog_loop_callback();
}


//=======================================================================
//  LOAD AND DISPLAY A LEVEL MAP.
//=======================================================================
errtype load_da_palette();

void DoLoadLevelMap(short cmd)
{
	FSSpec	fSpec;

	FSMakeFSSpec(gDataVref, gDataDirID, "\parchive.data", &fSpec);
//	object_data_load();
	
	switch (cmd)
	{
		case testLoadLevelR:
			load_current_map(4002, &fSpec);
			break;
		case testLoadLevel1:
			load_current_map(4102, &fSpec);
			break;
		case testLoadLevel2:
			load_current_map(4202, &fSpec);
			break;
		case testLoadLevel3:
			load_current_map(4302, &fSpec);
			break;
	}
	
	load_da_palette();
	amap_draw(oAMap(MFD_FULLSCR_MAP), TRUE);
}

//------------------------------------------------------------------------
void DoZoomCurrMap(short cmd)
{
	if (cmd == testZoomIn)
		level_gamedata.auto_maps[MFD_FULLSCR_MAP].zoom++;
	else
		level_gamedata.auto_maps[MFD_FULLSCR_MAP].zoom--;

	amap_draw(oAMap(MFD_FULLSCR_MAP), TRUE);
}

//------------------------------------------------------------------------
#define height_step fix_make(0,0x010000>>SLOPE_SHIFT)
#include "game_screen.h"
#if __profile__
#include "Profiler.h"
#endif

extern Boolean DoubleSize;
extern Boolean SkipLines;

void RenderTest(void)
 {
 	extern fauxrend_context 		*_sr;           /* current and default fauxrend contexts */

//   	fix 						eye[6] = {fix_make(0x18,0xc400),fix_make(0x16,0xb400),fix_make(1,0x0000),0,0,0};
   	fix 						eye[6] = {0x1e8a00,0x168900,0x22000,0xC000,0,0};
	FSSpec					fSpec;
	cams 						test_cam;
   	fauxrend_context 	*_frc;
	bool 						_rt_inside=TRUE;
	bool 						all_axis = FALSE;
	long						time,detailCheck;
	Str255					str;
	long						frames = 0L;
	fix						sinEye,cosEye;
	fix						moveAmt;
	Boolean					profileOn = false;
	extern void game_redrop_rad(int);
	int						size_left, size_top, size_wide, size_high;
	PicHandle				pic;
	Rect						r;
	Boolean				    	showFrames = false;
	extern 	void 			change_svga_screen_mode();
	extern 	frc 			*svga_render_context;
	extern void _fr_change_detail(int det);
	extern char _g3d_enable_blend;
	
	object_data_load();
	HideCursor();
	
	// start in slot view, full res
	size_left = SCREEN_VIEW_X<<1;
	size_top = SCREEN_VIEW_Y*2.4;
	size_wide = SCREEN_VIEW_WIDTH<<1;
	size_high = SCREEN_VIEW_HEIGHT*2.4;
	DoubleSize = false;

	change_svga_screen_mode();
	_frc = (fauxrend_context *) svga_render_context;
																																				// 	FR_DOUBLEB_MASK
	_frc = (fauxrend_context *) fr_place_view((frc *) FR_NEWVIEW, (void *) FR_DEFCAM,0L, 0|FR_WINDOWD_MASK|FR_CURVIEW_STRT, 0, 0, size_left, size_top, size_wide, size_high);
	_frc->detail = 2;
	
	FSMakeFSSpec(gDataVref, gDataDirID, "\parchive.data", &fSpec);
	load_current_map(4102, &fSpec);
	load_da_palette();
	gr_clear(0xff);
 	
 	pic = GetPicture(19010);
 	if (pic)
 	  {
	 	r = (*pic)->picFrame;
	 	OffsetRect(&r,-r.left, -r.top);
	 	OffsetRect(&r, 128,340);
	 	DrawPicture(pic,&r);
	 	ReleaseResource((Handle) pic);
 	  }
 	  
	_frc->camptr=NULL;
	fr_camera_create(&test_cam,CAMTYPE_ABS,eye,NULL);
	fr_camera_setdef(&test_cam);

	detailCheck = time = QuickTickCount();
 	while (!Button())
 	  {		
#if __profile__
 	  	if (kb_state(0x23) && !profileOn)	// P
 	  	 	{ProfilerInit(collectDetailed, bestTimeBase, 300, 30); profileOn = true;}
 	  	if (kb_state(0x1f) && profileOn) // O
 	  	  {
			ProfilerDump("\pShock prof");
			ProfilerTerm();
			profileOn = false;
 	  	  }
#endif
 	  	 
 	  	if (kb_state(0x38))
 	  		moveAmt = 0x1000;
 	  	else
 	  		moveAmt = 0x2400;
 	  		
		// forward
		if (kb_state(0x5B) || kb_state(0x22))
			 {	
				fix_sincos(eye[3],&sinEye,&cosEye);
			   	eye[0]+=fix_mul(sinEye,moveAmt);
				eye[1]+=fix_mul(cosEye,moveAmt);
			 }
			 
		// back
		if (kb_state(0x54) || kb_state(0x28))
			 {
				fix_sincos(eye[3],&sinEye,&cosEye);
			   	eye[0]-=fix_mul(sinEye,moveAmt);
				eye[1]-=fix_mul(cosEye,moveAmt);
			 }

		// turn left, right
		if (kb_state(0x56) || kb_state(0x26))
			eye[EYE_H]-=(0x1400>>1);
		if (kb_state(0x58) || kb_state(0x25))
			eye[EYE_H]+=(0x1400>>1);
		
		// look up, down
		if (kb_state(0x57) || kb_state(0x17))
			eye[EYE_P] = 0;
		if (kb_state(0x7E))
			eye[EYE_P]+=(moveAmt>>1);
		if (kb_state(0x7D))
			eye[EYE_P]-=(moveAmt>>1);
		
		// tilt head
		if (kb_state(0x59))
			eye[EYE_B]-=(moveAmt>>1);
		if (kb_state(0x53))
			eye[EYE_B]+=(moveAmt>>1);
				
		// move up, down
		if (kb_state(0x55) || kb_state(0x29))
			eye[2]-=moveAmt;
		if (kb_state(0x5c) || kb_state(0x23))
			eye[2]+=moveAmt;

		// blending
		if ((QuickTickCount()-detailCheck>20L) && kb_state(0x0B)) {_g3d_enable_blend = !_g3d_enable_blend; detailCheck = QuickTickCount();}
		
		// debugger
		if (kb_state(0x32)) Debugger();
		
		// double size
		if (kb_state(0x12))
		 {
		 	DoubleSize = !DoubleSize;
		 	
 			change_svga_screen_mode();
			_frc = (fauxrend_context *) svga_render_context;
			gr_clear(0xff);
		 }
		
		// skip lines
		if (kb_state(0x13))
		  {
		 	SkipLines = !SkipLines;
	         	gr_clear(0xff);
		  }
		  		  
		 // slot/full
		if (kb_state(1))	// slot
		  {
			full_game_3d = false;
			change_svga_screen_mode();
			_frc = (fauxrend_context *) svga_render_context;
			gr_clear(0xff);
		  
		 	pic = GetPicture(19010);
		 	if (pic)
		 	  {
			 	r = (*pic)->picFrame;
			 	OffsetRect(&r,-r.left, -r.top);
			 	OffsetRect(&r, 128,340);
			 	DrawPicture(pic,&r);
			 	ReleaseResource((Handle) pic);
		 	  }
		  }

		if (kb_state(3)) // full
		  {
			full_game_3d = true;
			change_svga_screen_mode();
			_frc = (fauxrend_context *) svga_render_context;
			gr_clear(0xff);
		  }

		if (kb_state(6)) // frame rate
		 {
			showFrames = 1-showFrames;
			 if (showFrames)
			 	gr_set_fcolor(0);
			 else
			 	gr_set_fcolor(0xff);
			 gr_urect(0,410,48,420);
			time = QuickTickCount();
			frames = 0L;
		 }
		 
		 
		// detail
		if ((QuickTickCount()-detailCheck>20L) && kb_state(2))
			{detailCheck = QuickTickCount(); _frc->detail++; if (_frc->detail>=4) _frc->detail = 0;}
		
		if (frames & 1)
			palette_advance_all_fx(QuickTickCount());		
		fr_camera_update(&test_cam,eye,CAM_UPDATE_NONE,NULL);
		fr_rend(_frc);
		
		frames++;
		if (showFrames && (QuickTickCount()-time>=60))
		 {
		 	gr_set_fcolor(0);
		 	gr_urect(0,410,48,420);
		 	numtostring(frames,(char *) str);
		 	strcat((char *) str,", 0");
		 	str[strlen((char *) str)-1] += _frc->detail;
		 	MoveTo(2,418);
		 	drawstring((char *) str);
		 	
		 	frames = 0;
		 	time = QuickTickCount();
		 }
	 }

 {
	CTabHandle		ctab;

	gr_clear(0xff);
	ctab = GetCTable(9003);														// Get the title screen CLUT
	if (ctab)	
	 {
		BlockMove(*ctab, *gMainColorHand, 8 + (sizeof(ColorSpec) * 256));
		SetEntries(1, 253, &(**(gMainColorHand)).ctTable[1]);
		ResetCTSeed();
		ReleaseResource((Handle)ctab);
	}
}
	DrawMenuBar();
	InvalRect(&gMainWindow->portRect); 
	ShowCursor();
 }