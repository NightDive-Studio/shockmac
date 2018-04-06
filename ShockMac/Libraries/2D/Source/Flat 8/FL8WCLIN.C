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
 * $Source: r:/prj/lib/src/2d/RCS/fl8wclin.c $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/10/17 15:00:00 $
 *
 * Routines to draw wire polys.
 *
 * This file is part of the 2d library.
 *
 */

#include "cnvdat.h"
#include "plytyp.h"
#include "scrdat.h"
#include "linfcn.h"
 
#define gr_get_ipal_index(r,g,b)   (long) ((((r)>>19) &0x1f) | (((g)>>14) & 0x3e0) | (((b)>>9) & 0x7c00))
#define do_hline_inc_x \
   do {                                                 \
      c=grd_ipal[gr_get_ipal_index(r0,g0,b0)];  \
      p[x]=c;                                           \
      r0+=dr,g0+=dg,b0+=db;                             \
      x++;                                              \
   } while (x<x_new)
#define do_hline_dec_x \
   if (x==x_new) {                                     \
      c=grd_ipal[gr_get_ipal_index(r0,g0,b0)]; \
      p[x]=c;                                          \
      r0+=dr,g0+=dg,b0+=db;                            \
   } else do {                                         \
      x--;                                             \
      c=grd_ipal[gr_get_ipal_index(r0,g0,b0)]; \
      p[x]=c;                                          \
      r0+=dr,g0+=dg,b0+=db;                            \
   } while (x>x_new)

// MLA #pragma off (unreferenced)
void gri_flat8_wire_poly_ucline_norm(long c, long parm, grs_vertex *v0, grs_vertex *v1) {
   #include "fl8wclin.h"
}
// MLA #pragma on (unreferenced)

#undef do_hline_inc_x
#define do_hline_inc_x \
   do {                                                 \
      c=grd_ipal[gr_get_ipal_index(r0,g0,b0)];  \
      p[x]=clut[c];                                     \
      r0+=dr,g0+=dg,b0+=db;                             \
      x++;                                              \
   } while (x<x_new)
#undef do_hline_dec_x
#define do_hline_dec_x \
   if (x==x_new) {                                     \
      c=grd_ipal[gr_get_ipal_index(r0,g0,b0)]; \
      p[x]=clut[c];                                    \
      r0+=dr,g0+=dg,b0+=db;                            \
   } else do {                                         \
      x--;                                             \
      c=grd_ipal[gr_get_ipal_index(r0,g0,b0)]; \
      p[x]=clut[c];                                    \
      r0+=dr,g0+=dg,b0+=db;                            \
   } while (x>x_new)

void gri_flat8_wire_poly_ucline_clut(long c, long parm, grs_vertex *v0, grs_vertex *v1) {
   uchar *clut=(uchar *)parm;
   #include "fl8wclin.h"
}

