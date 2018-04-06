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
 * $Source: r:/prj/cit/src/RCS/fixtrmfd.c $
 * $Revision: 1.13 $
 * $Author: xemu $
 * $Date: 1994/10/16 15:52:03 $
 *
 *
 */

#include "gamescr.h"
#include "mfdint.h"
#include "mfdext.h"
#include "mfddims.h"
#include "objapp.h"
#include "tools.h"
#include "mfdart.h"
#include "gamestrn.h"
#include "cybstrng.h"
#include "objuse.h"
#include "objbit.h"
#include "cit2d.h"
#include "fullscrn.h"
#include "gr2ss.h"

// ============================================================
//                   THE FIXTURE MFD
// ============================================================


/* This is the MFD for buttons that zoom into the MFD. */ 


// -------
// DEFINES
// -------

#define TEXT_LABEL_X       6
#define TEXT_LABEL_Y       5
#define TEXT_LABEL_W       62
#define TEXT_LABEL_H       17

#define BUTTON_STATE_X     27
#define BUTTON_STATE_Y     32
#define BUTTON_STATE_W     20
#define BUTTON_STATE_H     15

#define TEXT_COLOR   0x4C

extern void check_panel_ref(bool punt);

typedef struct _fixture_data
{
   ObjID last_obj;
   uchar last_state;
} fixture_data;

#define MFD_FIXTURE_DATA(lr) ((fixture_data*)&mfd_fdata[MFD_FIXTURE_FUNC][lr*3])
#define FIXTURE_STATE (mfd_fdata[MFD_FIXTURE_FUNC][7])

// ----------
//  PROTOTYPES
// ----------
void mfd_fixture_expose(MFD* mfd, ubyte control);
bool mfd_fixture_handler(MFD* m, uiEvent* e);

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

void mfd_fixture_expose(MFD* mfd, ubyte control)
{
   bool full = control & MFD_EXPOSE_FULL;
   if (control == 0)  // MFD is drawing stuff
   {
      panel_ref_unexpose(mfd->id,MFD_FIXTURE_FUNC);
      return;
   }
   if (control & MFD_EXPOSE) // Time to draw stuff
   {
      fixture_data* fd = MFD_FIXTURE_DATA(mfd->id);

      // set panel_ref so that we get pulled away if we travel too far.

      // clear update rects
      mfd_clear_rects();
      // set up canvas
      gr_push_canvas(pmfd_canvas);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

      // Clear the canvas by drawing the background bitmap
      if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
         gr_bitmap(&mfd_background, 0, 0);
      draw_res_bm(REF_IMG_MFDButtonBack,0,0);
      if (full || player_struct.panel_ref != fd->last_obj)
      {
         short w,h;
         bool wrap = mfd_string_wrap;
         char buf[256];
         if (objs[player_struct.panel_ref].info.make_info != 0)
            get_string(REF_STR_Name0+objs[player_struct.panel_ref].info.make_info,buf,sizeof(buf));
         else get_object_long_name(ID2TRIP(player_struct.panel_ref),buf,sizeof(buf));
         gr_set_font((grs_font*)ResLock(MFD_FONT));
         wrap_text(buf,TEXT_LABEL_W);
         mfd_string_wrap = FALSE;
         gr_string_size(buf,&w,&h);
         w = (TEXT_LABEL_W - w)/2;
         h = (TEXT_LABEL_H - h)/2;
         mfd_draw_string(buf,TEXT_LABEL_X + w,TEXT_LABEL_Y + h,TEXT_COLOR,TRUE);
         ResUnlock(MFD_FONT);
         fd->last_obj = player_struct.panel_ref;
         mfd_string_wrap = wrap;
      }
      // this is button code. 
      FIXTURE_STATE = objs[player_struct.panel_ref].info.current_frame;
      if (objs[player_struct.panel_ref].info.inst_flags & CLASS_INST_FLAG2)
         FIXTURE_STATE = !FIXTURE_STATE;
      if (full || FIXTURE_STATE != fd->last_state)
      {
         // Hey, this is all button code that's going to have to be
         // yanked out and moved elsewhere
         int ref = REF_IMG_On + ((FIXTURE_STATE == 0) ? 1 : 0);
         short xoff = (BUTTON_STATE_W - res_bm_width(ref))/2;
         short yoff = (BUTTON_STATE_H - res_bm_height(ref))/2;

         draw_res_bm(ref,BUTTON_STATE_X + xoff,BUTTON_STATE_Y + yoff);
         mfd_add_rect(BUTTON_STATE_X,BUTTON_STATE_Y,BUTTON_STATE_X+BUTTON_STATE_W,BUTTON_STATE_Y+BUTTON_STATE_H);
         fd->last_state = FIXTURE_STATE;
      }

      // on a full expose, make sure to draw everything
 
      if (full)
         mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

      // Pop the canvas
      gr_pop_canvas();
      // Now that we've popped the canvas, we can send the 
      // updated mfd to screen
      mfd_update_rects(mfd);

   }
  
}

// ----------------
// HANDLER FUNCTION
// ----------------

bool mfd_fixture_handler(MFD* m, uiEvent* e)
{
	uiMouseEvent	*me = (uiMouseEvent *)e;
	LGRect brect = { { BUTTON_STATE_X, BUTTON_STATE_Y},
							{BUTTON_STATE_X + BUTTON_STATE_W, BUTTON_STATE_Y + BUTTON_STATE_H }};
	LGPoint pos = me->pos;
	pos.x -= m->rect.ul.x;
	pos.y -= m->rect.ul.y;
	if (me->action & MOUSE_LDOWN && RECT_TEST_PT(&brect,pos))
	{
		object_use(player_struct.panel_ref, TRUE, OBJ_NULL);
		mfd_notify_func(MFD_FIXTURE_FUNC, MFD_INFO_SLOT, TRUE, MFD_ACTIVE, TRUE);
		return TRUE;
	}
	return FALSE;
}
