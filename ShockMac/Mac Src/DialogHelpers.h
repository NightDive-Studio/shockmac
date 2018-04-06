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
//		DialogHelpers.h	-	Utility routines and user items for dialog boxes.
//
//====================================================================================


//--------------------
//  Constants
//--------------------
#define kBtnOK				1
#define kBtnCancel			2
#define	kUsrOKOutline	3

#define kSliderWidth		112
#define kSliderHeight		12
#define kSliderEnd			6
#define	kSliderBackPict			8990
#define	kThumbIcon				8991
#define	kSliderBackPictDim	8992
#define	kThumbIconDim			8993

typedef pascal void (*SliderCallbackProcPtr)(short value);

//--------------------
//  Globals
//--------------------
extern Boolean		gGrayOK;
extern Boolean		gIgnoreGray;
extern Str255		gDimTitle;
extern Boolean		gDimmed;
extern MenuHandle	gPopupMenuHdl;
extern short			gPopupSel;
extern short			gSliderLastPos;
extern Boolean		gSliderDimmed;

extern short			gNewGameSel[4];

//--------------------
//  Prototypes
//--------------------
void FlashButton(WindowPtr dlog, short itemN);
pascal void OKButtonUser(WindowPtr dlog, short itemN);

pascal Boolean ShockFilterProc(DialogPtr dlog, EventRecord *evt, short *itemHit);
pascal Boolean ShockAlertFilterProc(DialogPtr dlog, EventRecord *evt, short *itemHit);

pascal void DimStaticUser(WindowPtr dlog, short itemN);

pascal void PopupMenuUser(WindowPtr dlog, short itemN);
pascal void PopupMenuUserNG(WindowPtr dlog, short itemN);

pascal void GroupBoxUser(WindowPtr dlog, short itemNum);

void SetupSlider(void);
void SetSliderBitmaps(void);
pascal void DrawSlider(WindowPtr dlog, short itemNum);
short DoSliderTracking(WindowPtr dlog, short itemNum, SliderCallbackProcPtr cb);
void FreeSlider(void);
