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
 *  $Source: n:/project/lib/src/2d/RCS/fl8clin.h $
 *  $Revision: 1.1 $
 *  $Author: lmfeeney $
 *  $Date: 1994/06/11 00:49:17 $
 */

/* This file is an UNCOMPILABLE code fragment */

/* Draw a gouraud-shaded line as specified by endpoint rgb value...
   weird precision bugs abound due to precision errors 

   5/94 These have been (completely?) corrected.  See test/clintest.c

   See also note.txt for argument for correctness and precision safety
   of current algorithm.
*/

/* NB: directionality policy -- reversible
   Lines are drawn in order of increasing x or increasing y, 
   for lines that have greater x or y extent, repsectively.
   This is done for all lines, including horiz., vert., and 
   45' lines (increasing x).
*/

 /* NB: endpoint policy -- inclusive 

    The left and top endpoints are inclusive, i.e. 'trunc'-ed.  The
    right and bottom endpoints exclude the ceiling.  This is
    calculated by subtracting epsilon (i.e. 1/65536) from its
    fixed-point representation, then trunc'ing.
    
    This makes sense if you note that open interval on right 
         < ceil (x)  
    is the same as
         <= trunc (x - e) 

   This means that it might not be necessary to go through the ugliness
   of swapping endpoints, but I'm not convinced that there aren't 
   precision problems involved.  Since wire-poly's overdraw, it's
   really necessary to ensure that the same points are always drawn.
*/

   fix x0, y0, x1, y1;
   fix dx, dy;			/* deltas in x and y */
   fix t;                       /* tmp */

   uchar r0, g0, b0, r1, g1, b1; /* rgb values of endpt colors */
   fix r,g,b;			/* current intensities */
   fix dr, dg, db;		/* deltas for each of rgb */
   long i;			/* color index */

   uchar *p;			/* ptr into canvas */


   x0 = v0->x; y0 = v0->y;
   x1 = v1->x; y1 = v1->y;

   r0 = (uchar) (v0->u); g0 = (uchar) (v0->v); b0 = (uchar) (v0->w);
   r1 = (uchar) (v1->u); g1 = (uchar) (v1->v); b1 = (uchar) (v1->w);

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

   dx = fix_trunc (x1) - fix_trunc(x0);		/* x extent in pixels, (macro is flakey) */
   dx = fix_abs (dx);
   dy = fix_trunc (y1) - fix_trunc(y0);		/* y extent in pixels */
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

      if (x0 < x1 ) {
	 r = fix_make(r0,0); g = fix_make(g0,0); b = fix_make(b0,0);
	 dr = fix_div(fix_make_nof(r1-r0),dx);
	 dg = fix_div(fix_make_nof(g1-g0),dx);
	 db = fix_div(fix_make_nof(b1-b0),dx);

	 p = grd_bm.bits + grd_bm.row * (fix_int(y0)); /* set canvas ptr */

      }
      else {
	 t = x0; x0 = x1; x1 = t;
	 t = y0; y0 = y1; y1 = t;

	 r = fix_make(r1, 0); g = fix_make(g1,0); b = fix_make(b1,0);
	 dr = fix_div(fix_make_nof(r0-r1),dx);
	 dg = fix_div(fix_make_nof(g0-g1),dx);
	 db = fix_div(fix_make_nof(b0-b1),dx);

	 p = grd_bm.bits + grd_bm.row * (fix_int (y0)); 
      }

      if ((fix_int(y0)) == (fix_int(y1))) {
	 while (x0 <= x1) {
	    i = macro_get_ipal(r,g,b);
	    macro_plot_rgb(x0,p,i);
	    x0++;
	    r += dr; g += dg; b += db;
	 }
      }
      else if (y0  < y1) {
	dy = fix_div ((y1 - y0), dx);
	while (x0 <= x1) {
	  i = macro_get_ipal(r,g,b);
	  macro_plot_rgb(x0,p,i);
	  x0 ++;
	  y0 += dy;
	  p += (grd_bm.row & (-(fix_frac (y0) < dy)));
	  r += dr; g += dg; b += db;
	}
      }
      else {
	dy = fix_div ((y0 - y1), dx);
	while (x0 <= x1) {
	  i = macro_get_ipal(r,g,b);
	  macro_plot_rgb(x0,p,i);
	  x0 ++;
	  p -= (grd_bm.row & (-(fix_frac (y0) < dy)));
	  y0 -= dy;
	  r += dr; g += dg; b += db;
	}
      }
    }

   else if (dy > dx) {

      y0 = fix_int (y0); y1 = fix_int (y1);

      if (y0 < y1 ) {
	 r = fix_make(r0,0); g = fix_make(g0,0); b = fix_make(b0,0);
	 dr = fix_div(fix_make_nof(r1-r0),dy);
	 dg = fix_div(fix_make_nof(g1-g0),dy);
	 db = fix_div(fix_make_nof(b1-b0),dy);

	 p = grd_bm.bits + grd_bm.row * y0;

      }
      else {
	 t = x0; x0 = x1; x1 = t;
	 t = y0; y0 = y1; y1 = t;

	 r = fix_make(r1, 0); g = fix_make(g1,0); b = fix_make(b1,0);
	 dr = fix_div(fix_make_nof(r0-r1),dy);
	 dg = fix_div(fix_make_nof(g0-g1),dy);
	 db = fix_div(fix_make_nof(b0-b1),dy);

	 p = grd_bm.bits + grd_bm.row * y0; 
      }
      
      if ((fix_int(x0)) == (fix_int(x1))) {
	 x0 = fix_int (x0);
	 while (y0 <= y1) {
	    i = macro_get_ipal(r,g,b);
	    macro_plot_rgb(x0,p,i);
	    y0++;
	    p += grd_bm.row;
	    r += dr; g += dg; b += db;
	 }
      }      
      else {
	 dx = fix_div ((x1 - x0), dy);
	 while (y0 <= y1) {
	    i = macro_get_ipal(r,g,b);
	    macro_plot_rgb(fix_fint(x0),p,i);
	    x0 += dx;
	    y0++;
	    p += grd_bm.row;
	    r += dr; g += dg; b += db;
	 }
      }
   }
   else {			/* dy == dx, walk the x axis, all integers */

      x0 = fix_int (x0); x1 = fix_int (x1);
      y0 = fix_int (y0); y1 = fix_int (y1);

      if (x0 < x1 ) {
	 r = fix_make(r0,0); g = fix_make(g0,0); b = fix_make(b0,0);
	 dr = fix_div(fix_make_nof(r1-r0),dx);
	 dg = fix_div(fix_make_nof(g1-g0),dx);
	 db = fix_div(fix_make_nof(b1-b0),dx);

	 p = grd_bm.bits + grd_bm.row * y0; /* set canvas ptr */

      }
      else {
	 t = x0; x0 = x1; x1 = t;
	 t = y0; y0 = y1; y1 = t;

	 r = fix_make(r1, 0); g = fix_make(g1,0); b = fix_make(b1,0);
	 dr = fix_div(fix_make_nof(r0-r1),dx);
	 dg = fix_div(fix_make_nof(g0-g1),dx);
	 db = fix_div(fix_make_nof(b0-b1),dx);

	 p = grd_bm.bits + grd_bm.row * y0; 
      }

      if (y0 < y1) {
	while (y0 <= y1) {
	  i = macro_get_ipal(r,g,b);
	  macro_plot_rgb(x0,p,i);
	  x0++;
	  y0++;
	  p += grd_bm.row;
	  r += dr; g += dg; b += db;
	}
      }
      else {
	while (y0 >= y1) {
	  i = macro_get_ipal(r,g,b);
	  macro_plot_rgb(x0,p,i);
	  x0++;
	  y0--;
	  p -= grd_bm.row;
	  r += dr; g += dg; b += db;
	}
      }
   }

