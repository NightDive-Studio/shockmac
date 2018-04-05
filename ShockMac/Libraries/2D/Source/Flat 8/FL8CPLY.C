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
 * $Source: r:/prj/lib/src/2d/RCS/fl8cply.c $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/10/17 14:59:58 $
 *
 * Routines for drawing flat shaded polygons onto a flat 8 canvas.
 *
 * This file is part of the 2d library.
 */

#include "fix.h"
#include "cnvdat.h"
#include "tmapint.h"
#include "gente.h"
#include "poly.h"
#include "rgb.h"
#include "scrdat.h"

// prototypes
int gri_cpoly_loop (grs_tmap_loop_info *ti);
void gri_cpoly_init (grs_tmap_loop_info *ti);
void gri_clut_cpoly_init (grs_tmap_loop_info *ti);

int gri_cpoly_loop (grs_tmap_loop_info *ti)
{
   int x,d;
   fix dx,frac;
   fix r,g,b,dr,dg,db;

   do {
      dx=ti->right.x-ti->left.x;
      frac=fix_ceil(ti->left.x)-ti->left.x;

      r=ti->left.u;
      dr=fix_div(ti->right.u-r,dx);
      r+=fix_mul(frac,dr);

      g=ti->left.v;
      dg=fix_div(ti->right.v-g,dx);
      g+=fix_mul(frac,dg);

      b=ti->left.i;
      db=fix_div(ti->right.i-b,dx);
      b+=fix_mul(frac,db);

      if ((d=fix_cint(ti->right.x)-fix_cint(ti->left.x)) > 0) {
         switch (ti->bm.hlog) {
         case GRL_OPAQUE:
            for (x=fix_cint(ti->left.x); x<fix_cint(ti->right.x); x++)
            {
               int j=gr_index_rgb(r,g,b);
               ti->d[x]=grd_ipal[j];
               r+=dr, g+=dg, b+=db;
            }
            break;
         case GRL_CLUT:
            for (x=fix_cint(ti->left.x); x<fix_cint(ti->right.x); x++)
            {
               int j=gr_index_rgb(r,g,b);
               ti->d[x]=ti->clut[grd_ipal[j]];
               r+=dr, g+=dg, b+=db;
            }
            break;
         }
      } else if (d<0) {
         return TRUE;
      }
      /* update span extrema and destination. */
      ti->left.x+=ti->left.dx;
      ti->right.x+=ti->right.dx;
      ti->left.u+=ti->left.du;
      ti->right.u+=ti->right.du;
      ti->left.v+=ti->left.dv;
      ti->right.v+=ti->right.dv;
      ti->left.i+=ti->left.di;
      ti->right.i+=ti->right.di;
      ti->d+=grd_bm.row;
   } while ((--(ti->n))>0);
   return FALSE;
}

void gri_cpoly_init (grs_tmap_loop_info *ti)
{
   ti->bm.hlog=GRL_OPAQUE;
   ti->d=ti->y*grd_bm.row+grd_bm.bits;
   ti->loop_func=(void (*)()) gri_cpoly_loop;
   ti->top_edge_func=(void (*)()) gri_rgbx_edge;
   ti->bot_edge_func=(void (*)()) gri_rgbx_edge;
}

void gri_clut_cpoly_init (grs_tmap_loop_info *ti)
{
   ti->bm.hlog=GRL_CLUT;
   ti->d=ti->y*grd_bm.row+grd_bm.bits;
   ti->loop_func=(void (*)()) gri_cpoly_loop;
   ti->top_edge_func=(void (*)()) gri_rgbx_edge;
   ti->bot_edge_func=(void (*)()) gri_rgbx_edge;
}
