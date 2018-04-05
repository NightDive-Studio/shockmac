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
//=====================================================================
//
//		System Shock - ©1994-1995 Looking Glass Technologies, Inc.
//
//		ShockHelp.c	-	Displays a help dialog with multiple topics.
//
//=====================================================================

#include <GestaltEqu.h>
#include "DialogHelpers.h"
#include "ShockHelp.h"

//-----------------
//  PROTOTYPES
//-----------------
Boolean GetHelpMgr(void);
pascal void DrawHelp(WindowPtr, int);

//-----------------
//  GLOBALS
//-----------------
int				gCurTopic = 1;
int				gPictLoaded = -1;
PicHandle		gHelpPictHdl;


//-----------------------------------------------------------------
// Add the help menu either under the Help menu or the the Apple menu
//-----------------------------------------------------------------
void AddHelpMenu(void)
 {
 	MenuHandle	mh;
	Str255			str;
	
	BlockMove("\pSystem Shock HelpÉ",str,32L);
		
	if (GetHelpMgr())													// If Help Mgr is available, stick Help
	{																			// menu item in the Help menu
		if (HMGetHelpMenuHandle(&mh) == noErr)
			if (mh)
				AppendMenu(mh, str);
	}
	{																			// Also put it in Apple menu after "About..."
		mh = GetMHandle(128);
		if (mh)
			InsMenuItem(mh, str, 1);
	}
 }

 
// ------------------------------------------------------------------
//  Useritem to draw a help pict
// ------------------------------------------------------------------
pascal void DrawHelp(WindowPtr dlog, int itemN)
{
	Rect		r,r2;
	short		itype;
	Handle	hand;
	
	GetDItem(dlog, itemN, &itype, &hand, &r);
	
	if (gCurTopic != gPictLoaded)
	{
		ReleaseResource((Handle)gHelpPictHdl);
		gPictLoaded = gCurTopic;
		gHelpPictHdl = GetPicture(2499+gCurTopic);
	}
	
	if (gHelpPictHdl == NULL)
	{
		MoveTo(r.left+15, r.top+30);
		DrawString("\pHelp topic not available");
	}
	else
	{
		r2 = (*gHelpPictHdl)->picFrame;
		OffsetRect(&r2,-r2.left,-r2.top);
		
		// align pict to upper left
		OffsetRect(&r2,r.left,r.top);
		DrawPicture(gHelpPictHdl,&r2); 
	}
}
 
 
//--------------------------------------------------------------------
//  Show the help dialog
//--------------------------------------------------------------------
void ShowShockHelp(void)
{
	DialogPtr			dlog;
	UserItemUPP 		btnOutlineProcPtr;
	UserItemUPP		popMenuProcPtr;
	UserItemUPP		picHelpProcPtr;
	short					itemhit,itype;
	GrafPtr				savePort;
	Handle				hand;
	Rect					r;
 	int					sel;
	Point					tempP;
	
	GetPort(&savePort);
	dlog = GetNewDialog(6000,0L,(WindowPtr) -1L);
	SetPort(dlog);
	
	btnOutlineProcPtr = NewUserItemProc(OKButtonUser);
 	GetDItem(dlog, 4, &itype, &hand, &r);
 	SetDItem(dlog, 4, itype, (Handle)btnOutlineProcPtr, &r);

	gPopupMenuHdl = GetMenu(6000);
	InsertMenu(gPopupMenuHdl, -1);
	gPopupSel = gCurTopic;
 
	popMenuProcPtr = NewUserItemProc(PopupMenuUser);
 	GetDItem(dlog, 5, &itype, &hand, &r);					
 	SetDItem(dlog, 5, itype, (Handle)popMenuProcPtr, &r);
 	
	picHelpProcPtr = NewUserItemProc(DrawHelp);
 	GetDItem(dlog, 7, &itype, &hand, &r);					
 	SetDItem(dlog, 7, itype, (Handle)picHelpProcPtr, &r);

 	gHelpPictHdl = GetPicture(2499+gCurTopic);
 	gPictLoaded = gCurTopic;
 	
 	// Handle the dialog events.
	do
	{
		ModalDialog(0L, &itemhit);
		if (itemhit == 5)										// If the popup was selected.
		{
			GetDItem(dlog, 5, &itype, &hand, &r);
			
			tempP.h = r.left+1;
			tempP.v = r.top+1;
			LocalToGlobal(&tempP);
			
			// check the current item
			CheckItem(gPopupMenuHdl,gCurTopic,true);
			sel = PopUpMenuSelect(gPopupMenuHdl,tempP.v,tempP.h,gCurTopic);
			CheckItem(gPopupMenuHdl,gCurTopic,false);
			if ((sel!=0) && (sel!=gCurTopic))
			{
				gCurTopic = sel;
				gPopupSel = gCurTopic;
				InvalRect(&r);
				
				// update to new pict
				GetDItem(dlog,7,&itype,&hand,&r);
				InvalRect(&r);
			}
		}
	}
	while (itemhit!=1);

	gPictLoaded = -1;
	ReleaseResource((Handle)gHelpPictHdl);
	
 	DeleteMenu(6000);
 	ReleaseResource((Handle)gPopupMenuHdl);
	
	DisposeRoutineDescriptor(picHelpProcPtr);
	DisposeRoutineDescriptor(popMenuProcPtr);
	DisposeRoutineDescriptor(btnOutlineProcPtr);

	SetPort(savePort);
	DisposDialog(dlog);
 }


//--------------------------------------------------------------------
//	  Returns TRUE if the Help Manager is installed.
//--------------------------------------------------------------------
Boolean GetHelpMgr(void)
 {
	long	result;
	
	if (!Gestalt(gestaltHelpMgrAttr, &result)) 
	 {
		if (result & 1L)
			return (TRUE);
	 }
	
	return(FALSE);
 }

/*
//--------------------------------------------------------------------
// find the longest menu item in this menu handle, and add an extra
// space to it if we are running under System 6
void PadMenu(MenuHandle menu)
 {
 	int			temp,i,num,width,largest;
 	char		str[64];
 	int			oldFont,oldSize;
 	Style		oldStyle;
 	
 	if (!HasSystem7)
 	 {	
 	 	oldFont = thePort->txFont;
 	 	oldSize = thePort->txSize;
 	 	oldStyle = thePort->txFace;
 	 	
 	 	TextFont(0);
 	 	TextSize(12);
 	 	TextFace(0);
 	 	
 	 	largest = 1;
 		GetItem(menu, 1, str);
 	 	width = StringWidth(str);

 		num = CountMItems(menu);
 		for (i=2; i<=num; i++)
 		 {
 		 	GetItem(menu, i, str);
 		 	temp = StringWidth(str);
 		 	if (temp>width)
 		 	 {
 		 	 	width = temp;
 		 	 	largest = i;
 		 	 }
 		 } 	
 		 
 		// add a space to the largest item
 		GetItem(menu, largest, str);
 		pstrcat(str,"\P ");
 		SetItem(menu,largest,str);

 	 	TextFont(oldFont);
 	 	TextSize(oldSize);
 	 	TextFace(oldStyle);
 	 }
 }

*/