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
 * $Source: n:/project/lib/src/2d/RCS/clppoly.c $
 * $Revision: 1.4 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:50:13 $
 * 
 * Routines for clipping fixed-point polygons to a rectangle.
 *
 * This file is part of the 2d library.
 *
 * $Log: clppoly.c $
 * Revision 1.4  1993/10/19  09:50:13  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.3  1993/10/01  15:38:57  kaboom
 * Cleaned up code some; converted to using clpcon.h instead of clip.h.
 * 
 * Revision 1.2  1993/08/10  15:27:13  kaboom
 * Reformatted function code.
 * 
 * Revision 1.1  1993/02/22  14:41:42  kaboom
 * Initial revision
 * 
 ********************************************************************
 * Log from old clip.c:
 *
 * Revision 1.11  1993/02/05  16:27:54  matt
 * Fixed gr_clip_fix_poly() and gr_clip_int_poly(), which got the
 * arrays mixed up when clipping the top edge.
 * 
 * Revision 1.8  1993/02/04  16:27:58  matt
 * Fixed gr_clip_fix_poly() by using new fix_mul_div() function to
 * preserve sign in computing new coordinates.
 */

#include "grs.h"
#include "buffer.h"
#include "clpcon.h"
#include "cnvdat.h"
#include "clpfcn.h"

int gr_clip_fix_poly (int n, fix *vlist, fix *clist)
{
   fix *tlist = (fix *)gr_alloc_temp (128*sizeof (fix));
   fix x0, y0;
   fix x1, y1;
   int i, j;

   /* clip left edge from vlist->tlist */
   x0 = vlist[2*(n-1)];
   y0 = vlist[2*(n-1)+1];
   for (i=j=0; i<n; i++) {
      x1 = vlist[2*i];
      y1 = vlist[2*i+1];

      if (x0 >= grd_fix_clip.left) {
         /* start point is inside half plane */
         if (x1 >= grd_fix_clip.left) {
            /* both points inside half plane.  output end point. */
            tlist[2*j] = x1;
            tlist[2*j+1] = y1;
            j++;
         } else {
            /* edge exits half plane.  output intersection. */
            fix dx = x1-x0;
            fix dy = y1-y0;

            tlist[2*j] = grd_fix_clip.left;
            tlist[2*j+1] = y0 + fix_mul_div (dy,(grd_fix_clip.left-x0),dx);
            j++;
         }
      } else {
         /* start point is outside half plane. */
         if (x1 >= grd_fix_clip.left) {
            /* edge enters half plane.  output intersection and end point. */
            fix dx = x1-x0;
            fix dy = y1-y0;

            if (x1 != grd_fix_clip.left) {
               tlist[2*j] = grd_fix_clip.left;
               tlist[2*j+1] = y0 + fix_mul_div (dy,(grd_fix_clip.left-x0),dx);
               j++;
            }
            tlist[2*j] = x1;
            tlist[2*j+1] = y1;
            j++;
         } else
            /* both points outside, eliminate edge. */
            ;
      }
      x0=x1; y0=y1;
   }

   /* clip top edge from tlist->clist */
   x0 = tlist[2*(j-1)];
   y0 = tlist[2*(j-1)+1];
   for (n=j, i=j=0; i<n; i++) {
      x1 = tlist[2*i];
      y1 = tlist[2*i+1];

      if (y0 >= grd_fix_clip.top) {
         /* start point is inside half plane */
         if (y1 >= grd_fix_clip.top) {
            /* both points inside half plane.  output end point. */
            clist[2*j] = x1;
            clist[2*j+1] = y1;
            j++;
         } else {
            /* edge exits half plane.  output intersection. */
            fix dx = x1-x0;
            fix dy = y1-y0;

            clist[2*j] = x0 + fix_mul_div (dx,(grd_fix_clip.top-y0),dy);
            clist[2*j+1] = grd_fix_clip.top;
            j++;
         }
      } else {
         /* start point is outside half plane. */
         if (y1 >= grd_fix_clip.top) {
            /* edge enters half plane.  output intersection and end point. */
            fix dx = x1-x0;
            fix dy = y1-y0;

            if (y1 != grd_fix_clip.top) {
               clist[2*j] = x0 + fix_mul_div (dx,(grd_fix_clip.top-y0),dy);
               clist[2*j+1] = grd_fix_clip.top;
               j++;
            }
            clist[2*j] = x1;
            clist[2*j+1] = y1;
            j++;
         } else
            /* both points outside, eliminate edge. */
            ;
      }
      x0=x1; y0=y1;
   }

   /* clip right edge from clist->tlist */
   x0 = clist[2*(j-1)];
   y0 = clist[2*(j-1)+1];
   for (n=j, i=j=0; i<n; i++) {
      x1 = clist[2*i];
      y1 = clist[2*i+1];

      if (x0 <= grd_fix_clip.right) {
         /* start point is inside half plane */
         if (x1 <= grd_fix_clip.right) {
            /* both points inside half plane.  output end point. */
            tlist[2*j] = x1;
            tlist[2*j+1] = y1;
            j++;
         } else {
            /* edge exits half plane.  output intersection. */
            fix dx = x1-x0;
            fix dy = y1-y0;

            tlist[2*j] = grd_fix_clip.right;
            tlist[2*j+1] = y0 + fix_mul_div (dy,(grd_fix_clip.right-x0),dx);
            j++;
         }
      } else {
         /* start point is outside half plane. */
         if (x1 <= grd_fix_clip.right) {
            /* edge enters half plane.  output intersection and end point. */
            fix dx = x1-x0;
            fix dy = y1-y0;

            if (x1 != grd_fix_clip.right) {
               tlist[2*j] = grd_fix_clip.right;
               tlist[2*j+1] = y0 + fix_mul_div (dy,(grd_fix_clip.right-x0),dx);
               j++;
            }
            tlist[2*j] = x1;
            tlist[2*j+1] = y1;
            j++;
         } else
            /* both points outside, eliminate edge. */
            ;
      }
      x0=x1; y0=y1;
   }

   /* clip bottom edge from tlist->clist */
   x0 = tlist[2*(j-1)];
   y0 = tlist[2*(j-1)+1];
   for (n=j, i=j=0; i<n; i++) {
      x1 = tlist[2*i];
      y1 = tlist[2*i+1];

      if (y0 <= grd_fix_clip.bot) {
         /* start point is inside half plane */
         if (y1 <= grd_fix_clip.bot) {
            /* both points inside half plane.  output end point. */
            clist[2*j] = x1;
            clist[2*j+1] = y1;
            j++;
         } else {
            /* edge exits half plane.  output intersection. */
            fix dx = x1-x0;
            fix dy = y1-y0;

            clist[2*j] = x0 + fix_mul_div (dx,(grd_fix_clip.bot-y0),dy);
            clist[2*j+1] = grd_fix_clip.bot;
            j++;
         }
      } else {
         /* start point is outside half plane. */
         if (y1 <= grd_fix_clip.bot) {
            /* edge enters half plane.  output intersection and end point. */
            fix dx = x1-x0;
            fix dy = y1-y0;

            if (y1 != grd_fix_clip.bot) {
               clist[2*j] = x0 + fix_mul_div (dx,(grd_fix_clip.bot-y0),dy);
               clist[2*j+1] = grd_fix_clip.bot;
               j++;
            }
            clist[2*j] = x1;
            clist[2*j+1] = y1;
            j++;
         } else
            /* both points outside, eliminate edge. */
            ;
      }
      x0=x1; y0=y1;
   }

   /* punt temporary vertex list. */
   gr_free_temp (tlist);

   /* return new number of vertices. */
   return j;
}
