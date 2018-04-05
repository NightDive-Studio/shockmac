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
 * $Source: r:/prj/lib/src/2d/RCS/gencnv.c $
 * $Revision: 1.23 $
 * $Author: kevin $
 * $Date: 1994/10/25 15:13:54 $
 * 
 */

// MLA, NOTE- the general canvas table functions are left as unimplemented ptrs since the
// general canvas code isn't really used directly(at least thats what they tell me).

#include "grs.h"
#include "grnull.h"
#include "general.h"
#include "icanvas.h"

typedef void (*ptr_type)();

void (*gen_canvas_table[GRD_CANVAS_FUNCS])() = {
   gr_null,                   /*  NO generic pixel routines! */
   gr_null,
   gr_null,
   gr_null,

   gr_null,
   gr_null,
   gr_null,
   gr_null,

 gr_not_imp, //   (ptr_type) gen_clear,               /* integral, straight primitives */
 gr_not_imp, //   (ptr_type) gen_upoint,
 gr_not_imp, //   (ptr_type) gen_point,
   gr_null, 		
   gr_null, 		
   gr_null, 		
   gr_null, 		
   (ptr_type) gen_urect,
   (ptr_type) gen_rect,
   (ptr_type) gen_ubox,
   (ptr_type) gen_box,

   gr_null, 	            /* fixed-point rendering primitives */
   gr_null,		
   gr_null, 	
   gr_null, 	
   gr_null, 	
   gr_null, 	
gr_not_imp, //   (ptr_type) temp_upoly,
gr_not_imp, //   (ptr_type) temp_poly,
gr_not_imp, //   (ptr_type) temp_uspoly,
gr_not_imp, //   (ptr_type) temp_spoly,
gr_not_imp, //   (ptr_type) temp_ucpoly,
gr_not_imp, //   (ptr_type) temp_cpoly,
gr_not_imp, //   (ptr_type) temp_utpoly,
gr_not_imp, //   (ptr_type) temp_tpoly,
gr_not_imp, //   (ptr_type) temp_ustpoly,
gr_not_imp, //   (ptr_type) temp_stpoly,

gr_not_imp, //   (ptr_type) gen_vox_rect,
gr_not_imp, //   (ptr_type) gen_vox_poly,
gr_not_imp, //   (ptr_type) gen_vox_cpoly,
gr_not_imp, //   (ptr_type) gen_interp2_ubitmap,
gr_not_imp, //   (ptr_type) gen_filter2_ubitmap,
gr_not_imp, //   (ptr_type) gen_roll_ubitmap,
gr_not_imp, //   (ptr_type) gen_roll_bitmap,

gr_not_imp, //   (ptr_type) temp_wall_umap,
   gr_null,
gr_not_imp, //   (ptr_type) temp_lit_wall_umap,
   gr_null,
gr_not_imp, //   (ptr_type) temp_clut_wall_umap,
   gr_null,

gr_not_imp, //   (ptr_type) temp_floor_umap,
   gr_null,
gr_not_imp, //   (ptr_type) temp_lit_floor_umap,
   gr_null,
gr_not_imp, //   (ptr_type) temp_clut_floor_umap,
   gr_null,
   
   gr_null,                   /* linear texture mappers */
   gr_null,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_lin_umap,
gr_not_imp, //   (ptr_type) temp_lin_map,
gr_not_imp, //   (ptr_type) temp_lin_umap,
gr_not_imp, //   (ptr_type) temp_lin_map,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_lin_umap,
gr_not_imp, //   (ptr_type) temp_lin_map,

   gr_null,                   /* lit linear texture mappers */
   gr_null,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_lit_lin_umap,
gr_not_imp, //   (ptr_type) temp_lit_lin_map,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_lit_lin_umap,
gr_not_imp, //   (ptr_type) temp_lit_lin_map,
   gr_null,
   gr_null,

   gr_null,                   /* clut linear texture mappers */
   gr_null,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_clut_lin_umap,
gr_not_imp, //   (ptr_type) temp_clut_lin_map,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_clut_lin_umap,
gr_not_imp, //   (ptr_type) temp_clut_lin_map,
gr_not_imp, //   (ptr_type) temp_clut_lin_umap,
gr_not_imp, //   (ptr_type) temp_clut_lin_umap,

   gr_null,                   /* solid linear mapper */
   gr_null,

   gr_null,                   /* perspective texture mappers */
   gr_null,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_per_umap,
   gr_null,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_per_umap,
   gr_null,
   gr_null,
   gr_null,

   gr_null,                   /* lit perspective texture mappers */
   gr_null,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_lit_per_umap,
   gr_null,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_lit_per_umap,
   gr_null,
   gr_null,
   gr_null,

   gr_null,                   /* clut perspective texture mappers */
   gr_null,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_clut_per_umap,
   gr_null,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_clut_per_umap,
   gr_null,
   gr_null,
   gr_null,

   gr_null,                   /* solid perspective mapper */
   gr_null,
   
gr_not_imp, //   (ptr_type) gen_int_ucircle,           /* curves, should change to fixed-point */
gr_not_imp, //   (ptr_type) gen_int_circle,
gr_not_imp, //   (ptr_type) gen_fix_ucircle,
gr_not_imp, //   (ptr_type) gen_fix_circle,
gr_not_imp, //   (ptr_type) gen_int_udisk,
gr_not_imp, //   (ptr_type) gen_int_disk,
gr_not_imp, //   (ptr_type) gen_fix_udisk,
gr_not_imp, //   (ptr_type) gen_fix_disk,
gr_not_imp, //   (ptr_type) gen_int_urod,
gr_not_imp, //   (ptr_type) gen_int_rod,
gr_not_imp, //   (ptr_type) gen_fix_urod,
gr_not_imp, //   (ptr_type) gen_fix_rod,
   
   gr_null,                   /* bitmap drawing functions. */
   gr_null,
gr_not_imp, //   (ptr_type) gen_mono_ubitmap,
gr_not_imp, //   (ptr_type) gen_mono_bitmap,
gr_not_imp, //   (ptr_type) gen_flat8_ubitmap,
gr_not_imp, //   (ptr_type) gen_flat8_bitmap,
gr_not_imp, //   (ptr_type) gen_flat24_ubitmap,
gr_not_imp, //   (ptr_type) gen_flat24_bitmap,
gr_not_imp, //   (ptr_type) gri_gen_rsd8_ubitmap,
gr_not_imp, //   (ptr_type) gri_gen_rsd8_bitmap,
gr_not_imp, //   (ptr_type) gen_tluc8_ubitmap,
gr_not_imp, //   (ptr_type) gen_tluc8_bitmap,

   gr_null,                   /* bitmap drawing functions through a clut. */
   gr_null,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) gen_flat8_clut_ubitmap,
gr_not_imp, //   (ptr_type) gen_flat8_clut_bitmap,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) unpack_rsd8_clut_ubitmap,
gr_not_imp, //   (ptr_type) unpack_rsd8_clut_bitmap,
   gr_null,
   gr_null,

gr_not_imp, //   (ptr_type) gen_rsd8_solid_ubitmap,
gr_not_imp, //   (ptr_type) gen_rsd8_solid_bitmap,

   gr_null,                   /* scaled bitmap drawing functions. */
   gr_null,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_scale_umap,
gr_not_imp, //   (ptr_type) temp_scale_map,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_scale_umap,
gr_not_imp, //   (ptr_type) temp_scale_map,
gr_not_imp, //   (ptr_type) temp_scale_umap,
gr_not_imp, //   (ptr_type) temp_scale_map,

gr_not_imp, //   (ptr_type) gen_rsd8_scale_solid_ubitmap,
gr_not_imp, //   (ptr_type) gen_rsd8_scale_solid_bitmap,

   gr_null,                   /* clut scale functions. */
   gr_null,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_clut_scale_umap,
gr_not_imp, //   (ptr_type) temp_clut_scale_map,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) temp_clut_scale_umap,
gr_not_imp, //   (ptr_type) temp_clut_scale_map,
gr_not_imp, //   (ptr_type) temp_clut_scale_umap,
gr_not_imp, //   (ptr_type) temp_clut_scale_map,

   gr_null,                   /* bitmap mask draw functions. */
   gr_null,
   gr_null,
   gr_null,
   gr_null,//span_mask_flat8_ubitmap,
   gr_null,//span_mask_flat8_bitmap,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,                      /* bitmap get functions. */
   gr_null,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) gen_get_flat8_ubitmap,
gr_not_imp, //   (ptr_type) gen_get_flat8_bitmap,
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
gr_not_imp, //   (ptr_type) gen_hflip_flat8_ubitmap,
gr_not_imp, //   (ptr_type) gen_hflip_flat8_bitmap,
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
gr_not_imp, //   (ptr_type) gen_clut_hflip_flat8_ubitmap,
gr_not_imp, //   (ptr_type) gen_clut_hflip_flat8_bitmap,
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
   gr_null,
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
   gr_null,
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
   gr_null,
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
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   (ptr_type) gen_font_ustring,               /* text/font functions. */
gr_not_imp, //   (ptr_type) gen_font_string,
   gr_null,
   gr_null,
gr_not_imp, //   (ptr_type) gen_font_uchar,
gr_not_imp, //   (ptr_type) gen_font_char,

   gr_null,               /* bitmap type specific functions */
   gr_null,

   gr_null,                   /* placeholders for primitiveless chains */
   gr_null,
};
