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
 * $Source: n:/project/lib/src/2d/RCS/genhfl8.c $
 * $Revision: 1.3 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:51:14 $
 *
 * Generic flat 8 bitmap horizontal flip routine.
 *
 * This file is part of the 2d library.
 *
 * $Log: genhfl8.c $
 * Revision 1.3  1993/10/19  09:51:14  kaboom
 * Replaced #include "grd.h" with new headers split from grd.h.
 * 
 * Revision 1.2  1993/10/02  01:17:24  kaboom
 * Changed include of clip.h to include of clpcon.h and/or clpfcn.h.
 * 
 * Revision 1.1  1993/06/06  15:09:57  kaboom
 * Initial revision
 */

#include "clpcon.h"
#include "cnvdat.h"
#include "grpix.h"
#include "grhbm.h"
#include "general.h"

/* draw an unclipped, horizontally flipped flat 8 bitmap to a
   canvas. */
void gen_hflip_flat8_ubitmap (grs_bitmap *bm, short x, short y)
{
   short r;                   /* right x coordinate */
   short b;                   /* bottom y coordinate */
   short cur_x;               /* current x */
   uchar *src;                /* pointer into source bitmap */

   r = x+bm->w-1;
   b = y+bm->h-1;
   src = bm->bits;
   for ( ; y<=b; y++, src+=bm->row-bm->w)
      for (cur_x=r; cur_x>=x; cur_x--, src++)
         gr_set_upixel (*src, cur_x, y);
}

/* draw a clipped, horizontally flipped flat 8bitmap to a canvas. */
int gen_hflip_flat8_bitmap (grs_bitmap *bm, short x, short y)
{
   short w,h;
   uchar *p;
   short r;
   short b;
   int extra;
   int code = CLIP_NONE;

   r = x+bm->w-1;
   b = y+bm->h-1;

   /* save stuff that clipping changes. */
   w = bm->w; h = bm->h; p = bm->bits;

   /* first check for trivial reject. */
   if (x>=grd_clip.right || r<grd_clip.left ||
       y>=grd_clip.bot || b<grd_clip.top)
      return CLIP_ALL;

   if (x < grd_clip.left) {            /* off left */
      bm->w -= grd_clip.left-x;
      x = grd_clip.left;
      code |= CLIP_LEFT;
   }
   if (r >= grd_clip.right) {          /* off right */
      extra = r-grd_clip.right+1;
      bm->w -= extra;
      bm->bits += extra;
      code |= CLIP_RIGHT;
   }
   if (y < grd_clip.top) {             /* off top */
      extra = grd_clip.top - y;
      bm->h -= extra;
      bm->bits += bm->row*extra;
      y = grd_clip.top;
      code |= CLIP_TOP;
   }
   if (b >= grd_clip.bot) {      /* off bottom */
      bm->h -= b-grd_clip.bot+1;
      code |= CLIP_BOT;
   }
   gr_hflip_flat8_ubitmap (bm, x, y);

   /* restore bitmap to normal. */
   bm->w = w; bm->h = h; bm->bits = p;
   return code;
}
