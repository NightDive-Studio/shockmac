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
 * $Source: n:/project/lib/src/2d/RCS/genmono.c $
 * $Revision: 1.4 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:51:20 $
 *
 * Generic monochrome bitmap routines.
 *
 * $Log: genmono.c $
 * Revision 1.4  1993/10/19  09:51:20  kaboom
 * Replaced #include "grd.h" with new headers split from grd.h.
 * 
 * Revision 1.3  1993/10/02  01:17:28  kaboom
 * Changed include of clip.h to include of clpcon.h and/or clpfcn.h.
 * 
 * Revision 1.2  1993/04/29  18:40:38  kaboom
 * Changed include of gr.h to smaller more specific grxxx.h.
 * 
 * Revision 1.1  1993/02/16  15:42:57  kaboom
 * Initial revision
 * 
 ********************************************************************
 * Log from old general.c:
 *
 * Revision 1.7  1992/12/11  13:58:09  kaboom
 * Changed all the calls from gr_set_pixel to gr_set_upixel in primitives
 * that have analytic clipping.  Changed chain in gen_mono_bitmap()
 * from general to specific (e.g., gr_ubitmap->gr_mono_ubitmap).
 */

#include "bit.h"
#include "bitmap.h"
#include "clpcon.h"
#include "clpfcn.h"
#include "cnvdat.h"
#include "grdbm.h"
#include "grpix.h"
#include "general.h"

/* draw a monochrome bitmap with calls to gr_set_pixel for maximum device
   independence and slowness. draws 1's in the source bitmap as currently
   set foreground, and 0's are bacground if opaque, or not drawn if trans-
   parent. */
void gen_mono_ubitmap (grs_bitmap *bm, short x, short y)
{
   short w, h;                         /* working width and height */
   short dst_x;                        /* destination x */
   int bit;                            /* bit from 0-7 in source byte */
   uchar *p_row;                       /* pointer to current row of bitmap */
   uchar *p;                           /* pointer to source byte */

   h = bm->h;
   p_row = bm->bits;

   if (bm->flags & BMF_TRANS)
   {
      /* transparent bitmap; draw 1's as fcolor, don't draw 0's. */
      while (h-- > 0)
      {
         /* set up scanline. */
         bit = bm->align;
         dst_x = x;
         p = p_row;
         w = bm->w;

         while (w-- > 0)
         {
            /* do current scanline. */
            if (*p & bitmask[bit])
               gr_set_pixel (grd_gc.fcolor, dst_x, y);
            dst_x++;
            if (++bit > 7)
            {
               bit = 0;
               p++;
            }
         }
   
         y++;
         p_row += bm->row;
      }
   }
   else
   {
      /* opaque bitmap; draw 1's as fcolor, 0's as bcolor. */
      while (h-- > 0)
      {
         bit = bm->align;
         dst_x = x;
         p = p_row;
         w = bm->w;

         while (w-- > 0)
         {
            if (*p & bitmask[bit])
               gr_set_upixel (grd_gc.fcolor, dst_x, y);
            else
               gr_set_upixel (grd_gc.bcolor, dst_x, y);
            dst_x++;
            if (++bit > 7)
            {
               bit = 0;
               p++;
            }
         }

         y++;
         p_row += bm->row;
      }
   }
}
   
/* clip monochrome bitmap against cliprect and jump to unclipped drawer. */
int gen_mono_bitmap (grs_bitmap *bm, short x, short y)
{
   short w,h;
   uchar align;
   uchar *p;
   int code = CLIP_NONE;
   
   /* save stuff that clipping changes. */
   w = bm->w; h = bm->h; align = bm->align; p = bm->bits;

   /* clip & draw that sucker. */
   code = gr_clip_mono_bitmap (bm, &x, &y);
   if (code != CLIP_ALL)
      gr_mono_ubitmap (bm, x, y);

   /* restore bitmap to normal. */
   bm->w = w; bm->h = h; bm->align = align; bm->bits = p;
   return code;
}
