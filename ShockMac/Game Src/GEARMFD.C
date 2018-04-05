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
 * $Source: r:/prj/cit/src/RCS/gearmfd.c $
 * $Revision: 1.8 $
 * $Author: xemu $
 * $Date: 1994/10/16 15:53:14 $
 *
 */

#include "mfdint.h"
#include "mfdext.h"
#include "mfddims.h"
#include "tools.h"
#include "otrip.h"
#include "objbit.h"
#include "mfdart.h"
#include "objuse.h"
#include "cit2d.h"
#include "cybstrng.h"
#include "gr2ss.h"


// ============================================================
//                   THE GEAR MFD
// ============================================================

// -------
// DEFINES
// -------
extern bool full_game_3d;

bool gear_active(ObjID obj);
void mfd_gear_expose(MFD* mfd, ubyte control);
bool mfd_gear_handler(MFD *m, uiEvent *e);

// ---------------
// EXPOSE FUNCTION
// ---------------

/* This gets called whenever the MFD needs to redraw or
   undraw.  
   The control value is a bitmask with the following bits:
   MFD_EXPOSE: Update the mfd, if MFD_EXPOSE_FULL is not set,
               update incrementally.  
   MFD_EXPOSE_FULL: Fully redraw the mfd, implies MFD_EXPOSE

   if no bits are set, the mfd is being "unexposed;" its display
   being pulled off the screen to make room for a different func.
*/

bool gear_active(ObjID obj)
{
   switch (ID2TRIP(obj))
   {
      case TRACBEAM_TRIPLE:
         return (objs[obj].info.inst_flags & CLASS_INST_FLAG) != 0;
         break;
   }
   return TRUE;
}

#define GEAR_BUTTON_H 13
#define GEAR_BUTTON_Y (MFD_VIEW_HGT - GEAR_BUTTON_H -1)


ushort button_bitmaps[] =
   {
     0,
     (ushort)REFINDEX(REF_IMG_Use),
     0,
     (ushort)REFINDEX(REF_IMG_Use),
     (ushort)REFINDEX(REF_IMG_Active),
     (ushort)REFINDEX(REF_IMG_Use),
   };

extern void mfd_item_micro_expose(bool full, int triple);

void mfd_gear_expose(MFD* mfd, ubyte control)
{
   int active = player_struct.actives[ACTIVE_GENERAL];
   bool full = control & MFD_EXPOSE_FULL;
   extern void draw_mfd_item_spew(Ref id, int n);
   if (control == 0)  // MFD is drawing stuff
   {
      // Do unexpose stuff here.  
   }
   if (control & MFD_EXPOSE) // Time to draw stuff
   {
      ObjID obj;
      // clear update rects
      mfd_clear_rects();
      // set up canvas
      gr_push_canvas(pmfd_canvas);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

      // Clear the canvas by drawing the background bitmap
      if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
         gr_bitmap(&mfd_background, 0, 0);

      if (active < 0 || active >= NUM_GENERAL_SLOTS)
      {
         mfd_notify_func(MFD_EMPTY_FUNC,MFD_ITEM_SLOT,TRUE,MFD_EMPTY,FALSE);
         goto cleanup_et_return;
      }
      obj = player_struct.inventory[active];

      if (obj == OBJ_NULL)
      {
         mfd_notify_func(MFD_EMPTY_FUNC,MFD_ITEM_SLOT,TRUE,MFD_EMPTY,FALSE);
         goto cleanup_et_return;
      }
      else
      {
         int id = MKREF(RES_mfdSpecial,button_bitmaps[objs[obj].info.type]+(gear_active(obj)?0:1));
         short wid = res_bm_width(id);
         short x = (MFD_VIEW_WID - wid)/2;

         mfd_item_micro_expose(full,ID2TRIP(obj));
         draw_mfd_item_spew(REF_STR_gearSpew0+objs[obj].info.type,1);
         draw_res_bm(id,x,GEAR_BUTTON_Y);
         mfd_add_rect(x,GEAR_BUTTON_Y,x + wid,MFD_VIEW_HGT);
      }

      // on a full expose, make sure to draw everything
 
      if (full)
         mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

cleanup_et_return:
      // Pop the canvas
      gr_pop_canvas();
      // Now that we've popped the canvas, we can send the 
      // updated mfd to screen
      mfd_update_rects(mfd);
   }
  
}


// -------
// HANDLER
// -------

bool mfd_gear_handler(MFD *m, uiEvent *e)
{
   bool retval = FALSE;
   int active = player_struct.actives[ACTIVE_GENERAL];
   LGRect r = { {0, GEAR_BUTTON_Y},{MFD_VIEW_WID, GEAR_BUTTON_Y+GEAR_BUTTON_H}};
   
   if ((e->subtype & (MOUSE_LDOWN|UI_MOUSE_LDOUBLE)) == 0)
      return FALSE;
   if (active >= 0 && active < NUM_GENERAL_SLOTS)
   {
      ObjID obj = player_struct.inventory[active];
      int id = MKREF(RES_mfdSpecial,button_bitmaps[objs[obj].info.type]+(gear_active(obj)?0:1));
      short wid = res_bm_width(id);
      r.ul.x = (MFD_VIEW_WID - wid)/2;
      r.lr.x = r.ul.x + wid;
      RECT_OFFSETTED_RECT(&r,m->rect.ul,&r);
      if (RECT_TEST_PT(&r,e->pos))
      {
         object_use(obj,TRUE,OBJ_NULL);
         retval = TRUE;
      }
   }
   return retval;
}
