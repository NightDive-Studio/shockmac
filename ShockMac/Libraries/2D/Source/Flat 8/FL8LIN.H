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
 * $Source: n:/project/lib/src/2d/RCS/fl8lin.h $
 * $Revision: 1.1 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 00:51:50 $
*/

/* this was originally the guts of flat8_fix_uline */


   fix x0, y0, x1, y1;          
   fix dx, dy;			/* deltas in x and y */
   fix t;			/* temporary fix */

   uchar *p;			/* pointer into canvas */

   x0 = v0->x; y0 = v0->y;
   x1 = v1->x; y1 = v1->y;

   /* set endpoints
      note that this cannot go negative or change octant, since the ==
      case is excluded */

   if (x0 < x1) {
      x1 -= 1;			/* e.g. - epsilon */
   }
   else if (x0 > x1) {
      x0 -= 1;
   }
        
   if (y0 < y1) {
      y1 -= 1;	
   }
   else if (y0 > y1) {
      y0 -= 1; 
   }


   dx = fix_trunc (x1) - fix_trunc(x0);	/* x extent in pixels, (macro is flakey) */
   dx = fix_abs (dx);
   dy = fix_trunc (y1) - fix_trunc(y0);	/* y extent in pixels */
   dy = fix_abs (dy);

   if (dx == 0 && dy == 0) 
     return;

   flat8_pixel_fill_init;

   /* three cases: absolute value dx < = > dy
      
      along the longer dimension, the fixpoint x0 (or y0) is treated 
      as an int
      
      the points are swapped if needed and the rgb initial and deltas
      are calculated accordingly

      there are two or three sub-cases - a horizontal or vertical line,
      and the dx or dy being added or subtracted.  dx and dy 
      are kept as absolute values and +/- is managed in 
      two separate inner loops if it is a y change, since you need to
      manage the canvas pointer

      if y is being changed by 'dy' and x is being incremented, do a 
      FunkyBitCheck (TM) to see whether the integer part of y has changed
      and if it has, resetting the the canvas pointer to the next row
      
      if x is being changed by 'dx' and y is being incremented, just
      add or subtract row to increment y in the canvas
      
      the endpoints are walked inclusively in all cases, see above

      45' degree lines are explicitly special cased -- because it 
      all runs as integers, but it's probably not frequent enough
      to justify the check

    */

   if (dx > dy) {
   
      x0 = fix_int (x0); x1 = fix_int (x1);

      if (x0 > x1) {
	 t = x0; x0 = x1; x1 = t;
	 t = y0; y0 = y1; y1 = t;
      }
      p = grd_bm.bits + grd_bm.row * (fix_int (y0)); 

      if ((fix_int(y0)) == (fix_int(y1))) {
	 		flat8_pixel_fill_row;
      }
      else if (y0  < y1) {
	 dy = fix_div ((y1 - y0), dx);
	 while (x0 <= x1) {
	    flat8_pixel_fill_xi;
	    x0 ++;
	    y0 += dy;
	    p += (grd_bm.row & (-(fix_frac (y0) < dy)));
	 }
      }
      else {
	 dy = fix_div ((y0 - y1), dx);
	 while (x0 <= x1) {
	    flat8_pixel_fill_xi;
	    x0 ++;
	    p -= (grd_bm.row & (-(fix_frac (y0) < dy)));
	    y0 -= dy;
	 }
      }
   }

   else if (dy > dx) {

      y0 = fix_int (y0); y1 = fix_int (y1);

      if (y0 > y1 ) {
	 t = x0; x0 = x1; x1 = t;
	 t = y0; y0 = y1; y1 = t;
      }

      p = grd_bm.bits + grd_bm.row * y0; 
      
      if ((fix_int(x0)) == (fix_int(x1))) {
	 x0 = fix_int (x0);
	 while (y0 <= y1) {
	    flat8_pixel_fill_xi;
	    y0++;
	    p += grd_bm.row;
	 }
      }      
      else {
	 dx = fix_div ((x1 - x0), dy);
	 while (y0 <= y1) {
	    flat8_pixel_fill_xf;
	    x0 += dx;
	    y0++;
	    p += grd_bm.row;
	 }
      }
   }
   else {			/* dy == dx, walk the x axis, all integers */

      x0 = fix_int (x0); x1 = fix_int (x1);
      y0 = fix_int (y0); y1 = fix_int (y1);

      if (x0 > x1 ) {
	 t = x0; x0 = x1; x1 = t;
	 t = y0; y0 = y1; y1 = t;
      }
      p = grd_bm.bits + grd_bm.row * y0; 

      if (y0 < y1) {
	 while (y0 <= y1) {
	    flat8_pixel_fill_xi;
	    x0++;
	    y0++;
	    p += grd_bm.row;
	 }
      }
      else {
	 while (y0 >= y1) {
	    flat8_pixel_fill_xi;
	    x0++;
	    y0--;
	    p -= grd_bm.row;
	 }
      }
   }
