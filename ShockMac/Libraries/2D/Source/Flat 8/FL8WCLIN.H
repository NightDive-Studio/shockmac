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
 * $Source: r:/prj/lib/src/2d/RCS/fl8wclin.h $
 * $Revision: 1.1 $
 * $Author: kevin $
 * $Date: 1994/08/04 09:58:31 $
 *
 * Main shell for drawing wire poly rgb lines.
 *
 * This file is part of the 2d library.
 *
 */

/* don't try to use this other than in fl8wclin.c */

   int d,y,y_max,x,x_new;
   fix x0,y0,r0,b0,g0;
   fix x1,y1,r1,b1,g1;
   fix dr,dg,db,x_fix,dx;
   uchar *p;

   if (v1->y>v0->y) {
      y=fix_cint(v0->y);
      y_max=fix_cint(v1->y);
   } else {
      y=fix_cint(v1->y);
      y_max=fix_cint(v0->y);
   }
   p=grd_bm.bits+y*grd_bm.row;

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
         p+=grd_bm.row;
      } while ((++y)<y_max-1);
      x=x_new;
      x_new=fix_cint(x1);
      do_hline_inc_x;
   } else {
      do {
         do_hline_dec_x;
         x_new=fix_cint(x_fix+=dx);
         p+=grd_bm.row;
      } while ((++y)<y_max-1);
      x_new=fix_cint(x1);
      do_hline_dec_x;
   }

