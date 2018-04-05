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
 * $Source: r:/prj/cit/src/RCS/fullscrn.c $
 * $Revision: 1.73 $
 * $Author: dc $
 * $Date: 1994/11/22 20:16:55 $
 */


#define __FULLSCRN_SRC

#include "ShockBitmap.h"
#include "Prefs.h"

#include "fullscrn.h"
#include "colors.h"
#include "criterr.h"
#include "cybstrng.h"
#include "frflags.h"
#include "frprotox.h"
#include "FrUtils.h"
#include "gameloop.h"
#include "gr2ss.h"
#include "hud.h"
#include "input.h"
#include "invent.h"
#include "mainloop.h"
#include "mfdext.h"
#include "miscqvar.h"
#include "objprop.h"
#include "otrip.h"
#include "palfx.h"
#include "rendtool.h"
#include "screen.h"
#include "sideicon.h"
#include "status.h"
#include "textmaps.h"
#include "tools.h"
#include "wares.h"

#ifdef NOT_YET //KLC stereo 

#include <config.h>

#ifdef STEREO_SUPPORT
#include <inp6d.h>
#include <i6dvideo.h>
#endif

#endif //NOT_YET


// -------
// GLOBALS
// -------
bool fullscrn_vitals = TRUE;
bool fullscrn_icons = TRUE;

extern bool inp6d_stereo_active;
extern bool inp6d_stereo;

#ifdef SVGA_SUPPORT
grs_screen *svga_screen=NULL;
frc *svga_render_context=NULL;
short svga_mode_data[] =
{ GRM_320x200x8, GRM_320x400x8, GRM_640x400x8, GRM_640x480x8, GRM_1024x768x8,
   GRM_320x200x8 };
char mickey_stupid[][2]=
{ {16,8},{16,4},{3,1},{2,1},{3,1},{16,8} };
short mode_id=3;  //KLC - start off in 640x480 in Mac version      old -  short mode_id=0;
#endif

#ifdef GADGET
#include <gadgets.h>
Gadget *fullroot_gadget;
#endif
uiSlab fullscreen_slab;

#define CFG_TIME_VAR "time_passes"

extern void status_bio_update_screenmode();
extern void mouse_unconstrain(void);
extern void olh_svga_deal(void);
extern void inv_change_fullscreen(bool on);
extern void mfd_change_fullscreen(bool on);
extern void game_redrop_rad(int rad_mod);
void change_svga_cursors();
void change_svga_screen_mode();


LGRegion fullroot_region_data, fullview_region_data;
LGRegion *fullroot_region=&fullroot_region_data;  // DUH

short base_mouse_xr,base_mouse_yr,base_mouse_thresh;

errtype fullscreen_init(void)
{
   extern void init_posture_meters(LGRegion*,bool);
   extern errtype wrapper_create_mouse_region(LGRegion*);
   extern LGRect fscrn_rect;

   generic_reg_init(TRUE ,fullroot_region,NULL,&fullscreen_slab,main_kb_callback,NULL);

   // Full-screen 3d view region
   fullview_region = &fullview_region_data;
   region_create(fullroot_region, fullview_region, &fscrn_rect, 1, 0, REG_USER_CONTROLLED|AUTODESTROY_FLAG, NULL, NULL, NULL, NULL);

   install_motion_mouse_handler(fullview_region, NULL);
   install_motion_keyboard_handler(fullroot_region);

//KLC - no wrapper in Mac   wrapper_create_mouse_region(fullview_region);
   create_invent_region(fullview_region, &pagebutton_region_full, &inventory_region_full);
   init_posture_meters(fullview_region, TRUE);
   screen_init_mfd(TRUE);
   screen_init_side_icons(fullview_region);

//   mouse_get_rate(&base_mouse_xr, &base_mouse_yr, &base_mouse_thresh);
//   base_mouse_xr = 16;
//   base_mouse_yr = 8;
//   base_mouse_thresh = 100;
   base_mouse_xr = 8;
   base_mouse_yr = 4;
   base_mouse_thresh = 100;

   full_visible = FULL_INVENT_MASK | FULL_L_MFD_MASK | FULL_R_MFD_MASK;

   return(OK);
}


// Draw all relevant overlays
errtype fullscreen_overlay()
{
   extern char last_message[128];
   extern void mfd_draw_button_panel(ubyte mfd_id);
   extern void fullscreen_refresh_mfd(ubyte mfd_id);
   extern void inv_update_fullscreen(bool full);
   extern bool game_paused;

   if (!global_fullmap->cyber)
   {
      mfd_draw_button_panel(MFD_RIGHT);
      mfd_draw_button_panel(MFD_LEFT);
   }
   fullscreen_refresh_mfd(MFD_RIGHT);
   if (global_fullmap->cyber)
      full_visible &= ~FULL_MFD_MASK(MFD_LEFT);
   fullscreen_refresh_mfd(MFD_LEFT);
   if (!game_paused)
      inv_update_fullscreen((full_visible & FULL_INVENT_MASK) != 0);
   if (fullscrn_vitals)
   {
      extern void update_meters(bool);
      status_vitals_update(TRUE);
      if (!global_fullmap->cyber)
         update_meters(TRUE);
   }
   if ((!global_fullmap->cyber) && (fullscrn_icons))
      side_icon_expose_all();

//KLC   uiSetCursor();

   return(OK);
}

// Set all appropriate things to convert us to full screen mode

void change_svga_cursors()
{
   ObjID old_obj;

   extern errtype make_options_cursor(void);
   extern void reload_motion_cursors(bool cyber);
   extern void free_cursor_bitmaps();
   extern void alloc_cursor_bitmaps(void);
   extern errtype biohelp_load_cursor();
   extern errtype load_misc_cursors();

   extern int last_side_icon;
   extern int last_invent_cnum;
   extern int last_mfd_cnum[NUM_MFDS];
   extern short object_on_cursor;
   short temp;

   ss_set_hack_mode(2,&temp);

// KLC - not needed   free_options_cursor();
//���   make_options_cursor();
   old_obj = object_on_cursor;
   if (old_obj != OBJ_NULL)
   {
      pop_cursor_object();
      push_cursor_object(old_obj);
   }
   free_cursor_bitmaps();
   alloc_cursor_bitmaps();
   reload_motion_cursors(global_fullmap->cyber);
   last_side_icon = -1;
   last_invent_cnum = -1;
   last_mfd_cnum[0] = -1;
   last_mfd_cnum[1] = -1;

   biohelp_load_cursor();
   load_misc_cursors();

   ss_set_hack_mode(0,&temp);
}

void change_svga_screen_mode()
{
	extern errtype inventory_update_screen_mode();
	extern errtype mfd_update_screen_mode();
	extern void view360_update_screen_mode();
	extern void amap_pixratio_set(fix ratio);
	extern bool redraw_paused;
	extern Boolean DoubleSize;
	
	uchar cur_pal[768];
	uchar *s_table;
	short cur_w, cur_h, cur_m;
	short mx,my;
	bool mode_change = FALSE;
	short temp;
	
	if (convert_use_mode != mode_id)
		mode_change = TRUE;
	if (mode_change)
	{
		int retval = -1;
		
		ui_mouse_get_xy(&mx,&my);
//		gr_get_pal(0,256,&cur_pal[0]);
		s_table = gr_get_light_tab();
	    uiHideMouse(NULL);
		while (retval == -1)
		{
/*KLC  for stereo support
         if (mode_id == 5)
            cur_m = i6d_ss->scr_mode;
         else
 */
			cur_m=svga_mode_data[mode_id];
			retval = gr_set_mode(cur_m, TRUE);
			if (retval == -1)
			{
				mode_id = (mode_id + 1 ) % 5;
			}
		}
		convert_use_mode = mode_id;
		cur_w=grd_mode_cap.w;
		cur_h=grd_mode_cap.h;
		if (svga_screen!=NULL)
			gr_free_screen(svga_screen);
		svga_screen=gr_alloc_screen(cur_w,cur_h);
		gr_set_screen(svga_screen);
	}
	else
	{
		cur_w=grd_mode_cap.w;
		cur_h=grd_mode_cap.h;
	}
	// calculate new pixel ratio for automap; force 1 for 320x200
	// KLC - we're never 320x200   amap_pixratio_set(svga_mode_data[mode_id]==GRM_320x200x8?FIX_UNIT:0);
	amap_pixratio_set(0);

	if (svga_render_context!=NULL)
		fr_free_view(svga_render_context);
	if (full_game_3d)
	 {
	 	if (DoubleSize)
	 	{
			if (!AllocDoubleBuffer(640, 480))
				DebugStr("\pCan't allocate low-res double buffer!"); 		//���Handle memory error!!
			svga_render_context = fr_place_view(FR_NEWVIEW, FR_DEFCAM, gMainOffScreen.Address,
																	FR_DOUBLEB_MASK|FR_WINDOWD_MASK,0,0,
	 																0,0,cur_w>>1,cur_h>>1);
	 	}
	 	else
	 	{
			FreeDoubleBuffer();
			svga_render_context = fr_place_view(FR_NEWVIEW, FR_DEFCAM, gMainOffScreen.Address,
																	FR_DOUBLEB_MASK|FR_WINDOWD_MASK,0,0, 
	 																0,0,cur_w,cur_h);
	 	}
	 }
	else
	{
	 	if (DoubleSize)
	 	{
			if (!AllocDoubleBuffer(536, 259))
				DebugStr("\pCan't allocate low-res double buffer!"); 		//���Handle memory error!!
			svga_render_context = fr_place_view(FR_NEWVIEW, FR_DEFCAM, gMainOffScreen.Address, 
																	FR_DOUBLEB_MASK|FR_WINDOWD_MASK|FR_CURVIEW_STRT, 0, 0, 
																	SCONV_X(SCREEN_VIEW_X)>>1, SCONV_Y(SCREEN_VIEW_Y)>>1, 
																	SCONV_X(SCREEN_VIEW_WIDTH)>>1, (SCONV_Y(SCREEN_VIEW_HEIGHT)+1)>>1);
		}
		else
		{
			FreeDoubleBuffer();
			svga_render_context = fr_place_view(FR_NEWVIEW, FR_DEFCAM, gMainOffScreen.Address, 
																	FR_DOUBLEB_MASK|FR_WINDOWD_MASK|FR_CURVIEW_STRT, 0, 0,
																	SCONV_X(SCREEN_VIEW_X), SCONV_Y(SCREEN_VIEW_Y), 
																	SCONV_X(SCREEN_VIEW_WIDTH), SCONV_Y(SCREEN_VIEW_HEIGHT));
	 	}
	}
	fr_use_global_detail(svga_render_context);
		_current_fr_context = svga_render_context;
	if (full_game_3d)
		_current_view = fullview_region;
	else
		_current_view = mainview_region;
	_current_3d_flag = DEMOVIEW_UPDATE;
	fr_set_view(_current_fr_context);

	// Recompute zoom!
	//   ss_recompute_zoom(_current_fr_context,old_mode);

	chg_set_flg(DEMOVIEW_UPDATE);
	if (mode_change)
	{
		if (mode_id == 0)
			game_redrop_rad(0);
		else
			game_redrop_rad(2+mode_id);
		
		ss_mouse_convert(&mx,&my,FALSE);
/*KLC  leave out until stereo view is needed
		if (mode_id == 5) // hack hack stereo hack
		{
         switch(i6d_device)
         {
            case I6D_CTM:
               temp_sz.x = 320;
               temp_sz.y = 200;
               break;
            case I6D_VFX1:
               temp_sz.x = i6d_ss->scr_w / 2;
               temp_sz.y = i6d_ss->scr_h;
               break;
         }
         uiUpdateScreenSize(temp_sz);
      }
      else
*/
			uiUpdateScreenSize(UI_DETECT_SCREEN_SIZE);
// KLC - Can't do this on Mac, can we?		mouse_put_xy(mx,my);
// KLC - don't need this.  Mac sets this globally.		mouse_set_rate(mickey_stupid[mode_id][0],mickey_stupid[mode_id][1],2);
//		gr_set_pal(0,256,&cur_pal[0]);
		gr_set_light_tab(s_table);
		uiShowMouse(NULL);
	}
	if (full_game_3d)
	{
		static_change_copy();
		mfd_change_fullscreen(TRUE);
	}
	status_bio_update_screenmode();
	ss_set_hack_mode(2,&temp);
	inventory_update_screen_mode();
	mfd_update_screen_mode();
	view360_update_screen_mode();
	ss_set_hack_mode(0,&temp);
	olh_svga_deal();
	
	change_svga_cursors();
//KLC	gamma_dealfunc(QUESTVAR_GET(GAMMACOR_QVAR));
	gamma_dealfunc(gShockPrefs.doGamma);
	redraw_paused=TRUE;
}


void fullscreen_start()
{
   extern LGRegion *pagebutton_region;
   extern LGRegion *inventory_region;
   extern void draw_page_buttons(bool full);
   extern bool inp6d_headset;

// Hey, we don't need to hide here because the mouse already gets hidden by fooscreen_exit
//   uiHideMouse(NULL);
   HotkeyContext = DEMO_CONTEXT;
   full_game_3d = TRUE;
   uiSetCurrentSlab(&fullscreen_slab);

   inventory_region = inventory_region_full;
   pagebutton_region = pagebutton_region_full;
#ifdef GADGET
   _current_root = fullroot_gadget;
#endif

#ifdef STEREO_SUPPORT
   if (inp6d_stereo)
      mode_id = 5;
#endif
   change_svga_screen_mode();

   inv_change_fullscreen(TRUE);
//���   mouse_unconstrain();
   player_struct.hardwarez_status[CPTRIP(FULLSCR_HARD_TRIPLE)] |= WARE_ON;
   string_message_info(REF_STR_FSMode);
   mfd_force_update();
   draw_page_buttons(TRUE);
#ifdef STEREO_SUPPORT
   if (inp6d_stereo)
   {
//      uchar cur_pal[768];
//      gr_get_pal(0,256,&cur_pal[0]);
//      uiHideMouse(NULL);
//      gr_set_mode(i6d_ss->scr_mode,TRUE);
//      gr_set_pal(0,256,&cur_pal[0]);
      if (i6d_ss->scr_mode==grd_mode)
      {
   	   i6d_ss->stereo_screen=grd_screen->c;
	      i6_video(I6VID_SET_MODE,i6d_ss);
         if (i6_video(I6VID_STR_SETUP,i6d_ss))
	      {
            Warning(("Stereo setup failed"));
            i6_video(I6VID_CLEAR_MODE,i6d_ss);
            inp6d_stereo_active=FALSE;
         }
         else
            inp6d_stereo_active=TRUE;
      }
   }
#endif
#ifdef PALFX_FADES
//���   if (pal_fx_on) palfx_fade_up(FALSE);
#endif
//KLC   uiShowMouse(NULL);
}

// Restore all appropriate things to put us back in normal
// screen mode
void fullscreen_exit()
{
   extern wrapper_panel_close();
#ifdef SVGA_SUPPORT
   uchar cur_pal[768];
   extern grs_screen *cit_screen;
   uchar *s_table;
#endif
   
#ifdef STEREO_SUPPORT
   if (mode_id == 5)
      mode_id = 0;
   if (inp6d_stereo_active)
   {
      i6_video(I6VID_CLEAR_MODE,i6d_ss);
      inp6d_stereo_active=FALSE;
   }
#endif
   uiHideMouse(NULL);

#ifdef SVGA_SUPPORT
   if ((_new_mode != GAME_LOOP) && (_new_mode != FULLSCREEN_LOOP))
   {
      s_table = gr_get_light_tab();
      gr_get_pal(0,256,&cur_pal[0]);
      gr_set_mode(GRM_320x200x8, TRUE);
      gr_set_screen(cit_screen);
      convert_use_mode = 0;
//KLC      change_svga_cursors();
//KLC      status_bio_update_screenmode();
   }
#endif
   if (_new_mode == -1)
      return;
   full_game_3d = FALSE;
   mfd_change_fullscreen(FALSE);
   inv_change_fullscreen(FALSE);
   player_struct.hardwarez_status[CPTRIP(FULLSCR_HARD_TRIPLE)] &= ~WARE_ON;
   hud_unset(HUD_MSGLINE);

/* KLC
#ifdef SVGA_SUPPORT
   if ((_new_mode != GAME_LOOP) && (_new_mode != FULLSCREEN_LOOP))
   {
      gr_set_pal(0,256,&cur_pal[0]);
      gr_set_light_tab(s_table);
   }
#endif
*/
}


// pushes a region down below the view
errtype full_lower_region(LGRegion *r)
{
   errtype retval;
   region_begin_sequence();
   retval = region_move(r, r->r->ul.x, r->r->ul.y, 0);
   region_end_sequence(FALSE);
   return(retval);
}

// pulls a region up above the view
errtype full_raise_region(LGRegion *r)
{
   errtype retval;
   region_begin_sequence();
   retval = region_move(r, r->r->ul.x, r->r->ul.y, 2);
   region_end_sequence(FALSE);
   return(retval);
}
