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
 * $Source: r:/prj/cit/src/RCS/mfdgump.c $
 * $Revision: 1.18 $
 * $Author: dc $
 * $Date: 1994/11/28 06:38:23 $
 *
 *
 */

#include "mfdint.h"
#include "mfdext.h"
#include "mfddims.h"
#include "objapp.h"
#include "tools.h"
#include "gamestrn.h"
#include "objuse.h"
#include "objbit.h"
#include "input.h"
#include "objprop.h"
#include "colors.h"
#include "objload.h"
#include "fullscrn.h"
#include "cit2d.h"
#include "gr2ss.h"

#include "cybstrng.h"
#include "mfdart.h"
#include "gamescr.h"

// ============================================================
//                   MFD CONTAINER GUMPS
// ============================================================


/* This is the MFD for buttons that zoom into the MFD. */ 


// -------
// DEFINES
// -------

#define MFD_GUMP_FUNC 23

#define NUM_CONTENTS 4
#define Y_STEP 15
#define FIRST_ITEM_Y 15
#define LEFT_MARGIN 5
#define CONTENTS_WID ((MFD_VIEW_WID - 2*LEFT_MARGIN)/2)
#define CONTENTS_HGT ((MFD_VIEW_HGT - FIRST_ITEM_Y - 5)/2)
extern char container_extract(ObjID *pidlist, int d1, int d2);
extern void container_stuff(ObjID *pidlist, int numobjs, int* d1, int* d2);
extern bool is_container(ObjID id, int** d1, int** d2);

#define LAST_INPUT_ROW (player_struct.mfd_func_data[MFD_GUMP_FUNC][0])
#define LAST_DOUBLE    (player_struct.mfd_func_data[MFD_GUMP_FUNC][1])


// -------
// GLOBALS
// -------
ObjID gump_idlist[NUM_CONTENTS];
uchar gump_num_objs;


// -----------
// PROTOTYPES
// -----------
void mfd_gump_expose(MFD* mfd, ubyte control);
void gump_clear(void);
bool gump_pickup(byte row);
bool gump_get_useful(void);
bool mfd_gump_handler(MFD* m, uiEvent* uie);


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

extern void check_panel_ref(bool punt);


void mfd_gump_expose(MFD* mfd, ubyte control)
{
   bool full = control & MFD_EXPOSE_FULL;
   if (control == 0)  // MFD is drawing stuff
   {
      panel_ref_unexpose(mfd->id,MFD_GUMP_FUNC);
      return;
   }
   if (control & MFD_EXPOSE) // Time to draw stuff
   {
      extern void mfd_item_micro_expose(bool full,int triple);
      ObjID id = player_struct.panel_ref;
      uchar i;

      // clear update rects
      mfd_clear_rects();
      // set up canvas
      gr_push_canvas(pmfd_canvas);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

      // Clear the canvas by drawing the background bitmap
      if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
         gr_bitmap(&mfd_background, 0, 0);
      mfd_item_micro_expose(full,ID2TRIP(id));

      if (full)
      {
         int *d1,*d2;
         is_container(id,&d1,&d2); // fill in d1 and d2;
         gump_num_objs = container_extract(gump_idlist,*d1,(d2 != NULL) ? *d2 : 0);
         for (i = gump_num_objs; i < sizeof(gump_idlist)/sizeof(gump_idlist[0]); i++)
            gump_idlist[i] = OBJ_NULL;
         LAST_INPUT_ROW = 0xFF;
      }
      gr_set_font((grs_font*)ResLock(MFD_FONT));
      if (gump_num_objs == 0)
      {
         short x,y;
         char* s = get_temp_string(REF_STR_EmptyGump);
         gr_string_size(s,&x,&y);
         x = (MFD_VIEW_WID - x) /2;
         y = (MFD_VIEW_HGT - y) /2;
         mfd_draw_string(s,x,y,GREEN_YELLOW_BASE,TRUE);
      }
      else 
         for (i = 0; i < gump_num_objs; i++)
         {
            short x,y;
            uchar r = i/2,c = i%2;
            if (gump_idlist[i] != OBJ_NULL)
            {
               grs_bitmap* bm = bitmaps_2d[OPNUM(gump_idlist[i])];
               x = LEFT_MARGIN  + ((c == 0) ? 0 : CONTENTS_WID) + (CONTENTS_WID - bm->w)/2;
               y = FIRST_ITEM_Y + ((r == 0) ? 0 : CONTENTS_HGT) + (CONTENTS_HGT - bm->h)/2;
               ss_bitmap(bm,x,y);
            }
            // the +1 in the last argument is to get 
            // mfd_add_rect to union adjacents...
         }
      mfd_add_rect(LEFT_MARGIN,FIRST_ITEM_Y,LEFT_MARGIN+2*CONTENTS_WID,FIRST_ITEM_Y+2*CONTENTS_HGT);
      ResUnlock(MFD_FONT);
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
extern void mouse_unconstrain(void);

void gump_clear(void)
{
   int i;

   for(i=0;i<NUM_CONTENTS;i++)
      gump_idlist[i]=OBJ_NULL;
}

bool gump_pickup(byte row)
{
   int *d1, *d2;
   ObjID cont = player_struct.panel_ref;
   extern void check_panel_ref(bool puntme);

//KLC   mouse_unconstrain();
   if (row < 0 || row >= gump_num_objs || gump_idlist[row] == OBJ_NULL) return FALSE;
   push_cursor_object(gump_idlist[row]);
   gump_idlist[row] = OBJ_NULL;
   if (row == gump_num_objs-1)
      gump_num_objs--;
   LAST_INPUT_ROW = 0xFF;
   LAST_DOUBLE = FALSE;
   // Here's where we update the container object
   is_container(cont,&d1,&d2);
   container_stuff(gump_idlist,gump_num_objs,d1,d2);
   
   if(*d1==0 && (d2==NULL || *d2==0))
      check_panel_ref(TRUE); // punt empty gump
   else
      mfd_notify_func(MFD_GUMP_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,FALSE);
   return TRUE;
}

bool gump_get_useful(void)
{
   int row;
   bool useless;
   bool obj_is_useless(ObjID oid);

   for(useless=0;useless<=1;useless++) {
      for(row=0;row<gump_num_objs;row++) {
         if(gump_idlist[row] && obj_is_useless(gump_idlist[row])==useless) {
            return(gump_pickup(row));
         }
      }
   }
   return FALSE;
}

bool mfd_gump_handler(MFD* m, uiEvent* uie)
{
   uiMouseEvent	*e = (uiMouseEvent *)uie;
   LGPoint pos = e->pos;
   byte row;
   short x,y;
   grs_bitmap* bm;

   pos.x -= m->rect.ul.x;
   pos.y -= m->rect.ul.y;
   row = (pos.y - FIRST_ITEM_Y)/CONTENTS_HGT;
   row = 2*row + (pos.x - LEFT_MARGIN)/CONTENTS_WID;

#ifdef RIGHT_BUTTON_GUMP_UI
   if (LAST_INPUT_ROW != 0xFF && row != LAST_INPUT_ROW)
   {
      if (e->buttons & (1 << MOUSE_RBUTTON))
      {
         return gump_pickup(LAST_INPUT_ROW);
      }
   }
#endif // RIGHT_BUTTON_GUMP_UI
   if (row < 0 || row >= gump_num_objs)
      return FALSE;
   if (LAST_DOUBLE && (e->action & MOUSE_LUP))
   {
      return gump_pickup(row);
   }
   if (!(e->action & (MOUSE_LDOWN|UI_MOUSE_LDOUBLE)))
      return FALSE;
#ifdef RIGHT_BUTTON_GUMP_UI
   if (!(e->action & (MOUSE_LDOWN|MOUSE_RDOWN|UI_MOUSE_LDOUBLE))
      && !(e->buttons & (1 << MOUSE_RBUTTON)))
         return FALSE;
#endif // RIGHT_BUTTON_GUMP_UI
   // Hey, this is a little extra work, but it gets the job done.
   bm = bitmaps_2d[OPNUM(gump_idlist[row])];
   x = LEFT_MARGIN  + ((row%2 == 0) ? 0 : CONTENTS_WID) + (CONTENTS_WID - bm->w)/2;
   y = FIRST_ITEM_Y + ((row/2 == 0) ? 0 : CONTENTS_HGT) + (CONTENTS_HGT - bm->h)/2;
   if (pos.x >= x && pos.x < x + bm->w && pos.y >= y && pos.y < y + bm->h)
   {
      if (e->action == UI_MOUSE_LDOUBLE)
      {
         LAST_DOUBLE = TRUE;
         return TRUE;
      }
      LAST_DOUBLE = FALSE;
      if (e->action & MOUSE_LDOWN)
      {
         extern void look_at_object(ObjID);
         if (gump_idlist[row] != OBJ_NULL)
            look_at_object(gump_idlist[row]);
      }
#ifdef RIGHT_BUTTON_GUMP_UI
      if (e->action & MOUSE_RDOWN)
      {
//KLC         mouse_constrain_xy(m->rect.ul.x,m->rect.ul.y,m->rect.lr.x-1,m->rect.lr.y-1);   
         LAST_INPUT_ROW = row;
         return TRUE;
      }
      if (e->action & MOUSE_RUP)
         return gump_pickup(row);
#endif // RIGHT_BUTTON_GUMP_UI
   }
   else if (e->buttons & (1 << MOUSE_RBUTTON))
   {
      return gump_pickup(row);
   }
   LAST_DOUBLE = FALSE;
//KLC   mouse_unconstrain();
   return FALSE;
}
