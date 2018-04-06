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
 * $Source: r:/prj/lib/src/2d/RCS/fl8ply.c $
 * $Revision: 1.1 $
 * $Author: kevin $
 * $Date: 1994/08/16 13:11:09 $
 *
 * Routines for drawing flat shaded polygons onto a flat 8 canvas.
 *
 * This file is part of the 2d library.
 */

#include "cnvdat.h"
#include "tmapint.h"
#include "gente.h"
#include "poly.h"
#include <string.h>
#include "tlucdat.h"
#include "lg.h"

// prototypes
int gri_poly_loop (grs_tmap_loop_info *ti);
void gri_solid_poly_init (grs_tmap_loop_info *ti);
void gri_poly_init (grs_tmap_loop_info *ti);
void gri_clut_poly_init (grs_tmap_loop_info *ti);
void gri_tpoly_init (grs_tmap_loop_info *ti);
void gri_clut_tpoly_init (grs_tmap_loop_info *ti);


int gri_poly_loop (grs_tmap_loop_info *ti)
{
	int d;
	uchar c=(uchar )(ti->bm.bits); /* actually, fill_parm */
	uchar *bm_bits = ti->bm.bits;
	uchar *ti_d = ti->d;
	fix 	ti_left_x = ti->left.x;
	fix		ti_right_x = ti->right.x;
	uchar	*ti_clut = ti->clut;
	uchar ti_hlog = ti->bm.hlog;
	ushort	grow = grd_bm.row;
	fix 	ti_left_dx = ti->left.dx;
	fix 	ti_right_dx = ti->right.dx;
	
	do {
	  if ((d=fix_cint(ti_right_x)-fix_cint(ti_left_x)) > 0) {
	     switch (ti_hlog) {
	        int x;
	     case GRL_OPAQUE:
	        LG_memset(ti_d+fix_cint(ti_left_x), c, d);
	        break;
	     case GRL_TLUC8:
	        for (x=fix_cint(ti_left_x); x<fix_cint(ti_right_x); x++)
	           ti_d[x]=bm_bits[ti_d[x]];
	        break;
	     case GRL_CLUT|GRL_TLUC8:
	        for (x=fix_cint(ti_left_x); x<fix_cint(ti_right_x); x++)
	           ti_d[x]=ti_clut[bm_bits[ti_d[x]]];
	        break;
	     }
	  } else if (d<0) {
	     return TRUE;
	  }
	  /* update span extrema and destination. */
	  ti_d+=grow;
	  ti_left_x+=ti_left_dx;
	  ti_right_x+=ti_right_dx;
	} while ((--(ti->n))>0);
	
	ti->d = ti_d;
	ti->right.x = ti_right_x;
	ti->left.x = ti_left_x;
	return FALSE;
}

void gri_solid_poly_init (grs_tmap_loop_info *ti)
{
   ti->bm.bits=ti->clut;      /* set fill_parm */
   ti->bm.hlog=GRL_OPAQUE;
   ti->d=ti->y*grd_bm.row+grd_bm.bits;
   ti->loop_func= (void (*)()) gri_poly_loop;
   ti->top_edge_func= (void (*)()) gri_x_edge;
   ti->bot_edge_func= (void (*)()) gri_x_edge;
}

void gri_poly_init (grs_tmap_loop_info *ti)
{
   ti->bm.hlog=GRL_OPAQUE;
   ti->d=ti->y*grd_bm.row+grd_bm.bits;
   ti->loop_func= (void (*)()) gri_poly_loop;
   ti->top_edge_func= (void (*)()) gri_x_edge;
   ti->bot_edge_func= (void (*)()) gri_x_edge;
}

void gri_clut_poly_init (grs_tmap_loop_info *ti)
{
   ti->bm.bits=(uchar *)(ti->clut[(int )(ti->bm.bits)]);
   ti->bm.hlog=GRL_OPAQUE;
   ti->d=ti->y*grd_bm.row+grd_bm.bits;
   ti->loop_func= (void (*)()) gri_poly_loop;
   ti->top_edge_func= (void (*)()) gri_x_edge;
   ti->bot_edge_func= (void (*)()) gri_x_edge;
}

void gri_tpoly_init (grs_tmap_loop_info *ti)
{
   if (tluc8tab[(uchar )(ti->bm.bits)]!=NULL) {
      ti->bm.bits=tluc8tab[(uchar )(ti->bm.bits)];
      ti->bm.hlog=GRL_TLUC8;
   } else {
      ti->bm.hlog=GRL_OPAQUE;
   }
   ti->d=ti->y*grd_bm.row+grd_bm.bits;
   ti->loop_func= (void (*)()) gri_poly_loop;
   ti->top_edge_func= (void (*)()) gri_x_edge;
   ti->bot_edge_func= (void (*)()) gri_x_edge;
}

void gri_clut_tpoly_init (grs_tmap_loop_info *ti)
{
   if (tluc8tab[(uchar )(ti->bm.bits)]!=NULL) {
      ti->bm.bits=tluc8tab[(uchar )(ti->bm.bits)];
      ti->bm.hlog=GRL_TLUC8|GRL_CLUT;
   } else {
      ti->bm.bits=(uchar *)(ti->clut[(int )(ti->bm.bits)]);
      ti->bm.hlog=GRL_OPAQUE;
   }
   ti->d=ti->y*grd_bm.row+grd_bm.bits;
   ti->loop_func= (void (*)()) gri_poly_loop;
   ti->top_edge_func= (void (*)()) gri_x_edge;
   ti->bot_edge_func= (void (*)()) gri_x_edge;
}
