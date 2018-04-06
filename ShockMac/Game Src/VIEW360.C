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
 * $Source: r:/prj/cit/src/RCS/view360.c $
 * $Revision: 1.33 $
 * $Author: xemu $
 * $Date: 1994/10/27 04:53:06 $
 */

#include <string.h>

#include "frprotox.h"
#include "frcamera.h"
#include "frflags.h"
#include "invent.h"
#include "mfdint.h"
#include "mfdext.h"
#include "mfddims.h"
#include "wares.h"
#include "mainloop.h"
#include "gameloop.h"
#include "tools.h"
#include "frflags.h"
#include "musicai.h"
#include "sfxlist.h"
#include "objsim.h"
#include "faketime.h"
#include "gamestrn.h"
#include "colors.h"
#include "fullscrn.h"
#include "invpages.h"
#include "view360.h"
#include "gr2ss.h"

#include "otrip.h"
#include "cybstrng.h"
#include "gamescr.h"

extern bool dirty_inv_canvas;

// -------
// GLOBALS
// -------
frc* view360_contexts[NUM_360_CONTEXTS];         // the renderer contexts for each view window
frc* view360_fullscreen_contexts[NUM_360_CONTEXTS];
#define CONTEXT ((full_game_3d) ? view360_fullscreen_contexts : view360_contexts)
bool view360_active_contexts[NUM_360_CONTEXTS]; // which contexts should actually draw
#define ACTIVE view360_active_contexts
uchar view360_context_views[NUM_360_CONTEXTS];  // which view is being shown by a given context
#define VIEW view360_context_views 

bool view360_message_obscured = FALSE;
bool view360_render_on        = FALSE;
short view360_last_update = 0;
bool view360_is_rendering = FALSE;

// ---------
// INTERNALS
// ---------
void view360_setup_mode(uchar mode);
void view360_restore_inventory(void);
int view360_fullscrn_draw_callback(void* , void* vbm, int x, int y, int flg);
bool inv_is_360_view(void);
void view360_init(void);
void view360_shutdown(void);
void view360_update_screen_mode(void);
void view360_render(void);
void mfd_view360_expose(MFD* mfd, ubyte control);
void view360_turnon(bool visible, bool real_start);
void view360_turnoff(bool visible,bool real_stop);
bool view360_check(void);


// Set up/turn on all contexts & cameras for the specified mode.
void view360_setup_mode(uchar mode)
{
   ubyte version = player_struct.hardwarez[CPTRIP(SENS_HARD_TRIPLE)];
   if (version > 2 && mode == MODE_360 || mode == MODE_270)
   {
      VIEW[LEFT_CONTEXT] =  CAMANG_LEFT;
      ACTIVE[LEFT_CONTEXT] = TRUE;
      VIEW[RIGHT_CONTEXT] = CAMANG_RIGHT;
      ACTIVE[RIGHT_CONTEXT] = TRUE;
      mfd_notify_func(MFD_3DVIEW_FUNC,MFD_INFO_SLOT,TRUE,MFD_ACTIVE,FALSE);
      mfd_change_slot(MFD_LEFT,MFD_INFO_SLOT);
      mfd_change_slot(MFD_RIGHT,MFD_INFO_SLOT);
   }
   if (mode == MODE_360)
   {
      inventory_clear();
      VIEW[MID_CONTEXT] = CAMANG_BACK;
      ACTIVE[MID_CONTEXT] = TRUE;
      inv_last_page = inventory_page;
      inventory_page = INV_3DVIEW_PAGE;
   }
   if (mode == MODE_REAR)
   {
      int mfd;
      for (mfd = 0; mfd < NUM_MFDS; mfd++)
      {
         if (mfd_get_func(mfd,player_struct.mfd_current_slots[mfd]) == MFD_3DVIEW_FUNC)
            break;
      }
      if (mfd >= NUM_MFDS)
         mfd = mfd_grab();
      VIEW[mfd] = CAMANG_BACK;
      ACTIVE[mfd] = TRUE;
      mfd_notify_func(MFD_3DVIEW_FUNC,MFD_INFO_SLOT,TRUE,MFD_ACTIVE,FALSE);
      mfd_change_slot(mfd,MFD_INFO_SLOT);
   }
}

void view360_restore_inventory()
{
   if (_current_loop == GAME_LOOP)
   {
      extern void inv_change_fullscreen(bool on);
      chg_set_flg(INVENTORY_UPDATE);
      inv_change_fullscreen(full_game_3d);
      view360_message_obscured = FALSE;
      inv_last_page = INV_BLANK_PAGE;
      message_info(""); // This should be NULL as soon as the 2d can handle it.
   }
   ACTIVE[MID_CONTEXT] = FALSE;
}

// what/where are these???
extern grs_canvas _offscreen_mfd, _fullscreen_mfd, inv_view360_canvas;

static bool rendered_inv_fullscrn = FALSE;

extern void shock_hflip_in_place(grs_bitmap* bm);

int view360_fullscrn_draw_callback(void*, void* vbm, int, int, int)
{
//KLC   shock_hflip_in_place((grs_bitmap *)vbm);
   return FALSE;
}


// ---------
// EXTERNALS
// ---------

bool inv_is_360_view(void)
{
   return ACTIVE[MID_CONTEXT];
}

// then or in with palette
//#define VIEW360_BASEFR (FR_DOUBLEB_MASK|FR_DOHFLIP_MASK)
#define VIEW360_BASEFR (FR_DOUBLEB_MASK)

void view360_init(void)
{
   frc* c;
   uchar *canv;
   short x,y,w,h;
   
   canv=_offscreen_mfd.bm.bits;
   x = MFD_VIEW_LFTX;
   y = MFD_VIEW_Y;
   w = MFD_VIEW_WID;
   h = MFD_VIEW_HGT;
#ifdef SVGA_SUPPORT
   ss_point_convert(&x,&y,FALSE);
   ss_point_convert(&w,&h,FALSE);
   h = min(h, 137);
#endif
   view360_contexts[LEFT_CONTEXT]  = fr_place_view(FR_NEWVIEW,FR_DEFCAM,canv,VIEW360_BASEFR|FR_CURVIEW_LEFT,0,0,x,y,w,h);
   c = view360_fullscreen_contexts[LEFT_CONTEXT] = fr_place_view(FR_NEWVIEW,FR_DEFCAM,canv,VIEW360_BASEFR|FR_CURVIEW_LEFT,0,0,x,y,w,h);
   fr_set_callbacks(c,view360_fullscrn_draw_callback,NULL,NULL);
   
   x = MFD_VIEW_RGTX;
   y = MFD_VIEW_Y;
   w = MFD_VIEW_WID;
   h = MFD_VIEW_HGT;
#ifdef SVGA_SUPPORT
   ss_point_convert(&x,&y,FALSE);
   ss_point_convert(&w,&h,FALSE);
   h = min(h, 137);
#endif
   view360_contexts[RIGHT_CONTEXT] = fr_place_view(FR_NEWVIEW,FR_DEFCAM,canv,VIEW360_BASEFR|FR_CURVIEW_RGHT,0,0,x,y,w,h);
   canv=_fullscreen_mfd.bm.bits;
   c = view360_fullscreen_contexts[RIGHT_CONTEXT] = fr_place_view(FR_NEWVIEW,FR_DEFCAM,canv,VIEW360_BASEFR|FR_CURVIEW_RGHT,0,0,x,y,w,h);
   fr_set_callbacks(c,view360_fullscrn_draw_callback,NULL,NULL);
   
   x = GAME_MESSAGE_X;
   y = GAME_MESSAGE_Y;
   w = INV_FULL_WD;
   h = INV_FULL_HT;
#ifdef SVGA_SUPPORT
   ss_point_convert(&x,&y,FALSE);
   ss_point_convert(&w,&h,FALSE);
#endif
   canv=inv_view360_canvas.bm.bits;
   view360_contexts[MID_CONTEXT]   = fr_place_view(FR_NEWVIEW,FR_DEFCAM,canv,VIEW360_BASEFR|FR_CURVIEW_BACK,0,REAR_FOV,x,y,w,h);
   c = view360_fullscreen_contexts[MID_CONTEXT] = fr_place_view(FR_NEWVIEW,FR_DEFCAM,canv,VIEW360_BASEFR|FR_CURVIEW_BACK,0,REAR_FOV,x,y,w,h);
   fr_set_callbacks(c,view360_fullscrn_draw_callback,NULL,NULL);
}

void view360_shutdown(void)
{
   int i;
   for (i=LEFT_CONTEXT; i<=MID_CONTEXT; i++)
   {
      fr_free_view(view360_contexts[i]);
      fr_free_view(view360_fullscreen_contexts[i]);
   }
}

void view360_update_screen_mode()
{
   view360_shutdown();
   view360_init();
}


char update_string[30] = "";

void view360_render(void)
{
   bool on = FALSE;
   int i;
   if (inventory_page != INV_3DVIEW_PAGE && ACTIVE[MID_CONTEXT])
   {
      view360_restore_inventory();
   }
   view360_message_obscured = ACTIVE[MID_CONTEXT];
  if (ACTIVE[MID_CONTEXT] && player_struct.hardwarez[CPTRIP(SENS_HARD_TRIPLE)] == 1)
   {
      short update = *tmd_ticks/CIT_CYCLE;
      if (dirty_inv_canvas && update == view360_last_update && (full_game_3d || !rendered_inv_fullscrn))
      {
         short basex = INVENTORY_PANEL_X;
         short basey = INVENTORY_PANEL_Y + INVENTORY_PANEL_HEIGHT;
         LGRect r;
         char buf[sizeof(update_string)];
         short w,h;
         if (strlen(update_string) + 1 >= sizeof(update_string))
            return;
         if (update_string[0] == '\0')
            get_string(REF_STR_View360Update,buf,sizeof(buf));
         else strcpy(buf,".");
         if (full_game_3d)
         {
            basex = 0;
            basey = INV_FULL_HT;
            gr_push_canvas(&inv_view360_canvas);
         }
         else
         {
            gr_push_canvas(grd_screen_canvas);
         }
         gr_set_fcolor(WHITE);
         gr_set_font((grs_font*)ResLock(RES_tinyTechFont));
         gr_string_size(update_string,&w,&h);
         r.ul.x = basex+w+2;
         r.ul.y = basey-h-2;
         r.lr.x = r.ul.x + gr_string_width(buf);
         r.lr.y = r.ul.y + h;
         if (!full_game_3d)
            uiHideMouse(&r);
         res_draw_text(RES_tinyTechFont,buf,r.ul.x,r.ul.y);
         if (!full_game_3d)
            uiShowMouse(&r);
         ResUnlock(RES_tinyTechFont);
         gr_pop_canvas();
         strcat(update_string,buf);
         return;
      }
      update_string[0] = '\0';
      view360_last_update = update;
      rendered_inv_fullscrn = FALSE;
   }
   if (ACTIVE[MID_CONTEXT])
   {
      dirty_inv_canvas = TRUE;
   }
   
	// Render the 360 view scenes.
	
	view360_is_rendering = TRUE;	
	for (i = 0; i < NUM_360_CONTEXTS; i++)
	if (ACTIVE[i])
	{
		fr_rend(CONTEXT[i]);
		if (full_game_3d)
		{
#ifdef STEREO_SUPPORT
			if (convert_use_mode == 5)
				full_visible = VISIBLE_BIT(i);
			else
#endif
			full_visible |= VISIBLE_BIT(i);
		}
		on = TRUE;
	}
	view360_is_rendering = FALSE;
	view360_render_on = on;
   
   if (on == !(player_struct.hardwarez_status[HARDWARE_360] & WARE_ON))
      use_ware(WARE_HARD,HARDWARE_360);
}

// ------------------
// MFD FUNC FOR VIEWS
// ------------------

void mfd_view360_expose(MFD* mfd, ubyte control)
{
   ACTIVE[mfd->id] = control;
}


// --------------------------
// WARE STARTUP/SHUTDOWN CODE
// --------------------------

#define MODE_MASK 0xC0
#define MODE_SHF  6
#define VIEW_MODE(s) (((s) & MODE_MASK) >> MODE_SHF)
#define VIEW_MODE_SET(s,v) ((s) = ((s) & ~MODE_MASK) | ((v) << MODE_SHF))

void view360_turnon(bool visible, bool)
{
   int s = player_struct.hardwarez_status[HARDWARE_360];

   if (visible)
   {
      view360_setup_mode(VIEW_MODE(s));
      chg_set_flg(_current_3d_flag);
   }
   view360_render_on = TRUE;

}

void view360_turnoff(bool visible,bool real_stop)
{
   int i;
   // restore inventory
   if (visible)
   {
      if (real_stop && ACTIVE[MID_CONTEXT])
      {
         inventory_page = inv_last_page;
         if (inventory_page < 0) inventory_page = 0;
         if (full_game_3d)
         {
            full_visible &= ~VISIBLE_BIT(MID_CONTEXT);
         }
         else
         {
            view360_restore_inventory();
            inventory_clear();
            inventory_draw();
         }
      }
      // empty the mfd slot
      if (ACTIVE[LEFT_CONTEXT] || ACTIVE[RIGHT_CONTEXT] && real_stop)
         mfd_notify_func(MFD_EMPTY_FUNC,MFD_INFO_SLOT,TRUE,MFD_EMPTY,FALSE);
      // turn off all views
      for (i = 0; i < NUM_360_CONTEXTS; i++)
      {
         ACTIVE[i] = FALSE;
      }
      if (real_stop)
         play_digi_fx(SFX_VIDEO_DOWN, 1);
   }
   view360_render_on = view360_message_obscured   =  FALSE;
}
      
bool view360_check()
{
   extern uchar hack_takeover;
   if (hack_takeover)
     return(FALSE);
   return(TRUE);
}
