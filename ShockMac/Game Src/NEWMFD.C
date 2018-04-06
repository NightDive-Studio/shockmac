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
#define __NEWMFD_SRC

// NEWMFD.C

/*
 * $Source: r:/prj/cit/src/RCS/newmfd.c $
 * $Revision: 1.153 $
 * $Author: xemu $
 * $Date: 1994/11/21 22:03:39 $
 *
 */


// Source code for controlling the multi-function displays (MFDs)
// All MFD infrastructure belongs here, all expose/handler callbacks
// belong in mfdfunc.c

#include <stdlib.h>
#include <string.h>

#include "screen.h"                                     // for the root region
#include "fullscrn.h"
#include "mfdint.h"
#include "mfdext.h"
#include "mfddims.h"
#include "input.h"
#include "player.h"
#include "tools.h"
#include "mainloop.h"
#include "gameloop.h"
#include "mfdart.h"                                // For blank MFD art
#include "gamescr.h"
#include "musicai.h"                               // for digital FX
#include "sfxlist.h"                               // same
#include "citres.h"
#include "weapons.h"
#include "colors.h"
#include "cit2d.h"
#include "gamestrn.h"
#include "popups.h"
#include "statics.h"
#include "gr2ss.h"
//#include <inp6d.h>
//#include <i6dvideo.h>

#include "cybstrng.h"


// -----------------------
// Player_Struct Accessors
// -----------------------

#define mfd_empty_func(mfd_num) player_struct.mfd_empty_funcs[(mfd_num)]
#define mfd_index(mfd_num)      player_struct.mfd_current_slots[(mfd_num)]
#define set_mfd_to_slot(mfd_id,vs,as) \
   player_struct.mfd_virtual_slots[(mfd_id)][(vs)] = (as)
#define set_default_to_func(mfd_num,fnum) \
   player_struct.mfd_empty_funcs[(mfd_num)] = (fnum)
#define mfd_get_active_func(mfd_id) \
   mfd_get_func((mfd_id),(mfd_index((mfd_id))))

// -------
// Globals
// -------

#define MFD_NUM_BTTNS MFD_NUM_VIRTUAL_SLOTS


MFD        mfd[2];                           // Our actual MFD's
bool       Flash = TRUE;                     // State of blinking buttons
LGCursor    mfd_bttn_cursors[NUM_MFDS];
grs_bitmap  mfd_bttn_bitmaps[NUM_MFDS];

grs_canvas _offscreen_mfd, _fullscreen_mfd;

#define mfdL mfd[MFD_LEFT]
#define mfdR mfd[MFD_RIGHT]


// -----------
// Prototypes
// -----------
void mfd_language_change(void);
void mfd_set_slot(ubyte mfd_id, ubyte newSlot, bool OnOff);
void mfd_draw_all_buttons(ubyte mfd_id);
errtype mfd_update_screen_mode();
errtype mfd_clear_all();
void mfd_change_fullscreen(bool on);

void mfd_clear_func(ubyte func_id);
int mfd_choose_func(int my_func, int my_slot);
void mfd_zoom_rect(LGRect* start, int mfdnum);
void fullscreen_refresh_mfd(ubyte mfd_id);

bool mfd_object_cursor_handler(uiEvent* ev, LGRegion*, int which_mfd);
bool mfd_scan_opacity(int mfd_id, LGPoint epos);

void mfd_draw_button_panel(ubyte mfd_id);
void mfd_draw_button(ubyte mfd_id, ubyte b);
void mfd_select_button(int which_panel, int which_button);

void mfd_default_mru( uchar func );
void set_mfd_from_defaults(int mfd_id, uchar func, uchar slot);
void cap_mfds_with_func(uchar func, uchar max);


//KLC  dbg_mfd_state used to be here.



// ------------------
//    INITIALIZERS
// ------------------

// ---------------------------------------------------------------------------
// init_newmfd()               
//
// Initialize the MFD system (called from init_all() in init.c)

void init_newmfd()
{
   ubyte i;

   // Set the default MFD function
   set_default_to_func(MFD_LEFT,  MFD_EMPTY_FUNC);
   set_default_to_func(MFD_RIGHT, MFD_EMPTY_FUNC);

   // Now set actual MFD slots to point at virtual slots
   for (i = 0; i < NUM_MFDS; i++) mfd[i].id = i;

   player_struct.mfd_current_slots[MFD_LEFT]  = MFD_WEAPON_SLOT;
   player_struct.mfd_current_slots[MFD_RIGHT] = MFD_ITEM_SLOT;

   for (i = 0; i < MFD_NUM_VIRTUAL_SLOTS; i++) {
      set_mfd_to_slot(MFD_LEFT,  i, i);
      set_mfd_to_slot(MFD_RIGHT, i, i);
   }

   for (i = 0; i < MFD_NUM_FUNCS; i++)
      if (mfd_funcs[i].flags & MFD_INCREMENTAL)
         player_struct.mfd_func_status[i]|=1 << 4;

   chg_set_flg(MFD_UPDATE);
   
   return;
}


// ---------------------------------------------------------------------------
// init_newmfd_button_cursors()
//
// Initialize the twelve goofy cursors, each of which hovers over an MFD button,
// as spec'd in last nights warren/artist/programmers meeting (SPAZ 8/5)


static char* cursor_strings[MFD_NUM_BTTNS];
static char  cursor_strbuf[128];


void mfd_language_change(void)
{
   load_string_array(REF_STR_MFDCursor,cursor_strings,cursor_strbuf,sizeof(cursor_strbuf),MFD_NUM_BTTNS);
}

void init_newmfd_button_cursors()
{
   int i;
   mfd_language_change();
   for (i = 0; i < NUM_MFDS; i++)
   {
      LGCursor* c = &mfd_bttn_cursors[i];
      grs_bitmap* bm = &mfd_bttn_bitmaps[i];
      LGPoint offset = {0,0};
      make_popup_cursor(c,bm,cursor_strings[i],i,TRUE, offset);
   }
}


// ---------------------------------------------------------------------------
// screen_init_mfd_draw()
//
// Basically, just draw the friggin' buttons and set mfd's to their
// first slot. (called from screen_start() in screen.c)

void screen_init_mfd_draw()
{
   mfd_set_slot(MFD_LEFT, mfd_index(MFD_LEFT), TRUE);
   mfd_set_slot(MFD_RIGHT, mfd_index(MFD_RIGHT), TRUE);

   mfd_draw_all_buttons(MFD_LEFT);
   mfd_draw_all_buttons(MFD_RIGHT);

   return;
}

#ifdef SVGA_SUPPORT
#define MAX_WD(x) (fix_int(fix_mul_div(fix_make((x),0),fix_make(1024,0),fix_make(320,0))))
#define MAX_HT(y) (fix_int(fix_mul_div(fix_make((y),0),fix_make(768,0),fix_make(200,0))))
#endif

// ---------------------------------------------------------------------------
// screen_init_mfd();
//
// Declare the appropriate regions for the MFD's and their button panels.
// (called from screen_start() in screen.c)

void screen_init_mfd(bool fullscrn)
{
   static bool done_init = FALSE;
   FrameDesc *f;
   int id;                     
   int lval, rval;

   lval = MFD_LEFT;                    // Screen callbacks need to know their
   rval = MFD_RIGHT;                   // left from their right, thusly

// Set up the Rect structures for MFD screen-space
   
// Left View Window
   mfdL.rect.ul.x      = MFD_VIEW_LFTX;
   mfdL.rect.ul.y      = MFD_VIEW_Y;
   mfdL.rect.lr.x      = MFD_VIEW_LFTX  + MFD_VIEW_WID;
   mfdL.rect.lr.y      = MFD_VIEW_Y     + MFD_VIEW_HGT;

// Right View Window
   mfdR.rect.ul.x      = MFD_VIEW_RGTX;
   mfdR.rect.ul.y      = MFD_VIEW_Y;
   mfdR.rect.lr.x      = MFD_VIEW_RGTX  + MFD_VIEW_WID;
   mfdR.rect.lr.y      = MFD_VIEW_Y     + MFD_VIEW_HGT;

// Left Button Panel
   mfdL.bttn.rect.ul.x = MFD_BTTN_LFTX;
   mfdL.bttn.rect.ul.y = MFD_BTTN_Y;
   mfdL.bttn.rect.lr.x = MFD_BTTN_LFTX  + MFD_BTTN_WID;
   mfdL.bttn.rect.lr.y = MFD_BTTN_Y     + MFD_BTTN_HGT;

// Right Button Panel
   mfdR.bttn.rect.ul.x = MFD_BTTN_RGTX;
   mfdR.bttn.rect.ul.y = MFD_BTTN_Y;
   mfdR.bttn.rect.lr.x = MFD_BTTN_RGTX  + MFD_BTTN_WID;
   mfdR.bttn.rect.lr.y = MFD_BTTN_Y     + MFD_BTTN_HGT;

// Now, actually create the four regions, and add handlers
   if (!fullscrn)
   {
      macro_region_create(root_region, &(mfdL.reg), &(mfdL.rect));
      macro_region_create(root_region, &(mfdR.reg), &(mfdR.rect));
      macro_region_create(root_region, &(mfdL.bttn.reg), &(mfdL.bttn.rect));
      macro_region_create(root_region, &(mfdR.bttn.reg), &(mfdR.bttn.rect));
      
      uiInstallRegionHandler(&(mfdL.reg),      (UI_EVENT_MOUSE | UI_EVENT_MOUSE_MOVE),
         mfd_view_callback,   (void*)MFD_LEFT,  &id);
      uiInstallRegionHandler(&(mfdR.reg),      (UI_EVENT_MOUSE | UI_EVENT_MOUSE_MOVE),
         mfd_view_callback,   (void*)MFD_RIGHT, &id);
   	
      uiInstallRegionHandler(&(mfdL.bttn.reg), (UI_EVENT_MOUSE | UI_EVENT_MOUSE_MOVE),
         mfd_button_callback, (void*)MFD_LEFT,  &id);
      uiInstallRegionHandler(&(mfdR.bttn.reg), (UI_EVENT_MOUSE | UI_EVENT_MOUSE_MOVE),
         mfd_button_callback, (void*)MFD_RIGHT, &id);
   }
   else
   {
      uiCursorStack *cs;
      macro_region_create(fullview_region, &(mfdL.reg2), &(mfdL.rect));
      uiGetRegionCursorStack(&(mfdL.reg),&cs);
      uiSetRegionCursorStack(&(mfdL.reg2),cs);
      macro_region_create(fullview_region, &(mfdR.reg2), &(mfdR.rect));
      uiGetRegionCursorStack(&(mfdR.reg),&cs);
      uiSetRegionCursorStack(&(mfdR.reg2),cs);
      region_create(fullview_region, &(mfdL.bttn.reg2), &(mfdL.bttn.rect),2,0,REG_USER_CONTROLLED,NULL,NULL,NULL,NULL);
      uiGetRegionCursorStack(&(mfdL.bttn.reg),&cs);
      uiSetRegionCursorStack(&(mfdL.bttn.reg2),cs);
      region_create(fullview_region, &(mfdR.bttn.reg2), &(mfdR.bttn.rect),2,0,REG_USER_CONTROLLED,NULL,NULL,NULL,NULL);
      uiGetRegionCursorStack(&(mfdR.bttn.reg),&cs);
      uiSetRegionCursorStack(&(mfdR.bttn.reg2),cs);
      
      uiInstallRegionHandler(&(mfdL.reg2),      (UI_EVENT_MOUSE | UI_EVENT_MOUSE_MOVE),
         mfd_view_callback_full,   (void*)MFD_LEFT,  &id);
      uiInstallRegionHandler(&(mfdR.reg2),      (UI_EVENT_MOUSE | UI_EVENT_MOUSE_MOVE),
         mfd_view_callback_full,   (void*)MFD_RIGHT, &id);
         
      uiInstallRegionHandler(&(mfdL.bttn.reg2), (UI_EVENT_MOUSE | UI_EVENT_MOUSE_MOVE),
         mfd_button_callback, (void*)MFD_LEFT,  &id);
      uiInstallRegionHandler(&(mfdR.bttn.reg2), (UI_EVENT_MOUSE | UI_EVENT_MOUSE_MOVE),
         mfd_button_callback, (void*)MFD_RIGHT, &id);
   }

   if (!done_init)
   {
      done_init = TRUE;
      
      // Pull in the background bitmap
      f = (FrameDesc*)RefLock(REF_IMG_bmBlankMFD);
      mfd_background = f->bm;
//KLC      mfd_background.bits = (uchar *)NewPtr(MAX_WD(MFD_VIEW_WID)*MAX_HT(MFD_VIEW_HGT));
      mfd_background.bits = (uchar *)NewPtr(f->bm.w * f->bm.h);
      LG_memcpy(mfd_background.bits,(f+1),f->bm.w * f->bm.h);
      RefUnlock(REF_IMG_bmBlankMFD);

      gr_init_canvas(&_offscreen_mfd, mfd_canvas_bits, BMT_FLAT8, MFD_VIEW_WID, MFD_VIEW_HGT);
      gr_init_canvas(&_fullscreen_mfd,mfd_background.bits, BMT_FLAT8, MFD_VIEW_WID, MFD_VIEW_HGT);
      pmfd_canvas = &_offscreen_mfd;
      init_newmfd_button_cursors();
      mfd_init_funcs();
   }
   return;
}

#ifdef SVGA_SUPPORT
errtype mfd_update_screen_mode()
{
   if (convert_use_mode == 0)
   {
      gr_init_canvas(&_offscreen_mfd, mfd_canvas_bits, BMT_FLAT8, MFD_VIEW_WID, MFD_VIEW_HGT);
      gr_init_canvas(&_fullscreen_mfd,mfd_background.bits, BMT_FLAT8, MFD_VIEW_WID, MFD_VIEW_HGT);
   }
   else
   {
      gr_init_canvas(&_offscreen_mfd, mfd_canvas_bits, BMT_FLAT8, 148, 137);
      gr_init_canvas(&_fullscreen_mfd,mfd_background.bits, BMT_FLAT8, 148, 137);
   }
   return(OK);
}

errtype mfd_clear_all()
{
   if (full_game_3d)
   {
      gr_push_canvas(&_offscreen_mfd);
      gr_clear(0);
      gr_pop_canvas();
      gr_push_canvas(&_fullscreen_mfd);
      gr_clear(0);
      gr_pop_canvas();
   }
   return(OK);
}
#endif


// ---------------------------------------------
// mfd_change_fullscreen(bool on);
//
// set up and clean up for fullscreen mode. 

void mfd_change_fullscreen(bool on)
{
   if (on)
   {
      gr_push_canvas(&_fullscreen_mfd);
      gr_clear(0);
      gr_pop_canvas();
      gr_push_canvas(&_offscreen_mfd);
      gr_clear(0);
      gr_pop_canvas();
   }
   else
   {
      // we use the mfd background for the canvas, so 
      // put the background bitmap back
      grs_bitmap *bm = lock_bitmap_from_ref(REF_IMG_bmBlankMFD);
      RefUnlock(REF_IMG_bmBlankMFD);
      LG_memcpy(_fullscreen_mfd.bm.bits,bm->bits,bm->w*bm->h);
   }
}


// ---------------------------------------------------------------------------
// keyboard_init_mfd()
//
// Tell the function keys that they're supposed to map to our button panels.
// (Called from init_input() in input.c)

void keyboard_init_mfd()
{
   extern void install_keypad_hotkeys(void);
/* KLC leave out F-keys and char codes.

   hotkey_add(KEY_F1, DEMO_CONTEXT,mfd_button_callback_kb,(void*)0);
   hotkey_add(KEY_F2, DEMO_CONTEXT,mfd_button_callback_kb,(void*)1);
   hotkey_add(KEY_F3, DEMO_CONTEXT,mfd_button_callback_kb,(void*)2);
   hotkey_add(KEY_F4, DEMO_CONTEXT,mfd_button_callback_kb,(void*)3);
   hotkey_add(KEY_F5, DEMO_CONTEXT,mfd_button_callback_kb,(void*)4);
   hotkey_add(KEY_F6, DEMO_CONTEXT,mfd_button_callback_kb,(void*)5);
   hotkey_add(KEY_F7, DEMO_CONTEXT,mfd_button_callback_kb,(void*)6);
   hotkey_add(KEY_F8, DEMO_CONTEXT,mfd_button_callback_kb,(void*)7);
   hotkey_add(KEY_F9, DEMO_CONTEXT,mfd_button_callback_kb,(void*)8);
   hotkey_add(KEY_F10,DEMO_CONTEXT,mfd_button_callback_kb,(void*)9);
*/
   install_keypad_hotkeys();
}


// --------------
//    FROBBERS
// --------------

// ---------------------------------------------------------------------------
// set_slot_to_func()
//
// Sets a slot to point to a given function struct, and sets status too.

void set_slot_to_func(ubyte snum, ubyte fnum, MFD_Status stat)
{

      player_struct.mfd_all_slots[snum] = fnum;

      if ((player_struct.mfd_slot_status[snum] == MFD_FLASH) && (stat == MFD_ACTIVE)) ;
      else player_struct.mfd_slot_status[snum] = stat;

   return;
}

// ---------------------------------------------------------------------------
// mfd_clear_func()
//
// Set a functions last update to current game time, clear its CHANGEBIT
// field if set.

void mfd_clear_func(ubyte func_id)
{
   mfd_funcs[func_id].last = player_struct.game_time;

   player_struct.mfd_func_status[func_id] &= ~MFD_CHANGEBIT;
   player_struct.mfd_func_status[func_id] &= ~MFD_CHANGEBIT_FULL;
   return;
}

#define MFD_STEREO_HACK_MODE  6

// ---------------------------------------------------------------------------
// mfd_notify_func()
//
// Let a function know its been changed and needs re-exposure.
// Also check a specified slot to see if that function is there: if
// is not, we might grab it and put it there.  We also set the slot's
// status as demanded.

//#define MFD_STEREO_HACK_MODE  ((i6d_device == I6D_CTM) ? 6 : 7)
void mfd_notify_func(ubyte fnum, ubyte snum, bool Grab, MFD_Status stat, bool Full)
{
   ubyte i, j;
   int oldf = player_struct.mfd_all_slots[snum];
   byte mfd_but[NUM_MFDS];

   if (fnum == NOTIFY_ANY_FUNC) fnum = oldf;

   for (i = 0; i < NUM_MFDS; i++)
      mfd_but[i] = -1;

   if ((oldf != fnum) && (Grab))
   {
      player_struct.mfd_all_slots[snum] = fnum;
      Full = TRUE;
   }

   player_struct.mfd_func_status[fnum]           |= MFD_CHANGEBIT;
   if (Full)
   {
      void mfd_default_mru(uchar func);
#ifdef SVGA_SUPPORT
      uchar old_over = gr2ss_override;
      short temp;
      gr2ss_override = OVERRIDE_ALL;
      ss_set_hack_mode(MFD_STEREO_HACK_MODE,&temp);
#endif

      for (i = 0; i < NUM_MFDS; i++)
      {
         if (oldf != fnum && player_struct.mfd_current_slots[i] == snum)
         {
            mfd_funcs[oldf].expose(&(mfd[i]),0);
         }
      }
#ifdef SVGA_SUPPORT
      ss_set_hack_mode(0,&temp);
      gr2ss_override = old_over;
#endif
      player_struct.mfd_func_status[fnum] |= MFD_CHANGEBIT_FULL;
      mfd_default_mru(fnum);
   }

   if (player_struct.mfd_all_slots[snum] == fnum)
   {
      set_slot_to_func(snum,fnum,stat);

      // Find which buttons we have to redraw because of the change
      for (i = 0; i < NUM_MFDS; i++)
      {
         for (j = 0; j < MFD_NUM_VIRTUAL_SLOTS; j++)
         {         
            if (player_struct.mfd_virtual_slots[i][j] == snum)
            {
               mfd_but[i] = j;
               if (player_struct.mfd_current_slots[i] == j)
               {
                  player_struct.mfd_slot_status[snum] = stat;
                  if (stat == MFD_FLASH)
                     player_struct.mfd_slot_status[snum] = MFD_ACTIVE;
               }
            }
         }
      }
 
      // Now redraw them
      if (_current_loop <= FULLSCREEN_LOOP && !global_fullmap->cyber)
         for (i = 0; i < NUM_MFDS; i++)
            if (mfd_but[i] != -1)
               mfd_draw_button(i, mfd_but[i]);
   }
  
   chg_set_flg(MFD_UPDATE);
  
   return;
}

// ---------------------------------------------------------------------------
// mfd_get_func()
//
// Returns an MFD's slot's function.

ubyte mfd_get_func(ubyte mfd_id, ubyte s)
{
   ubyte slot_num;

   slot_num = player_struct.mfd_virtual_slots[mfd_id][s];

   if (player_struct.mfd_slot_status[slot_num] == MFD_EMPTY)
      return player_struct.mfd_empty_funcs[mfd_id];
   else
      return player_struct.mfd_all_slots[slot_num];
}
 
// ---------------------------------------------------------------------------
// mfd_set_slot()
//
// Sets the mfd to a given slot without caring about turning off what was
// previously there, or any change-related state.  Permits usage from both
// initializer and slot-changer.

void mfd_set_slot(ubyte mfd_id, ubyte newSlot, bool OnOff)
{
	MFD_Func *f;
	ubyte old_index;
	ubyte f_id;
	ubyte new_slot;
	ubyte old_slot;
	
	old_index = player_struct.mfd_current_slots[mfd_id];
	old_slot  = player_struct.mfd_virtual_slots[mfd_id][old_index];
	new_slot  = player_struct.mfd_virtual_slots[mfd_id][newSlot];

	if (!OnOff && ((player_struct.mfd_slot_status[old_slot] != MFD_EMPTY) ||
		(player_struct.mfd_slot_status[new_slot] != MFD_EMPTY)))
	{
		uchar old_over = gr2ss_override;
		short temp;
		gr2ss_override = OVERRIDE_ALL;
		ss_set_hack_mode(MFD_STEREO_HACK_MODE,&temp);
		f_id = mfd_get_func(mfd_id, newSlot);
		f = &(mfd_funcs[f_id]);
		f->expose(&(mfd[mfd_id]), 0);
		ss_set_hack_mode(0,&temp);
		gr2ss_override = old_over;
	}

	if (player_struct.mfd_slot_status[new_slot] == MFD_FLASH)
	{
		player_struct.mfd_slot_status[new_slot] = MFD_ACTIVE;
		
		// We have to tell other panel about this button change!
		if (global_fullmap->cyber)
			if (mfd_id == MFD_LEFT)
				mfd_draw_button(MFD_RIGHT, newSlot);
			else
				mfd_draw_button(MFD_LEFT,  newSlot);
	}

	if (OnOff)
	{
		player_struct.mfd_current_slots[mfd_id] = newSlot;
		mfd_force_update_single(mfd_id);
		if (full_game_3d)
		{
			if (!(full_visible & visible_mask(mfd_id)))
			{
				if (mfd_id == MFD_LEFT)
					gr_push_canvas(&_offscreen_mfd);
				else
					gr_push_canvas(&_fullscreen_mfd);
				gr_clear(0);
				gr_pop_canvas();
			}
#ifdef STEREO_SUPPORT
			if (convert_use_mode == 5)
				full_visible = visible_mask(mfd_id);
			else
#endif
			{
				full_visible |= visible_mask(mfd_id);
			}
			full_raise_region(&mfd[mfd_id].reg2);
			chg_set_sta(FULLSCREEN_UPDATE);
		}
	}

	return;
}

// ---------------------------------------------------------------------------
// mfd_change_slot()
//
// Shifts an mfd over to a new slot. 

void mfd_change_slot(ubyte mfd_id, ubyte new_slot)
{
   ubyte old;

   if (global_fullmap->cyber && (new_slot != MFD_INFO_SLOT || mfd_id != MFD_RIGHT)) return; // no slots in c-space
   old = player_struct.mfd_current_slots[mfd_id];

   if (new_slot == old && !full_game_3d) return;

   // Tell old slot it needs to stop drawing whatever it was in that mfd
   if (new_slot != old)
      mfd_set_slot(mfd_id, old, FALSE);

   // Set to new slot and draw its graphics if neccessary
   mfd_set_slot(mfd_id, new_slot, TRUE);

   // Update the buttons
   if (!global_fullmap->cyber)
   {
      mfd_draw_button(mfd_id, old);
      mfd_draw_button(mfd_id, new_slot);
   }

   return;
}


// ---------------------------------------------------------------------------
// mfd_grab()
//
// Picks an MFD to grab (i.e. the MFD whose information is 
// lowest priority.  Returns the mfd id. 

int mfd_grab(void)
{
   int i;
   ubyte min = 0;
   int id;
   for (i = 0; i < NUM_MFDS; i++)
   {
      ubyte slot = player_struct.mfd_current_slots[i];
      ubyte func = mfd_get_func(i,slot);
      ubyte p = mfd_funcs[func].priority;
      if (p > min)
      {
         min = p;
         id = i;
      }
   }
   return id;
}


// ---------------------------------------------------------------------------
// mfd_grab_func()
//
// Like mfd_grab(), except specifies a func number.  If any mfd is already 
// set to that func, returns that mfd id instead of the lowest prority.
// Otherwise, if there is already an mfd on the given slot, returns that
// mfd.  If neither of these conditions holds, returns the mfd with the
// lowest priority.  If other mfds are on the same slot as the one we
// are grabbing for a new func, try to restore to them. 

// mfd_choose_func() does the same thing as mfd_grab_func, but does not
// assume we necessarily actually want to grab the mfd it returns, and
// therefore does not restore to other mfds on the same slot.
//
int mfd_choose_func(int my_func, int my_slot)
{
   int i;
   ubyte min = 0;
   ubyte slot, func, p;
   int lowid, retval, sameslotid=-1;

   for (i = 0; i < NUM_MFDS; i++)
   {
      slot = player_struct.mfd_current_slots[i];
      func = mfd_get_func(i,slot);
      p = mfd_funcs[func].priority;

      if (func == my_func) return i;
      if (slot == my_slot) sameslotid=i;
      if (p > min)
      {
         min = p;
         lowid = i;
      }
   }
   if(sameslotid!=-1)
      retval=sameslotid;
   else
      retval=lowid;

   return retval;
}

int mfd_grab_func(int my_func, int my_slot)
{
   ubyte mfd,slot;
   int i;

   mfd=mfd_choose_func(my_func, my_slot);
   slot=player_struct.mfd_current_slots[mfd];

   // if more than one mfd is on the slot we're grabbing, try
   // restoring to the other slots.
   for (i = 0; i < NUM_MFDS; i++)
   {
      if (i!=mfd && player_struct.mfd_current_slots[i]==slot) {
         restore_mfd_slot(i);
         break;
      }
   }
   return mfd;
}

// -----------------------------------------------------------------------
// mfd_yield_func()
//
//   If no mfd has func as its current function, returns FALSE.  Otherwise,
// if *mfd_id is NUM_MFDS, sets mfd_id to the lowest mfd id which has func as
// its function and returns TRUE.  Otherwise, sets mfd_id to the lowest 
// such id which is greater than the one provided and returns TRUE, or
// returns FALSE if there is no such greater mfd.  Thus acts as an
// iterator on mfd's with the given func.  
//   Why, you may ask?  'Cause it's pretty much exactly as easy as a
// function which just finds out if some mfd has this current function,
// which is what I need, and it's loads more generally useful.  Har har.

bool mfd_yield_func(int func, int* mfd_id)
{
   int id;

   for( id=(*mfd_id != NUM_MFDS) ?(*mfd_id)+1:0 ; id<NUM_MFDS ; id++) {
      if(mfd_get_active_func(id)==func) {
         *mfd_id=id;
         return TRUE;
      }
   }
   return FALSE;
}                                                                

// -----------------------------------------------------------
// mfd_zoom_rect(Rect* start, int mfd)
//
// Zooms a rect from the specified starting point to the 
// the indicated rect.  

void mfd_zoom_rect(LGRect* start, int mfdnum)
{
   extern void zoom_rect(LGRect* start, LGRect* end);
   LGRect r1, r2;
   play_digi_fx(SFX_ZOOM_BOX,1);
   r1 = *start;
   r2 = mfd[mfdnum].rect;
   zoom_rect(&r1,&r2);
}



// ------------------------
//    CALLBACK FUNCTIONS
// ------------------------

// ------------------------------------------------------------------
// mfd_object_cursor_handler() gets called for events in the MFD
// region with an object on the cursor.  


extern bool inventory_add_object(ObjID,bool);
bool object_button_down = FALSE;

bool mfd_object_cursor_handler(uiEvent* ev, LGRegion*, int which_mfd)
{
   bool retval = FALSE;
   int trip, mid;
   int new_slot=-1;
   ObjID obj = object_on_cursor;
   if (ev->type != UI_EVENT_MOUSE) return TRUE;
   if (ev->subtype & (MOUSE_RDOWN|MOUSE_LDOWN))
   {
      object_button_down = TRUE;
      retval = TRUE;
   }
   if ((ev->subtype & (MOUSE_LUP|MOUSE_RUP)) && object_button_down)
   {
      extern uchar gump_num_objs;
      bool is_gump = mfd_get_active_func(which_mfd)==MFD_GUMP_FUNC && gump_num_objs!=0;

      object_button_down = FALSE;
      retval = TRUE;
      if (inventory_add_object(object_on_cursor,!is_gump))
      {
         if (!is_gump)
         {
            switch(objs[obj].obclass)
            {
            case CLASS_GUN:
               new_slot=MFD_WEAPON_SLOT;
               break;
            case CLASS_AMMO:
               trip = current_weapon_trip();
               if(trip!=-1 && gun_takes_ammo(trip,ID2TRIP(obj))
                  && player_struct.mfd_current_slots[which_mfd] == MFD_WEAPON_SLOT)
                  ; // do nothing
               else
                  new_slot=MFD_ITEM_SLOT;
               break;

            case CLASS_SOFTWARE:
               if (objs[obj].subclass == SOFTWARE_SUBCLASS_DATA)
               {
                  new_slot=MFD_INFO_SLOT;
                  break;
               }
            default:
               new_slot=MFD_ITEM_SLOT;
            }
            for(mid=0;mid<NUM_MFDS;mid++) {
               if(mid!=which_mfd && mfd_index(mid)==new_slot)
                  restore_mfd_slot(mid);
            }
            mfd_change_slot(which_mfd,new_slot);
            mfd_force_update_single(which_mfd);
         }
         pop_cursor_object();
      }
   }
   return retval;
}


// ---------------------------------------------------------------------------
// mfd_view_callback()
//
// The callback for the MFD view windows.  Triggered by mouseclicks inside
// the regions.

#define SEARCH_MARGIN 2

bool mfd_scan_opacity(int mfd_id, LGPoint epos)
{
   bool retval = FALSE;
   LGPoint pos = epos;
   short x,y;
   grs_canvas* cv = ((int)mfd_id == MFD_RIGHT) ?  &_fullscreen_mfd : &_offscreen_mfd;

   pos.x -= mfd[mfd_id].reg.abs_x;
   pos.y -= mfd[mfd_id].reg.abs_y;
   gr_push_canvas(cv);
   for (x = pos.x - SEARCH_MARGIN; x <= pos.x + SEARCH_MARGIN; x++)
      for(y = pos.y - SEARCH_MARGIN; y <= pos.y + SEARCH_MARGIN; y++)
         if (gr_get_pixel(x,y) != 0)
            retval = TRUE;
   gr_pop_canvas();
   return retval;
}



bool mfd_view_callback_full(uiEvent *e, LGRegion *r, void *udata)
{
   bool retval = FALSE;
   uchar mask;
   if ((int)udata == MFD_RIGHT)
      mask = FULL_R_MFD_MASK;
   else
      mask = FULL_L_MFD_MASK;
   if (full_visible & mask)
   {
      retval = mfd_view_callback(e,r,udata);
      if (!retval)
      {
         retval = mfd_scan_opacity((int)udata,e->pos);
      }
   }
   return retval;
}

bool mfd_view_callback(uiEvent *e, LGRegion *r, void *udata)
{
   int i;
   int which_mfd;
   MFD *m;
   ubyte func_id;
   MFD_Func *f;

   LGRegion dummy; // dummy
   dummy = *r;   // dummy

   which_mfd = (int) udata;

   // We should pass on info to the appropriate slot's current handler
   if (which_mfd == MFD_LEFT) m = &mfdL;
   else                       m = &mfdR;

   if (input_cursor_mode == INPUT_OBJECT_CURSOR)
      return mfd_object_cursor_handler(e,r,which_mfd);
   else object_button_down = FALSE;
   func_id = mfd_get_active_func(which_mfd);
   f = &(mfd_funcs[func_id]);
   if (f->simp && f->simp(m, e)) return TRUE;
   for (i = 0; i < f->handler_count; i++)
   {
      LGPoint pos = e->pos;
#ifdef STEREO_SUPPORT
      if (convert_use_mode == 5)
      {
         pos.y -= m->rect.ul.y;
         switch (i6d_device)
         {
            case I6D_CTM:
               if (which_mfd == 0)
                  pos.x -= m->rect.ul.x;
               else
                  pos.x -= (m->rect.ul.x << 1);
               break;
            case I6D_VFX1:
               Warning(("original pos.x = %d, m->rect.ul.x = %d!\n",pos.x,m->rect.ul.x));
               pos.x -= (m->rect.ul.x);
               break;
         }
      }
      else
      {
#endif
         pos.x -= m->rect.ul.x;
         pos.y -= m->rect.ul.y;
#ifdef STEREO_SUPPORT
      }
#endif
      if (RECT_TEST_PT(&f->handlers[i].r,pos))
         if (f->handlers[i].proc(m,e,&f->handlers[i]))
            return TRUE;
   }

   return FALSE;
}


// ---------------------------------------------------------------------------
// mfd_button_callback()
//
// The callback for the MFD button panels.  Triggered by mouseclicks inside
// the button panels.
int last_mfd_cnum[NUM_MFDS] = {-1,-1};
bool mfd_button_callback(uiEvent *e, LGRegion *r, void *udata)
{
   uiMouseEvent *m;
   int cnum, which_panel, which_button;
   div_t result;

#ifndef NO_DUMMIES
   LGRegion dummy;     dummy = *r;
#endif

   if (global_fullmap->cyber)
   {
      uiSetRegionDefaultCursor(r,NULL);
      return FALSE;
   }
   else 
   {
      m           = (uiMouseEvent *) e;
      which_panel = (int)            udata;

      // Divide mouseclick height to discover which button we meant
      result = div((m->pos.y - MFD_BTTN_Y), MFD_BTTN_SZ + MFD_BTTN_BLNK);
      which_button = result.quot;

      cnum = which_button;

      if (player_struct.mfd_slot_status[which_button] == MFD_UNAVAIL || !popup_cursors)
      {
         if ((cnum != last_mfd_cnum[which_panel])) {
            last_mfd_cnum[which_panel] = cnum;
            uiSetRegionDefaultCursor(r,&globcursor);
         }
      }
      if (player_struct.mfd_slot_status[which_button] != MFD_UNAVAIL)
      {
         if ((cnum != last_mfd_cnum[which_panel]) && popup_cursors)
         {
            LGPoint offset = {0,0};
            last_mfd_cnum[which_panel] = cnum;
            DisposePtr((Ptr)mfd_bttn_bitmaps[which_panel].bits);
            make_popup_cursor(&mfd_bttn_cursors[which_panel],&mfd_bttn_bitmaps[which_panel],cursor_strings[cnum],which_panel,TRUE, offset);
            uiSetRegionDefaultCursor(r,&mfd_bttn_cursors[which_panel]);
         }

         if (!(m->action & (MOUSE_LDOWN|UI_MOUSE_LDOUBLE))) return TRUE; // ignore all but left clickdowns

         // If things are ok, select button
         if ((result.rem < MFD_BTTN_SZ) && (which_button < MFD_NUM_VIRTUAL_SLOTS))
            mfd_select_button(which_panel, which_button);
      }
   }

   return TRUE;
}


// ---------------------------------------------------------------------------
// mfd_button_callback_kb()
//
// The callback for the MFD button panels, as triggered by function keys

bool mfd_button_callback_kb(short keycode, ulong context, void *data)
{
   int which_panel, which_button;
   int fkeynum;

   if (!global_fullmap->cyber)
   {
      fkeynum = (int) context; // dummy funcs
      fkeynum = (int) keycode;

      fkeynum = (int) data;

      if (fkeynum >= MFD_NUM_VIRTUAL_SLOTS) which_panel = MFD_RIGHT;
      else                                  which_panel = MFD_LEFT;

      which_button = ((int)data) % MFD_NUM_VIRTUAL_SLOTS;

      mfd_select_button(which_panel, which_button);
   }

   return TRUE;
}

// ---------------------------------------------------------------------------
// mfd_select_button()
//
// A more specific version of mfd_button_callback(), where we've figured
// out which button exactly has been hit, whether because we came here
// straight from a function key or through the mouse callback parser.
// The passed argument is the equivalent of the function key number, even
// if we got it from the mouse handler.

void mfd_select_button(int which_panel, int which_button)
{
   // Play sound effect
   ubyte old = player_struct.mfd_current_slots[which_panel];
   int hnd;
   hnd = play_digi_fx(SFX_MFD_BUTTON,1);
   if (hnd >= 0)
   {
      snd_digi_parms *ssp;
      ssp = snd_sample_parms(hnd);

      // Woo hoo, hardcode city
      if (which_button < 5)
         ssp->pan = 30;
      else
         ssp->pan = 97;
   }
   if (full_game_3d && which_button == old && (full_visible & visible_mask(which_panel)))
   {
      full_visible &= ~visible_mask(which_panel);
   }
   else 
      mfd_change_slot((ubyte) which_panel, (ubyte) which_button);

   return;
}



// ---------------------------------------------------------------------------
// mfd_update()
//
// This is what gets called from the main loop each frame.

void mfd_update()
{
   static bool LastFlash = FALSE;
   ubyte   steps_cache[NUM_MFDS] = { 0, 0};

   int    i, j;
   ubyte  slots[NUM_MFDS];
   ubyte   status_cache[NUM_MFDS];
   
      Flash = (bool) ((player_struct.game_time/MFD_BTTN_FLASH_TIME) %2);

      if (!global_fullmap->cyber)
        for (i = 0; i < NUM_MFDS; i++) {
            for (j = 0; j < MFD_NUM_VIRTUAL_SLOTS; j++) {
               slots[i] = player_struct.mfd_virtual_slots[i][j];
               if (player_struct.mfd_slot_status[slots[i]] == MFD_FLASH)
               {
                chg_set_flg(MFD_UPDATE);
                  if (LastFlash != Flash) mfd_draw_button(i, j);
               }
            }
         }
      if (LastFlash != Flash) LastFlash = Flash;

      // Is it time to update appropriate mfd's?
      // Check only current slots, and look at flag to see
      // if they need constant update


#ifndef BAD_BITS_BUG_FIXED
      _fullscreen_mfd.bm.bits = mfd_background.bits;
      _offscreen_mfd.bm.bits  = mfd_canvas_bits;
#endif // BAD_BITS_BUG_FIXED


// This code totally depends on our item func implementation. 
   i = NUM_MFDS;
   if (mfd_yield_func(MFD_ITEM_FUNC,&i))
   {
      extern void update_item_mfd(void);
      update_item_mfd();
   }


   // Build the status cache. 
   for(i = 0; i < NUM_MFDS; i++)
   {
      ubyte f_id = mfd_get_active_func(i);
      MFD_Func* f = &(mfd_funcs[f_id]);
      status_cache[i] = (player_struct.mfd_func_status[f_id] );
      if (f->flags & MFD_INCREMENTAL)
      {
         long deltat = (player_struct.game_time - f->last)>>4;
         ubyte increment = (player_struct.mfd_func_status[f_id] >> 4);
         ubyte num_steps = ( increment > 0) ? max(0,deltat/increment) : 0;
         steps_cache[i] = num_steps;
         chg_set_flg(MFD_UPDATE);
      }

   }


   // Now update the stati that need it. 
   for (i = 0; i < NUM_MFDS; i++)
   {
      if ((status_cache[i] & (MFD_CHANGEBIT|MFD_CHANGEBIT_FULL)) || steps_cache[i] > 0)
      {
         ubyte f_id = mfd_get_active_func(i);
         mfd_clear_func(f_id);
         mfd_update_current_slot(i,status_cache[i],steps_cache[i]);
      }
   }
   return;
}

// ---------------------------------------------------------------------------
// mfd_update_current_slot()
//
// See if we need to update anything in the current slot being
// viewed in an MFD.  Returns TRUE if it updated a function.

bool mfd_update_current_slot(ubyte mfd_id,ubyte status,ubyte num_steps)
{
   MFD_Func *f;
   ubyte f_id;
   ubyte control;
   MFD *m;

   extern ObjID check_panel_ref(bool puntme);
   extern bool mfd_distance_remove(ubyte func);
  
   f_id = mfd_get_active_func(mfd_id);
   f    = &(mfd_funcs[f_id]);
   m    = &(mfd[mfd_id]);
   if (player_struct.panel_ref == OBJ_NULL && mfd_distance_remove(f_id))
   {
      check_panel_ref(TRUE);
   }

   // If the change bit is set, or if the function is incremental
   // and enough time has gone by, then we need to expose

      {
#ifdef SVGA_SUPPORT
         uchar old_over = gr2ss_override;
         short temp;
         gr2ss_override = OVERRIDE_ALL;
         ss_set_hack_mode(MFD_STEREO_HACK_MODE,&temp);
#endif
         control = (num_steps << 4) | MFD_EXPOSE;
         if (full_game_3d || (status & MFD_CHANGEBIT_FULL))
            control |= MFD_EXPOSE_FULL;

         // Okay, if we are in full screen mode, secretly switch the fine canvas
         // usually served in this restaurant with our own Folger's brand canvas
 
         pmfd_canvas = (full_game_3d && mfd_id == MFD_RIGHT) ? &_fullscreen_mfd : &_offscreen_mfd;
         if (full_game_3d && (control & MFD_EXPOSE_FULL))
         {
            gr_push_canvas(pmfd_canvas);
            gr_clear(0);
            gr_pop_canvas();
         }

         f->expose(m, control);   // pass # steps + flags to
#ifdef SVGA_SUPPORT
         ss_set_hack_mode(0,&temp);
         gr2ss_override = old_over;
#endif


         return TRUE;
      }

}


// ---------------------------------------------------------------------------
// mfd_force_update()
//
// Forces a redraw of both the button panels and mfd slots

void mfd_force_update()
{
   ubyte i;
   for (i = 0; i < NUM_MFDS; i++) {
      mfd_force_update_single(i);
   }
}

// ---------------------------------------------------------------------------
// mfd_force_update()
//
// Forces a redraw of one of the button panels and mfd slots

void mfd_force_update_single(int which_mfd)
{
   ubyte      f_id, s_id;
   MFD_Status stat;

      if (_current_loop <= FULLSCREEN_LOOP)
         mfd_draw_all_buttons(which_mfd);

      f_id = mfd_get_active_func(which_mfd);
      s_id = player_struct.mfd_virtual_slots[which_mfd][mfd_index(which_mfd)];
      stat = player_struct.mfd_slot_status[s_id];

      mfd_notify_func(f_id, s_id, FALSE, stat, TRUE);

   return;
}

//--------------------------------------------------------
// fullscreen_refresh_mfd()
//
// re-blits a single mfd.

void fullscreen_refresh_mfd(ubyte mfd_id)
{
   ushort a,b,c,d;
   LGRect r;
   MFD* m = &mfd[mfd_id];
   bool visible = (full_visible & visible_mask(mfd_id)) != 0;
#ifdef SVGA_SUPPORT
   uchar old_over = gr2ss_override;
#endif
   if (visible)
   {
      pmfd_canvas = (mfd_id == MFD_RIGHT) ? &_fullscreen_mfd : &_offscreen_mfd;

      r.ul = MakePoint(0,0);
      r.lr = MakePoint(MFD_VIEW_WID,MFD_VIEW_HGT);
      RECT_MOVE(&r,m->rect.ul);
      STORE_CLIP(a,b,c,d);
#ifdef SVGA_SUPPORT
      gr2ss_override = OVERRIDE_ALL;
#endif
#ifdef STEREO_SUPPORT
      if (convert_use_mode == 5)
      {
         pmfd_canvas->bm.flags |= BMF_TRANS;
         if (mfd_id == 0)
         {
            ss_safe_set_cliprect(r.ul.x, 0, r.lr.x << 1, r.lr.y);
            if (i6d_device == I6D_CTM)
               ss_noscale_bitmap(&(pmfd_canvas->bm),m->rect.ul.x,-5);
            else
               ss_noscale_bitmap(&(pmfd_canvas->bm),m->rect.ul.x,m->rect.ul.y);
         }
         else
         {
            ss_safe_set_cliprect(r.ul.x >> 1,0,r.lr.x,r.lr.y);
            if (i6d_device == I6D_CTM)
               ss_noscale_bitmap(&(pmfd_canvas->bm),m->rect.ul.x >> 1,-5);
            else
               ss_noscale_bitmap(&(pmfd_canvas->bm),m->rect.ul.x >> 1,m->rect.ul.y);
         }
         pmfd_canvas->bm.flags &= ~BMF_TRANS;
      }
      else
      {
#endif
         ss_safe_set_cliprect(r.ul.x,r.ul.y,r.lr.x,r.lr.y);
         pmfd_canvas->bm.flags |= BMF_TRANS;
         ss_noscale_bitmap(&(pmfd_canvas->bm),m->rect.ul.x,m->rect.ul.y);
         pmfd_canvas->bm.flags &= ~BMF_TRANS;
#ifdef STEREO_SUPPORT
      }
#endif
#ifdef SVGA_SUPPORT
      gr2ss_override = old_over;
#endif
      RESTORE_CLIP(a,b,c,d);
   }
   region_set_invisible(&m->reg2,!visible);
}


// ---------------------------
//    MFD INTERNAL GRAPHICS
// ---------------------------

// ---------------------------------------------------------------------------
// mfd_draw_button()
//
// Draws a button in a given color code depending on its status.

bool cyber_button_back_door = FALSE;

void mfd_draw_button(ubyte mfd_id, ubyte b)
{
   MFD *m;
   LGRect r;
   ubyte slot;
#ifdef SVGA_SUPPORT
   uchar old_over = gr2ss_override;
   gr2ss_override = OVERRIDE_ALL;
#endif

   if (global_fullmap->cyber && full_game_3d)
      return;

      m = &(mfd[mfd_id]);

      r.ul.x = m->bttn.rect.ul.x + 1;
      r.ul.y = m->bttn.rect.ul.y + 1;
      r.ul.y += (b * (MFD_BTTN_SZ + MFD_BTTN_BLNK));

      r.lr.x = r.ul.x + MFD_BTTN_WID-1;
      r.lr.y = r.ul.y + MFD_BTTN_SZ;

      slot = player_struct.mfd_virtual_slots[mfd_id][b];
   
      if      (player_struct.mfd_slot_status[slot] == MFD_EMPTY)  gr_set_fcolor(MFD_BTTN_EMPTY);
      else if (player_struct.mfd_slot_status[slot] == MFD_ACTIVE) gr_set_fcolor(MFD_BTTN_ACTIVE);
      else if (player_struct.mfd_slot_status[slot] == MFD_UNAVAIL) gr_set_fcolor(MFD_BTTN_UNAVAIL);
      else if (player_struct.mfd_slot_status[slot] == MFD_FLASH) {
         if (Flash) gr_set_fcolor((long)MFD_BTTN_FLASH);    // Is blink on or off?
         else       gr_set_fcolor((long)MFD_BTTN_EMPTY);     // Draw appropriately
      }

      if (mfd_index(m->id) == b) gr_set_fcolor((long)MFD_BTTN_SELECT); // current

      uiHideMouse(&r);
//KLC - chg for new art      ss_rect(r.ul.x, r.ul.y, r.lr.x-2, r.lr.y-2);
{
	short		bx = (mfd_id == 0) ? 3 : 629;
	short		by = 333 + (b*26);
	gr_rect(bx, by, bx+6, by+17);
}
#ifdef SVGA_SUPPORT
      gr2ss_override = old_over;
#endif
      uiShowMouse(&r);

   return;
}

// ---------------------------------------------------------------------------
// mfd_draw_button_panel()
//
// Draws an MFD's panel of buttons, and their associated background art as well

#define MFD_PANEL_Y           326
#define MFD_LEFT_PANEL_X      1
#define MFD_RIGHT_PANEL_X     627

void mfd_draw_button_panel(ubyte mfd_id)
{
   int x[2] = { MFD_LEFT_PANEL_X, MFD_RIGHT_PANEL_X };

   draw_hires_resource_bm(REF_IMG_bmMFDButtonBackground, x[mfd_id], MFD_PANEL_Y);
   mfd_draw_all_buttons(mfd_id);
   return;
}

// ---------------------------------------------------------------------------
// mfd_draw_all_buttons()
//
// Draws an MFD's panel of buttons.

void mfd_draw_all_buttons(ubyte mfd_id)
{
   ubyte i;

   for (i = 0; i < MFD_NUM_VIRTUAL_SLOTS; i++)
      mfd_draw_button(mfd_id, i);

   return;
}


// ---------------------------------------------------------------------------
// mfd_draw_string()
//
// Draws a string to the mfd canvas, at a relative x, y location.  It is
// the calling expose functions responsibility to recopy from the canvas
// to the screen.  Returns a point describing the pixel dimensions of the string


bool mfd_string_wrap = TRUE;
ubyte mfd_string_shadow = MFD_SHADOW_FULLSCREEN;

LGPoint mfd_full_draw_string(char *s, short x, short y, long c, int font, bool DrawString, bool transp)
{
   LGPoint siz;
   short w,h;
   ushort sc1,sc2,sc3,sc4;
   short border = 0;
   grs_font* thefont = (grs_font*)ResLock(font);

   x = min(max(x,0),MFD_VIEW_WID-1);
   y = min(max(y,0),MFD_VIEW_HGT-1);
   STORE_CLIP(sc1,sc2,sc3,sc4);
   if (full_game_3d
      && mfd_string_shadow == MFD_SHADOW_FULLSCREEN
      || mfd_string_shadow == MFD_SHADOW_ALWAYS)
      border = 1;

   gr_set_font(thefont);
   if (mfd_string_wrap)
      wrap_text(s,MFD_VIEW_WID - x-1);
   gr_string_size(s,&w,&h);
   w = min(w,MFD_VIEW_WID-x-border);
   h = min(h,MFD_VIEW_HGT-y-border);
   siz.x = w; siz.y = h;
   if (w <= 0 || h <= 0) goto out;
   ss_safe_set_cliprect(max(x-border,0),
                   max(y-border,0),
                   x+w+border,y+h+border);
   if (!full_game_3d && !transp)
//KLC - chg for new art      ss_bitmap(&mfd_background,0,0);
      gr_bitmap(&mfd_background,0,0);
   if (DrawString)
   {
      gr_set_fcolor(c);
      draw_shadowed_string(s,x,y,border > 0);
   }
   mfd_add_rect(x-border,y-border,x+w+border,y+h+border);
out: 
   if (mfd_string_wrap)
      unwrap_text(s);
   ResUnlock(font);
   RESTORE_CLIP(sc1,sc2,sc3,sc4);

   return siz;
}

LGPoint mfd_draw_font_string(char *s, short x, short y, long c, int font, bool DrawString)
{
   // Hey, this used to always specify non-transparent strings,but that just plain
   // seemed wrong, so I switched it.... -- Xemu
   return mfd_full_draw_string(s, x, y, c, font, DrawString, TRUE);
}


LGPoint mfd_draw_string(char *s, short x, short y, long c, bool DrawString)
{
   return mfd_draw_font_string(s,x,y,c,RES_tinyTechFont,DrawString);
}

// ----------------------------------------------------------------------
// mfd_draw_bitmap() draws a bitmap and adds its rect to the 
// update list.  

void mfd_draw_bitmap(grs_bitmap* bmp, short x, short y)
{
   ss_bitmap(bmp,x,y);
   mfd_add_rect(x,y,x+bmp->w,y+bmp->h);
}

// --------------------------------------------------------------------------
// mfd_partial_clear()
//
// Clears a portion of an mfd canvas

void mfd_partial_clear(LGRect *r)
{
   if (!full_game_3d)
   {
      ss_safe_set_cliprect(r->ul.x,r->ul.y,r->lr.x,r->lr.y);
      mfd_add_rect(r->ul.x,r->ul.y,r->lr.x,r->lr.y);
//KLC - chg for new art      ss_bitmap(&mfd_background, 0, 0);
      gr_bitmap(&mfd_background, 0, 0);
   }

   return;
}


// -------------------------------------------------------------------
// UPDATE RECT STUFF
// 
// Here we are collecting a list refresh rectangles which will be 
// updated from the off-screen mfd canvas

#define NUM_MFD_RECTS 16
LGRect mfd_update_list[NUM_MFD_RECTS];
int  mfd_num_updates = 0;


// -------------------------------------------------------------------
//
// mfd_clear_rects(), clears the update list to empty

void mfd_clear_rects(void)
{
   mfd_num_updates = 0;
}

// -------------------------------------------------------------------
//
// mfd_add_rect() adds a rect to the rect list. 

errtype mfd_add_rect(short x, short y, short x1, short y1)
{
   int i;
   LGRect r;
   short tmp;
   // check for invalid rect
   if(x>x1) {
      tmp=x;
      x=x1;
      x1=tmp;
   }
   if(y>y1) {
      tmp=y;
      y=y1;
      y1=tmp;
   }
   r.ul.x = x;
   r.ul.y = y;
   r.lr.x = x1;
   r.lr.y = y1;
   for (i = 0; i < mfd_num_updates; i++)
      if (RECT_TEST_SECT(&mfd_update_list[i],&r))
      {
         // If we intersect with some existing rect, union it in to r and 
         // Delete it from the list
         RECT_UNION(&mfd_update_list[i],&r,&r);
         if (i != mfd_num_updates-1)
            mfd_update_list[i] = mfd_update_list[mfd_num_updates-1];
         mfd_num_updates--;
         i = 0; // We might now intersect a previous one.  
      }
   if (mfd_num_updates >= NUM_MFD_RECTS)
      return ERR_DOVERFLOW;
   mfd_update_list[mfd_num_updates++] = r;
   return OK;
}

// -------------------------------------------------------------------
//
// mfd_update_rects() Updates all rects in the update list, then clears
// the update list. 

void mfd_update_rects(MFD* m)
{
   int i;
   for(i = 0; i < mfd_num_updates; i++)
   {
      LGRect* r = &mfd_update_list[i];
      // Filter out degenerate rects!
      if ((r->lr.x <= r->ul.x) || (r->lr.y <= r->ul.y))
         continue;
      mfd_update_display(m,r->ul.x,r->ul.y,r->lr.x,r->lr.y);
   }
   mfd_num_updates = 0;
}


// --------------------------------------------------------------------------
// mfd_update_display()
//
// Updates a portion of the view window from canvas.

void mfd_update_display(MFD *m, short x0, short y0, short x1, short y1)
{
   ushort a,b,c,d;
   uchar old_over = gr2ss_override;

   if (!full_game_3d)
   {
      LGRect r;

      if ((x0 > x1) || (y0 > y1))
         return;

      r.ul.x = x0;
      r.ul.y = y0;
      r.lr.x = x1;
      r.lr.y = y1;

      RECT_OFFSETTED_RECT(&r,m->rect.ul,&r);
      if (!RECT_TEST_SECT(&r,&m->rect)) return; 
      RectSect(&r,&m->rect,&r);

      gr_push_canvas(grd_screen_canvas);
      uiHideMouse(&r);
      STORE_CLIP(a,b,c,d);
      gr2ss_override = OVERRIDE_ALL;
      ss_safe_set_cliprect(r.ul.x,r.ul.y,r.lr.x,r.lr.y);
      ss_noscale_bitmap(&(pmfd_canvas->bm),m->rect.ul.x,m->rect.ul.y);

      gr2ss_override = old_over;
      uiShowMouse(&r);
      RESTORE_CLIP(a,b,c,d);
      gr_pop_canvas();
   }
  
   return;
}


// ***********************************************
// **** SAVE/RESTORE and DEFAULT MFD MANAGER *****
// ***********************************************

// for saving and restoring mfd settings around "panel" mfd's, we
// use our secret 6th slot.

typedef bool (*mfd_def_qual)(void);

extern bool mfd_target_qual(void);
extern bool mfd_automap_qual(void);
extern bool mfd_weapon_qual(void);

typedef struct {
   uchar          func;
   uchar          slot;
   mfd_def_qual   qual;
} mfd_default;

mfd_default default_mfds[] = {
   { MFD_MAP_FUNC, MFD_MAP_SLOT, &mfd_automap_qual },
   { MFD_WEAPON_FUNC, MFD_WEAPON_SLOT, &mfd_weapon_qual },
   { MFD_TARGET_FUNC, MFD_TARGET_SLOT, &mfd_target_qual }
};

#define NUM_MFD_DEFAULTS (sizeof(default_mfds)/sizeof(mfd_default))

void mfd_default_mru( uchar func )
{
   int i, pos=-1;
   mfd_default tmp;

   for( i=0; i<NUM_MFD_DEFAULTS; i++ ) {
      if(default_mfds[i].func == func) {
         pos=i;
         tmp=default_mfds[i];
         break;
      }
   }
   if(pos<0) return; // func not in default list

   for( i=pos; i>0; i--)
      default_mfds[i]=default_mfds[i-1];
   default_mfds[0]=tmp;
}


void save_mfd_slot(int mfd_id)
{
	uchar func,mask;
	int slot;
	
	if (global_fullmap->cyber)
		return;
	if (player_struct.mfd_save_slot[mfd_id] < 0)
	{
		slot = player_struct.mfd_current_slots[mfd_id];
		
		func = mfd_get_func(mfd_id,slot);
		if (!(mfd_funcs[func].flags & MFD_NOSAVEREST))
		{
			player_struct.mfd_save_slot[mfd_id] = slot;
			set_slot_to_func(MFD_SPECIAL_SLOT+mfd_id,func,MFD_ACTIVE);
			if (full_game_3d)
			{
				mask=visible_mask(mfd_id);
				player_struct.mfd_save_vis&=~mask;
				player_struct.mfd_save_vis|=(mask&full_visible);
			}
		}
	}
}


// sets mfd to given slot and func, unless the func passed in is MFD_EMPTY_FUNC
// or some MFD already has that func.  In that case, set to some other slot
// from a list of hopefully useful defaults. 
// note that if you pass in MFD_EMPTY_FUNC, a new setting is always selected,
// so the slot argument is ignored.

void set_mfd_from_defaults(int mfd_id, uchar func, uchar slot)
{
	uchar def, mid;
	bool check;
	
	def=0;
	do
	{
		check=FALSE;
		for (mid=0;mid<NUM_MFDS;mid++)
		{
			if(func==MFD_EMPTY_FUNC || func==mfd_get_func(mid,player_struct.mfd_current_slots[mid]))
			{
				// don't restore func that we already have on some mfd.
				if(default_mfds[def].qual())
				{
					func=default_mfds[def].func;
					slot=default_mfds[def].slot;
				}
				def++;
				check=TRUE;
				break;
			}
		}
	} while(check && def<NUM_MFD_DEFAULTS);

	if (func==MFD_EMPTY_FUNC)
		slot = (global_fullmap->cyber) ? MFD_INFO_SLOT : MFD_ITEM_SLOT;  // failure case
	mfd_notify_func(func,slot,TRUE,MFD_ACTIVE,TRUE);
	if(!full_game_3d || (full_visible & FULL_MFD_MASK(mfd_id)))
		mfd_change_slot(mfd_id,slot);
}

// scans throught the mfd's looking for mfd's that are set to the given slot/func.
// once it have found max such mfd's, starts setting any subsequent mfd's
// with that func to defaults as above.

void cap_mfds_with_func(uchar func, uchar max)
{
   int mid;

   for(mid=0;mid<NUM_MFDS;mid++) {
      if(mfd_get_func(mid,player_struct.mfd_current_slots[mid])==func) {
         if(max==0)
            restore_mfd_slot(mid);
         else
            max--;
      }
   }
}

void restore_mfd_slot(int mfd_id)
{
   uchar func, slot;
   if(global_fullmap->cyber) return;
   if(full_game_3d && !(visible_mask(mfd_id)&full_visible)) return;
   if(player_struct.mfd_save_slot[mfd_id]<0)
   {
      func = MFD_EMPTY_FUNC;
      slot = MFD_INFO_SLOT;
   }
   else
   {
      func = player_struct.mfd_all_slots[MFD_SPECIAL_SLOT+mfd_id];
      slot = player_struct.mfd_save_slot[mfd_id];
   }

   set_mfd_from_defaults(mfd_id,func,slot);
   player_struct.mfd_save_slot[mfd_id] = -1;
   full_visible&=~(visible_mask(mfd_id));
#ifdef STEREO_SUPPORT
   if (convert_use_mode == 5)
      full_visible = (player_struct.mfd_save_vis&visible_mask(mfd_id));
   else
#endif
   {
      full_visible|=(player_struct.mfd_save_vis&visible_mask(mfd_id));
   }
}
