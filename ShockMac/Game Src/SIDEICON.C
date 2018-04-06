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
 * $Source: r:/prj/cit/src/RCS/sideicon.c $
 * $Revision: 1.53 $
 * $Author: mahk $
 * $Date: 1994/11/23 04:31:51 $
 *
 */

#include <stdlib.h>
#include <string.h>

#include "input.h"
#include "sideicon.h"
#include "sideart.h"
#include "popups.h"
#include "gamestrn.h"
#include "cybstrng.h"
#include "tools.h"
#include "mainloop.h"
#include "game_screen.h"
#include "fullscrn.h"
#include "wares.h"
#include "objsim.h"
#include "objclass.h"
#include "otrip.h"
#include "player.h"
#include "faketime.h"
#include "mfdext.h"
#include "gameloop.h"
#include "textmaps.h"
#include "criterr.h"
#include "objapp.h"
#include "objwarez.h"
#include "hkeyfunc.h"
#include "musicai.h"
#include "sfxlist.h"
#include "gr2ss.h"
#include "canvchek.h"


// ---------
// Constants
// ---------

#define NUM_SIDE_ICONS 10

#define SIDE_ICONS_TOP_Y      24
#define SIDE_ICONS_LEFT_X     4
#define SIDE_ICONS_RIGHT_X    300
#define SIDE_ICONS_HEIGHT     15
#define SIDE_ICONS_WIDTH      15
#define SIDE_ICONS_VSPACE     8

#define ICON_ART_ITEMS        2
#define ICON_ART_OFF          0
#define ICON_ART_ON           1
#define ICON_ART_BACKGROUND   255   

#define FLASH_RATE 256
#define MAX_FLASH_COUNT 12

// ----------------
// Local Prototypes
// ----------------

void side_icon_language_change(void);
bool side_icon_mouse_callback(uiEvent *e, LGRegion *r, void *udata);
void zoom_side_icon_to_mfd(int icon,int waretype, int wnum);
void zoom_to_side_icon(LGPoint from, int icon);
bool side_icon_hotkey_func(ushort keycode, ulong context, int i);
void side_icon_draw_bm(LGRect *r, ubyte icon, ubyte art);

// ----------
// Structures
// ----------


typedef struct _side_icon {
   LGRect   r;
   bool flashstate;
   ubyte flashcount;
   ubyte state;
} SIDE_ICON;

typedef struct _icon_data
{
   byte waretype;
   long waretrip;
   int flashfx;
} ICON_DATA;




// -------
// Globals
// -------

SIDE_ICON  side_icons[NUM_SIDE_ICONS];
#ifdef PRELOAD_BITMAPS
grs_bitmap side_icon_bms[NUM_SIDE_ICONS][ICON_ART_ITEMS];
grs_bitmap side_icon_background;
#else
#define side_icon_bmid(icon,art) (MKREF(RES_SideIconArt, (ICON_ART_ITEMS * icon) + art + 1))
#define side_icon_backid         (MKREF(RES_SideIconArt,0))
#endif 

#ifdef PROGRAM_SIDEICON
static char shiftnums[]=")!@#$%^&*(";
static uchar programmed_sideicon=0;
#endif

// this is in wares.c
extern long ware_base_triples[NUM_WARE_TYPES]; 

#define IDX_OF_TYPE(type,trip) (OPTRIP(trip) - OPTRIP(ware_base_triples[type]))

static ICON_DATA icon_data[NUM_SIDE_ICONS] =
{
 
  { WARE_HARD, BIOSCAN_HARD_TRIPLE},  
  { WARE_HARD, FULLSCR_HARD_TRIPLE},  
  { WARE_HARD, SENS_HARD_TRIPLE},  
  { WARE_HARD, LANTERN_HARD_TRIPLE},  
  { WARE_HARD, SHIELD_HARD_TRIPLE},
 
  { WARE_HARD, INFRA_GOG_TRIPLE},  
  { WARE_HARD, NAV_HARD_TRIPLE},  
  { WARE_HARD, VIDTEX_HARD_TRIPLE, SFX_EMAIL}, 
  { WARE_HARD, MOTION_HARD_TRIPLE},  
  { WARE_HARD, JET_HARD_TRIPLE},  
};

grs_bitmap	icon_cursor_bm[2];
LGCursor		icon_cursor[2];
static char		*cursor_strings[NUM_SIDE_ICONS];
static char		cursor_strbuf[128];


// ============
// INITIALIZERS
// ============

// ---------------------------------------------------------------------------
// init_all_side_icons()
//
// Initialize all side icons to "unset" settings (should be called before
// wares_init()!).  Also sets up their on-screen locations.
// And, as of 7/22, loads in all the bitmaps from memory.

void side_icon_language_change(void)
{
   load_string_array(REF_STR_IconCursor,cursor_strings,cursor_strbuf,sizeof(cursor_strbuf),NUM_SIDE_ICONS);
}

void init_all_side_icons()
{
	int i;
	
	// Now, figure out on-screen locations
	
	for (i = 0; i < (NUM_SIDE_ICONS / 2); i++)             						 // left side first
	{
		side_icons[i].r.ul.x = SIDE_ICONS_LEFT_X;
		side_icons[i].r.ul.y = SIDE_ICONS_TOP_Y + (i * (SIDE_ICONS_HEIGHT + SIDE_ICONS_VSPACE));
		
		side_icons[i].r.lr.x = side_icons[i].r.ul.x + SIDE_ICONS_WIDTH;
		side_icons[i].r.lr.y = side_icons[i].r.ul.y + SIDE_ICONS_HEIGHT;
	}
	
	for (i = (NUM_SIDE_ICONS / 2); i < NUM_SIDE_ICONS; i++) 		// now right side
	{
		side_icons[i].r.ul.x = SIDE_ICONS_RIGHT_X;
		side_icons[i].r.ul.y = side_icons[i-(NUM_SIDE_ICONS/2)].r.ul.y;
		
		side_icons[i].r.lr.x = side_icons[i].r.ul.x + SIDE_ICONS_WIDTH;
		side_icons[i].r.lr.y = side_icons[i].r.ul.y + SIDE_ICONS_HEIGHT;
	}
}

void init_side_icon_popups(void)
{
	side_icon_language_change();
	for (int i = 0; i < 2; i++)
	{
		LGPoint offset = {0, -1};
		LGCursor* c = &icon_cursor[i];
		grs_bitmap* bm = &icon_cursor_bm[i];
		make_popup_cursor(c,bm,cursor_strings[i*NUM_SIDE_ICONS/2],i+POPUP_ICON_LEFT, TRUE, offset);
	}
}


#ifdef DUMMY	//��� not yet, bucko

void init_side_icon_hotkeys(void)
{
   bool side_icon_hotkey_func(ushort key, ulong context, int i);
   bool side_icon_progset_hotkey_func(ushort key, ulong context, int i);
   bool lantern_change_setting_hkey(ushort key, ulong context, void* i);
   bool shield_change_setting_hkey(ushort key, ulong context, void* i);
   bool side_icon_prog_hotkey_func(ushort key, ulong context, void* notused);
   int i;

   hotkey_add(KB_FLAG_ALT|KB_FLAG_DOWN|'4',DEMO_CONTEXT,lantern_change_setting_hkey, NULL);
   hotkey_add(KB_FLAG_ALT|KB_FLAG_DOWN|'5',DEMO_CONTEXT,shield_change_setting_hkey, NULL);

   hotkey_add(KB_FLAG_DOWN|'0',DEMO_CONTEXT,(hotkey_callback)side_icon_hotkey_func,(void*)(NUM_SIDE_ICONS-1));
#ifdef PROGRAM_SIDEICON
   hotkey_add('`',DEMO_CONTEXT,(hotkey_callback)side_icon_prog_hotkey_func,NULL);
   hotkey_add(KB_FLAG_DOWN|shiftnums[0],DEMO_CONTEXT,(hotkey_callback)side_icon_progset_hotkey_func,(void*)(NUM_SIDE_ICONS-1));
#endif
   for (i = 0; i < NUM_SIDE_ICONS -1; i++) {
      hotkey_add(KB_FLAG_DOWN|('1'+i),DEMO_CONTEXT,(hotkey_callback)side_icon_hotkey_func,(void*)i);
#ifdef PROGRAM_SIDEICON
      hotkey_add(KB_FLAG_DOWN|shiftnums[1+i],DEMO_CONTEXT,(hotkey_callback)side_icon_progset_hotkey_func,(void*)i);
#endif
   }
}

#endif // DUMMY ���


// ---------------------------------------------------------------------------
// init_side_icon() 
//


// ---------------------------------------------------------------------------
// screen_init_side_icons();
//
// Declare the appropriate regions for the side icons
// (called from screen_start() in screen.c)


void screen_init_side_icons(LGRegion* root)
{
   int id;
   LGRegion *left_region, *right_region;
   LGRect r;
   left_region = (LGRegion *)NewPtr(sizeof(LGRegion));
   right_region = (LGRegion *)NewPtr(sizeof(LGRegion));

   // Wow, having a LGRegion for each of the side icons is totally uncool
   // Let's just have two regions, and figure out from there.  
   
   r.ul = side_icons[0].r.ul;
   r.lr = side_icons[(NUM_SIDE_ICONS-1)/2].r.lr;
   macro_region_create_with_autodestroy(root, left_region, &r);
   uiInstallRegionHandler(left_region, UI_EVENT_MOUSE|UI_EVENT_MOUSE_MOVE, &side_icon_mouse_callback, (void *)0, &id);
   uiSetRegionDefaultCursor(left_region,NULL);

   r.ul = side_icons[(NUM_SIDE_ICONS+1)/2].r.ul;
   r.lr = side_icons[(NUM_SIDE_ICONS-1)].r.lr;
   macro_region_create_with_autodestroy(root, right_region, &r);
   uiInstallRegionHandler(right_region, UI_EVENT_MOUSE|UI_EVENT_MOUSE_MOVE, &side_icon_mouse_callback, (void *)((NUM_SIDE_ICONS+1)/2), &id);
   uiSetRegionDefaultCursor(right_region,NULL);

   return;
}

// =========
// SELECTION
// =========

// ----------------------------------------------------------------
// select_side_icon() selects a given side icon.


void zoom_side_icon_to_mfd(int icon,int waretype, int wnum)
{
   extern ubyte waretype2invtype[];
   extern void mfd_zoom_rect(LGRect* start, int mfdnum);

   int mfd;

   mfd = mfd_grab_func(MFD_EMPTY_FUNC,MFD_ITEM_SLOT);
   mfd_zoom_rect(&side_icons[icon].r,mfd);
   set_inventory_mfd(waretype2invtype[waretype],wnum,TRUE);
   mfd_change_slot(mfd,MFD_ITEM_SLOT);
}

// ---------------------------------------------------------------------------
// side_icon_mouse_callback()
//
// Callback function for mouse clicks inside the side icons.

extern LGCursor globcursor;

int last_side_icon = -1;
bool side_icon_mouse_callback(uiEvent *e, LGRegion *r, void *udata)
{
   extern bool fullscrn_icons;
   bool retval = FALSE;
   uiMouseEvent *m;
   int i, type, num;

   if (!global_fullmap->cyber && !(full_game_3d && !fullscrn_icons))
   {
      int ver;
      m = (uiMouseEvent *)e;

  
      i = (int) udata + (e->pos.y - SIDE_ICONS_TOP_Y)/(SIDE_ICONS_HEIGHT + SIDE_ICONS_VSPACE);
      type = icon_data[i].waretype;
      num  = IDX_OF_TYPE(type,icon_data[i].waretrip);
      ver = get_player_ware_version(type,num);

      if (!RECT_TEST_PT(&side_icons[i].r,e->pos) || ver == 0)
      {
         uiSetRegionDefaultCursor(r,&globcursor);
         last_side_icon = -1;
         return FALSE;
      }
	 if (popup_cursors)
      {
         if (last_side_icon != i)
         {
            uchar side = i*2/NUM_SIDE_ICONS;
            LGCursor* c = &icon_cursor[side];
            grs_bitmap* bm = &icon_cursor_bm[side];
            LGPoint offset = {0, -1};

            DisposePtr((Ptr)bm->bits);
            make_popup_cursor(c,bm,cursor_strings[i],side+POPUP_ICON_LEFT, TRUE, offset);
            uiSetRegionDefaultCursor(r,c);
            last_side_icon = i;
         }
      }
/*���
      if (m->action & MOUSE_RDOWN)
      {
         zoom_side_icon_to_mfd(i,type,num);
         retval = TRUE;
      }
*/
 
      if (!(m->action & MOUSE_LDOWN)) return retval; // ignore click releases
   //   mprintf("  Side Icon %d: CYBER(%d,%d) [%x] REAL(%d,%d) [%x]\n",
   //      i, side_icons[i].cyber_type, side_icons[i].cyber_num,
   //      side_icons[i].cyber_set, side_icons[i].real_type,
   //      side_icons[i].real_num, side_icons[i].real_set);

      if (type >= 0) use_ware(type, num);
      retval = TRUE;
   }
   if (global_fullmap->cyber || (full_game_3d && !fullscrn_icons) || !popup_cursors)
   {
      last_side_icon = -1;
      uiSetRegionDefaultCursor(r,NULL);
   }

   return retval;
}


bool side_icon_hotkey_func(ushort, ulong, int i)
{
   int type = icon_data[i].waretype;
   int num  = IDX_OF_TYPE(type,icon_data[i].waretrip);
   if ((!global_fullmap->cyber) || (i == 1))
   {
      if (type >= 0) use_ware(type, num);
   }
   return TRUE;
}

#ifdef PROGRAM_SIDEICON
bool side_icon_progset_hotkey_func(ushort keycode, ulong context, int i)
{
   char mess[80];
   int l;
   programmed_sideicon=i;
   get_string(REF_STR_PresetSideicon,mess,80);
   l=strlen(mess);
   get_object_short_name(icon_data[i].waretrip,mess+l,80-l);
   message_info(mess);
   return TRUE;
}

bool side_icon_prog_hotkey_func(ushort keycode, ulong context, void* notused)
{
   return(side_icon_hotkey_func(keycode, context, programmed_sideicon));
}
#endif



// ========
// GRAPHICS
// ========

// ---------------------------------------------------------------------------
// side_icon_expose_all()
//
// Sort of an initial-draw-everything type of routine

void side_icon_expose_all()
{
   ubyte i;

   for (i = 0; i < NUM_SIDE_ICONS; i++)
      side_icon_expose(i);

   return;
}

// ----------------------------------------------------
// zoom_to_side_icon(Point from, int icon)
// zooms a LGRect to a side icon and then exposes it.

void zoom_to_side_icon(LGPoint from, int icon)
{
   extern void zoom_rect(LGRect* s,LGRect* f);
   LGRect start = { { -3, -3},{3,3}};
   LGRect dest;
   RECT_MOVE(&start,from);
   dest = side_icons[icon].r;
   zoom_rect(&start,&dest);
   side_icon_expose(icon);
}
   


// ---------------------------------------------------------------------------
// side_icon_draw_bm()
//
// Draws a side icon of the specified ware, version, and status.

void side_icon_draw_bm(LGRect *r, ubyte icon, ubyte art)
{
#ifdef SVGA_SUPPORT
   uchar old_over = gr2ss_override;
   gr2ss_override = OVERRIDE_ALL;
#endif
   if (is_onscreen()) uiHideMouse(r);
   if (art == ICON_ART_BACKGROUND)
//KLC - chg for new art      draw_raw_resource_bm(side_icon_backid,r->ul.x,r->ul.y);
      draw_hires_resource_bm(side_icon_backid, SCONV_X(r->ul.x), SCONV_Y(r->ul.y));
   else
//KLC - chg for new art      draw_raw_resource_bm(side_icon_bmid(icon,art), r->ul.x, r->ul.y);
      draw_hires_resource_bm(side_icon_bmid(icon,art), SCONV_X(r->ul.x), SCONV_Y(r->ul.y));
   if (is_onscreen()) uiShowMouse(r);
#ifdef SVGA_SUPPORT
   gr2ss_override = old_over;
#endif
   return;
}

// ---------------------------------------------------------------------------
// side_icon_expose()
//
// Draw a side icon appropriately, depending on the ware it points at,
// and its state.

void side_icon_expose(ubyte icon_num)
{
   ubyte *player_wares, *player_status;
   WARE  *wares;
   int   type, num, n;
   LGRect  *r;
   extern bool fullscrn_icons;

   if (full_game_3d && (global_fullmap->cyber || !(fullscrn_icons)))
      return;
   r = &(side_icons[icon_num].r);

   type      = icon_data[icon_num].waretype;
   num       = IDX_OF_TYPE(type,icon_data[icon_num].waretrip);

   if (type < 0) return;
   get_ware_pointers(type, &player_wares, &player_status, &wares, &n);
   
   // Possible expose cases:
   // 
   // 1) We don't have the appropriate ware for that side icon.
   if (player_wares[num] == 0) {

      // Expose screen background
//      Spew(DSRC_TESTING_Test9,("icon number %d is empty\n",icon_num));
      if (!full_game_3d)
         side_icon_draw_bm(r, icon_num, ICON_ART_BACKGROUND);
   }

   // 3) We have the ware, and it's trying to get our attention.
   //
   else if (player_status[num] & WARE_FLASH)
   {
      bool fs = ((*tmd_ticks/FLASH_RATE)%2);
      if (fs == side_icons[icon_num].flashstate && !full_game_3d)
         return;

      if (fs)
      {
         side_icon_draw_bm(r, icon_num, ICON_ART_ON);
         if (fs != side_icons[icon_num].flashstate)
            if (icon_data[icon_num].flashfx!=0)
               if (QUESTBIT_GET(0x12c))
                  play_digi_fx(icon_data[icon_num].flashfx,1);
      }
      else 
         side_icon_draw_bm(r, icon_num, ICON_ART_OFF);
      if (fs != side_icons[icon_num].flashstate)
      {
         side_icons[icon_num].flashcount ++;
         if (side_icons[icon_num].flashcount >= MAX_FLASH_COUNT)
         {
            side_icons[icon_num].flashcount = 0;
            player_status[num] &= ~WARE_FLASH;
         }
      }
      side_icons[icon_num].flashstate = fs;
   }

   // 2) We have the ware, but we're turning it off.
   //
   else if (!(player_status[num] & WARE_ON)) {

      // Expose darkened bitmap of current version
//      Spew(DSRC_TESTING_Test9,("icon number %d is off\n",icon_num));
      side_icon_draw_bm(r, icon_num, ICON_ART_OFF);
   } else {

      // Expose normal (active) bitmap of current ware version
      side_icon_draw_bm(r, icon_num, ICON_ART_ON);
   }
  
   return;
}

// ---------------------------------------------------------------------------
// side_icon_load_bitmaps()
//
// Load the bitmaps for all side icons and states from the resource system.

errtype side_icon_load_bitmaps()
{                                
#ifdef PRELOAD_BITMAPS
   RefTable *side_icon_rft;
   int i, j, index /*, file_handle */;

//  file_handle = ResOpenFile("sideart.res");
//   if (file_handle < 0) critical_error(CRITERR_RES|6); 
   
   side_icon_rft = ResLock(RES_SideIconArt);
   load_bitmap_from_res(&side_icon_background, RES_SideIconArt, 0, side_icon_rft, FALSE, NULL, NULL);

   for (i = 0; i < NUM_SIDE_ICONS; i++) {

      for (j = 0; j < ICON_ART_ITEMS; j++) {
 
         index = (ICON_ART_ITEMS * i) + j + 1;
         load_bitmap_from_res(&(side_icon_bms[i][j]), RES_SideIconArt, index, side_icon_rft, FALSE, NULL, NULL);
      }
   }
   ResUnlock(RES_SideIconArt);
//   ResCloseFile(file_handle);
#endif

   return(OK);
}

errtype side_icon_free_bitmaps()
{
#ifdef PRELOAD_BITMAPS
   int i,j,index;
   Free(side_icon_background.bits);
   for (i = 0; i < NUM_SIDE_ICONS; i++) {

      for (j = 0; j < ICON_ART_ITEMS; j++) {

         index = (ICON_ART_ITEMS * i) + j;
         Free(side_icon_bms[i][j].bits);
      }
   }
#endif
   return(OK);
}
