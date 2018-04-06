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
 * $Source: r:/prj/cit/src/RCS/screen.c $
 * $Revision: 1.95 $
 * $Author: mahk $
 * $Date: 1994/11/22 22:53:04 $
 */

// Source code for the Citadel Screen routines

#define __SCREEN_SRC

#include "criterr.h"
#include "screen.h"
#include "tools.h"
#include "gamescr.h"
#include "mfdext.h"
#include "sideicon.h"
#include "status.h"
#include "statics.h"
#include "loops.h"
#include "input.h"
#include "colors.h"
#include "palfx.h"
#include "faketime.h"
#include "init.h"
#include "mainloop.h"
#include "frprotox.h"
#include "frflags.h"
#include "rendtool.h"
#include "render.h"
#include "cybmem.h"
#include "citres.h"
#include "gr2ss.h"
#include "invent.h"
#include "invdims.h"

/*
KLC - stereo
#include "LG_menus.h"	// can't be just "menus" since thats a mac include
#include "config.h"
#include "inp6d.h"
#include "i6dvideo.h"
*/

#ifdef CURSOR_BACKUPS
#include "loopdbg.h"
#include "string.h"
#endif

#define CFG_TIME_VAR "time_passes"

LGRect Inv_rect;

#ifdef SVGA_SUPPORT
extern grs_screen *svga_screen;
extern frc *svga_render_context;
extern short svga_mode_data[];
extern short mode_id;
#endif

LGRect *inventory_rect = &Inv_rect, *status_rect, mess_rect;
LGRect real_status_rect;
LGRegion mv_region_data, msg_region_data, status_region_data;
uiSlab main_slab;
LGRegion *msg_region;

errtype _screen_init_mouse(LGRegion* r, uiSlab* slab, bool do_init);
errtype _screen_background(void);

extern void mouse_unconstrain(void);
byte pal_shf_id;
LGCursor vmail_cursor;

LGRect fscrn_rect={{0,0},{320,200}};
LGRect svga_rect = {{0,0},{1024,768}};

LGRegion root_region_data;
LGRegion *root_region=&root_region_data;	

// prototypes
errtype load_misc_cursors(void);

void generic_reg_init(bool create_it, LGRegion *reg, LGRect *rct, uiSlab *slb, void *key_h, void *maus_h)
{
   int callid;
   if (rct==NULL)
     rct=&fscrn_rect;
   if (create_it)
	   region_create(NULL, reg, rct, 0, 0, REG_USER_CONTROLLED|AUTODESTROY_FLAG, NULL, NULL, NULL, NULL);
   if (key_h!=NULL)
	   uiInstallRegionHandler(reg,UI_EVENT_KBD_COOKED, (uiHandlerProc) key_h, NULL, &callid);
   if (maus_h!=NULL)
	   uiInstallRegionHandler(reg,UI_EVENT_MOUSE, (uiHandlerProc) maus_h, NULL, &callid);
   if (slb!=NULL)
   {
	   uiMakeSlab(slb, reg, &globcursor);
	   uiGrabSlabFocus(slb,reg, ALL_EVENTS);
   }
}

// Initialize the main game screen and draw it's initial state
LGRect mainview_rect;
errtype screen_init(void)
{
   extern void init_posture_meters(LGRegion*,bool);
   extern errtype wrapper_create_mouse_region(LGRegion*);
   extern LGRegion *fullview_region;
   int callid;
   extern void (*ui_mouse_convert)(short *px, short *py,bool down);
   extern void (*ui_mouse_convert_round)(short *px, short *py,bool down);

   // God this is stupid, maybe I'll get it right next project
   status_rect = &real_status_rect;

   // Create all the appropriate regions for to make input happen
   // Root LGRegion
   generic_reg_init(TRUE ,root_region,NULL,NULL,NULL,NULL);

   // Main view LGRegion
   mainview_rect.ul.x = SCREEN_VIEW_X;   mainview_rect.ul.y = SCREEN_VIEW_Y;
   mainview_rect.lr.x = mainview_rect.ul.x + SCREEN_VIEW_WIDTH;
   mainview_rect.lr.y = mainview_rect.ul.y + SCREEN_VIEW_HEIGHT;
   mainview_region = &mv_region_data;
   region_create(root_region, mainview_region, &mainview_rect, 0, 0, REG_USER_CONTROLLED|AUTODESTROY_FLAG, NULL, NULL, NULL, NULL);

   // Initialize da mousie
   _screen_init_mouse(root_region, &main_slab, TRUE);	// KLC - moved here

   install_motion_mouse_handler(mainview_region, NULL);

#ifdef SVGA_SUPPORT
   gr2ss_register_init(0,320,200);
   gr2ss_register_mode(0,320,400);
   gr2ss_register_mode(0,640,400);
   gr2ss_register_mode(0,640,480);
//   gr2ss_register_mode(0,1024,768);
#ifdef STEREO_SUPPORT
   if (i6d_device == I6D_VFX1)
   {
     Warning(("size = %d, %d!\n",i6d_ss->scr_w,i6d_ss->scr_h));
     gr2ss_register_mode(0,320,240);
     gr2ss_register_mode(0,640,240);     // VFX Hack Mode
	 }
   else
   {
      gr2ss_register_mode(0,320,100); // note secret stereo mode
      gr2ss_register_mode(0,640,350);     // CTM Hack Mode
   }
#endif
#endif

   // Install mouse converter...
   ui_mouse_convert = ss_mouse_convert;
   ui_mouse_convert_round = ss_mouse_convert_round;

   // Inventory LGRegion
   create_invent_region(root_region, &pagebutton_region_game, &inventory_region_game);
   screen_init_mfd(FALSE);        // sets up regions + mouse callbacks
   screen_init_side_icons(root_region);


   // Message-line LGRegion
   mess_rect.ul.x = GAME_MESSAGE_X;
   mess_rect.ul.y = GAME_MESSAGE_Y;
   mess_rect.lr.x = mess_rect.ul.x + INVENTORY_PANEL_WIDTH;
   mess_rect.lr.y = mess_rect.ul.y + 10;
   msg_region = &msg_region_data;
   region_create(root_region, msg_region, &mess_rect, 0, 0, REG_USER_CONTROLLED|AUTODESTROY_FLAG, NULL, NULL, NULL, NULL);

   // Status LGRegion
   status_rect->ul.x = STATUS_X; status_rect->ul.y = STATUS_Y;
   status_rect->lr.x = status_rect->ul.x + STATUS_WIDTH;
   status_rect->lr.y = status_rect->ul.y + STATUS_HEIGHT;
   status_region = &status_region_data;
   region_create(root_region, status_region, status_rect, 0, 0, REG_USER_CONTROLLED|AUTODESTROY_FLAG, NULL, NULL, NULL, NULL);

   status_bio_init();

   // lean-o-meter
   init_posture_meters(root_region,FALSE);

/* KLC
   // option LGCursor LGRegion
   wrapper_create_mouse_region(root_region);
 */
   // Install basic input handlers
   uiInstallRegionHandler(root_region,UI_EVENT_KBD_COOKED, &main_kb_callback, NULL, &callid);

   install_motion_keyboard_handler(root_region);   

   // we already do this in generic_reg_init
   // Grab all input!
   uiGrabSlabFocus(&main_slab, root_region, ALL_EVENTS);

   return(OK);
}

extern void game_redrop_rad(int rad_mod);

void screen_start()
{
   extern errtype load_da_palette();
   extern LGRegion *pagebutton_region, *inventory_region;
#ifdef SVGA_SUPPORT
   extern void change_svga_screen_mode();
#endif

/* ��� Not yet
   // Check the config system to see if time should automatically be running
   if (config_get_raw(CFG_TIME_VAR, NULL, 0)) time_passes = TRUE;
*/
  
	HotkeyContext = DEMO_CONTEXT;
	uiSetCurrentSlab(&main_slab);
	inventory_region = inventory_region_game;
	pagebutton_region = pagebutton_region_game;
	
	// A rather strange function for a Mac program, but we'll keep it.
	change_svga_screen_mode();

   gr_clear(0xFF);
   status_bio_set(GAMESCR_BIO);
   screen_draw();
//KLC   uiShowMouse(NULL);
   chg_set_flg(DEMOVIEW_UPDATE);
   chg_set_flg(MFD_UPDATE);
   chg_set_flg(VITALS_UPDATE);
   status_bio_start();
   status_vitals_update(TRUE);
// KLC - not needed anymore   mouse_unconstrain();
#ifdef PALFX_FADES
//��� later   if (pal_fx_on) palfx_fade_up(FALSE);
#endif
   return;
}

void screen_exit()
{
   extern wrapper_panel_close(); 
#ifdef SVGA_SUPPORT
   uchar cur_pal[768];
   extern grs_screen *cit_screen;
   uchar *s_table;
#endif

   status_bio_end();
   uiHideMouse(NULL);

#ifdef SVGA_SUPPORT
   if ((_new_mode != GAME_LOOP) && (_new_mode != FULLSCREEN_LOOP))
   {
      extern void change_svga_cursors();
      extern void status_bio_update_screenmode();

      s_table = gr_get_light_tab();
      gr_get_pal(0,256,&cur_pal[0]);
      gr_set_screen(cit_screen);
      convert_use_mode = 0;
//KLC      change_svga_cursors();
//KLC      status_bio_update_screenmode();
   }
#endif
   if (_new_mode == -1)
      return;

/* KLC
#ifdef PALFX_FADES
   if (pal_fx_on) 
      palfx_fade_down();
   else {
      gr_set_fcolor(BLACK);
      gr_rect(0,0,320,200);
   }
#endif

#ifdef SVGA_SUPPORT
   if ((_new_mode != GAME_LOOP) && (_new_mode != FULLSCREEN_LOOP))
   {
      gr_set_pal(0,256,&cur_pal[0]);
      gr_set_light_tab(s_table);
   }
#endif
*/
}

// Draw the whole durned screen!
// Note that this algorithm does NOT draw the main view area,
// or the associated HUD -- 
// that is handled by direct calls from the main loop

errtype screen_draw(void)
{
   extern void update_meters(bool);
   // Just go through and draw all the component parts....
   // In theory, they should all be clever enough to redraw
   // efficiently.  Alternatively, this should only be called 
   // very few times, and in general just the changing parts
   // get a signal to draw themselves.
   uiHideMouse(NULL);
   _screen_background();

   screen_init_mfd_draw();
   side_icon_expose_all();

   inventory_clear();
   inventory_draw();
   status_bio_draw();
   status_vitals_init();
   status_vitals_update(TRUE);
   update_meters(TRUE);
   uiShowMouse(NULL);

   return(OK);
}

errtype _screen_background(void)
{
//KLC - chg for new art   Ref back_id = REF_IMG_bmGamescreenBackground;
//KLC - chg for new art   draw_raw_res_bm_extract(back_id, 0, 0);
	draw_hires_resource_bm(REF_IMG_bmGamescreenBackground, 0, 0);
	return(OK);
}

// Stop doing graphics things
errtype screen_shutdown(void)
{
   region_destroy(status_region, FALSE);
   region_destroy(msg_region, FALSE);
   region_destroy(mainview_region, FALSE);

//   Free(status_rect); umm, see, now we point at it, so dont free it
   return(OK);
}

static grs_bitmap _targbm;
static grs_bitmap _waitbm;
static grs_bitmap _firebm;
//static grs_bitmap _vmailbm;
extern grs_bitmap slider_cursor_bmap;
extern LGCursor slider_cursor;

errtype load_misc_cursors(void)
{
   static bool misc_cursors_loaded = FALSE;

   if (misc_cursors_loaded)
   {
      if (_targbm.bits != NULL)  DisposPtr((Ptr) _targbm.bits);
      if (_waitbm.bits != NULL)  DisposPtr((Ptr) _waitbm.bits);
      if (_firebm.bits != NULL)  DisposPtr((Ptr) _firebm.bits);
//      if (_vmailbm.bits != NULL)  DisposPtr((Ptr) _vmailbm.bits);
      if (slider_cursor_bmap.bits != NULL)  DisposPtr((Ptr) slider_cursor_bmap.bits);
   }
   load_hires_bitmap_cursor(&globcursor,   &_targbm,  REF_IMG_bmTargetCursor, TRUE);
   load_hires_bitmap_cursor(&wait_cursor,  &_waitbm,  REF_IMG_bmWaitCursor, TRUE);
   load_hires_bitmap_cursor(&fire_cursor,  &_firebm,  REF_IMG_bmFireCursor, TRUE);
//   load_hires_bitmap_cursor(&vmail_cursor, &_vmailbm, REF_IMG_bmVmailCursor, TRUE);
   load_hires_bitmap_cursor(&slider_cursor, &slider_cursor_bmap, REF_IMG_bmMfdPhaserCursor, TRUE);
   misc_cursors_loaded = TRUE;
   return(OK);
}

errtype _screen_init_mouse(LGRegion* r, uiSlab* slab, bool do_init)
{
   extern errtype ui_init_cursors(void);
   
   ui_init_cursors();			// KLC - do this here, take out of uiInit.
   load_misc_cursors();

   // Entirely arbitrarily, screen does the uiInit.
   // only one of the slab creators needs to do this.
   uiMakeSlab(slab,r,&globcursor);
   if (do_init)
      uiInit(slab);
#ifdef INPUT_CHAINING
/* ����do we ever need this?
   if (config_get_raw(CHAINING_VAR,NULL,0))
      kb_set_flags(kb_get_flags()|KBF_CHAIN);*/
#endif // INPUT_CHAINING

   uiHideMouse(NULL);
// KLC - no longer needed   if (mouse_put_xy(100,100) == ERR_NODEV)
//                            critical_error(CRITERR_CFG|0);
   return(OK);
}

