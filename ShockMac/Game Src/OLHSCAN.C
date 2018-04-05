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
 * $Source: r:/prj/cit/src/RCS/olhscan.c $
 * $Revision: 1.6 $
 * $Author: dc $
 * $Date: 1994/11/21 09:10:45 $
 */

#include <stdlib.h>
#include <string.h>

#include "faketime.h"

#include "frtypes.h"
#include "frintern.h"
#include "frparams.h"
#include "frflags.h"
#include "olhint.h"
#include "objects.h"
#include "objprop.h"
#include "screen.h"
#include "fullscrn.h"


#define SCAN_OBJ_LIST 8

#define MIN_OLH_RADIUS 2
#define MAX_OLH_RADIUS 5

ubyte olh_radius = MIN_OLH_RADIUS;

#define BOTTOM_MARGIN (SCAN_HGT - 15/SCAN_RATIO)
#define RIGHT_MARGIN  (OLH_WRAP_WID/SCAN_RATIO)

fauxrend_context* olh_full_context = NULL;

extern fauxrend_context* svga_render_context;

#define FULL_SCAN_WID (FULL_VIEW_WIDTH/SCAN_RATIO)
#define FULL_SCAN_HGT (FULL_VIEW_HEIGHT/SCAN_RATIO)

void olh_init_single_scan(fauxrend_context **outxt, fauxrend_context *intxt);
void olh_init_scan(void);
void olh_free_scan(void);
void olh_svga_deal(void);
ushort olh_scan_objs(void);



void olh_init_single_scan(fauxrend_context **outxt, fauxrend_context *intxt)
{
   uchar* mem = ((grs_canvas*)fr_get_canvas(intxt))->bm.bits;

   *outxt = (fauxrend_context *)fr_place_view(FR_NEWVIEW,FR_DEFCAM,mem,
					FR_CURVIEW_STRT|FR_DOUBLEB_MASK|FR_PICKUPM_MASK,
      				0, 0, 0, 0, intxt->xwid/SCAN_RATIO, intxt->ywid/SCAN_RATIO);
   fr_set_callbacks(*outxt, NULL, NULL, NULL);
}

/*KLC - no longer used
void olh_init_scan(void)
{
   olh_init_single_scan(&olh_full_context, (fauxrend_context *)full_game_fr_context);
}
*/
void olh_free_scan(void)
{
   if (olh_full_context)
      fr_free_view(olh_full_context);
}

fix x_mul=fix_make(1,0), y_mul=fix_make(1,0);
void olh_svga_deal(void)
{
   if (olh_full_context)
      fr_free_view(olh_full_context);
   olh_init_single_scan(&olh_full_context,svga_render_context);
   x_mul=fix_div(fix_make(320,0),fix_make(grd_mode_cap.w,0));
   y_mul=fix_div(fix_make(200,0),fix_make(grd_mode_cap.h,0));
}

extern int last_real_time;

ushort olh_scan_objs(void)
{
   int xl,yl;
   int col;
   int (*fr_ptr_idx)(void)=fr_get_idx;
   short x,y;
   struct _obj_scandata
   {
      ObjID obj;
      int x,y;  // TOTAL x and y coordinates for all samples
      ushort count;
   } objdata[SCAN_OBJ_LIST];
   short objcount = 0;
#ifdef SVGA_SUPPORT
   fauxrend_context* fr = olh_full_context;
#else
   fauxrend_context* fr = (full_game_3d) ? olh_full_context : olh_context;
#endif
   ubyte save_radius = _frp.view.radius;
   
   // Do a monochrome render
//   olh_replace_view(SCREEN_CONTEXT->xwid/SCAN_RATIO,SCREEN_CONTEXT->ywid/SCAN_RATIO);
//   _fr_top(fr);
   _fr_glob_flags|=FR_PICKUPM_MASK;
   fr_cur_obj_col=FR_CUR_OBJ_BASE;
   fr_get_idx=fr_pickup_idx;
   _frp.view.radius = olh_radius;
   fr_rend(fr);
   _frp.view.radius = save_radius;
   fr_get_idx=fr_ptr_idx;
   _fr_glob_flags&=~FR_PICKUPM_MASK;

   if (*tmd_ticks - last_real_time < CIT_CYCLE/15)
      olh_radius = min(olh_radius+1,MAX_OLH_RADIUS);
   else if (*tmd_ticks - last_real_time > CIT_CYCLE/10)
      olh_radius = max(olh_radius-1,MIN_OLH_RADIUS);


   xl = yl = 0;

//#define CRAZY_DEBUGGING_BLIT
#ifdef CRAZY_DEBUGGING_BLIT 
   gr_push_canvas(grd_screen_canvas);
   gr_bitmap(&fr->draw_canvas.bm,0,200-fr->draw_canvas.bm.h);
   gr_pop_canvas();
#endif 

   olh_object.obj = OBJ_NULL;
   // collect samples
   for (y = yl; y < fr->draw_canvas.bm.h; y ++)
      for (x = xl; x < fr->draw_canvas.bm.w; x ++)
      {
         if (y > BOTTOM_MARGIN
            && x > fr->draw_canvas.bm.w - RIGHT_MARGIN)
            break;
         col=(int)(*((fr->draw_canvas.bm.bits)+(y*fr->draw_canvas.bm.row)+(x)));
         if ((col>=FR_CUR_OBJ_BASE)&&(col<fr_cur_obj_col))         // if we are actually exactly over an object
         {
            ObjID obj = (ObjID)fr_col_to_obj[col-FR_CUR_OBJ_BASE];
            if (olh_candidate(obj))
            {
               int i;
               for (i = 0; i < objcount; i++)
                  if (objdata[i].obj == obj)
                  {
                     objdata[i].x += x;
                     objdata[i].y += y;
                     objdata[i].count++;
                     goto found;
                  }
               // not found, so add the object
               if (objcount >= SCAN_OBJ_LIST) // no more room in list, how sad.
                  continue;
               i = objcount++;
               objdata[i].obj = obj;
               objdata[i].x = x;
               objdata[i].y = y;
               objdata[i].count = 1;
            found:
//               mprintf ("found %d at (%d,%d) (x,y) now (%d,%d), count now %d\n",obj,x,y,objdata[i].x,objdata[i].y,objdata[i].count);
               ;
            }
         }
      }

   // now pick the best one.  
   if (objcount > 0)
   {
      int i;
      uint best_weight = 0;
      for (i = 0; i < objcount; i++)
      {
         struct _obj_scandata *dat = &objdata[i];
         LGPoint pos = MakePoint((short)(dat->x/dat->count),(short)(dat->y/dat->count));
         uint weight =  dat->count * (fr->xwid + fr->ywid)/(abs(2*pos.x - fr->xwid) + abs(2*pos.y - fr->ywid) + 1);
         if (weight > best_weight)
         {
            olh_object.obj = dat->obj;
            olh_object.loc = pos;
            best_weight = weight;
         }
      }
#ifdef SVGA_SUPPORT
	   {
         fix tmp;
         tmp=fix_make(olh_object.loc.x,0); tmp=fix_mul(tmp,x_mul); olh_object.loc.x=fix_int(tmp);
         tmp=fix_make(olh_object.loc.y,0); tmp=fix_mul(tmp,y_mul); olh_object.loc.y=fix_int(tmp);
      }
#endif
   }
   else 
      olh_object.obj = OBJ_NULL;
   return OBJ_NULL;
   
}
