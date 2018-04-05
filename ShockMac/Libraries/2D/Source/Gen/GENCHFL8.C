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
 * $Source: n:/project/lib/src/2d/RCS/genchfl8.c $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1993/10/26 02:38:07 $
 *
 * Generic routines to draw a horizontally flipped flat 8 bitmap.
 *
 * This file is part of the 2d library.
 *
 * $Log: genchfl8.c $
 * Revision 1.3  1993/10/26  02:38:07  kevin
 * Use default clut if passed cl=NULL.
 * Clipped version now uses dispatch macro to call unclipped version.
 * 
 * Revision 1.2  1993/10/19  09:51:06  kaboom
 * Replaced #include "grd.h" with new headers split from grd.h.
 * 
 * Revision 1.1  1993/10/06  14:40:15  kevin
 * Initial revision
 */

#include "clpcon.h"
#include "cnvdat.h"
#include "scrmac.h"
#include "grpix.h"
#include "general.h"
#include "grclhbm.h"

/* draw an unclipped, horizontally flipped flat 8 bitmap to a
   canvas. */
void gen_clut_hflip_flat8_ubitmap(grs_bitmap *bm, short x, short y, uchar *cl)
{
   short r;                   /* right x coordinate */
   short b;                   /* bottom y coordinate */
   short cur_x;               /* current x */
   uchar *src;                /* pointer into source bitmap */

   if (cl == NULL) cl=gr_get_clut();
   r = x+bm->w-1;
   b = y+bm->h-1;
   src = bm->bits;
   for ( ; y<=b; y++, src+=bm->row-bm->w)
      for (cur_x=r; cur_x>=x; cur_x--, src++)
         gr_set_upixel (cl[*src], cur_x, y);
}

/* draw a clipped, horizontally flipped flat 8bitmap to a canvas. */
int gen_clut_hflip_flat8_bitmap (grs_bitmap *bm, short x, short y, uchar *cl)
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
   gr_clut_hflip_flat8_ubitmap (bm, x, y, cl);

   /* restore bitmap to normal. */
   bm->w = w; bm->h = h; bm->bits = p;
   return code;
}
