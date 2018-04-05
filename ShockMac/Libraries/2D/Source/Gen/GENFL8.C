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
 * $Source: r:/prj/lib/src/2d/RCS/genfl8.c $
 * $Revision: 1.7 $
 * $Author: kevin $
 * $Date: 1994/11/01 11:45:21 $
 *
 * Generic flat 8 bitmap routines.
 *
 * $Log: genfl8.c $
 * Revision 1.7  1994/11/01  11:45:21  kevin
 * Don't try to draw bitmaps with height or width of zero.
 * 
 * Revision 1.6  1994/08/16  13:06:23  kevin
 * gen_flat8_bitmap chains to gr_bitmap instead of gr_flat8_bitmap
 * so it may now be used with translucent bitmaps as well.
 * 
 * Revision 1.5  1993/10/19  09:51:11  kaboom
 * Replaced #include "grd.h" with new headers split from grd.h.
 * 
 * Revision 1.4  1993/10/02  01:17:22  kaboom
 * Changed include of clip.h to include of clpcon.h and/or clpfcn.h.
 * 
 * Revision 1.3  1993/06/04  10:28:04  kaboom
 * Inlined clipping code so clipped bitmap can be called from an
 * interrupt service routine.
 * 
 * Revision 1.2  1993/04/29  18:40:27  kaboom
 * Changed include of gr.h to smaller more specific grxxx.h.
 * 
 * Revision 1.1  1993/02/16  15:42:37  kaboom
 * Initial revision
 * 
 ********************************************************************
 * Log from old general.c:
 *
 * Revision 1.9  1992/12/14  18:13:37  kaboom
 * Fixed bug in gen_flat8_bitmap -- was checking result of clip before
 * doing the clip.
 *
 * Revision 1.7  1992/12/11  13:58:09  kaboom
 * Changed all the calls from gr_set_pixel to gr_set_upixel in primitives
 * that have analytic clipping.  Changed chain in gen_mono_bitmap()
 * from general to specific (e.g., gr_ubitmap->gr_mono_ubitmap).
 */

#include "bitmap.h"
#include "clpcon.h"
#include "cnvdat.h"
#include "grdbm.h"
#include "grpix.h"
#include "general.h"

/* bozo flat8 bitmap drawer. */
void gen_flat8_ubitmap (grs_bitmap *bm, short x, short y)
{
   uchar *src  = bm->bits;
   short right = x+bm->w;
   short bot   = y+bm->h;
   short cur_x;

   if (bm->flags & BMF_TRANS) {
      for ( ; y<bot; y++, src+=bm->row-bm->w) {
         for (cur_x=x ; cur_x<right; cur_x++, src++)
            if (*src)
               gr_set_upixel (*src, cur_x, y);
      }
   }
   else {
      for ( ; y<bot; y++, src+=bm->row-bm->w) {
         for (cur_x=x ; cur_x<right; cur_x++, src++)
            gr_set_upixel (*src, cur_x, y);
      }
   }
}

/* clip flat8 bitmap against cliprect and jump to unclipped drawer. */
int gen_flat8_bitmap (grs_bitmap *bm, short x, short y)
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
   if ((bm->h>0)&&(bm->w>0)) 
      gr_ubitmap (bm, x, y);

   /* restore bitmap to normal. */
   bm->w = w; bm->h = h; bm->bits = p;
   return code;
}
