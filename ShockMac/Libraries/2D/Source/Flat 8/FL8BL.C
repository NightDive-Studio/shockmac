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
 * $Source: r:/prj/lib/src/2d/RCS/fl8bl.c $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/09/08 01:12:51 $
 * 
 * Generic routines for texture mapping rsd bitmaps.
 *
 * This file is part of the 2d library.
 *
 */

#include "bitmap.h"
#include "grs.h"
#include "ifcn.h"
#include "rsdunpck.h"
#include "tmapint.h"
#include "tmaptab.h"
#include "lg.h"

extern void flat8_flat8_smooth_hv_double_ubitmap(grs_bitmap *src, grs_bitmap *dst);
void gri_trans_blend_clut_lin_umap_init(grs_tmap_loop_info *ti);


void gri_trans_blend_clut_lin_umap_init(grs_tmap_loop_info *ti)
{
   if (grd_unpack_buf!=NULL) {
      grs_bitmap tbm;

      tbm.row=2*ti->bm.w;
      if (ti->bm.bits==grd_unpack_buf)
         tbm.bits=ti->bm.bits+ti->bm.row*ti->bm.h;
      else
         tbm.bits=grd_unpack_buf;
               
      flat8_flat8_smooth_hv_double_ubitmap(&(ti->bm),&tbm);
      ti->n=BMT_FLAT8*GRD_FUNCS+GRC_TRANS_CLUT_BILIN;
      ti->bm.bits=tbm.bits;
      ti->bm.row=tbm.row;
      ti->bm.w*=2;
      ti->bm.h*=2;
      ((void (*)(grs_tmap_loop_info *))(grd_tmap_init_table[ti->n]))(ti);
   }
}
