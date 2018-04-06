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
 * $Source: n:/project/lib/src/2d/RCS/clpslin.c $
 * $Revision: 1.2 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 01:13:24 $
*/

/*  clip and fill shaded line -- using  line fill table and grs_vertex interfaces.
    These will be the preferred interfaces and are called from the original
    gr_xxx canvas table x,y routines.
 */

#include <stdlib.h>
#include "clpcon.h"
#include "clpfcn.h"
#include "clpltab.h"
#include "grrend.h"
#include "maxmin.h"

/* The amount of copying into and out of vertex's is quite
   disgusting. 

   Also, becuase of the vertex interface, we've introduced a new
   function call that we can't inline because it needs to return a 
   value.
 */

int gri_sline_clip (grs_vertex *v0, grs_vertex *v1)
{
   int r;

   fix x0, y0, x1, y1;
   fix xb0, xb1, yb0, yb1;
   fix i0, i1;
   fix di;
   fix pixels, pixels_0, pixels_1;

   /* transfer back to x,y  -- stolen from old gen_slin */

   x0 = v0->x; y0 = v0->y; i0 = v0->i;
   x1 = v1->x; y1 = v1->y; i1 = v1->i;

   xb0 = x0; xb1 = x1; yb0 = y0; yb1 = y1;

   pixels = max(fix_abs(y1-y0),fix_abs(x1-x0));

   if (pixels != 0) di = fix_div(i1-i0,pixels);

   r = gr_clip_fix_line (&x0, &y0, &x1, &y1);

   if (r != CLIP_ALL) {

      // If x,y changed and we need to clip intensities,

      if ((i0 != i1) &&
          ((x0 != xb0) || (y0 != yb0) || (x1 != xb1) || (y1 != yb1))) {
         
         pixels_0 = max(fix_abs(yb0-y0),fix_abs(xb0-x0)); // # pixels lost
         pixels_1 = max(fix_abs(yb1-y1),fix_abs(xb1-x1)); // for endpoints

         i0 += fix_mul(di, pixels_0);
         i1 -= fix_mul(di, pixels_1);

      }
    }

   /* and transfer back to v0, v1 */
   (v0->x) = x0;    (v0->y) = y0;    (v0->i) = i0;
   (v1->x) = x1;    (v1->y) = y1;    (v1->i) = i1;

   return r;
}

