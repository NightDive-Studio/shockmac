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
 * $Source: n:/project/lib/src/2d/RCS/genulin.c $
 * $Revision: 1.1 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 02:31:36 $
*/

#include "fix.h"
#include "plytyp.h"
#include "pixfill.h"
#include "linfcn.h"

/* This file is a slight modification of the routine gen_fix_uline
   which was originally in the file genlin.c.  It lives in 
   the uline_fill_table and is accessed via the line_fill_vector.
 */

/* Draw an unclipped fixed-point line with calls to 
   grd_upixel_fill -- this fills in the pixel according to the
   current fill type (in the grd_pixel_fill_table) */

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

void gri_gen_uline_fill (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
   fix x0, x1, y0, y1;        /* actually use x and y */
   fix dx, dy;		      /* delta's in x and y */
   fix t;                     /* temporary fix */
   
   x0 = v0->x; y0 = v0->y;
   x1 = v1->x; y1 = v1->y;

   /* set endpoints
      note that this cannot go negative or change octant, since the ==
      case is excluded */

   if (x0 < x1) 
      x1 -= 1;			/* e.g. - epsilon */
   else if (x0 > x1) 
     x0 -= 1;
   
   if (y0 < y1)
      y1 -= 1;	
   else if (y0 > y1) 
      y0 -= 1; 

   dx = fix_trunc (x1) - fix_trunc(x0);	/* x extent in pixels, (macro is flakey) */
   dx = fix_abs (dx);
   dy = fix_trunc (y1) - fix_trunc(y0);	/* y extent in pixels */
   dy = fix_abs (dy);

   if (dx == 0 && dy == 0) 
     return;

   /* three cases: absolute value dx < = > dy
      
      the variable (x0 or y0) for the long dimension is treated as a fix
      and incremented using ++, the other is kept as fixpoint and 
      incremented by the fixpoint delta (dx or dy).  Then it's shifted 
      to int for pixel drawing.
      
      the points are swapped if needed and the rgb initial and deltas
      are calculated accordingly

      there are two sub-cases - a horizontal or vertical line,
      and the dx or dy being added or subtracted. 

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

      if ((fix_int(y0)) == (fix_int(y1))) {
	 y0 = fix_int (y0);	
	 while (x0 <= x1) {
	    grd_pixel_fill (c, parm, x0, y0);
	    x0++;
	 }
      }
      else {
	 dy = fix_div ((y1 - y0), dx);
	 while (x0 <= x1) {
	    grd_pixel_fill(c, parm, x0, fix_int(y0));
	    x0 ++;
	    y0 += dy;
	 }
      }
   }

   else if (dy > dx) {

      y0 = fix_int (y0); y1 = fix_int (y1);

      if (y0 > y1 ) {
	 t = x0; x0 = x1; x1 = t;
	 t = y0; y0 = y1; y1 = t;
      }

      if ((fix_int(x0)) == (fix_int(x1))) {
	 x0 = fix_int (x0);
	 while (y0 <= y1) {
	    grd_pixel_fill(c, parm, x0, y0);
	    y0++;
	 }
      }      
      else {
	 dx = fix_div ((x1 - x0), dy);
	 while (y0 <= y1) {
	    grd_pixel_fill(c, parm, fix_int(x0), y0);
	    x0 += dx;
	    y0++;
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

      if (y0 < y1) {
	 while (y0 <= y1) {
	    grd_pixel_fill(c, parm, x0, y0);
	    x0++;
	    y0++;
	 }
      }
      else {
	 while (y0 >= y1) {
	    grd_pixel_fill(c, parm, x0, y0);
	    x0++;
	    y0--;
	 }
      }
   }
}
