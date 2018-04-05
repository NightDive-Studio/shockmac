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
 * $Source: n:/project/lib/src/2d/RCS/fl8ntrp2.c $
 * $Revision: 1.4 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:54:21 $
 *
 * Routine for scaling up by 2 with interpolation onto a gen canvas
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8ntrp2.c $
 * Revision 1.4  1993/10/19  09:54:21  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.3  1993/10/08  01:15:24  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.2  1993/10/02  00:37:56  kaboom
 * Fixed bug---wasn't drawing the last line.
 * 
 * Revision 1.1  1993/08/05  20:06:42  jaemz
 * Initial revision
 */

#include "bitmap.h"
#include "cnvdat.h"
#include "flat8.h"
#include "scrdat.h"

/* Assumes bm is flat, assumes canvas is big enough to 
   hold it, as it is, unclipped */
void flat8_interp2_ubitmap (grs_bitmap *bm)
{
   grs_rgb a,b;
   int i,j;
   uchar *c;
   uchar *src,*dst;
   int dd,ds;

   /* Cycle through rows and do horizontal strips */
   dd = 2*grd_bm.row - grd_bm.w;
   ds = bm->row - bm->w;

   dst = grd_bm.bits;
   src = bm->bits;

   for (j=bm->h;j>0;j--) {
      for (i=bm->w;i>0;i--) {

         a = *src;
         src++;
         b = *src;

         *dst = a;
         dst++;

         /* load them with the rgb values */
         a = grd_bpal[a];
         b = grd_bpal[b];
         a = (a>>1) + (b>>1);
         c = grd_ipal;
         a = a>>5;
         c += a & 0x1f;
         a = a>>6;
         c += a & 0x3e0;
         a = a>>6;
         c += a & 0x7c00;

         *dst = *c;
         dst++;
      }
      dst += dd;
      src += ds;
   }

   /* Cycle through rows and fill in missing strips */
   src = grd_bm.bits;
   for(i=(bm->w)*2-1;i>=0;i--) {
      src = grd_bm.bits + i;
      for (j=bm->h;j>1;j--) {
         a = *src;
         src += 2*grd_bm.row;
         b = *src;
         src -= grd_bm.row;

         a = grd_bpal[a];
         b = grd_bpal[b];
         a = (a>>1) + (b>>1);
         c = grd_ipal;
         a = a>>5;
         c += a & 0x1f;
         a = a>>6;
         c += a & 0x3e0;
         a = a>>6;
         c += a & 0x7c00;
         *src = *c;
         src += grd_bm.row;
      }
      src+=grd_bm.row; *src=0;
   }
}
