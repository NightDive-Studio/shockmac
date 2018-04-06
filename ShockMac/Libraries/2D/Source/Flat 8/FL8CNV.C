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
 * $Source: r:/prj/lib/src/2d/RCS/fl8cnv.c $
 * $Revision: 1.78 $
 * $Author: kevin $
 * $Date: 1994/11/12 02:21:59 $
 * 
 * General-purpose routines for drawing into straight-8 bitmaps.
 * This file is part of the 2d library.
 *
 */

#include "grs.h"
#include "flat8.h"
#include "general.h"
#include "grnull.h"
#include "icanvas.h"

typedef void (*ptr_type)();

void (*flat8_canvas_table[GRD_CANVAS_FUNCS])() = {
   (ptr_type) flat8_set_upixel,          /* 8-bit pixel set/get */
   (ptr_type) gen_set_pixel,
   (ptr_type) flat8_get_upixel,
   (ptr_type) flat8_get_pixel,

   (ptr_type) flat8_set_upixel24,        /* 24-bit pixel set/get */
   (ptr_type) flat8_set_pixel24,
   (ptr_type) flat8_get_upixel24,
   (ptr_type) flat8_get_pixel24,

   (ptr_type) flat8_clear,               /* integral, straight primitives */
   (ptr_type) temp_upoint,
   (ptr_type) temp_point,
   (ptr_type) flat8_set_upixel,          /* 8-bit pixel set/get */
   (ptr_type) gen_set_pixel_interrupt,
   gr_null, 		
   gr_null, 		
   (ptr_type) gen_urect,
   (ptr_type) gen_rect,
   (ptr_type) gen_ubox,
   (ptr_type) gen_box,

   gr_null,                   /* fixed-point rendering primitives */
   gr_null,       
   gr_null, 		
   gr_null, 		
   gr_null, 		
   gr_null, 		
   (ptr_type) temp_upoly,
   (ptr_type) temp_poly,
   (ptr_type) temp_uspoly,
   (ptr_type) temp_spoly,
   (ptr_type) temp_ucpoly,
   (ptr_type) temp_cpoly,
   (ptr_type) temp_utpoly,
   (ptr_type) temp_tpoly,
   (ptr_type) temp_ustpoly,
   (ptr_type) temp_stpoly,

   (ptr_type) gen_vox_rect,
   (ptr_type) gen_vox_poly,
   (ptr_type) gen_vox_cpoly,
   (ptr_type) flat8_interp2_ubitmap,
   (ptr_type) flat8_filter2_ubitmap,
gr_not_imp, //   (ptr_type) gen_roll_ubitmap,  MLA - not used?
gr_not_imp, //   (ptr_type) gen_roll_bitmap,   MLA - not used?

   (ptr_type) temp_wall_umap,
   gr_null,
   (ptr_type) temp_lit_wall_umap,
   gr_null,
   (ptr_type) temp_clut_wall_umap,
   gr_null,

   (ptr_type) temp_floor_umap,
   gr_null,
   (ptr_type) temp_lit_floor_umap,
   gr_null,
   (ptr_type) temp_clut_floor_umap,
   gr_null,

   gr_null,                   /* linear texture mappers */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) temp_lin_umap,
   (ptr_type) temp_lin_map,
   (ptr_type) temp_lin_umap,
   (ptr_type) temp_lin_map,
   (ptr_type) temp_lin_umap,
   (ptr_type) temp_lin_map,
   (ptr_type) temp_lin_umap,
   (ptr_type) temp_lin_map,

   gr_null,                   /* lit linear texture mappers */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) temp_lit_lin_umap,
   (ptr_type) temp_lit_lin_map,
   gr_null,
   gr_null,
   (ptr_type) temp_lit_lin_umap,
   (ptr_type) temp_lit_lin_map,
   gr_null,
   gr_null,

   gr_null,                   /* clut linear texture mappers */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) temp_clut_lin_umap,
   (ptr_type) temp_clut_lin_map,
   gr_null,
   gr_null,
   (ptr_type) temp_clut_lin_umap,
   (ptr_type) temp_clut_lin_map,
   (ptr_type) temp_clut_lin_umap,
   (ptr_type) temp_clut_lin_map,

   gr_null,                   /* solid linear mapper */
   gr_null,

   gr_null,                   /* perspective texture mappers */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) temp_per_umap,
   (ptr_type) temp_per_map,
   gr_null,
   gr_null,
   (ptr_type) temp_per_umap,
   (ptr_type) temp_per_map,
   gr_null,
   gr_null,

   gr_null,                   /* lit perspective texture mappers */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) temp_lit_per_umap,
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) temp_lit_per_umap,
   gr_null,
   gr_null,
   gr_null,

   gr_null,                   /* clut perspective texture mappers */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) temp_clut_per_umap,
   (ptr_type) temp_clut_per_map,
   gr_null,
   gr_null,
   (ptr_type) temp_clut_per_umap,
   (ptr_type) temp_clut_per_map,
   gr_null,
   gr_null,

   gr_null,                   /* solid perspective mapper */
   gr_null,

   (ptr_type) gen_int_ucircle,           /* curves, should change to fixed-point */
   (ptr_type) gen_int_circle,
   (ptr_type) gen_fix_ucircle,
   (ptr_type) gen_fix_circle,
   (ptr_type) gen_int_udisk,
   (ptr_type) gen_int_disk,
   (ptr_type) gen_fix_udisk,
   (ptr_type) gen_fix_disk,
   (ptr_type) gen_int_urod,
   (ptr_type) gen_int_rod,
   (ptr_type) gen_fix_urod,
   (ptr_type) gen_fix_rod,

// MLA - added these two for the device functions
   (ptr_type) flat8_flat8_ubitmap,               /* bitmap drawing functions. */
   (ptr_type) gen_flat8_bitmap,
   (ptr_type) flat8_mono_ubitmap,
   (ptr_type) gen_mono_bitmap,
   (ptr_type) temp_flat8_ubitmap,
   (ptr_type) gen_flat8_bitmap,
   (ptr_type) gen_flat24_ubitmap,
   (ptr_type) gen_flat24_bitmap,
   (ptr_type) temp_rsd8_ubitmap,
   (ptr_type) temp_rsd8_bitmap,
   (ptr_type) temp_tluc8_ubitmap,
   (ptr_type) gen_tluc8_bitmap,

   gr_null,                   /* bitmap drawing functions through a clut. */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) temp_flat8_clut_ubitmap,
   (ptr_type) gen_flat8_clut_bitmap,
   gr_null,
   gr_null,
   (ptr_type) unpack_rsd8_clut_ubitmap,
   (ptr_type) unpack_rsd8_clut_bitmap,
   gr_null,
   gr_null,

   gr_null,                   /* rsd8 solid bitmap functions.  No longer used. */
   gr_null,

   gr_null,                   /* bitmap scale functions. */
   gr_null,
   (ptr_type) flat8_mono_scale_ubitmap,
   (ptr_type) flat8_mono_scale_bitmap,
   (ptr_type) temp_scale_umap,
   (ptr_type) temp_scale_map,
   gr_null,
   gr_null,
   (ptr_type) temp_scale_umap,
   (ptr_type) temp_scale_map,
   (ptr_type) temp_scale_umap,
   (ptr_type) temp_scale_map,

   gr_null,                   /* rsd8 solid scale functions.  No longer used. */
   gr_null,

   gr_null,                   /* clut scale functions. */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) temp_clut_scale_umap,
   (ptr_type) temp_clut_scale_map,
   gr_null,
   gr_null,
   (ptr_type) temp_clut_scale_umap,
   (ptr_type) temp_clut_scale_map,
   (ptr_type) temp_clut_scale_umap,
   (ptr_type) temp_clut_scale_map,

   gr_null,                   /* bitmap mask draw functions. */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) flat8_flat8_ubitmap,
   (ptr_type) temp_flat8_mask_bitmap,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,                   /* bitmap get functions. */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) flat8_get_flat8_ubitmap,
   (ptr_type) gen_get_flat8_bitmap,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,                   /* bitmap horizontal flip functions */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) flat8_hflip_flat8_ubitmap,
   (ptr_type) gen_hflip_flat8_bitmap,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,                   /* bitmap clut horizontal flip functions */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) flat8_clut_hflip_flat8_ubitmap,
   (ptr_type) gen_clut_hflip_flat8_bitmap,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,                   /* bitmap horizontal doubling. */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) flat8_flat8_h_double_ubitmap,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,                   /* bitmap vertical doubling. */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) flat8_flat8_v_double_ubitmap,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,                   /* bitmap horizontal and vertical doubling. */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,                   /* bitmap smooth horizontal doubling. */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) flat8_flat8_smooth_h_double_ubitmap,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,                   /* bitmap smooth vertical doubling. */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,                   /* bitmap smooth horizontal and vertical doubling. */
   gr_null,
   gr_null,
   gr_null,
   (ptr_type) flat8_flat8_smooth_hv_double_ubitmap,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   (ptr_type) gen_font_ustring,               /* text/font functions. */
   (ptr_type) gen_font_string,
   (ptr_type) gen_font_scale_ustring,	
   (ptr_type) gen_font_scale_string,
   (ptr_type) gen_font_uchar,
   (ptr_type) gen_font_char,

   (ptr_type) flat8_calc_row,            /* utility functions. */
   (ptr_type) flat8_sub_bitmap,

   gr_null,                   /* placeholders for primitiveless chains */
   gr_null,
};
