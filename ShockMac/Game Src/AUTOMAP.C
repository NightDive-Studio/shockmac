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
 * $Source: r:/prj/cit/src/RCS/automap.c $
 * $Revision: 1.43 $
 * $Author: xemu $
 * $Date: 1994/10/16 15:56:00 $
 */

#include <string.h>
#include <stdlib.h>

#include "faketime.h"

#include "input.h"
#include "player.h"
#include "newmfd.h"
#include "mfdint.h"
#include "mfddims.h"
#include "automap.h"
#include "colors.h"
#include "rcolors.h"
#include "tools.h"
#include "mainloop.h"
#include "gameloop.h"
#include "wares.h"
#include "lvldata.h"
#include "cit2d.h"
#include "gr2ss.h"
#include "gamestrn.h"
#include "sfxlist.h"
#include "musicai.h"

#include "mfdart.h"
#include "gamescr.h"
#include "cybstrng.h"


#define WORKING_INC_MFDS

#define AUTOMAP_ZOOM        0
#define AUTOMAP_STATION     1
#define AUTOMAP_STATES      2

#define ZOOM_STR get_temp_string(REF_STR_AutomapMFDButtons)
#define FULL_STR get_temp_string(REF_STR_AutomapMFDButtons+1)
#define SIDE_STR get_temp_string(REF_STR_AutomapMFDButtons+2)

#define BTXT_HGT (MFD_VIEW_HGT-7)

// what is all this bullshit, anyway?

#define SetAutomapMode(m,mode) \
   player_struct.mfd_func_data[MFD_MAP_FUNC][(m)] = \
   (player_struct.mfd_func_data[MFD_MAP_FUNC][(m)] & 0xfc) + (mode)
#define GetAutomapMode(m) \
   (player_struct.mfd_func_data[MFD_MAP_FUNC][(m)] & 0x03)

#define SetLastAutomapMode(m,mode) \
   player_struct.mfd_func_data[MFD_MAP_FUNC][(m)] = \
   (player_struct.mfd_func_data[MFD_MAP_FUNC][(m)] & 0xf3) + ((mode) << 2)
#define GetLastAutomapMode(m) \
   ((player_struct.mfd_func_data[MFD_MAP_FUNC][(m)] & 0x0c) >> 2)

#define AutomapLastUpdated (player_struct.mfd_func_data[MFD_MAP_FUNC][2])


// -----------
//  PROTOTYPES
// -----------
int mfd_to_map(int mid);
void automap_expose_cross_section(MFD *m, ubyte tic);
void automap_expose_zoom(MFD *m, ubyte tac);


// -------
// Globals
// -------
static long last_update=0;
extern bool full_game_3d;

// ===========================================================================
//                         * THE AUTOMAP CODE *        
// ===========================================================================

// ---------------------------------------------------------------------------
// mfd_map_init()
// 
// Initializes the automap settings on either side to appropriate values.

errtype mfd_map_init(MFD_Func *)
{
   // set them up to be different cur and last, and left zoom, right station
   player_struct.mfd_func_data[MFD_MAP_FUNC][MFD_LEFT]=(AUTOMAP_STATION<<2)+AUTOMAP_ZOOM;
   player_struct.mfd_func_data[MFD_MAP_FUNC][MFD_RIGHT]=(AUTOMAP_ZOOM<<2)+AUTOMAP_STATION;
   AutomapLastUpdated=1;
#ifdef WORKING_INC_MFDS
   player_struct.mfd_func_status[MFD_MAP_FUNC]|=1<<4;
#endif
   return OK;
}

int mfd_to_map(int mid)
{
   int i=0, m;

   for(m=0;m<mid;m++) {
      if(mfd_get_func(m,player_struct.mfd_current_slots[m])==MFD_MAP_FUNC)
         i++;
   }
   return i;
}

// ---------------------------------------------------------------------------
// mfd_map_handler()
//
// Simply toggles a bit which tells the expose function whether to draw
// the overhead map view, or the cross-level segment of the station

#define MODE_RIGHT 25
#define OPT_LEFT  (MFD_VIEW_WID-25)

bool mfd_map_handler(MFD *m, uiEvent *e)
{
   bool retval = FALSE;
   ubyte map_state;
   uiMouseEvent *mouse;
   int mapid=mfd_to_map(m->id);

   // If we don't have an automap, we shouldn't do shit.
   if (player_struct.hardwarez[HARDWARE_AUTOMAP] == 0) return FALSE;
   
   mouse = (uiMouseEvent *) e;
   if (!(mouse->action & MOUSE_LDOWN)) return FALSE;       // ignore click releases

   if (e->pos.y>m->rect.lr.y-8)     // bottom row
   {
      int xp=e->pos.x-m->rect.ul.x;

      if (xp<MODE_RIGHT)
      {
	      map_state = GetAutomapMode(mapid);
   	   SetAutomapMode(mapid, (map_state+1)%AUTOMAP_STATES);
	      SetLastAutomapMode(mapid,map_state);
	      mfd_notify_func(MFD_MAP_FUNC, MFD_MAP_SLOT, FALSE, MFD_ACTIVE, FALSE);
	      last_update=0;
         play_digi_fx(SFX_MAP_ZOOM,1);
	      return TRUE;
      }
      else if (xp>OPT_LEFT)
      {
         _new_mode=AUTOMAP_LOOP;
	      chg_set_flg(GL_CHG_LOOP);
         play_digi_fx(SFX_MAP_ZOOM,1);
      }
      else if (GetAutomapMode(mapid)==AUTOMAP_ZOOM)
      {
         int zfac;
         play_digi_fx(SFX_MAP_ZOOM,1);
         if (xp<(MFD_VIEW_WID/2))
            zfac=1;
         else
            zfac=-1;
         amap_zoom(oAMap(mapid),FALSE,zfac);
      }
   }
   else if(GetAutomapMode(mapid)!=AUTOMAP_STATION)
   {
      char buf[80];
      int tmpx=e->pos.x-m->rect.ul.x, tmpy=e->pos.y-m->rect.ul.y;
      if (amap_deal_with_map_click(oAMap(mapid),&tmpx,&tmpy)!=NULL)
      {
         retval = amap_get_note(oAMap(mapid),buf);
         if (full_game_3d && !retval)
         {
            extern bool mfd_scan_opacity(int mfd,LGPoint pos);
            retval = mfd_scan_opacity(m->id,e->pos);
         }
         if (retval)
         {
            strtoupper(buf);
	         message_info(buf);
            retval = TRUE;
         }

      }
   }

   return retval;
}


// ---------------------------------------------------------------------------
// mfd_map_expose()
//
#include "fullscrn.h"

void mfd_map_expose(MFD *m, ubyte control)
{
   ubyte map_state, last_map_state;
   int mapid=mfd_to_map(m->id);

   if ((control & MFD_EXPOSE) && ((!full_game_3d)||(full_visible&(visible_mask(m->id)))))
   {
      // If we don't have an automap, we shouldn't do shit.
      if (player_struct.hardwarez[HARDWARE_AUTOMAP] == 0)
       {
         char buf[128];
         short w,h;
         gr_push_canvas(pmfd_canvas);
         ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
         if (!full_game_3d)
//KLC - chg for new art            draw_res_bm(REF_IMG_bmBlankMFD, 0, 0);
   			draw_hires_resource_bm(REF_IMG_bmBlankMFD, 0, 0);
         draw_raw_resource_bm(MKREF(RES_mfdArtOverlays,MFD_ART_TRIOP),0,0);
         get_string(REF_STR_NoAutomap,buf,sizeof(buf));
         gr_set_font((grs_font*)ResLock(MFD_FONT));
         wrap_text(buf,MFD_VIEW_WID-2);
         gr_string_size(buf,&w,&h);
         gr_set_fcolor(RED_BASE+5);
         draw_shadowed_string(buf,(MFD_VIEW_WID-w)/2,(MFD_VIEW_HGT-h)/2,full_game_3d);
         ResUnlock(MFD_FONT);
         gr_pop_canvas();
         mfd_update_display(m, 0, 0, MFD_VIEW_WID, MFD_VIEW_HGT);
        return;
       }

      // what is all this stuff???
      map_state      = GetAutomapMode(mapid);
      last_map_state = GetLastAutomapMode(mapid);

      if (!(control & MFD_EXPOSE_FULL) && map_state == last_map_state)
      {
	      if (map_state==AUTOMAP_STATION)
         {
	         if (!(control & MFD_EXPOSE_FULL))
		         return;
         }
	      else
	      {
#define WORKING_INC_MFDS
#ifndef WORKING_INC_MFDS
   	      if (last_update+(CIT_CYCLE>>3)>(*tmd_ticks))
             { mfd_notify_func(MFD_MAP_FUNC, MFD_MAP_SLOT, FALSE, MFD_ACTIVE, FALSE); return; }
	         else
#endif
            {
               if ((AutomapLastUpdated==mapid)&&(GetAutomapMode((mapid==0)?1:0)==AUTOMAP_ZOOM))
               {  // we were last done, and both of us need to be done, so give the other guy a chance...
                  AutomapLastUpdated=0xff;      // neither of us, so next time either will work
#ifndef WORKING_INC_MFDS
                  mfd_notify_func(MFD_MAP_FUNC, MFD_MAP_SLOT, FALSE, MFD_ACTIVE, FALSE);
#endif
                  return;
               }
               AutomapLastUpdated=mapid;
#ifndef WORKING_INC_MFDS
	            last_update=*tmd_ticks;
#endif
            }
	      }
      }
      else
      {
         if (map_state==AUTOMAP_ZOOM)
            if (!oAMap(mapid)->init)
            {
               automap_init(player_struct.hardwarez[HARDWARE_AUTOMAP],mapid);
               oAMap(mapid)->zoom++;
            }
      }
      SetLastAutomapMode(mapid,map_state);
      gr_push_canvas(pmfd_canvas);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
      if (!full_game_3d)
//KLC - chg for new art         draw_res_bm(REF_IMG_bmBlankMFD, 0, 0);
         gr_bitmap(&mfd_background, 0, 0);
      switch(map_state)
      {
      case AUTOMAP_ZOOM:    automap_expose_zoom(m,control);          break;
      case AUTOMAP_STATION: automap_expose_cross_section(m,control); break;
      }
      gr_pop_canvas();
      mfd_update_display(m, 0, 0, MFD_VIEW_WID, MFD_VIEW_HGT);
   }

   return;
}

// ---------------------------------------------------------------------------
// automap_expose_cross_section()
//
// The expose function for drawing the cross-section mode for the automap

#define BUF_SIZE 10

void automap_expose_cross_section(MFD *, ubyte)
{
   grs_font *mfdamapfont;
   char buf[BUF_SIZE];
   short w,h;
   extern char *fsmap_get_lev_str(char* buf, int size);

   draw_res_bm(MKREF(RES_mfdArtOverlays,MFD_ART_STATN), 0, 0);
   draw_res_bm(MKREF(RES_mfdArtOverlays,MFD_ART_LVL(player_struct.level)), 0, 0);
   mfdamapfont=(grs_font *)ResLock(RES_tinyTechFont);
   gr_set_font(mfdamapfont);
   gr_set_fcolor(ORANGE_8_BASE+2);
   fsmap_get_lev_str(buf,BUF_SIZE);
   gr_string_size(buf,&w,&h);
// KLC   draw_shadowed_string(fsmap_get_lev_str(buf,BUF_SIZE),MFD_VIEW_WID-w-2,1,full_game_3d);
   draw_shadowed_string(buf, MFD_VIEW_WID-w-2, 1, full_game_3d);
   ResUnlock(RES_tinyTechFont);

   mfdamapfont=(grs_font *)ResLock(RES_mfdFont);
   gr_set_font(mfdamapfont);
   gr_set_fcolor(ORANGE_8_BASE);
   draw_shadowed_string(ZOOM_STR,1,BTXT_HGT,full_game_3d);
   draw_shadowed_string(FULL_STR,MFD_VIEW_WID-25,BTXT_HGT,full_game_3d);
   ResUnlock(RES_mfdFont);

   return;
}

// ---------------------------------------------------------------------------
// automap_expose_zoom_in()
//
// The expose function for drawing the zoomed in mode for the automap

void automap_expose_zoom(MFD *m, ubyte)
{
   grs_font *mfdamapfont;
   int mapid=mfd_to_map(m->id);

   amap_draw(oAMap(mapid),0);
   mfdamapfont=(grs_font *)ResLock(RES_mfdFont);
   gr_set_font(mfdamapfont);
   gr_set_fcolor(ORANGE_8_BASE);
   draw_shadowed_string(SIDE_STR,1,BTXT_HGT,full_game_3d);
   draw_shadowed_string("+   -",(MFD_VIEW_WID/2)-10,BTXT_HGT,full_game_3d);
   draw_shadowed_string(FULL_STR,MFD_VIEW_WID-25,BTXT_HGT,full_game_3d);
   ResUnlock(RES_mfdFont);
#ifndef WORKING_INC_MFDS
   mfd_notify_func(MFD_MAP_FUNC, MFD_MAP_SLOT, FALSE, MFD_ACTIVE, FALSE);
#endif
   return;
}
