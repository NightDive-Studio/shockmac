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
 * $Source: n:/project/lib/src/2d/RCS/gentl8.c $
 * $Revision: 1.2 $
 * $Author: baf $
 * $Date: 1994/01/14 12:40:57 $
 *
 * Generic tluc 8 bitmap routines.
 *
 * $Log: gentl8.c $
 * Revision 1.2  1994/01/14  12:40:57  baf
 * Lit translucency reform.
 * 
 * Revision 1.1  1993/12/01  21:24:27  baf
 * Initial revision
 * 
 */

#include "bitmap.h"
#include "clpcon.h"
#include "cnvdat.h"
#include "grdbm.h"
#include "grpix.h"
#include "tlucdat.h"
#include "general.h"

/* bozo tluc8 bitmap drawer. */
void gen_tluc8_ubitmap (grs_bitmap *bm, short x, short y)
{
   uchar *src  = bm->bits;
   short right = x+bm->w;
   short bot   = y+bm->h;
   short cur_x;

   if (bm->flags & BMF_TRANS) {
      for ( ; y<bot; y++, src+=bm->row-bm->w) {
         for (cur_x=x ; cur_x<right; cur_x++, src++)
            if (*src) {
               if (tluc8tab[*src] == NULL)
                  gr_set_upixel(*src, cur_x, y);
               else gr_set_upixel(
                  tluc8tab[*src][gr_get_upixel(cur_x, y)],
                  cur_x, y);
               }
      }
   }
   else {
      for ( ; y<bot; y++, src+=bm->row-bm->w) {
         for (cur_x=x ; cur_x<right; cur_x++, src++) {
               if (tluc8tab[*src] == NULL)
                  gr_set_upixel(*src, cur_x, y);
               else gr_set_upixel(
                  tluc8tab[*src][gr_get_upixel(cur_x, y)],
                  cur_x, y);
         }
      }
   }
}

/* clip flat8 bitmap against cliprect and jump to unclipped drawer. */
int gen_tluc8_bitmap (grs_bitmap *bm, short x, short y)
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
   gr_tluc8_ubitmap (bm, x, y);

   /* restore bitmap to normal. */
   bm->w = w; bm->h = h; bm->bits = p;
   return code;
}
