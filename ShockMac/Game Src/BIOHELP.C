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
 * $Source: r:/prj/cit/src/RCS/biohelp.c $
 * $Revision: 1.9 $
 * $Author: xemu $
 * $Date: 1994/10/20 18:59:37 $
 *
 */

#include "mfdint.h"
#include "mfdext.h"
#include "mfddims.h"
#include "mfdgadg.h"
#include "status.h"
#include "gamestrn.h"
#include "mfdint.h"
#include "tools.h"
#include "cit2d.h"
#include "fullscrn.h"

#include "cybstrng.h"
#include "mfdart.h"
#include "gr2ss.h"


// ============================================================
//                   THE BIO HELP MFD
// ============================================================


// -------
// DEFINES
// -------
bool status_track_free(int track);
bool status_track_active(int track);
void status_track_activate(int track, bool active);

errtype mfd_biohelp_init(MFD_Func* f);
void mfd_biohelp_expose(MFD* mfd, ubyte control);
bool mfd_biohelp_button_handler(MFD* m, LGPoint bttn, uiEvent* ev, void* data);
bool mfd_biohelp_handler(MFD* m, uiEvent* e);
bool biohelp_region_mouse_handler(uiMouseEvent* ev, LGRegion* r, void* data);
errtype biohelp_load_cursor();
errtype biohelp_create_mouse_region(LGRegion* root);


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

#define MFD_BIOHELP_FUNC 25

#define ARROW_WID 5
#define ARROW_X (MFD_VIEW_WID -1 - ARROW_WID)
#define ARROW_Y (MFD_VIEW_HGT - 10)


#define LEFT_MARGIN     1
#define TOP_MARGIN      1
#define NUM_BUTTONS     4
#define BARRY_HGT       (MFD_VIEW_HGT - 2*TOP_MARGIN)
#define BARRY_WID       (ARROW_X - LEFT_MARGIN)
#define BUTTON_WID      12
#define BUTTON_HGT      11
#define TEXT_HGT        5

#define ITEM_COLOR  (0x5A)


#define LAST_ACTIVE_BITS(mfd) (player_struct.mfd_func_data[MFD_BIOHELP_FUNC][mfd])
#define LAST_USED_BITS(mfd)   (player_struct.mfd_func_data[MFD_BIOHELP_FUNC][mfd+6])
#define BIOHELP_PAGE (player_struct.mfd_func_data[MFD_BIOHELP_FUNC][2]) 
#define NUM_TRACKS   (player_struct.mfd_func_data[MFD_BIOHELP_FUNC][3])

void mfd_biohelp_expose(MFD* mfd, ubyte control)
{
   bool full = control & MFD_EXPOSE_FULL;
   if (control == 0)  // MFD is drawing stuff
   {
      // Do unexpose stuff here.  
   }
   if (control & MFD_EXPOSE) // Time to draw stuff
   {
      int i;
      int firsttrack = 0;
      int track = 0;
      ubyte bits = 0;
      // clear update rects
      mfd_clear_rects();
      // set up canvas
      gr_push_canvas(pmfd_canvas);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

      // Clear the canvas by drawing the background bitmap
      if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
           gr_bitmap(&mfd_background, 0, 0);

      // figure out where to start, and what tracks are active.
      for (track = 0,i = 0; track < NUM_BIO_TRACKS; track++)
      {
         if (status_track_active(track))
         {
            bits |= 1 << track;
         }
         if (!status_track_free(track))
         {
            if (i == NUM_BUTTONS*BIOHELP_PAGE)
               firsttrack = track;
            LAST_USED_BITS(mfd->id) |= 1 << track; 
            i++;
         }
         else
            LAST_USED_BITS(mfd->id) &= ~(1 << track);
      }
      if (i > NUM_BUTTONS)
      {
         int id = REF_IMG_TinyArrowUp + BIOHELP_PAGE;
         draw_raw_resource_bm(id,ARROW_X,ARROW_Y);
      }
      NUM_TRACKS = i;
      full = full || bits != LAST_ACTIVE_BITS(mfd->id);
      if (full)
         for (i = 0,track = firsttrack; i < NUM_BUTTONS && track < NUM_BIO_TRACKS; i++,track++)
         {
            char buf[50];
            short x,y;
            while(status_track_free(track))
            {
               track++;
               if (track >= NUM_BIO_TRACKS)
                  goto break_out;
            }
            x = LEFT_MARGIN;
            y = TOP_MARGIN + BARRY_HGT*i/NUM_BUTTONS;
            draw_raw_resource_bm(REF_IMG_BioIcon1 + track,x,y);
            if (!(bits & (1 << track)))
               draw_raw_resource_bm(REF_IMG_BioIconNot,x,y);
            mfd_add_rect(x,y,x+BARRY_WID,y+BARRY_HGT);
            x += BUTTON_WID + LEFT_MARGIN;
            y += (BUTTON_HGT - TEXT_HGT)/2;
            get_string(REF_STR_BioHelpBase+track,buf,sizeof(buf));
            mfd_draw_string(buf, x, y, ITEM_COLOR, TRUE);
         }  
   break_out:
      LAST_ACTIVE_BITS(mfd->id) = bits;


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


// --------
// HANDLERS 
// --------

bool mfd_biohelp_button_handler(MFD*, LGPoint bttn, uiEvent* ev, void*)
{
   int track = -1;
   int i = 0;
   if (!(ev->subtype & MOUSE_LDOWN))
      return FALSE;
   while (i <= bttn.y+NUM_BUTTONS*BIOHELP_PAGE)
   {
      track++;
      if (track >= NUM_BIO_TRACKS)
         return FALSE;
      if (!status_track_free(track))
         i++;
   }
   status_track_activate(track,!status_track_active(track));
   if (status_track_active(track))
      player_struct.active_bio_tracks |= 1 << track;
   else
      player_struct.active_bio_tracks &= ~(1 << track);
   mfd_notify_func(MFD_BIOHELP_FUNC,MFD_INFO_SLOT,FALSE,MFD_ACTIVE,FALSE);
   return TRUE;
}


bool mfd_biohelp_handler(MFD* m, uiEvent* e)
{
   bool retval = FALSE;
   LGPoint pos = e->pos;
   if (NUM_TRACKS <= NUM_BUTTONS)
      return FALSE;
   if (!(e->subtype & MOUSE_LDOWN))
      return FALSE;
   pos.x -= m->rect.ul.x;
   pos.y -= m->rect.ul.y;
   if (pos.x > ARROW_X && pos.y > ARROW_Y)
   {
      BIOHELP_PAGE = !BIOHELP_PAGE;
      mfd_notify_func(MFD_BIOHELP_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, TRUE);
      retval = TRUE;
   }
   return retval;
}


bool biohelp_region_mouse_handler(uiMouseEvent* ev, LGRegion*, void*)
{
   if (ev->action & (MOUSE_LDOWN|MOUSE_RDOWN))
   {
      extern void mfd_zoom_rect(LGRect*,int);
      LGRect start = {{ -5, -5}, {5,5}};
      int mfd = mfd_grab_func(MFD_BIOHELP_FUNC,MFD_INFO_SLOT);
	   RECT_MOVE(&start,ev->pos);
      mfd_zoom_rect(&start,mfd);

      mfd_notify_func(MFD_BIOHELP_FUNC,MFD_INFO_SLOT,TRUE,MFD_ACTIVE,TRUE);
      mfd_change_slot(mfd,MFD_INFO_SLOT);
      return TRUE;
   }
   return FALSE;
}


LGCursor biohelp_cursor;
grs_bitmap biohelp_cursor_bmap;

errtype biohelp_load_cursor()
{
   errtype err; 
   static bool cursor_loaded = FALSE;
   extern errtype simple_load_res_bitmap_cursor(LGCursor* c, grs_bitmap* bmp, Ref rid);
   if (cursor_loaded)
      DisposePtr((Ptr)biohelp_cursor_bmap.bits);
   err = simple_load_res_bitmap_cursor(&biohelp_cursor,&biohelp_cursor_bmap,REF_IMG_QuestionCursor);
   if (err != OK) return err;
   cursor_loaded = TRUE;
   return(err);
}

errtype biohelp_create_mouse_region(LGRegion* root)
{ 
   errtype err; 
   int id;
   LGRect r = { { STATUS_X, 0}, {STATUS_X+GAMESCR_BIO_WIDTH,GAMESCR_BIO_HEIGHT}};
   LGRegion* reg = (LGRegion*)NewPtr(sizeof(LGRegion));

   if (reg == NULL) return ERR_NOMEM;
   err = region_create(root,reg,&r,2,0,REG_USER_CONTROLLED|AUTODESTROY_FLAG,NULL,NULL,NULL,NULL);
   if (err != OK) return err;
   err = uiInstallRegionHandler(reg,UI_EVENT_MOUSE,(uiHandlerProc)biohelp_region_mouse_handler,NULL,&id);
   if (err != OK) return err;
   biohelp_load_cursor();
   uiSetRegionDefaultCursor(reg,&biohelp_cursor);
   return OK;
}

errtype mfd_biohelp_init(MFD_Func* f)
{
   errtype err;
   LGPoint bsize = { BARRY_WID, BUTTON_HGT};
   LGPoint bdims = { 1, NUM_BUTTONS};
   LGRect r = { { LEFT_MARGIN, TOP_MARGIN},
              { ARROW_X, TOP_MARGIN + BARRY_HGT }};
   extern LGRegion* root_region;
   err = biohelp_create_mouse_region(root_region);
   if (err != OK) return err;
   err = MFDBttnArrayInit(&f->handlers[0],&r,bdims,bsize,mfd_biohelp_button_handler,NULL);
   if (err != OK) return err;
   f->handler_count = 1;
   return OK;
}
