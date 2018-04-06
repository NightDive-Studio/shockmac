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
 * $Source: r:/prj/lib/src/2d/RCS/fl8tf.h $
 * $Revision: 1.14 $
 * $Author: kevin $
 * $Date: 1994/10/25 16:51:42 $
 *
 * Table functions.
 * Flat8 canvas.
 *
 * This file is part of the 2d library.
 *
 */

#include "tmapint.h"
#include "pertyp.h"

/* pixel primitives */
extern void flat8_set_upixel(long color, short x, short y);
extern void flat8_clut_set_upixel (long color, short x, short y);
extern void flat8_xor_set_upixel (long color, short x, short y);
extern void flat8_blend_set_upixel (long color, short x, short y);
extern void flat8_solid_set_upixel (long color, short x, short y);

/* blit primitives */
#if (defined(powerc) || defined(__powerc))	
extern void flat8_flat8_ubitmap (grs_bitmap *bm, short x, short y);
#else
extern asm void flat8_flat8_ubitmap (grs_bitmap *bm, short x, short y);
#endif

extern void flat8_tluc8_ubitmap (grs_bitmap *bm, short x, short y);
extern int gri_flat8_mask_bitmap(grs_bitmap *bm, short x, short y, grs_stencil *sten);
extern void gri_flat8_clut_ubitmap (grs_bitmap *bm, short x, short y, uchar *cl);
extern int gri_flat8_mask_fill_clut_bitmap(grs_bitmap *bm, short x, short y, grs_stencil *sten);
extern void gri_flat8_fill_clut_ubitmap (grs_bitmap *bm, short x, short y);
extern void gri_flat8_rsd8_ubitmap(grs_bitmap *bm, short x, short y);
extern int gri_flat8_rsd8_bitmap (grs_bitmap *bm, short x_left, short y_top);

/* inner loop initializers */
/* normal fill: */
extern void gri_trans_blend_clut_lin_umap_init(grs_tmap_loop_info *ti);
extern void gri_opaque_true_lin_umap_init();
extern void gri_opaque_clut_true_lin_umap_init();

extern void gri_opaque_lin_umap_init(grs_tmap_loop_info *tli);
extern void gri_trans_lin_umap_init(grs_tmap_loop_info *tli);
extern void gri_opaque_lit_lin_umap_init(grs_tmap_loop_info *tli);
extern void gri_trans_lit_lin_umap_init(grs_tmap_loop_info *tli);
extern void gri_opaque_clut_lin_umap_init(grs_tmap_loop_info *tli);
extern void gri_trans_clut_lin_umap_init(grs_tmap_loop_info *tli);

extern void gri_opaque_floor_umap_init(grs_tmap_loop_info *tli);
extern void gri_trans_floor_umap_init(grs_tmap_loop_info *tli);
extern void gri_opaque_lit_floor_umap_init(grs_tmap_loop_info *tli);
extern void gri_trans_lit_floor_umap_init(grs_tmap_loop_info *tli);
extern void gri_opaque_clut_floor_umap_init(grs_tmap_loop_info *tli);
extern void gri_trans_clut_floor_umap_init(grs_tmap_loop_info *tli);

extern void gri_opaque_wall_umap_init(grs_tmap_loop_info *tli);
extern void gri_trans_wall_umap_init(grs_tmap_loop_info *tli);
extern void gri_opaque_lit_wall_umap_init(grs_tmap_loop_info *tli);
extern void gri_trans_lit_wall_umap_init(grs_tmap_loop_info *tli);
extern void gri_opaque_clut_wall_umap_init(grs_tmap_loop_info *tli);
extern void gri_trans_clut_wall_umap_init(grs_tmap_loop_info *tli);

extern void gri_opaque_wall1d_umap_init(grs_tmap_loop_info *tli);
extern void gri_trans_wall1d_umap_init(grs_tmap_loop_info *tli);
extern void gri_opaque_clut_wall1d_umap_init(grs_tmap_loop_info *tli);
extern void gri_trans_lit_wall1d_umap_init(grs_tmap_loop_info *tli);
extern void gri_opaque_lit_wall1d_umap_init(grs_tmap_loop_info *tli);
extern void gri_trans_clut_wall1d_umap_init(grs_tmap_loop_info *tli);

extern void gri_opaque_per_umap_hscan_init(grs_bitmap *bm, grs_per_setup *ps);
extern void gri_trans_per_umap_hscan_init(grs_bitmap *bm, grs_per_setup *ps);
extern void gri_opaque_per_umap_vscan_init(grs_bitmap *bm, grs_per_setup *ps);
extern void gri_trans_per_umap_vscan_init(grs_bitmap *bm, grs_per_setup *ps);
extern void gri_opaque_lit_per_umap_hscan_init(grs_bitmap *bm, grs_per_setup *ps);
extern void gri_trans_lit_per_umap_hscan_init(grs_bitmap *bm, grs_per_setup *ps);

extern void gri_opaque_lit_per_umap_vscan_init(grs_bitmap *bm, grs_per_setup *ps);
extern void gri_trans_lit_per_umap_vscan_init(grs_bitmap *bm, grs_per_setup *ps);
extern void gri_opaque_clut_per_umap_hscan_init(grs_bitmap *bm, grs_per_setup *ps);
extern void gri_trans_clut_per_umap_hscan_init(grs_bitmap *bm, grs_per_setup *ps);
extern void gri_opaque_clut_per_umap_vscan_init(grs_bitmap *bm, grs_per_setup *ps);
extern void gri_trans_clut_per_umap_vscan_init(grs_bitmap *bm, grs_per_setup *ps);

extern int gri_opaque_scale_umap_init(grs_tmap_loop_info *tli);
extern int gri_trans_scale_umap_init(grs_tmap_loop_info *tli, grs_vertex **vert);
extern int gri_opaque_clut_scale_umap_init(grs_tmap_loop_info *tli);
extern int gri_trans_clut_scale_umap_init(grs_tmap_loop_info *tli);

extern void gri_poly_init (grs_tmap_loop_info *ti);
extern void gri_spoly_init (grs_tmap_loop_info *ti);
extern void gri_cpoly_init (grs_tmap_loop_info *ti);
extern void gri_tpoly_init (grs_tmap_loop_info *ti);
extern void gri_stpoly_init (grs_tmap_loop_info *ti);

/* clut fill: */
extern void gri_clut_poly_init (grs_tmap_loop_info *ti);
extern void gri_clut_spoly_init(grs_tmap_loop_info *ti);
extern void gri_clut_cpoly_init(grs_tmap_loop_info *ti);
extern void gri_clut_tpoly_init(grs_tmap_loop_info *ti);
extern void gri_clut_stpoly_init(grs_tmap_loop_info *ti);

/* solid fill: */
extern void gri_solid_poly_init(grs_tmap_loop_info *ti);
extern void gri_trans_solid_lin_umap_init(grs_tmap_loop_info *ti);
extern void gri_trans_solid_floor_umap_init(grs_tmap_loop_info *ti);
extern void gri_trans_solid_wall_umap_init(grs_tmap_loop_info *ti);
extern void gri_trans_solid_per_umap_hscan_init(grs_bitmap *bm, grs_per_setup *ps);
extern void gri_trans_solid_per_umap_vscan_init(grs_bitmap *bm, grs_per_setup *ps);
extern int gri_opaque_solid_scale_umap_init(grs_tmap_loop_info *info, grs_vertex **vert);
extern int gri_trans_solid_scale_umap_init(grs_tmap_loop_info *tli, grs_vertex **vert);

/* translucent bitmaps */
extern void gri_tluc8_opaque_lin_umap_init(grs_tmap_loop_info *tli);
extern void gri_tluc8_trans_lin_umap_init(grs_tmap_loop_info *tli);
extern void gri_tluc8_opaque_lit_lin_umap_init();
extern void gri_tluc8_trans_lit_lin_umap_init();
extern void gri_tluc8_opaque_clut_lin_umap_init(grs_tmap_loop_info *tli);
extern void gri_tluc8_trans_clut_lin_umap_init(grs_tmap_loop_info *tli);

extern void gri_tluc8_opaque_scale_umap_init(grs_tmap_loop_info *tli);
extern void gri_tluc8_trans_scale_umap_init(grs_tmap_loop_info *tli);
extern void gri_tluc8_opaque_clut_scale_umap_init(grs_tmap_loop_info *tli);
extern void gri_tluc8_trans_clut_scale_umap_init(grs_tmap_loop_info *tli);

