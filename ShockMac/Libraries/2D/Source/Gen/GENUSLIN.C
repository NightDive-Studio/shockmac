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
 * $Source: n:/project/lib/src/2d/RCS/genuslin.c $
 * $Revision: 1.1 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 02:33:59 $
 *
 * Routine to draw an rgb shaded line to a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: genuslin.c $
 * Revision 1.1  1994/06/11  02:33:59  lmfeeney
 * Initial revision
 * 
 * Revision 1.6  1994/05/06  18:19:52  lmfeeney
 * rewritten for greater accuracy and speed
 * 
 * Revision 1.5  1993/10/02  01:04:01  kaboom
 * Fixed clipping include files.
 * 
 * Revision 1.4  1993/07/03  22:51:02  spaz
 * Bugfix; treated clipper return code spastically
 *
 * Revision 1.3  1993/07/01  20:32:29  spaz
 * Set last pixel explicitly to i1 in h and vlines
 *
 * Revision 1.2  1993/06/30  00:43:13  spaz
 * Changed a check for (x0>x1) to fix_int(x0) version,
 * because it was negating the delta uselessly for
 * near-vertical lines.
 *
 * Revision 1.1  1993/06/22  20:14:28  spaz
 * Initial revision
 */

#include <stdlib.h>
#include "fix.h"
#include "plytyp.h"
#include "pixfill.h"
#include "linfcn.h"

/* Draw a gouraud-shaded line, specified by indices into the palette.
   be warned, weird precision bugs abound (check out test programs
   in /project/lib/src/2d/test). 
   
   5/94: Precision errors have been (entirely?) eliminated.  See
   correctness argument in note.txt.

*/

/*
 draw unclipped shaded line using grd_pixel_fill for fill
 information -- note that the solid fill mode cannot use this routine
 -- the gen routine only passes the computed color -- arguably it
 should check, but that would be slow and painful and if the cline
 function is accessed by the fill vector, this function is never
 called when the fill mode is solid 
*/

// MLA #pragma off (unreferenced)
#define macro_plot_i(x,y,i) 	 grd_pixel_fill (i, parm, x, y)

void gri_gen_usline_fill (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
   fix x0, y0, x1, y1; 
   fix dx, dy;                /* delta's in x and y */
   fix t;                     /* temporary fix */
   
   fix i0, i1;
   fix di;                    /* # colors per x-pixel */

   /* set endpoints 
      note that this cannot go negative or change octant, since the ==
      case is excluded */

   x0 = v0->x; y0 = v0->y;
   x1 = v1->x; y1 = v1->y;

   i0 = (fix) v0->i; i1 = (fix) v1->i;

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
      
      there are two sub-cases - a horizontal or vertical line,
      and the dx or dy being added  

      it might be better to use a funky bit check (tm) and keep an
      integer y, similar to the canvas, rather than keep doing shift
      in the inner loop

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

      di = fix_div ((i1-i0), dx);

      if ((fix_int(y0)) == (fix_int(y1))) {
	 y0 = fix_int (y0);
	 while (x0 <= x1) {
	    macro_plot_i (x0, y0, fix_fint (i0));
	    x0++;
	    i0 += di;
	 }
      }
      else {
	 dy = fix_div ((y1 - y0), dx);
	 while (x0 <= x1) {
	    macro_plot_i (x0, fix_int(y0), fix_fint(i0));
	    x0 ++;
	    y0 += dy;
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

      di = fix_div((i1-i0), dy);

      if ((fix_int(x0)) == (fix_int(x1))) {
	 x0 = fix_int (x0);
	 while (y0 <= y1) {
	    macro_plot_i (x0, y0, fix_fint(i0));
	    y0++;
	    i0 += di;
	 }
      }      
      else {
	 dx = fix_div ((x1 - x0), dy);
	 while (y0 <= y1) {
	    macro_plot_i(fix_fint(x0), y0, fix_fint (i0));
	    x0 += dx;
	    y0++;
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

      di = fix_div((i1-i0),dx);

      if (y0 < y1) {
	 while (y0 <= y1) {
	    macro_plot_i(x0, y0, fix_fint(i0));
	    x0++;
	    y0++;    
	    i0+= di;
	 }
      }
      else {
	 while (y0 >= y1) {
	    macro_plot_i(x0,y0, fix_fint (i0));
	    x0++;
	    y0--;
	    i0 += di;
	 }
      }
   }
}

