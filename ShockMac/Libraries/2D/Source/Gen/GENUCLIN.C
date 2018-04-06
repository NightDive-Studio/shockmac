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
 * $Source: r:/prj/lib/src/2d/RCS/genuclin.c $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/10/17 15:00:02 $
*/

/* This file contains originally the routine gen_fix_ucline, found in
   genclin.c. */

#include <stdlib.h>
#include "grd.h"
#include "plytyp.h"
#include "rgb.h"
#include "scrdat.h"
#include "pixfill.h"
#include "linfcn.h"

/* draw unclipped rgb shaded line using grd_pixel_fill for fill
 information -- note that the solid fill mode cannot use this routine
 -- the gen routine only passes the computed color -- arguably it
 should check, but that would be a pain and if the cline function is
 accessed by the fill vector, this function is never called when the
 fill mode is solid 
*/

#define fix_make_nof(x)           fix_make(x,0x0000)
#define macro_get_ipal(r,g,b)     (long) ((r>>19) &0x1f) | ((g>>14) & 0x3e0) | ((b>>9) & 0x7c00)

/* the color passed into the line fill is the calculated one, not the
   param -- see above 
 */
// MLA #pragma off (unreferenced)
#define macro_plot_rgb(x,y,i)     grd_pixel_fill(grd_ipal[i],parm,x,y)

void gri_gen_ucline_fill (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
   fix x0, y0, x1, y1;
   fix dx, dy;			/* deltas in x and y */
   fix t;			/* tmp */

   uchar r0, g0, b0, r1, g1, b1; /* rgb values of endpt colors */
   fix r,g,b;			/* current intensities */
   fix dr, dg, db;		/* deltas for each of rgb */
   long i;			/* color index */

   /* NB: this code mimics the code in fl8clin.c for flat8_ucline's.
      this leads to much more separation among e.g +/- dx and dy is
      ncessary, since increments in the x and y directions can be treated
      identically, rather than manipulating a canvas pointer.  however, 
      eventually, i'd like the flat8 and gen to derive from the 
      same code with just different defines for increments, etc.
      */

   /* NB: directionality policy -- reversible
      Lines are drawn in order of increasing x or increasing y, 
      for lines that have greater x or y extent, repsectively.
      This is done for all lines, including horiz., vert., and 
      45' lines (increasing x).
      */

   x0 = v0->x; y0 = v0->y;
   x1 = v1->x; y1 = v1->y;

   if (x0 < x1) {
      x1 -= 1;			/* e.g. - epsilon */

   }
   else if (x0 > x1) {
      x0 -= 1;
   }

   if (y0 < y1) {
      y1 -= 1;			/* e.g. - epsilon */
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
      
      the variable (x0 or y0) for the long dimension is treated as a fix
      and incremented using ++, the other is kept as fixpoint and 
      incremented by the fixpoint delta (dx or dy).  Then it's shifted 
      to int for pixel drawing.
      
      there are two sub-cases - a horizontal or vertical line,
      and the dx or dy being added or subtracted.  
      
      the endpoints are walked inclusively in all cases --
      
      45' degree lines are explicitly special cased -- more efficient,
      since everything is an integer 
      */

   r0 = (uchar) (v0->u); g0 = (uchar) (v0->v); b0 = (uchar) (v0->w);
   r1 = (uchar) (v1->u); g1 = (uchar) (v1->v); b1 = (uchar) (v1->w);

   if (dx > dy) {
      
      x0 = fix_int (x0); x1 = fix_int (x1);

      if (x0 < x1 ) {
	 r = fix_make(r0,0); g = fix_make(g0,0); b = fix_make(b0,0);
	 dr = fix_div(fix_make_nof(r1-r0),dx);
	 dg = fix_div(fix_make_nof(g1-g0),dx);
	 db = fix_div(fix_make_nof(b1-b0),dx);
      }
      else {
	 t = x0; x0 = x1; x1 = t;
	 t = y0; y0 = y1; y1 = t;

	 r = fix_make(r1, 0); g = fix_make(g1,0); b = fix_make(b1,0);
	 dr = fix_div(fix_make_nof(r0-r1),dx);
	 dg = fix_div(fix_make_nof(g0-g1),dx);
	 db = fix_div(fix_make_nof(b0-b1),dx);
      }

      if ((fix_int(y0)) == (fix_int(y1))) {
	 y0 = fix_int (y0);
	 while (x0 <= x1) {
	    i = macro_get_ipal(r,g,b);
	    macro_plot_rgb(x0,y0,i);
	    x0++;
	    r += dr; g += dg; b += db;
	 }
      }
      else {
	 dy = fix_div ((y1 - y0), dx);
	 while (x0 <= x1) {
	    i = macro_get_ipal(r,g,b);
	    macro_plot_rgb (x0, fix_fint(y0) ,i);
	    x0 ++;
	    y0 += dy;
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
      }
      else {
	 t = x0; x0 = x1; x1 = t;
	 t = y0; y0 = y1; y1 = t;

	 r = fix_make(r1, 0); g = fix_make(g1,0); b = fix_make(b1,0);
	 dr = fix_div(fix_make_nof(r0-r1),dy);
	 dg = fix_div(fix_make_nof(g0-g1),dy);
	 db = fix_div(fix_make_nof(b0-b1),dy);
      }
      
      if ((fix_int(x0)) == (fix_int(x1))) {
	 x0 = fix_int (x0);
	 while (y0 <= y1) {
	    i = macro_get_ipal(r,g,b);
	    macro_plot_rgb(x0,y0,i);
	    y0++;
	    r += dr; g += dg; b += db;
	 }
      }      
      else {
	 dx = fix_div ((x1 - x0), dy);
	 while (y0 <= y1) {
	    i = macro_get_ipal(r,g,b);
	    macro_plot_rgb(fix_fint(x0),y0,i);
	    x0 += dx;
	    y0++;
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
      }
      else {
	 t = x0; x0 = x1; x1 = t;
	 t = y0; y0 = y1; y1 = t;

	 r = fix_make(r1, 0); g = fix_make(g1,0); b = fix_make(b1,0);
	 dr = fix_div(fix_make_nof(r0-r1),dx);
	 dg = fix_div(fix_make_nof(g0-g1),dx);
	 db = fix_div(fix_make_nof(b0-b1),dx);
      }

      if (y0 < y1) {
	 while (y0 <= y1) {
	    i = macro_get_ipal(r,g,b);
	    macro_plot_rgb(x0, y0, i);
	    x0++;
	    y0++;
	    r += dr; g += dg; b += db;
	 }
      }
      else {
	 while (y0 >= y1) {
	    i = macro_get_ipal(r,g,b);
	    macro_plot_rgb(x0, y0, i);
	    x0++;
	    y0--;
	    r += dr; g += dg; b += db;
	 }
      }
   }
}
