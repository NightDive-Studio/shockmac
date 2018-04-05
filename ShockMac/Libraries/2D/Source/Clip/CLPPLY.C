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
 * $Source: r:/prj/lib/src/2d/RCS/clpply.c $
 * $Revision: 1.4 $
 * $Author: kevin $
 * $Date: 1994/08/10 01:32:39 $
 * 
 * Routines for clipping a polygon to a rectangle.
 *
 * This file is part of the 2d library.
 *
 * $Log: clpply.c $
 * Revision 1.4  1994/08/10  01:32:39  kevin
 * fixed typo.
 * 
 * Revision 1.3  1993/10/19  09:50:11  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.2  1993/10/15  12:07:40  baf
 * Removed reference to clip.h
 * 
 * Revision 1.1  1993/10/01  15:40:46  kaboom
 * Initial revision
 */

#include "grs.h"
#include "buffer.h"
#include "clpcon.h"
#include "clpfcn.h"
#include "cnvdat.h"
#include "plytyp.h"
#include "poly.h"

/* clips a polygon's screen coordinates and vertex parameters to the
   current clipping rectangle.  the polygon has n vertices, each with
   l fixed-point values (including x,y), pointed to by vpl.  the
   destination buffer is returned in *pcplist.  pass in *pcplist of NULL
   to have it allocated.
   note: this can be improved a lot.  pointed should be coded for trivial
   accept/reject.  code can be saved in the enter/exit cases by noting
   the change in the in/out status. */
int gr_clip_poly(int n, int l, grs_vertex **vpl, grs_vertex ***pcplist)
{
   fix **cplist;
   fix **tplist;
   fix *tlist;
   int i;                  /* source vertex index */
   int j;                  /* destination vertex index */
   int k;
   int new_v = 0;
   fix *v0, *v1;
   fix num, den;

   if (*pcplist)
      cplist = (fix **)*pcplist;
   else
      cplist = (fix **)gr_alloc_temp(2*n*(l+2)*sizeof(fix *));
   tplist = (fix **)(cplist+2*n);
   tlist = (fix *)(tplist+2*n);

   /* clip left edge from vpl->tplist */
   v0 = ((fix **)vpl)[n-1];
   for (i=j=0; i<n; i++) {
      v1 = ((fix **)vpl)[i];

      if (v0[0] >= grd_canvas->gc.clip.f.left) {
         /* start point is inside half plane */
         if (v1[0] >= grd_canvas->gc.clip.f.left)
            /* both points inside half plane.  output end point. */
            tplist[j++] = v1;
         else {
            /* edge exits half plane.  output intersection. */
            num = grd_canvas->gc.clip.f.left-v0[0];
            den = v1[0]-v0[0];
            tlist[new_v] = grd_canvas->gc.clip.f.left;
            tlist[new_v+1] = v0[1]+fix_mul_div(v1[1]-v0[1], num, den);
            for (k=2; k<l; k++)
               tlist[new_v+k] = v0[k]+fix_mul_div(v1[k]-v0[k], num, den);
            tplist[j++] = &tlist[new_v];
            new_v += k;
         }
      } else {
         /* start point is outside half plane. */
         if (v1[0] >= grd_canvas->gc.clip.f.left) {
            /* edge enters half plane.  output intersection and end point. */
            if (v1[0] != grd_canvas->gc.clip.f.left) {
               num = grd_canvas->gc.clip.f.left-v0[0];
               den = v1[0]-v0[0];
               tlist[new_v] = grd_canvas->gc.clip.f.left;
               tlist[new_v+1] = v0[1]+fix_mul_div(v1[1]-v0[1], num, den);
               for (k=2; k<l; k++)
                  tlist[new_v+k] = v0[k]+fix_mul_div(v1[k]-v0[k], num, den);
               tplist[j++] = &tlist[new_v];
               new_v += k;
            }
            tplist[j++] = v1;
         } else
            /* both points outside, eliminate edge. */
            ;
      }
      v0=v1;
   }

   /* clip top edge from tplist->cplist */
   v0 = tplist[j-1];
   for (n=j, i=j=0; i<n; i++) {
      v1 = tplist[i];

      if (v0[1] >= grd_canvas->gc.clip.f.top) {
         /* start point is inside half plane */
         if (v1[1] >= grd_canvas->gc.clip.f.top)
            /* both points inside half plane.  output end point. */
            cplist[j++] = v1;
         else {
            /* edge exits half plane.  output intersection. */
            num = grd_canvas->gc.clip.f.top-v0[1];
            den = v1[1]-v0[1];
            tlist[new_v] = v0[0]+fix_mul_div(v1[0]-v0[0], num, den);
            tlist[new_v+1] = grd_canvas->gc.clip.f.top;
            for (k=2; k<l; k++)
               tlist[new_v+k] = v0[k]+fix_mul_div(v1[k]-v0[k], num, den);
            cplist[j++] = &tlist[new_v];
            new_v += k;
         }
      } else {
         /* start point is outside half plane. */
         if (v1[1] >= grd_canvas->gc.clip.f.top) {
            /* edge enters half plane.  output intersection and end point. */
            if (v1[1] != grd_canvas->gc.clip.f.top) {
               num = grd_canvas->gc.clip.f.top-v0[1];
               den = v1[1]-v0[1];
               tlist[new_v] = v0[0]+fix_mul_div(v1[0]-v0[0], num, den);
               tlist[new_v+1] = grd_canvas->gc.clip.f.top;
               for (k=2; k<l; k++)
                  tlist[new_v+k] = v0[k]+fix_mul_div(v1[k]-v0[k], num, den);
               cplist[j++] = &tlist[new_v];
               new_v += k;
            }
            cplist[j++] = v1;
         } else
            /* both points outside, eliminate edge. */
            ;
      }
      v0=v1;
   }

   /* clip right edge from cplist->tplist */
   v0 = cplist[j-1];
   for (n=j, i=j=0; i<n; i++) {
      v1 = cplist[i];

      if (v0[0] <= grd_canvas->gc.clip.f.right) {
         /* start point is inside half plane */
         if (v1[0] <= grd_canvas->gc.clip.f.right)
            /* both points inside half plane.  output end point. */
            tplist[j++] = v1;
         else {
            /* edge exits half plane.  output intersection. */
            num = grd_canvas->gc.clip.f.right-v0[0];
            den = v1[0]-v0[0];
            tlist[new_v] = grd_canvas->gc.clip.f.right;
            tlist[new_v+1] = v0[1]+fix_mul_div(v1[1]-v0[1], num, den);
            for (k=2; k<l; k++)
               tlist[new_v+k] = v0[k]+fix_mul_div(v1[k]-v0[k], num, den);
            tplist[j++] = &tlist[new_v];
            new_v += k;
         }
      } else {
         /* start point is outside half plane. */
         if (v1[0] <= grd_canvas->gc.clip.f.right) {
            /* edge enters half plane.  output intersection and end point. */
            if (v1[0] != grd_canvas->gc.clip.f.right) {
               num = grd_canvas->gc.clip.f.right-v0[0];
               den = v1[0]-v0[0];
               tlist[new_v] = grd_canvas->gc.clip.f.right;
               tlist[new_v+1] = v0[1]+fix_mul_div(v1[1]-v0[1], num, den);
               for (k=2; k<l; k++)
                  tlist[new_v+k] = v0[k]+fix_mul_div(v1[k]-v0[k], num, den);
               tplist[j++] = &tlist[new_v];
               new_v += k;
            }
            tplist[j++] = v1;
         } else
            /* both points outside, eliminate edge. */
            ;
      }
      v0=v1;
   }

   /* clip bottom edge from tplist->cplist */
   v0 = tplist[j-1];
   for (n=j, i=j=0; i<n; i++) {
      v1 = tplist[i];

      if (v0[1] <= grd_canvas->gc.clip.f.bot) {
         /* start point is inside half plane */
         if (v1[1] <= grd_canvas->gc.clip.f.bot)
            /* both points inside half plane.  output end point. */
            cplist[j++] = v1;
         else {
            /* edge exits half plane.  output intersection. */
            num = grd_canvas->gc.clip.f.bot-v0[1];
            den = v1[1]-v0[1];
            tlist[new_v] = v0[0]+fix_mul_div(v1[0]-v0[0], num, den);
            tlist[new_v+1] = grd_canvas->gc.clip.f.bot;
            for (k=2; k<l; k++)
               tlist[new_v+k] = v0[k]+fix_mul_div(v1[k]-v0[k], num, den);
            cplist[j++] = &tlist[new_v];
            new_v += k;
         }
      } else {
         /* start point is outside half plane. */
         if (v1[1] <= grd_canvas->gc.clip.f.bot) {
            /* edge enters half plane.  output intersection and end point. */
            if (v1[1] != grd_canvas->gc.clip.f.bot) {
               num = grd_canvas->gc.clip.f.bot-v0[1];
               den = v1[1]-v0[1];
               tlist[new_v] = v0[0]+fix_mul_div(v1[0]-v0[0], num, den);
               tlist[new_v+1] = grd_canvas->gc.clip.f.bot;
               for (k=2; k<l; k++)
                  tlist[new_v+k] = v0[k]+fix_mul_div(v1[k]-v0[k], num, den);
               cplist[j++] = &tlist[new_v];
               new_v += k;
            }
            cplist[j++] = v1;
         } else
            /* both points outside, eliminate edge. */
            ;
      }
      v0=v1;
   }

   *pcplist = (grs_vertex **)cplist;

   /* return new number of vertices. */
   return j;
}
