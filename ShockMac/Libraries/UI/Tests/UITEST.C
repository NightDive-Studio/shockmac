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
/*
 * $Source: n:/project/lib/src/ui/RCS/uitest.c $
 * $Revision: 1.10 $
 * $Author: jak $
 * $Date: 1994/03/02 18:41:12 $
 */

#include <stdio.h>
#include <string.h>
#include <gadgets.h>
#include <slider.h>
#include <pushbutt.h>
#include <butarray.h>
#include <qboxgadg.h>
#include <textgadg.h>
#include <menu.h>
#include <plain.h>
#include <lg.h>
#include <dbg.h>
//#include <mprintf.h>
#include <hotkey.h>
#include <kb.h>
#include <kbcook.h>
#include <mouse.h>
#include <event.h>
#include <cursors.h>
#include <slab.h>
#include <tng.h>
#include <keydefs.h>
#include <gamescr.h>
#include <resgadg.h>
//#include <fault.h>

#include "InitMac.h"
#include "ShockBitmap.h"

WindowPtr	gMainWindow;

int done;
int current;

#define TEST_NUM 3

Gadget *p[TEST_NUM], *mn, *menu_gadg, *sub_menu;
/*
errtype move_it (int, int, int);

Gadget *qbox2 = NULL;
short test2_short = 40,test2_short2 = 0xBAD2;
byte test2_byte = 22;
int test2_int = 9;
bool test2_bool = TRUE;
*/

#pragma require_prototypes off
/*
bool test_panel_close(void *vg, void *ud)
{
#ifndef NO_DUMMIES
   void *dummy;
   dummy = vg;
   dummy = ud;
#endif
   gadget_destroy(&qbox2);
   return(FALSE);
}

bool qbox_test(short keycode, ulong context, void *data)
{
   Point testpoint, ss;
   testpoint.x = 60;
   testpoint.y = 80;
   ss.x = 180;
   ss.y = 10;

   if (qbox2 == NULL)
   {
      qbox2 = gad_qbox_start(mn, testpoint, 11, NULL, QB_ADDCLOSE|QB_ALIGNMENT, "testqbox", ss);
      gad_qbox_add("Blecho", QB_SHORT_SLOT, &test2_short, QB_RD_ONLY);
      gad_qbox_add("Will crash?", QB_BOOL_SLOT, &test2_bool, QB_ARROWS);
      gad_qbox_add("Hextest", QB_SHORT_SLOT, &test2_short2, QB_HEX);
      gad_qbox_add("Bintest", QB_BYTE_SLOT, &test2_byte, QB_BINARY);
      gad_qbox_add("Octtest", QB_INT_SLOT, &test2_int, QB_OCTAL);
      gad_qbox_end_full(&qbox2);
   }
}
*/
bool menu_test1(short keycode, ulong context, void *data)
{
//   mprintf("Hey, test 1: %s\n",(char *)data);
   return(TRUE);
}

bool menu_test2(short keycode, ulong context, void *data)
{
//   mprintf("THIS IS TEST TWO!! BAHAHAHAHAHAH!\n");
   return(TRUE);
}

bool view_gadget_draw(void *vg, void *ud)
{
   LGRect r;
   Gadget *g = (Gadget *)vg;

   region_abs_rect(g->rep,g->rep->r,&r);   
   gr_set_fcolor(100);
   gr_rect(r.ul.x, r.ul.y, r.lr.x - 1, r.lr.y - 1);
   return(FALSE);
}


bool test_callback(void *uid_g, void *ud)
{
   Gadget *g = (Gadget *)uid_g;
   void *dummy = ud;
   int val = tng_slider_getvalue(g->tng_data);

//   mprintf("TEST CALLBACK from %s!!!!!!\n",GD_NAME(g->rep));
	{
		RGBColor	black = {0, 0, 0};
		RGBColor	white = {0xffff, 0xffff, 0xffff};
		char		buff[100];
		Rect		r;

		sprintf(buff, "Slider value: %d", val);

		SetRect(&r, 0, -35, 300, -15);
		PaintRect(&r);
		
		TextFont(geneva);										// Draw some instructions.
		TextSize(9);
		TextMode(srcOr);
	
		MoveTo(1, -20);
		RGBForeColor(&white);
		DrawText(buff, 0, strlen(buff));
		RGBForeColor(&black);
	}

   return(TRUE);
}


bool barry_callback(void *uid_g, void *ud)
{
   Gadget *g;
   int i,j;
   void *dummy;
   dummy = ud;
   g = (Gadget *)uid_g;
   for (i=0; i<TNG_BA_MSIZE(g->tng_data).x; i++)
   {
      for (j=0; j<TNG_BA_MSIZE(g->tng_data).y; j++)
      {
         if (TNG_BA_SELECTED(g->tng_data,i,j))
         {
			RGBColor	black = {0, 0, 0};
			RGBColor	white = {0xffff, 0xffff, 0xffff};
			char		buff[100];
			Rect		r;
	
            sprintf(buff, "Button pressed at (%d, %d)!",i,j);
	
			SetRect(&r, 0, -35, 300, -15);
			PaintRect(&r);
			
			TextFont(geneva);										// Draw some instructions.
			TextSize(9);
			TextMode(srcOr);
		
			MoveTo(1, -20);
			RGBForeColor(&white);
			DrawText(buff, 0, strlen(buff));
			RGBForeColor(&black);
//            if ((i==0) && (j  ==0))
//               gad_menu_popup_at_mouse(menu_gadg);
//            TNG_BA_SELECTED(g->tng_data,i,j) = FALSE;
         }
      }
   }
   return(FALSE);
}

int test_pb_CB(Gadget *g, void *ud)
{
   void *dummy;
   Gadget *dummy2;
   dummy = ud;
   dummy2 = g;
	{
		RGBColor	black = {0, 0, 0};
		RGBColor	white = {0xffff, 0xffff, 0xffff};
		char		buff[100];
		Rect		r;

   		sprintf(buff, "Quickbox pushbutton, yo! magic word is '%s'.",(char *)ud);

		SetRect(&r, 0, -35, 300, -15);
		PaintRect(&r);
		
		TextFont(geneva);										// Draw some instructions.
		TextSize(9);
		TextMode(srcOr);
	
		MoveTo(1, -20);
		RGBForeColor(&white);
		DrawText(buff, 0, strlen(buff));
		RGBForeColor(&black);
	}
   return(0);
}

bool quit_callback(void *uid_g, void *ud)
{
   void *dummy;
   Gadget *dummy2;
   dummy = ud;
   dummy2 = (Gadget *)uid_g;
   done = 1;
   return(TRUE);
}

/*
int cbcount = 0;

void goofy_mouse_callback(mouse_event* e, void* data)
{
   cbcount++;
   gr_ubitmap((grs_bitmap*)data,e->x,e->y);
}
*/

LGCursor globcursor;

void gadtest_init_mouse(LGRegion* r, uiSlab* slab)
{
   FrameDesc* f = (FrameDesc *)RefLock(REF_IMG_bmTargetCursor);
   LGPoint spot;
   f->bm.bits = (uchar *)(f+1);
   spot.x = f->bm.w/2;
   spot.y = f->bm.h/2;
   uiMakeBitmapCursor(&globcursor,&f->bm,spot);
   uiMakeSlab(slab,r,&globcursor);
   uiInit(slab);
//   mouse_constrain_xy(0,0,grd_cap->w,grd_cap->h);
//   mouse_put_xy(100,100);
}


bool goofy_key_callback(uiEvent* h, LGRegion* r, void* state)
{
   void *dummy;
   LGRegion *dummy2;
   int rv;
   short inkey = h->subtype;
   dummy2 = r;
   dummy = state;
//   if (h->type != UI_EVENT_KBD_COOKED || (inkey & KB_FLAG_SPECIAL) || !(inkey & KB_FLAG_DOWN)) return FALSE;
   if (h->type != UI_EVENT_KBD_COOKED)
   {
     DebugStr("\pKey's not cooked!");
     return FALSE;
   }
   if (inkey & KB_FLAG_SPECIAL)
   {
     DebugStr("\pIsn't that special!");
     return FALSE;
   }
   if (!(inkey & KB_FLAG_DOWN))
   {
     DebugStr("\pKey's not down!");
     return FALSE;
   }

   switch(inkey & 0xFF)
   {
   /*
      case 'i': move_it(0, -5, 0); break;
      case 'k': move_it(0, +5, 0); break;
      case 'j': move_it(-5, 0, 0); break;
      case 'l': move_it(+5, 0, 0); break;
      case '>': move_it(0, 0, +1); break;
      case '<': move_it(0, 0, -1); break;
      case '+':
      case '=': current++;
         if (current == TEST_NUM) current = 0;
         mprintf("current = %d! %s, (%d, %d)(%d, %d), %d\n",current, GD_NAME(p[current]->rep),
            RECT_EXPAND_ARGS(p[current]->rep->r),p[current]->rep->z);
         break;
      case '-': current--;
         if (current <0) current = TEST_NUM - 1;
         mprintf("current = %d! %s, (%d, %d)(%d, %d), %d\n",current, GD_NAME(p[current]->rep),
            RECT_EXPAND_ARGS(p[current]->rep->r),p[current]->rep->z);
         break;
      */
      case 'q': done = 1; break;
      /*
      case 'z':
         gadget_display(mn,NULL);
         break;
      case 'o':
         rv = region_obscured(p[current]->rep, p[current]->rep->r);
         mprintf("obscuration factor = %d\n",rv);
         break;
      case 'm':
         gad_menu_popup_at_mouse(menu_gadg);
         break;
      */
   }
   hotkey_dispatch(h->subtype);
   return TRUE;
}

char *test_strings[4] = { "Alpha", "Beta", "Gamma", "Delta" };

void main(void)
{
   Gadget *quit;
   LGRect r2, rq, r3, r1, r4, r5, r6;
   LGPoint ext,menu_pt,sub_menu_pt;
   Ref qid, id2;
   uiSlab slab;
   Ref ids[18];
   char *list[10];
   int cid, callid, idnum;
   int i;

   int test_int, test_int2, test_int3, test_int4;
   short test_short1, hack_short;
   char *test_text, *test_text2;
   byte zco = 0;
   bool test_bool;
   LGPoint ss, spac, bord;
   int min, max;

extern short gMainVRef;
	OSErr	err;
	long	dirID, temp;
	short	vref;
	FSSpec	fSpec;
	char	macfname[64];

//   mprintf ("test this sucker\n");

	// Mac setup
	InitMac();
	CheckConfig();

	SetupWindows(&gMainWindow);
	SetupOffscreenBitmaps();

	// Set up an FSSpec for opening resource files.
 	GetWDInfo(gMainVRef, &vref, &dirID, &temp);
	
   // Open resource stuff.
   // Note that we need to call ResOpenFile on all the resource
   // files we want to use -- pretty straightforward.
	ResInit();

	strcpy(macfname, (char *)"\pMac resgadg.RES");
	FSMakeFSSpec(vref, dirID, (ConstStr255Param)macfname, &fSpec); // Make the file's spec.
	ResOpenFile(&fSpec);
	
	strcpy(macfname, (char *)"\pMac gamescr.RES");
	FSMakeFSSpec(vref, dirID, (ConstStr255Param)macfname, &fSpec); // Make the file's spec.
	ResOpenFile(&fSpec);
	
	strcpy(macfname, (char *)"\pMac resgui.RES");
	FSMakeFSSpec(vref, dirID, (ConstStr255Param)macfname, &fSpec); // Make the file's spec.
	ResOpenFile(&fSpec);

//   mprintf("Geez, before anything...\n");

	hotkey_init(10);

   // Create the basic gadget.  In theory, for other screen modes, one would
   // simply specify a different display type.  However, as of the writing of
   // this comment, those aren't really implemented.
   ext.x = 640; ext.y = 480;
   uiDoubleClickTime = 0;
   region_begin_sequence();
   mn = gadget_init(DISPLAY_MAC, ext);

   HideCursor();							// Hide Mac mouse pointer
   gadtest_init_mouse(mn->rep,&slab);

   uiInstallRegionHandler(mn->rep,UI_EVENT_KBD_COOKED,goofy_key_callback,NULL,&callid);
   uiGrabFocus(mn->rep, ALL_EVENTS);



//   mprintf("before quit button creation\n");

   HotkeyContext = 0xFFFF;

   rq.ul.x = 300; rq.ul.y = 0;
   rq.lr.x = 328; rq.lr.y = 42;
   id2 = REF_IMG_bmFriendPBA;
   quit = gad_pushbutton_create(mn, &rq, 0, RESOURCE_TYPE, &id2, NULL, "quitbutton");
   gad_callback_install(quit, TNG_EVENT_SIGNAL, TNG_SIGNAL_SELECT, &quit_callback, NULL, &cid);
//   mprintf("after quit button...\n");
//   gadget_display(quit, NULL);

/*   r2.ul.x = 330; r2.ul.y = 70;
   r2.lr.x = 350; r2.lr.y = 180;
   p[0] = gad_slider_create(mn, &r2, 5, SL_VERTICAL, 10, 0, 100, NULL, "slider");
   gad_callback_install(p[0], TNG_EVENT_SIGNAL, TNG_SIGNAL_CHANGED, &test_callback, NULL, &cid);

  r1.ul.x = 100; r1.ul.y = 170;
  r1.lr.x = 280; r1.lr.y = 290;
  p[2] = gad_text_create(mn, &r1, 4, TNG_TG_SCROLLBARS, NULL, "text_guy");
//  tt_parse_string(TNG_TG_TT(p[2]->tng_data), "Hey Mr. DJ.\n");
/*
   sub_menu_pt.x = 380;
   sub_menu_pt.y = 80;
   sub_menu = gad_menu_create(mn, &sub_menu_pt, 16, NULL, 60, "sub_menu");
   gad_menu_add_line(sub_menu, "Cindy", &menu_test1, 'd'|KB_FLAG_ALT|KB_FLAG_DOWN, 0xFFFF, "Cindy!", "help me!");
   gad_menu_add_line(sub_menu, "Marsha", &menu_test1, 'e'|KB_FLAG_ALT|KB_FLAG_DOWN, 0xFFFF, "Marsha!", "help me!");
   gad_menu_add_line(sub_menu, "Jan", &menu_test1, 'f'|KB_FLAG_ALT|KB_FLAG_DOWN, 0xFFFF, "Jan!", "help me!");
   gad_menu_add_line(sub_menu, "Cindy", &menu_test1, 'd'|KB_FLAG_ALT|KB_FLAG_DOWN, 0xFFFF, "Cindy!", "help me!");
   gad_menu_add_line(sub_menu, "Marsha", &menu_test1, 'e'|KB_FLAG_ALT|KB_FLAG_DOWN, 0xFFFF, "Marsha!", "help me!");
   gad_menu_add_line(sub_menu, "Jan", &menu_test1, 'f'|KB_FLAG_ALT|KB_FLAG_DOWN, 0xFFFF, "Jan!", "help me!");
   gad_menu_add_line(sub_menu, "Cindy", &menu_test1, 'd'|KB_FLAG_ALT|KB_FLAG_DOWN, 0xFFFF, "Cindy!", "help me!");
   gad_menu_add_line(sub_menu, "Marsha", &menu_test1, 'e'|KB_FLAG_ALT|KB_FLAG_DOWN, 0xFFFF, "Marsha!", "help me!");
   gad_menu_add_line(sub_menu, "Jan", &menu_test1, 'f'|KB_FLAG_ALT|KB_FLAG_DOWN, 0xFFFF, "Jan!", "help me!");
   gad_menu_add_line(sub_menu, "Cindy", &menu_test1, 'd'|KB_FLAG_ALT|KB_FLAG_DOWN, 0xFFFF, "Cindy!", "help me!");
   gad_menu_add_line(sub_menu, "Cindy", &menu_test1, 'd'|KB_FLAG_ALT|KB_FLAG_DOWN, 0xFFFF, "Cindy!", "help me!");
   gad_menu_add_line(sub_menu, "Marsha", &menu_test1, 'e'|KB_FLAG_ALT|KB_FLAG_DOWN, 0xFFFF, "Marsha!", "help me!");
   gad_menu_add_line(sub_menu, "Jan", &menu_test1, 'f'|KB_FLAG_ALT|KB_FLAG_DOWN, 0xFFFF, "Jan!", "help me!");

   menu_pt.x = 340;
   menu_pt.y = 40;
   menu_gadg = gad_menu_create(mn, &menu_pt, 15, NULL, 60, "menu_gadg");
   gad_menu_add_line(menu_gadg, "Joe", &menu_test1, 'a'|KB_FLAG_CTRL|KB_FLAG_DOWN, 0xFFFF, "data guy", "Hey, this is some help text");
   gad_menu_add_line(menu_gadg, "Fred", &menu_test1, 'b'|KB_FLAG_CTRL|KB_FLAG_DOWN, 0xFFFF, "foobie foobie", "Talk about some help!");
//   gad_menu_add_line(menu_gadg, "Zany", &menu_test2, KEY_F12|KB_FLAG_DOWN, 0xFFFF, NULL, "Wacko boy writes a help string");
   gad_menu_add_line(menu_gadg, "Zany", &menu_test2, 0, 0xFFFF, NULL, "Wacko boy writes a help string");
   gad_menu_add_line(menu_gadg, "Zany", &menu_test2, 0, 0xFFFF, NULL, "Wacko boy writes a help string");
   gad_menu_add_line(menu_gadg, "Zany", &menu_test2, 'D'|KB_FLAG_CTRL|KB_FLAG_DOWN, 0xFFFF, NULL, "Wacko boy writes a help string");
//   gad_menu_add_line(menu_gadg, "Zany", &qbox_test, 'n'|KB_FLAG_CTRL|KB_FLAG_DOWN, 0xFFFF, NULL, "Wacko boy writes a help string");
   gad_menu_add_submenu(menu_gadg, "Hair of Gold", sub_menu);
¥/


//   mprintf("before buttonarray creation\n");
   r3.ul.x = 10; r3.ul.y = 10;
   p[0] = gad_buttonarray_create(mn, r3.ul, 0,
      5, 5, 
      4, 4,  
      9, 9, 1, TNG_BA_OUTLINE_MODE | TNG_BA_TIGHTPACK, NULL, "buttonarray1");
   qid = REF_IMG_bmTargetCursor;
   for (i=0; i<20; i++)
   {
      gad_buttonarray_addbutton(p[0], RESOURCE_TYPE, &qid);
   }
   gadget_display(p[0],NULL);
   gad_callback_install(p[0], TNG_EVENT_SIGNAL, TNG_SIGNAL_SELECT, &barry_callback, NULL, &cid);

   r4.ul.x = 10; r4.ul.y = 90;
   p[2] = gad_buttonarray_create(mn, r4.ul, 0,
      3, 4, 
      3, 3,  
      100, 20, 1, NULL, NULL, "buttonarray2");
   p[2]->tng_data->style->font = RES_citadelFont;
//   p[2]->tng_data->style->font = RES_largeTechFont;
   p[2]->tng_data->style->textColor = 1;
   for (i=0; i<10; i++)
      list[i] = (char *)NewPtr(10 * sizeof(char));
   sprintf(list[0], "Alpha");
   sprintf(list[1], "Beta");
   sprintf(list[2], "Gamma");
   sprintf(list[3], "Delta");
   sprintf(list[4], "Psi");
   sprintf(list[5], "Alpha");
   sprintf(list[6], "Beta");
   sprintf(list[7], "Gamma");
   sprintf(list[8], "Delta");
   sprintf(list[9], "Psi");
   for (i=0; i<10; i++)
      gad_buttonarray_addbutton(p[2], TEXT_TYPE, list[i]);
//   gadget_display(p[2],NULL);


//   mprintf("Before Quickbox creation\n");
   r5.ul.x = 370; r5.ul.y = 250;
   test_short1 = 341;
   test_int = 22;
   test_int2 = 1;
   test_int3 = 1;
   test_int4 = 1;
   hack_short = 1;
   test_bool = TRUE;
   max = 100; min = 1;
   ss.x = 220; ss.y = 20;
   spac.x = 0; spac.y = 0;
   bord.x = 1; bord.y = 1;
   test_text2 = (char *)NewPtr(20 * sizeof(char));
   sprintf(test_text2, "Mr. Lifto rules!");
   p[1] = gad_qbox_start_full(mn, r5.ul, 10, NULL, QB_ALIGNMENT, "quickbox", ss, spac, bord, NULL, NULL);
   gad_qbox_add("Foobar", QB_SHORT_SLOT, &test_short1, QB_RD_ONLY);
   gad_qbox_add_parm("Zappita", QB_INT_SLOT, &test_int4, QB_ARROWS|QB_BOUNDED, (void *)0, (void *)10);
   gad_qbox_add("Baz Baz", QB_BOOL_SLOT, &test_bool, QB_ARROWS);
   gad_qbox_add_parm("Z-coord", QB_BYTE_SLOT, &zco, QB_CYCLE|QB_ARROWS, (void *)0, (void *)10);
   gad_qbox_add_parm("Hack", QB_SHORT_SLOT, &hack_short, QB_CYCLE|QB_ARROWS, (void *)0, (void *)10);
//   gad_qbox_add_parm("StringMe", QB_BYTE_SLOT, &zco, QB_STRINGSET, (void *)test_strings, (void *)4);
   gad_qbox_add_parm("Quux", QB_INT_SLOT, &test_int2, QB_SLIDER, (void *)min, (void *)max);
   gad_qbox_add("This is a really a lot longer title", QB_TEXT_SLOT, NULL, QB_RD_ONLY);
   gad_qbox_add_parm("iggy iggy", QB_INT_SLOT, &test_int3, QB_BOUNDED|QB_ARROWS,(void *)10,(void *)40);
   gad_qbox_add_parm("Push Me", QB_PUSHBUTTON_SLOT, &test_pb_CB, QB_NO_OPTION, "foofie!", &qid);
   gad_qbox_end();

   r6.ul.x = 150; r6.ul.y = 40;
   r6.lr.x = 300; r6.lr.y = 80;
   p[2] = gad_plain_create(mn, &r6, 5, "plain_guy");
   gad_callback_install(p[2], TNG_EVENT_SIGNAL, TNG_SIGNAL_EXPOSE, &view_gadget_draw, NULL, &cid);
   gadget_display(p[2], NULL);
*/
//   mprintf("Before region end\n");
   region_end_sequence(TRUE);

//   gadget_display(mn,NULL);

   current = 0;
   done = 0;
//   mprintf("Before region handler installation\n");

//   mprintf("Before poll loop\n");
   while(!done)
   {
      uiPoll();
   }

   kb_shutdown();
   mouse_shutdown();
   gadget_destroy(&mn);
   gr_close();
   hotkey_shutdown();
   ResTerm();
   ShowCursor();
   CleanupAndExit();
}

/*
errtype move_it(int dx, int dy, int dz)
{
   Point pt;
   int nz;

   pt = p[current]->rep->r->ul;
   pt.x += dx;
   pt.y += dy;

   nz = p[current]->rep->z + dz;
   gadget_move(p[current], pt, nz);
   mprintf("MOVE_IT! %s, (%d, %d, %d)\n",GD_NAME(p[current]->rep),pt.x, pt.y, nz);
   return (OK);
}
*/
