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
 * $Source: n:/project/lib/src/2d/RCS/clplin2.c $
 * $Revision: 1.1 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 01:17:48 $
*/

#include <stdlib.h>
#include "grs.h"
#include "clpcon.h"
#include "clpfcn.h"
#include "grrect.h"
#include "clpltab.h"

/* The amount of copying into and out of vertex's is quite
   disgusting. 

   Also, becuase of the vertex interface, we've introduced a new
   function call that we can't inline because it needs to return a 
   value.
 */

int gri_line_clip (grs_vertex *v0, grs_vertex *v1)
{
   int code0;                 /* clip code for (x0,y0) */
   int code1;                 /* code for (x1,y1) */
   int code;                  /* code for current point */
   fix dx;                    /* x distance */
   fix dy;                    /* y distance */
   fix *px;                   /* pointer to x current coordinate */
   fix *py;                   /* pointer to y current coordinate */

   dx = v1->x - v0->x;
   dy = v1->y - v0->y;

   while (1) {
      /* get codes for endpoints. */
      code0 = gr_clip_fix_code (v0->x, v0->y);
      code1 = gr_clip_fix_code (v1->x, v1->y);

      if (code0==0 && code1==0)        /* check trivial accept */
         return CLIP_NONE;
      else if ((code0&code1) != 0)     /* check for trivial reject */
         return CLIP_ALL;

      /* set current code and px&py.  first, for point0, then when it's
         dealt with, point1. */
      if (code0 != 0) {
         px = &(v0->x);
         py = &(v0->y);
         code = code0;
      } else {
         px = &(v1->x);
         py = &(v1->y);
         code = code1;
      }

      /* check for left/right clip; compute intersection. */
      if (code & CLIP_LEFT) {
         *py += fix_mul_div (dy, grd_fix_clip.left-*px, dx);
         *px = grd_fix_clip.left;
      } else if (code & CLIP_RIGHT) {
         *py += fix_mul_div (dy, grd_fix_clip.right-fix_make(1,0)-*px, dx);
         *px = grd_fix_clip.right-fix_make(1,0);
      }
      /* check for top/bottom clip; compute intersection. */
      if (code & CLIP_TOP) {
         *px += fix_mul_div (dx, grd_fix_clip.top-*py, dy);
         *py = grd_fix_clip.top;
      } else if (code & CLIP_BOT) {
         *px += fix_mul_div (dx, grd_fix_clip.bot-fix_make(1,0)-*py, dy);
         *py = grd_fix_clip.bot-fix_make(1,0);
      }
   }
}
