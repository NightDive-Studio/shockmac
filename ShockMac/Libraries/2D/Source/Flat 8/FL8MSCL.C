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
/* $Source: n:/project/lib/src/2d/RCS/fl8mscl.c $
 * $Revision: 1.1 $
 * $Author: lmfeeney $
 * $Date: 1994/04/09 00:18:42 $
 */

#include "bit.h"
#include "bitmap.h"
#include "clpcon.h"
#include "cnvdat.h"
#include "flat8.h"

void flat8_mono_scale_ubitmap (grs_bitmap *bm, short x, short y, short w, short h)
{
   fix x_scale;                        /* x scale factor */
   fix y_scale;                        /* y scale factor */
   fix x_src;                          /* fractional x in source bitmap */
   fix y_src;                          /* y */
   uchar *p_src;                       /* pointer into source bitmap */
   uchar *p_dst;                       /* pointer into destination bitmap */
   int index, bit;		       /* calculate indexes into char and bitmap */
   int i;

   /* if either width or height is to be 0, no problem. */
   if (w==0 || h==0)
      return;

   x_scale = (bm->w << 16) / w;                
   y_scale = (bm->h << 16) / h;

   y_src = y_scale>>1;
   p_dst = grd_bm.bits + y*grd_bm.row + x;

   if (bm->flags & BMF_TRANS)
      while (h-- > 0) {
         p_src = bm->bits + fix_int (y_src)*bm->row;
         x_src = (bm->align << 16) + (x_scale>>1);  

         for (i=0; i<w; i++) {
	   index = x_src >> (16 + 3);
	   bit = (x_src >> 16) & 0x0007;

	   if (p_src [index] & bitmask [bit])
	     *p_dst++ = grd_gc.fcolor;
	   else
	     p_dst++;
	   
	   x_src+=x_scale;
         }
         p_dst += grd_bm.row-w;
         y_src += y_scale;
       }
   else
      while (h-- > 0) {
         p_src = bm->bits + fix_int (y_src)*bm->row;
         x_src = (bm->align << 16) + (x_scale >> 4);

         for (i=0; i<w; i++) {
	   index = x_src >> (16 + 3);
	   bit = (x_src >> 16) & 0x0007;

	   if (p_src [index] & bitmask [bit])
	     *p_dst++ = grd_gc.fcolor;
	   else
	     *p_dst++ = grd_gc.bcolor;
	   
	   x_src+=x_scale;
         }

         p_dst += grd_bm.row-w;
         y_src += y_scale;
       }
 }


int flat8_mono_scale_bitmap (grs_bitmap *bm, short x, short y, short w, short h)
{
   fix x_left;
   fix x_scale;                        /* x scale factor */
   fix y_scale;                        /* y scale factor */
   fix x_src;                          /* fractional x in source bitmap */
   fix y_src;                          /* y */
   uchar *p_src;                       /* pointer into source bitmap */
   uchar *p_dst;
   int code;
   int index=0, bit=0; 			/* char, bit in char in bitmask */
   int i;

   /* if either width or height is to be 0, no problem. */
   if (w==0 || h==0)
      return CLIP_ALL;

   /* check for trivial reject clip. */
   if (x>grd_clip.right || x+w<=grd_clip.left ||
       y>grd_clip.bot || y+h<=grd_clip.top)
      return CLIP_ALL;

   x_scale = (bm->w<<16)/w;
   y_scale = (bm->h<<16)/h;
   x_left = y_src = 0;
   code = CLIP_NONE;

   if (x < grd_clip.left) {
      x_left = x_scale * (grd_clip.left-x);
      w -= grd_clip.left-x;
      x = grd_clip.left;
      code |= CLIP_LEFT;
   }
   if (x+w > grd_clip.right) {
      w = grd_clip.right-x;
      code |= CLIP_RIGHT;
   }
   if (y < grd_clip.top) {
      y_src = y_scale * (grd_clip.top-y);
      h -= grd_clip.top-y;
      y = grd_clip.top;
      code |= CLIP_TOP;
   }
   if (y+h > grd_clip.bot) {
      h = grd_clip.bot-y;
      code |= CLIP_BOT;
   }

   x_left += ( bm->align << 16 );
   x_left += ( x_scale >> 1 );
   y_src += y_scale >> 1;
   p_dst = grd_bm.bits + y*grd_bm.row + x;

   if (bm->flags & BMF_TRANS)
      while (h-- > 0) {
         p_src = bm->bits + fix_int (y_src)*bm->row;
         x_src = x_left;

         for (i=w; i>0; i--) {
	   index = x_src >> (16 + 3);
	   bit = (x_src >> 16) & 0x0007;

            if (p_src [index] & bitmask [bit])
               *p_dst++ = grd_gc.fcolor;
            else
               p_dst++;
	   
	   x_src += x_scale;
         }

         p_dst += grd_bm.row-w;
         y_src += y_scale;
      }
   else

      while (h-- > 0) {
         p_src = bm->bits + fix_int (y_src)*bm->row;
         x_src = x_left;

         for (i=w; i>0; i--) {

            if (p_src [index] & bitmask [bit])
               *p_dst++ = grd_gc.fcolor;
            else
               *p_dst++ = grd_gc.bcolor;
	   
	   x_src += x_scale;
         }

         p_dst += grd_bm.row-w;
         y_src += y_scale;
      }

   return code;

}


