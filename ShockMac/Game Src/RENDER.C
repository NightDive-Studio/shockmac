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
 * $Source: r:/prj/cit/src/RCS/render.c $
 * $Revision: 1.43 $
 * $Author: xemu $
 * $Date: 1994/10/27 04:55:24 $
 */

#include <string.h>

#define __RENDER_SRC
#include "render.h"
#include "frprotox.h"
#include "faketime.h"
#include "screen.h"
#include "fullscrn.h"
#include "mainloop.h"
#include "hudobj.h"
#include "cybmem.h"

// Hack camera stuff
#include "gamerend.h"
#include "frcamera.h"
#include "frflags.h"
#include "froslew.h"
#include "player.h"
#include "cybmem.h"

#include "rcolors.h"

#include "map.h"
#include "mapflags.h"
#include "wares.h"
#include "gr2ss.h"

frc *hack_cam_frcs[MAX_CAMERAS_VISIBLE];
grs_canvas hack_cam_canvases[MAX_CAMERAS_VISIBLE], static_canvas;
grs_bitmap *hack_cam_bitmaps[MAX_CAMERAS_VISIBLE], *static_bitmap;
cams hack_cam;
char camera_map[NUM_HACK_CAMERAS];
ObjID hack_frc_objs[MAX_CAMERAS_VISIBLE];
ObjID hack_cam_objs[NUM_HACK_CAMERAS];
ObjID hack_cam_surrogates[NUM_HACK_CAMERAS];

#define HACK_CAMERA_X   0
#define HACK_CAMERA_Y   0
#define HACK_CAMERA_WIDTH   ((start_mem < BIG_HACKCAM_THRESHOLD) ? 64 : 128)
#define HACK_CAMERA_HEIGHT  ((start_mem < BIG_HACKCAM_THRESHOLD) ? 64 : 128)
//#define HACK_CAMERA_WIDTH  128
//#define HACK_CAMERA_HEIGHT  128
//#define HACK_CAMERA_WIDTH  64
//#define HACK_CAMERA_HEIGHT 64

#define STATIC_HEIGHT   32
#define STATIC_WIDTH    32

#define FRAME_SKIP_MASK 0x03 
#define FRAME_PARITY_SHF 2 // skip every 4 frames when rendering screen images

uchar hack_cameras_needed = 0;
char curr_hack_cam = 0;
ulong hack_cam_fr_count = 0;
bool screen_static_drawn = FALSE;

// -----------
// PROTOTYPES
//------------
int hack_camera_draw_callback(grs_canvas*, grs_bitmap*, int, int, int );
void update_cspace_tiles(void);
void tile_hit(int mx, int my);


int hack_camera_draw_callback(grs_canvas*, grs_bitmap*, int, int, int )
{
//   gr_push_canvas(&hack_cam_canvases[curr_hack_cam]);
//   gr_bitmap(bm,0,0);
//   gr_pop_canvas();
   return(FALSE);
}

typedef int (*frdraw)(void *dstc, void *dstbm, int x, int y, int flg);

errtype init_hack_cameras()
{
   int i;
   grs_canvas *tmp_cnv;
   uchar *tmp_mem;

   static_bitmap = gr_alloc_bitmap(BMT_FLAT8, 0, STATIC_WIDTH, STATIC_HEIGHT);
   gr_make_canvas(static_bitmap, &static_canvas);
   gr_push_canvas(&static_canvas);
   gr_clear(0);
   gr_pop_canvas();

   fr_camera_create(&hack_cam, CAMTYPE_OBJ, (fix *)player_struct.rep, 0);
   for (i=0; i < MAX_CAMERAS_VISIBLE; i++)
   {
//      hack_cam_bitmaps[i] = gr_alloc_bitmap(BMT_FLAT8, 0, HACK_CAMERA_WIDTH, HACK_CAMERA_HEIGHT);
//      gr_make_canvas(hack_cam_bitmaps[i], &hack_cam_canvases[i]);
//      hack_cam_frcs[i] = fr_place_view(FR_NEWVIEW, &hack_cam, &hack_cam_canvases[i],
//      hack_cam_frcs[i] = fr_place_view(FR_NEWVIEW, &hack_cam, &hack_cam_bitmaps[i],

//      Warning(("start_mem = %d, BIG_HACKCAM = %d!\n",start_mem,BIG_HACKCAM_THRESHOLD));  
//      Warning(("HACK_CAMERA_WID = %d!\n",HACK_CAMERA_WIDTH));
      tmp_mem = (uchar *)NewPtr(HACK_CAMERA_WIDTH*HACK_CAMERA_HEIGHT);
      hack_cam_frcs[i] = fr_place_view(FR_NEWVIEW, &hack_cam, tmp_mem, FR_DOUBLEB_MASK|FR_HACKCAM_FLAG, 
                                         0, 0, HACK_CAMERA_X, HACK_CAMERA_Y, HACK_CAMERA_WIDTH, HACK_CAMERA_HEIGHT);
      fr_set_callbacks(hack_cam_frcs[i], (frdraw)hack_camera_draw_callback, NULL, NULL);
      tmp_cnv = (grs_canvas *)fr_get_canvas(hack_cam_frcs[i]);
      hack_cam_bitmaps[i]=&tmp_cnv->bm;

//      gr_push_canvas(&hack_cam_canvases);
//      gr_clear(0);
//      gr_pop_canvas();
   }

   for (i=0; i < NUM_HACK_CAMERAS; i++)
   {
      camera_map[i] = 0;
      hack_cam_objs[i] = OBJ_NULL;
      hack_cam_surrogates[i] = OBJ_NULL;
   }

   return(OK);
}

errtype shutdown_hack_cameras()
{
   int i;
   for (i=0; i < MAX_CAMERAS_VISIBLE; i++)
   {
      fr_free_view(hack_cam_frcs[i]);
      DisposePtr((Ptr)hack_cam_bitmaps[i]->bits);
   }
   return(OK);
}

errtype do_screen_static()
{
   extern void draw_full_static(grs_bitmap *stat_dest, int c_base);
   if (!screen_static_drawn)
      draw_full_static(static_bitmap, GRAY_8_BASE);
   return(OK);
}

errtype render_hack_cameras()
{
   int i, count;

   // efficiency good....
   if (hack_cameras_needed==0) return OK;

   hack_cam_fr_count++;
   count = 0;
   for (i=0; i < NUM_HACK_CAMERAS; i++)
   {
      if((hack_cameras_needed & (1<<i)) && !((hack_cam_objs[i] == OBJ_NULL) || (count == MAX_CAMERAS_VISIBLE)))
         camera_map[i] = ++count;
      else
         camera_map[i] = 0;
   }
   for (i=0; i < NUM_HACK_CAMERAS; i++)
   {
      if (camera_map[i] &&
            (hack_frc_objs[camera_map[i]-1]!=hack_cam_objs[i] ||
            (((hack_cam_fr_count&FRAME_SKIP_MASK)==0) &&
               (count==1 || (((hack_cam_fr_count>>FRAME_PARITY_SHF)^camera_map[i])&1)==0))
         ))
      {
         curr_hack_cam = camera_map[i]-1;
         hack_frc_objs[curr_hack_cam]=hack_cam_objs[i];
         fr_camera_update(&hack_cam, (void *)hack_cam_objs[i], 0, 0);
         fr_rend(hack_cam_frcs[camera_map[i]-1]);
      }
   }
   curr_hack_cam = 0;
   hack_cameras_needed = 0;
   return(OK);
}

// Okay, takeover and relinquish are what we do when you double
// click on a hack-camera-ed screen.  Note that we always relinquish
// back to the player, if this is insufficient it's easy to store off
// some of the camera data.
uchar hack_takeover = 0;
int hack_eye; // the fierce pirate

errtype hack_camera_takeover(int hack_cam)
{
   LGRect start = {{ -5, -5}, {5,5}};
   extern LGPoint use_cursor_pos;
   LGPoint ucp;
   extern void zoom_rect(LGRect *start, LGRect *end);
   extern Boolean DoubleSize;
   extern LGRect mainview_rect;
   extern LGRect fscrn_rect;
   cams *cam = fr_camera_getdef();

   // Turn off the 360 ware, if it is on
   if (WareActive(player_struct.hardwarez_status[HARDWARE_360]))
      use_ware(WARE_HARD,HARDWARE_360);

   // do a wacky zoom thing
   ucp = use_cursor_pos;
   if (!DoubleSize)
	   ss_point_convert(&(ucp.x),&(ucp.y),TRUE);
   RECT_MOVE(&start, ucp);
   zoom_rect(&start, (full_game_3d) ? &fscrn_rect : &mainview_rect);

   // level out the eye & store off old position
   hack_eye = eye_mods[1];
   eye_mods[1] = 0;

   fr_camera_update(cam, (void *)hack_cam_objs[hack_cam], 0, 0);
   hack_takeover = hack_cam+1;
   return(OK);
}

errtype hack_camera_relinquish()
{
   cams *cam = fr_camera_getdef();
   fr_camera_update(cam, (void *)PLAYER_OBJ, 0, 0);

   // force refresh of this hack camera.
   hack_frc_objs[camera_map[hack_takeover-1]-1]=OBJ_NULL;
   camera_map[hack_takeover-1]=0;

   hack_takeover = 0;
   eye_mods[1] = hack_eye;
   return(OK);
}


#define inc_area(tp) if (((tp>=0)&&(tp<64*64))) (*(tmp_ptr+tp))++
#define val_area(tp) (*(tmp_ptr+tp))

void update_cspace_tiles(void)
{
   int cur_tp, i, j, s, t, x;
   uchar *tmp_ptr=(uchar *)big_buffer;
   MapElem *mmp;

   LG_memset(tmp_ptr,0,64*64);
   mmp=MAP_GET_XY(0,0);
   for (i=0, cur_tp=0; i<64; i++)
      for (j=0; j<64; j++, cur_tp++, mmp++)
         if (me_bits_flip_x(mmp))
            for (s=-1; s<=1; s++)
               for (t=-1; t<=1; t++)
                  if (s|t)
                   { x=cur_tp+(s*64)+t; inc_area(x); }
   mmp=MAP_GET_XY(0,0);
   for (i=0, cur_tp=0; i<64; i++)
      for (j=0; j<64; j++, cur_tp++, mmp++)
         if (me_bits_flip(mmp))
         {
            if (me_bits_flip(mmp)==1)
               me_flip_set(mmp,3);
            else if (me_bits_flip(mmp)==3)
               me_flip_set(mmp,2);
            if (val_area(cur_tp)!=0)
		         if ((val_area(cur_tp)<2)||(val_area(cur_tp)>3))
		            me_flip_set(mmp,0);
         }
         else
            if (val_area(cur_tp)==3)
               me_flip_set(mmp,2);

#ifdef STATE_RULES
         switch (me_bits_flip(mmp))
         {
         case 2:
            if ((val_area(cur_tp)<2)||(val_area(cur_tp)>3))
	            me_flip_set(mmp,0);
            break;
         case 1:
            me_flip_set(mmp,3);
            break;
         case 0:
            if (val_area(cur_tp)==3)
               me_flip_set(mmp,2);
            break;
         case 3:
            if (((val_area(cur_tp)+i+j)&0xf)<4)
               me_flip_set(mmp,2);
         }
#endif
}

void tile_hit(int mx, int my)
{
   static int lx,ly;
   static long last_time=0;
   int dsq=((lx-mx)|(ly-my));
   MapElem *mp;

   if (dsq||((last_time+(CIT_CYCLE>>2)<*tmd_ticks)))
   {
      last_time=*tmd_ticks;
      mp=MAP_GET_XY(mx,my);
      if (dsq)
      {
	      me_flip_set(mp,1);
	      lx=mx; ly=my;
      }
      else if (me_bits_flip(mp))
	      me_flip_set(mp,0);
      else
	      me_flip_set(mp,1);
   }
}


#define LIFE_UPDATE_RATE (256>>2)

errtype render_run(void)
{
   extern bool view360_render_on;
   extern void view360_render(void);
   static long last_cspace_update=0;

#ifdef POPUPS_ALLOWED
   if (region_obscured(mainview_region,mainview_region->r) == UNOBSCURED)
#endif
   {
      screen_static_drawn = FALSE;
      current_num_hudobjs =0; //clear the hud objects
      rendrect = mainview_region->r;
      fr_rend(NULL);
      if (view360_render_on)
         view360_render();
      render_hack_cameras();
      if (global_fullmap->cyber)
         if ((last_cspace_update+LIFE_UPDATE_RATE)<(*tmd_ticks))
         {
            last_cspace_update=(*tmd_ticks)&(~(LIFE_UPDATE_RATE-1));
	         update_cspace_tiles();
         }
   }
   return OK;
}

