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
 * $Source: n:/project/lib/src/2d/RCS/fl8slin.h $
 * $Revision: 1.1 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 00:50:38 $
 *
 * Routine to draw a gouraud shaded line to a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8slin.h $
 * Revision 1.1  1994/06/11  00:50:38  lmfeeney
 * Initial revision
 * 
 * Revision 1.4  1994/05/06  18:19:13  lmfeeney
 * rewritten for greater accuracy and speed
 * 
 * Revision 1.3  1993/10/19  09:50:58  kaboom
 * Replaced #include <grd.h> with new headers split from grd.h.
 * 
 * Revision 1.2  1993/10/01  16:01:20  kaboom
 * Pared down includes to reduce dependencies.
 * 
 * Revision 1.1  1993/07/01  22:12:15  spaz
 * Initial revision
 */

/* Draw a goroud-shaded line, specified by indices into the palette.
   be warned, weird precision bugs abound (check out test programs
   in /project/lib/src/2d/test). 

   These have been mostly corrected.  See also note.txt for correctness
   arguement of new algorithm
*/

/* NB: directionality policy -- reversible
   Lines are drawn in order of increasing x or increasing y, 
   for lines that have greater x or y extent, repsectively.
   This is done for all lines, including horiz., vert., and 
   45' lines (increasing x).
*/

 /* NB: endpoint policy -- inclusive The left and top endpoints are
    'trunc'-ed.  The right and bottom endpoints exclude the ceiling,
    or equivlalently, include the pixel containing the line.  This is
    calculated by subtracting epsilon (i.e. 1/65536) from its
    fixed-point representation, then trunc'ing.
    
    This makes sense if you note that open interval right
         < ceil (x)  
    is the same as
         <= trunc (x - e) 
*/


   fix x0, y0, x1, y1;
   fix dx, dy;			/* deltas in x and y */
   fix t;			/* temporary fix */

   fix i0, i1;
   fix di;			/* delta intensity */

   uchar *p;			/* pointer into the canvas */


   x0 = v0->x; y0 = v0->y;
   x1 = v1->x; y1 = v1->y;

   i0 = (fix) v0->i; i1 = (fix) v1->i;

   /* set endpoints 
      note that this cannot go negative or change octant, since the ==
      case is excluded */

   if (x0 < x1) {
      x1  -= 1;			/* e.g. - epsilon */
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


   /* three cases: absolute value dx < = > dy
      
      along the longer dimension, the fixpoint x0 (or y0) is treated 
      as an int
      
      the points are swapped if needed and the rgb initial and deltas
      are calculated accordingly
      
      there are two or three sub-cases - a horizontal or vertical line,
      and the dx or dy being added or subtracted.  dx and dy 
      are kept as absolute values and +/- is managed in the 
      two separate inner loops if it's a dy, since you also have to
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
	 t = i0; i0 = i1; i1 = t;
      }
      
      p = grd_bm.bits + grd_bm.row * (fix_int (y0)); 
      di = fix_div ((i1-i0), dx);

      if ((fix_int(y0)) == (fix_int(y1))) {
	 while (x0 <= x1) {
	    macro_plot_i (x0, p, fix_fint (i0));
	    x0++;
	    i0 += di;
	 }
      }
      else if (y0 < y1) {
	 dy = fix_div ((y1 - y0), dx);
	 while (x0 <= x1) {
	    macro_plot_i (x0, p, fix_fint(i0));
	    x0 ++;
	    y0 += dy;
	    p += (grd_bm.row & (-(fix_frac (y0) < dy)));
	    i0 += di;
	 }
      }
      else {
	 dy = fix_div ((y0 - y1), dx);
	 while (x0 <= x1) {
	    macro_plot_i (x0, p, fix_fint(i0));
	    x0 ++;
	    p -= (grd_bm.row & (-(fix_frac (y0) < dy)));
	    y0 -= dy;	
	    i0 += di;
	 }
      }
   }
   else if (dy > dx) {

      y0 = fix_int (y0); y1 = fix_int (y1);

      if (y0 > y1) {
	 t = x0; x0 = x1; x1 = t;
	 t = y0; y0 = y1; y1 = t;
	 t = i0; i0 = i1; i1 = t;
      }

      p = grd_bm.bits + grd_bm.row * y0;      
      di = fix_div((i1-i0), dy);

      if ((fix_int(x0)) == (fix_int(x1))) {
	 x0 = fix_int (x0);
	 while (y0 <= y1) {
	    macro_plot_i(x0, p, fix_fint(i0));
	    y0++;
	    p += grd_bm.row;
	    i0 += di;
	 }
      }      
      else {
	 dx = fix_div ((x1 - x0), dy);
	 while (y0 <= y1) {
	    macro_plot_i(fix_fint(x0), p, fix_fint (i0));
	    x0 += dx;
	    y0++;
	    p += grd_bm.row;
	    i0 += di;
	 }
      }
   }
   else {			/* dy == dx, walk the x axis, all integers */

      x0 = fix_int (x0); x1 = fix_int (x1);
      y0 = fix_int (y0); y1 = fix_int (y1);

      if (x0 > x1 ) {
	 t = x0; x0 = x1; x1 = t;
	 t = y0; y0 = y1; y1 = t;
	 t = i0; i0 = i1; i1 = t;
      }

      p = grd_bm.bits + grd_bm.row * y0; /* set canvas ptr */
      di = fix_div((i1-i0),dx);

      if (y0 < y1) {
	 while (y0 <= y1) {
	    macro_plot_i(x0, p, fix_fint(i0));
	    x0++;
	    y0++;    
	    p += grd_bm.row;
	    i0+= di;
	 }
      }
      else {
	 while (y0 >= y1) {
	    macro_plot_i(x0,p, fix_fint (i0));
	    x0++;
	    y0--;
	    p -= grd_bm.row;
	    i0 += di;
	 }
      }
   }

