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
 * $Source: n:/project/lib/src/2d/RCS/fl8mono.c $
 * $Revision: 1.3 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:50:39 $
 * 
 * Routines for drawing monochrome bitmaps into a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8mono.c $
 * Revision 1.3  1993/10/19  09:50:39  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.2  1993/10/08  01:15:23  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.1  1993/02/16  14:15:56  kaboom
 * Initial revision
 * 
 ********************************************************************
 * Log from old flat8.c:
 *
 * Revision 1.5  1992/12/11  13:13:33  kaboom
 * Fixed bug in transparent case of flat8_mono_ubitmap -- destination
 * pointer wasn't getting incremented on transparent pixels.
 */

#include "bit.h"
#include "bitmap.h"
#include "cnvdat.h"
#include "flat8.h"

void flat8_mono_ubitmap (grs_bitmap *bm, short x, short y)
{
   short w, h;                /* working width and height */
   int bit;                   /* bit from 0-7 in source byte */
   uchar *p_row;              /* pointer to current row of bitmap */
   uchar *p_src;              /* pointer to source byte */
   uchar *p_dst;

   h = bm->h;
   p_row = bm->bits;
   p_dst = grd_bm.bits + y*grd_bm.row + x;

   if (bm->flags & BMF_TRANS)
   {
      /* transparent bitmap; draw 1's as fcolor, don't draw 0's. */
      while (h-- > 0)
      {
         /* set up scanline. */
         bit = bm->align;
         p_src = p_row;
         w = bm->w;

         while (w-- > 0)
         {
            /* do current scanline. */
            if (*p_src & bitmask[bit])
               *p_dst++ = grd_gc.fcolor;
            else
               p_dst++;
            if (++bit > 7)
            {
               bit = 0;
               p_src++;
            }
         }
         p_dst += grd_bm.row-bm->w;
         p_row += bm->row;
      }
   }
   else
   {
      /* opaque bitmap; draw 1's as fcolor, 0's as bcolor. */
      while (h-- > 0)
      {
         bit = bm->align;
         p_src = p_row;
         w = bm->w;

         while (w-- > 0)
         {
            if (*p_src & bitmask[bit])
               *p_dst++ = grd_gc.fcolor;
            else
               *p_dst++ = grd_gc.bcolor;
            if (++bit > 7)
            {
               bit = 0;
               p_src++;
            }
         }
         p_dst += grd_bm.row-bm->w;
         p_row += bm->row;
      }
   }
}
