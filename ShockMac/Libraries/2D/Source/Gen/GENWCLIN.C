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
 * $Source: r:/prj/lib/src/2d/RCS/genwclin.c $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/10/17 15:00:03 $
 *
 * Routines to draw wire polys.
 *
 * This file is part of the 2d library.
 *
 */

#include "pixfill.h"
#include "plytyp.h"
#include "scrdat.h"
#include "linfcn.h"

#define gr_get_ipal_index(r,g,b)   (long) ((((r)>>19) &0x1f) | (((g)>>14) & 0x3e0) | (((b)>>9) & 0x7c00))
#define do_hline_inc_x \
   do {                                         \
      c=grd_ipal[gr_get_ipal_index(r0,g0,b0)];  \
      grd_pixel_fill(c,parm,x,y);               \
      r0+=dr,g0+=dg,b0+=db;                     \
      x++;                                      \
   } while (x<x_new)
#define do_hline_dec_x \
   if (x==x_new) {                             \
      c=grd_ipal[gr_get_ipal_index(r0,g0,b0)]; \
      grd_pixel_fill(c,parm,x,y);              \
      r0+=dr,g0+=dg,b0+=db;                    \
   } else do {                                 \
      x--;                                     \
      c=grd_ipal[gr_get_ipal_index(r0,g0,b0)]; \
      grd_pixel_fill(c,parm,x,y);              \
      r0+=dr,g0+=dg,b0+=db;                    \
   } while (x>x_new)

void gri_gen_wire_poly_ucline(long c, long parm, grs_vertex *v0, grs_vertex *v1) {
   int d,y,y_max,x,x_new;
   fix x0,y0,r0,b0,g0;
   fix x1,y1,r1,b1,g1;
   fix dr,dg,db,x_fix,dx;

   if (v1->y>v0->y) {
      y=fix_cint(v0->y);
      y_max=fix_cint(v1->y);
   } else {
      y=fix_cint(v1->y);
      y_max=fix_cint(v0->y);
   }

   /* horizontal? */
   if (y_max-y<=1) {
      if (v1->x>v0->x) {
         x_new=fix_cint(v1->x),x=fix_cint(v0->x);
         r0=fix_make(v0->u,0),g0=fix_make(v0->v,0),b0=fix_make(v0->w,0);
         r1=fix_make(v1->u,0),g1=fix_make(v1->v,0),b1=fix_make(v1->w,0);
      }
      else {
         x_new=fix_cint(v0->x),x=fix_cint(v1->x);
         r0=fix_make(v1->u,0),g0=fix_make(v1->v,0),b0=fix_make(v1->w,0);
         r1=fix_make(v0->u,0),g1=fix_make(v0->v,0),b1=fix_make(v0->w,0);
      }
      d=x_new-x;
      if (d>0) {
         dr=(r1-r0)/d;
         dg=(g1-g0)/d;
         db=(b1-b0)/d;
         do_hline_inc_x;
      }
      return;
   }

   /* not horizontal */
   if (v1->y>v0->y) {
      y0=v0->y,x0=v0->x;
      r0=fix_make(v0->u,0),g0=fix_make(v0->v,0),b0=fix_make(v0->w,0);
      y1=v1->y,x1=v1->x;
      r1=fix_make(v1->u,0),g1=fix_make(v1->v,0),b1=fix_make(v1->w,0);
   } else {
      y1=v0->y,x1=v0->x;
      r1=fix_make(v0->u,0),g1=fix_make(v0->v,0),b1=fix_make(v0->w,0);
      y0=v1->y,x0=v1->x;
      r0=fix_make(v1->u,0),g0=fix_make(v1->v,0),b0=fix_make(v1->w,0);
   }
   dx=fix_div(x1-x0,y1-y0);
   if (fix_abs(dx)>FIX_UNIT) {
      d=fix_cint(x1)-fix_cint(x0);
      if (d<0) d=-d;
   } else {
      d=y_max-y;
   }
   dr=(r1-r0)/d;
   dg=(g1-g0)/d;
   db=(b1-b0)/d;
   x=fix_cint(x0);
   x_fix=x0+fix_mul(fix_ceil(y0)-y0,dx);
   x_new=fix_cint(x_fix);

   /* draw line */
   if (dx>=0) {
      do_hline_inc_x;
      do {
         x=x_new;
         x_new=fix_cint(x_fix+=dx);
         do_hline_inc_x;
      } while ((++y)<y_max-1);
      x=x_new;
      x_new=fix_cint(x1);
      do_hline_inc_x;
   } else {
      do {
         do_hline_dec_x;
         x_new=fix_cint(x_fix+=dx);
      } while ((++y)<y_max-1);
      x_new=fix_cint(x1);
      do_hline_dec_x;
   }
}
