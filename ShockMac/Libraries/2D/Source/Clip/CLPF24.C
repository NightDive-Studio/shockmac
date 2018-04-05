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
 * $Source: n:/project/lib/src/2d/RCS/clpf24.c $
 * $Revision: 1.3 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:50:01 $
 *
 * Routines for clipping flat 24 bitmaps to a rectangle.
 *
 * This file is part of the 2d library.
 *
 * $Log: clpf24.c $
 * Revision 1.3  1993/10/19  09:50:01  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.2  1993/10/01  15:38:30  kaboom
 * Cleaned up code some; converted to using clpcon.h instead of clip.h.
 * 
 * Revision 1.1  1993/09/02  19:41:21  kaboom
 * Initial revision
 */

#include "grs.h"
#include "clpcon.h"
#include "cnvdat.h"
#include "clip.h"

int gr_clip_flat24_bitmap (grs_bitmap *bm, short *x, short *y)
{
   int code = CLIP_NONE;
   int extra;
   int l, r, t, b;

   l=*x; r=l+bm->w; t=*y; b=t+bm->h;
   if (l>grd_clip.right || r<grd_clip.left ||
       t>grd_clip.bot || b<grd_clip.top)
      /* bitmap is completely clipped. */
      return CLIP_ALL;

   if (l < grd_clip.left) {
      /* bitmap is off the left edge of the window. */
      extra = grd_clip.left-l;
      bm->w -= extra;
      bm->bits += 3*extra;
      *x = grd_clip.left;
      code |= CLIP_LEFT;
   }
   if (r > grd_clip.right) {
      /* off the right edge of the window. */
      bm->w -= r-grd_clip.right;
      code |= CLIP_RIGHT;
   }
   if (t < grd_clip.top) {
      /* off the top of the window. */
      extra = grd_clip.top-t;
      bm->h -= extra;
      bm->bits += bm->row*extra;
      *y = grd_clip.top;
      code |= CLIP_TOP;
   }
   if (b > grd_clip.bot) {
      /* off the bottom of the window. */
      bm->h -= b-grd_clip.bot;
      code |= CLIP_BOT;
   }

   return code;
}
