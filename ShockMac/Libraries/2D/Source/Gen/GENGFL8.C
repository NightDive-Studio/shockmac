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
 * $Source: r:/prj/lib/src/2d/RCS/gengfl8.c $
 * $Revision: 1.6 $
 * $Author: kevin $
 * $Date: 1994/10/19 17:33:13 $
 *
 * Generic flat 8 bitmap capture routines.
 *
 * $Log: gengfl8.c $
 * Revision 1.6  1994/10/19  17:33:13  kevin
 * Save and restore 2d state in case we are run in an interrupt.
 * 
 * Revision 1.5  1993/12/15  11:26:45  kaboom
 * Inlined clipping code so can be called from an interrupt.
 * 
 * Revision 1.4  1993/10/19  09:51:13  kaboom
 * Replaced #include "grd.h" with new headers split from grd.h.
 * 
 * Revision 1.3  1993/10/02  01:17:23  kaboom
 * Changed include of clip.h to include of clpcon.h and/or clpfcn.h.
 * 
 * Revision 1.2  1993/04/29  18:40:35  kaboom
 * Changed include of gr.h to smaller more specific grxxx.h.
 * 
 * Revision 1.1  1993/02/16  15:42:44  kaboom
 * Initial revision
 */

#include "bitmap.h"
#include "clpcon.h"
#include "cnvdat.h"
#include "grgbm.h"
#include "grpix.h"
#include "grstate.h"
#include "general.h"

/* unclipped flat8 bitmap capture.  reads data from the current canvas at
   (x,y) into the bitmap described by bm.  the destination bitmap is always
   completely filled. */
void gen_get_flat8_ubitmap (grs_bitmap *bm, short x, short y)
{
   uchar *dst  = bm->bits;
   short right = x+bm->w;
   short bot   = y+bm->h;
   short cur_x;

   gr_push_state();
   if (bm->flags & BMF_TRANS) {
      for ( ; y<bot; y++, dst+=bm->row-bm->w) {
         for (cur_x=x ; cur_x<right; cur_x++, dst++)
            if (gr_get_upixel (cur_x, y))
               *dst = gr_get_upixel (cur_x, y);
      }
   } else {
      for ( ; y<bot; y++, dst+=bm->row-bm->w) {
         for (cur_x=x ; cur_x<right; cur_x++, dst++)
            *dst = gr_get_upixel (cur_x, y);
      }
   }
   gr_pop_state();
}

/* clipped flat8 bitmap capture.  reads from the current canvas at (x,y)
   into bm, clipping against the canvas.  any section of the destination
   bitmap that was clipped is not written to. */
int gen_get_flat8_bitmap (grs_bitmap *bm, short x, short y)
{
   short w,h;
   uchar *p;
   int extra;
   int code = CLIP_NONE;
   
   /* save stuff that clipping changes. */
   w = bm->w; h = bm->h; p = bm->bits;

   /* check for trivial reject. */
   if (x+bm->w<grd_clip.left || x>=grd_clip.right ||
       y+bm->h<grd_clip.top  || y>=grd_clip.bot)
      return CLIP_ALL;

   /* clip & draw that sucker. */
   if (x < grd_clip.left) {            /* off left edge */
      extra = grd_clip.left - x;
      bm->w -= extra;
      bm->bits += extra;
      x = grd_clip.left;
      code |= CLIP_LEFT;
   }
   if (x+bm->w > grd_clip.right) {     /* off right edge */
      bm->w -= x+bm->w-grd_clip.right;
      code |= CLIP_RIGHT;
   }
   if (y < grd_clip.top) {             /* off top */
      extra = grd_clip.top - y;
      bm->h -= extra;
      bm->bits += bm->row*extra;
      y = grd_clip.top;
      code |= CLIP_TOP;
   }
   if (y+bm->h > grd_clip.bot) {      /* off bottom */
      bm->h -= y+bm->h-grd_clip.bot;
      code |= CLIP_BOT;
   }
   gr_get_ubitmap (bm, x, y);

   /* restore bitmap to normal. */
   bm->w = w; bm->h = h; bm->bits = p;
   return code;
}
