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
 * $Source: n:/project/lib/src/2d/RCS/clpclin.c $
 * $Revision: 1.2 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 01:14:47 $
*/

/*  clip and fill colored line -- using  line fill table and grs_vertex interfaces.
    These will be the preferred interfaces and are called from the original
    gr_xxx canvas table x,y routines.
 */

#include <stdlib.h>
#include "clpcon.h"
#include "clpfcn.h"
#include "grrend.h"
#include "rgb.h"
#include "scrdat.h"
#include "clpltab.h"
#include "maxmin.h"

#define fix_make_nof(x)           fix_make(x,0x0000)

/* The amount of copying into and out of vertex's is quite
   disgusting. 

   Also, becuase of the vertex interface, we've introduced a new
   function call that we can't inline because it needs to return a 
   value.
 */

int gri_cline_clip (grs_vertex *v0, grs_vertex *v1)
{
   int r;
   fix x0, y0, x1, y1;
   fix xb0, xb1, yb0, yb1;
   uchar r0, g0, b0, r1, g1, b1;
   fix dr, dg, db;
   fix pixels, pixels_0, pixels_1;

   /* transfer out of v0, v1 -- from old gen_clin */
   x0 = v0->x; y0 = v0->y; 
   r0 = (uchar) (v0->u); g0 = (uchar) (v0->v); b0 = (uchar) (v0->w);
   x1 = v1->x; y1 = v1->y; 
   r1 = (uchar) (v1->u); g1 = (uchar) (v1->v); b1 = (uchar) (v1->w);


   xb0 = x0; xb1 = x1; yb0 = y0; yb1 = y1;

   pixels = max(fix_abs(y1-y0),fix_abs(x1-x0));

   if (pixels != 0) {
      dr = fix_div(fix_make_nof(r1-r0),pixels);
      dg = fix_div(fix_make_nof(g1-g0),pixels);
      db = fix_div(fix_make_nof(b1-b0),pixels);
   }
   
   r = gr_clip_fix_line (&x0, &y0, &x1, &y1);   
   if (r != CLIP_ALL) {

      if (((r0 != r1) || (g0 != g1) || (b0 != b1)) &&
          ((x0 != xb0) || (y0 != yb0) || (x1 != xb1) || (y1 != yb1))) {

         pixels_0 = max(fix_abs(yb0-y0),fix_abs(xb0-x0)); /* # pixels lost */
         pixels_1 = max(fix_abs(yb1-y1),fix_abs(xb1-x1)); /* for endpoints */

         r0 += fix_int(fix_mul(dr, pixels_0));
         g0 += fix_int(fix_mul(dg, pixels_0));
         b0 += fix_int(fix_mul(db, pixels_0));

         r1 -= fix_int(fix_mul(dr, pixels_1));
         g1 -= fix_int(fix_mul(dg, pixels_1));
         b1 -= fix_int(fix_mul(db, pixels_1));
      }
    }

   /* and transfer back to v0, v1 */
   (v0->x) = x0; (v0->y) = y0; 
   (v0->u) = r0; (v0->v) = g0; (v0->w) = b0;
   (v1->x) = x1; (v1->y) = y1; 
   (v1->u) = r1; (v1->v) = g1; (v1->w) = b1;
   
   return r;
 }


