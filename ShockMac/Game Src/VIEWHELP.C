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
 * $Source: r:/prj/cit/src/RCS/viewhelp.c $
 * $Revision: 1.7 $
 * $Author: xemu $
 * $Date: 1994/11/20 05:36:11 $
 *
 */
#include "mfdint.h"
#include "mfdext.h"
#include "mfddims.h"
#include "mfdgadg.h"
#include "gamestrn.h"
#include "mfdint.h"
#include "tools.h"
#include "hud.h"
#include "fullscrn.h"
#include "cit2d.h"
#include "gr2ss.h"

#include "gamescr.h"
#include "cybstrng.h"
#include "mfdart.h"
#include "miscqvar.h"


// ----------
//  PROTOTYPES
// ----------
void mfd_viewhelp_expose(MFD* mfd, ubyte control);
bool mfd_viewhelp_button_handler(MFD* m, LGPoint bttn, uiEvent* ev, void* data);
bool mfd_viewhelp_color_handler(MFD*, LGPoint bttn, uiEvent* ev, void*);
errtype install_color_handler(MFD_Func* f);
errtype mfd_viewhelp_init(MFD_Func* f);


// ============================================================
//                   THE VIEW HELP MFD
// ============================================================

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

#define MFD_VIEWHELP_FUNC 29

#define LEFT_MARGIN     1
#define TOP_MARGIN      1
#define NUM_BUTTONS     4
#define BARRY_HGT       (3*MFD_VIEW_HGT/4)
#define BARRY_WID       (MFD_VIEW_WID - LEFT_MARGIN)
#define BUTTON_WID      12
#define BUTTON_HGT      11
#define TEXT_HGT        5

#define COLOR_TITLE_Y (BARRY_HGT )
#define COLORS_X      (MFD_VIEW_WID/4)
#define COLORS_Y      (COLOR_TITLE_Y + 6)
#define COLORS_HGT    (MFD_VIEW_HGT - COLORS_Y - 1)
#define COLORS_WID    (MFD_VIEW_WID/2)
#define COLORS_BWID  7
#define COLORS_BHGT  7

#define ITEM_COLOR  (0x37)
#define DULL_ITEM_COLOR (0x5F)


#define LAST_ON_BITS(mfd) (player_struct.mfd_func_data[MFD_VIEWHELP_FUNC][mfd])
#define LAST_HUD_COLOR(mfd) (player_struct.mfd_func_data[MFD_VIEWHELP_FUNC][mfd+2])

#define BOOL_FIELD_STRING(n) (REF_STR_ViewHelpBase + (n))
#define BOOL_FIELD_ON_MSG(n) (REF_STR_ViewHelpOnMsg + (n))
#define BOOL_FIELD_OFF_MSG(n) (REF_STR_ViewHelpOffMsg + (n))
#define BOOL_FIELD_BMAP(n)   (REF_IMG_ViewIcon1 +(n))

#define BAR_SINISTER REF_IMG_BioIconNot

bool map_notes_on = TRUE;
extern bool fullscrn_vitals;
extern bool fullscrn_icons;

struct _field_data 
{
   bool* var;
   bool fullscrn;
   int qvar;
} checkbox_fields []  =
{
   { &fullscrn_vitals, TRUE, FULLSCRN_VITAL_QVAR },
   { &fullscrn_icons, TRUE, FULLSCRN_ICON_QVAR},
   { &map_notes_on, FALSE, AMAP_NOTES_QVAR},  
};

#define NUM_CHECKBOX_FIELDS (sizeof(checkbox_fields)/sizeof(struct _field_data))

void mfd_viewhelp_expose(MFD* mfd, ubyte control)
{
   bool full = control & MFD_EXPOSE_FULL;
   if (control == 0)  // MFD is drawing stuff
   {
      // Do unexpose stuff here.  
   }
   if (control & MFD_EXPOSE) // Time to draw stuff
   {
      int i;
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
      for (i = 0; i < NUM_CHECKBOX_FIELDS; i++)
      {
         struct _field_data *fd = &checkbox_fields[i];

         if (*fd->var)
         {
            bits |= 1 << i;
         }
      }
      full = full || bits != LAST_ON_BITS(mfd->id) || hud_color_bank != LAST_HUD_COLOR(mfd->id);
      if (full)
      {
         for (i = 0; i < NUM_CHECKBOX_FIELDS ; i++)
         {
            ubyte clr = ITEM_COLOR;
            short w,h;
            char buf[50];
            short x,y;

            x = LEFT_MARGIN;
            y = TOP_MARGIN + BARRY_HGT*i/NUM_CHECKBOX_FIELDS;
            draw_raw_resource_bm(BOOL_FIELD_BMAP(i),x,y);
            if (!(bits & (1 << i)))
               draw_raw_resource_bm(BAR_SINISTER,x,y);
            mfd_add_rect(x,y,x+BARRY_WID,y+BARRY_HGT);
            x += BUTTON_WID + LEFT_MARGIN;
            get_string(BOOL_FIELD_STRING(i),buf,sizeof(buf));
            gr_set_font((grs_font*)ResLock(MFD_FONT));
            gr_string_size(buf,&w,&h);
            y += (BUTTON_HGT-h)/2;
            if (checkbox_fields[i].fullscrn && !full_game_3d)
               clr = DULL_ITEM_COLOR;
            mfd_draw_string(buf, x, y, clr, TRUE);
            ResUnlock(MFD_FONT);

         }
         mfd_draw_string(get_temp_string(REF_STR_HudColorsTitle),COLORS_X, COLOR_TITLE_Y, ITEM_COLOR, TRUE);
         for (i = 0; i < HUD_COLOR_BANKS; i++)
         {
            short x = COLORS_X + (COLORS_WID-COLORS_BWID)*i/(HUD_COLOR_BANKS-1);
            gr_set_fcolor(hud_colors[i][2]);
            ss_rect(x,COLORS_Y,x + COLORS_BWID, COLORS_Y + COLORS_BHGT);
            gr_set_fcolor(hud_colors[i][0]);
            ss_rect(x+2,COLORS_Y+2,x + COLORS_BWID-2, COLORS_Y + COLORS_BHGT-2);
            if (i == hud_color_bank)
            {
               gr_set_fcolor(ITEM_COLOR);
               ss_box(x-1,COLORS_Y-1,x+COLORS_BWID+1,COLORS_Y + COLORS_BHGT + 1);
            }
         }

      }
      LAST_ON_BITS(mfd->id) = bits;
      LAST_HUD_COLOR(mfd->id) = hud_color_bank;

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

bool mfd_viewhelp_button_handler(MFD*, LGPoint bttn, uiEvent* ev, void*)
{
   int track = -1;
   int i = bttn.y;
   if (!(ev->subtype & (MOUSE_LDOWN|UI_MOUSE_LDOUBLE)))
      return FALSE;
   *checkbox_fields[i].var = !*checkbox_fields[i].var;
   QUESTVAR_SET(checkbox_fields[i].qvar, *checkbox_fields[i].var);
   string_message_info((*checkbox_fields[i].var)? BOOL_FIELD_ON_MSG(i) : BOOL_FIELD_OFF_MSG(i));
   mfd_notify_func(MFD_VIEWHELP_FUNC,MFD_INFO_SLOT,FALSE,MFD_ACTIVE,FALSE);
   return TRUE;
}

bool mfd_viewhelp_color_handler(MFD*, LGPoint bttn, uiEvent* ev, void*)
{
   if (!(ev->subtype & (MOUSE_LDOWN|UI_MOUSE_LDOUBLE)))
      return FALSE;
   hud_color_bank = bttn.x;
   QUESTVAR_SET(HUDCOLOR_QVAR, hud_color_bank);
   mfd_notify_func(MFD_VIEWHELP_FUNC,MFD_INFO_SLOT,FALSE,MFD_ACTIVE,FALSE);
   return TRUE;
}

errtype install_color_handler(MFD_Func* f)
{
   errtype err;
   LGPoint bsize = { COLORS_BWID, COLORS_BHGT};
   LGPoint bdims = { HUD_COLOR_BANKS, 1 };
   LGRect r = { { COLORS_X, COLORS_Y},
              { COLORS_X + COLORS_WID, COLORS_Y + COLORS_HGT }};

   err = MFDBttnArrayInit(&f->handlers[f->handler_count++],&r,bdims,bsize,mfd_viewhelp_color_handler,NULL);
   return err;
}


errtype mfd_viewhelp_init(MFD_Func* f)
{
   errtype err;
   LGPoint bsize = { BARRY_WID, BUTTON_HGT};
   LGPoint bdims = { 1, NUM_CHECKBOX_FIELDS};
   LGRect r = { { LEFT_MARGIN, TOP_MARGIN},
              { LEFT_MARGIN + BARRY_WID, TOP_MARGIN + BARRY_HGT }};
   err = MFDBttnArrayInit(&f->handlers[0],&r,bdims,bsize,mfd_viewhelp_button_handler,NULL);
   if (err != OK) return err;
   f->handler_count = 1;
   return install_color_handler(f);
}
