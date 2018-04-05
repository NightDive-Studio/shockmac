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
 * $Source: r:/prj/lib/src/2d/RCS/fl8sft.h $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/10/25 15:11:30 $
 *
 * Solid fill mode function table.
 * flat8 canvas.
 *
 * This file is part of the 2d library.
 */
/* FILL_SOLID */
/* BMT_DEVICE */
   (ptr_type) flat8_solid_set_upixel,    /* pixel primitve */
   gr_null,    /* reserved wire poly line */
   gr_null,    /* solid hline */

   gr_null,    /* bitmap blitter */
   gr_null,    /* stencil clipped bitmap blitter */
   gr_null,    /* clut bitmap blitter */

   gr_null,    /* hflipped bitmap blitter */
   gr_null,    /* clut hflipped bitmap blitter */

   gr_null,    /* masked bitmap blitter */

   gr_null,    /* horizontal bitmap doubler */
   gr_null,    /* vertical bitmap doubler */
   gr_null,    /* horizontal and vertical bitmap doubler */

   gr_null,    /* blended horizontal bitmap doubler */
   gr_null,    /* blended vertical bitmap doubler */
   gr_null,    /* blended horizontal and vertical bitmap doubler */

   gr_null,    /* Scalers */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   (ptr_type) gri_solid_poly_init,   /* solid polygon primitives */
   gr_null,

   gr_null,   /* Linear mappers */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Bilinear mappers */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Floor */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Wall2d */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Wall1d */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Perspective -hscan */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Perspective -vscan */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

/* BMT_MONO */
   gr_null,    /* solid line */
   gr_null,    /* solid wire poly line */
   gr_null,    /* gouraud hline */

   gr_null,    /* bitmap blitter */
   gr_null,    /* stencil clipped bitmap blitter */
   gr_null,    /* clut bitmap blitter */

   gr_null,    /* hflipped bitmap blitter */
   gr_null,    /* clut hflipped bitmap blitter */

   gr_null,    /* masked bitmap blitter */

   gr_null,    /* horizontal bitmap doubler */
   gr_null,    /* vertical bitmap doubler */
   gr_null,    /* horizontal and vertical bitmap doubler */

   gr_null,    /* blended horizontal bitmap doubler */
   gr_null,    /* blended vertical bitmap doubler */
   gr_null,    /* blended horizontal and vertical bitmap doubler */

   gr_null,   /* Scalers */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   (ptr_type) gri_solid_poly_init,   /* gouraud polygon primitives */
   gr_null,

   gr_null,   /* Linear mappers */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Linear */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Floor */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Wall2d */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Wall1d */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Perspective -hscan */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Perspective -vscan */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

/* BMT_FLAT8 */
   gr_null,    /* gouraud line */
   gr_null,    /* gouraud wire poly line */
   gr_null,    /* rgb hline */

   gr_null,    /* bitmap blitter */
   gr_null,    /* stencil clipped bitmap blitter */
   gr_null,    /* clut bitmap blitter */

   gr_null,    /* hflipped bitmap blitter */
   gr_null,    /* clut hflipped bitmap blitter */

   gr_null,    /* masked bitmap blitter */

   gr_null,    /* horizontal bitmap doubler */
   gr_null,    /* vertical bitmap doubler */
   gr_null,    /* horizontal and vertical bitmap doubler */

   gr_null,    /* blended horizontal bitmap doubler */
   gr_null,    /* blended vertical bitmap doubler */
   gr_null,    /* blended horizontal and vertical bitmap doubler */

   (ptr_type) gri_opaque_solid_scale_umap_init,   /* Scalers */
   (ptr_type) gri_trans_solid_scale_umap_init,
   (ptr_type) gri_opaque_solid_scale_umap_init,
   (ptr_type) gri_trans_solid_scale_umap_init,
   (ptr_type) gri_opaque_solid_scale_umap_init,
   (ptr_type) gri_trans_solid_scale_umap_init,

   (ptr_type) gri_solid_poly_init,   /* RGB polygon primitives */
   gr_null,

   (ptr_type) gri_solid_poly_init,   /* Linear mappers */
   gr_null,
   (ptr_type) gri_solid_poly_init,
   gr_null,
   (ptr_type) gri_solid_poly_init,
   gr_null,

   (ptr_type) gri_solid_poly_init,   /* Bilinear */
   (ptr_type) gri_trans_solid_lin_umap_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) gri_trans_solid_lin_umap_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) gri_trans_solid_lin_umap_init,

   (ptr_type) gri_solid_poly_init,   /* Floor */
   (ptr_type) gri_trans_solid_floor_umap_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) gri_trans_solid_floor_umap_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) gri_trans_solid_floor_umap_init,

   (ptr_type) gri_solid_poly_init,   /* Wall2d */
   (ptr_type) gri_trans_solid_wall_umap_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) gri_trans_solid_wall_umap_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) gri_trans_solid_wall_umap_init,

   (ptr_type) gri_solid_poly_init,   /* Wall1d */
   (ptr_type) gri_trans_solid_wall_umap_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) gri_trans_solid_wall_umap_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) gri_trans_solid_wall_umap_init,

   (ptr_type) gri_solid_poly_init,   /* Perspective -hscan */
   (ptr_type) gri_trans_solid_per_umap_hscan_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) gri_trans_solid_per_umap_hscan_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) gri_trans_solid_per_umap_hscan_init,

   (ptr_type) gri_solid_poly_init,   /* Perspective -vscan */
   (ptr_type) gri_trans_solid_per_umap_vscan_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) gri_trans_solid_per_umap_vscan_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) gri_trans_solid_per_umap_vscan_init,

/* BMT_FLAT24 */
   gr_null,    /* rgb line */
   gr_null,    /* rgb wire poly line */
   gr_null,    /* solid vline */

   gr_null,    /* bitmap blitter */
   gr_null,    /* stencil clipped bitmap blitter */
   gr_null,    /* clut bitmap blitter */

   gr_null,    /* hflipped bitmap blitter */
   gr_null,    /* clut hflipped bitmap blitter */

   gr_null,    /* masked bitmap blitter */

   gr_null,    /* horizontal bitmap doubler */
   gr_null,    /* vertical bitmap doubler */
   gr_null,    /* horizontal and vertical bitmap doubler */

   gr_null,    /* blended horizontal bitmap doubler */
   gr_null,    /* blended vertical bitmap doubler */
   gr_null,    /* blended horizontal and vertical bitmap doubler */

   gr_null,   /* Scalers */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   (ptr_type) gri_solid_poly_init,   /* translucent polygon primitives */
   gr_null,

   gr_null,   /* Linear mappers */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Linear */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Floor */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Wall2d */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Wall1d */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Perspective -hscan */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Perspective -vscan */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

/* BMT_RSD8 */
   gr_null,    /* translucent line */
   gr_null,    /* translucent wire poly line */
   gr_null,    /* gouraud vline */

   (ptr_type) gri_gen_rsd8_ubitmap,    /* bitmap blitter */
   (ptr_type) gri_gen_rsd8_bitmap,    /* stencil clipped bitmap blitter */
   gr_null,    /* clut bitmap blitter */

   gr_null,    /* hflipped bitmap blitter */
   gr_null,    /* clut hflipped bitmap blitter */

   gr_null,    /* masked bitmap blitter */

   gr_null,    /* horizontal bitmap doubler */
   gr_null,    /* vertical bitmap doubler */
   gr_null,    /* horizontal and vertical bitmap doubler */

   gr_null,    /* blended horizontal bitmap doubler */
   gr_null,    /* blended vertical bitmap doubler */
   gr_null,    /* blended horizontal and vertical bitmap doubler */

   (ptr_type) gri_opaque_solid_scale_umap_init,
   (ptr_type) rsd8_tm_init,   
   (ptr_type) gri_opaque_solid_scale_umap_init,
   (ptr_type) rsd8_tm_init,
   (ptr_type) gri_opaque_solid_scale_umap_init,
   (ptr_type) rsd8_tm_init,

   (ptr_type) gri_solid_poly_init,  /* shaded translucent polygon primitives */
   gr_null,

   (ptr_type) gri_solid_poly_init,  /* Linear mappers */
   (ptr_type) rsd8_tm_init, 
   (ptr_type) gri_solid_poly_init,
   (ptr_type) rsd8_tm_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) rsd8_tm_init,

   (ptr_type) gri_solid_poly_init,  /* Linear */
   (ptr_type) rsd8_tm_init,   
   (ptr_type) gri_solid_poly_init,
   (ptr_type) rsd8_tm_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) rsd8_tm_init,

   (ptr_type) gri_solid_poly_init,  /* Floor */
   (ptr_type) rsd8_tm_init,   
   (ptr_type) gri_solid_poly_init,
   (ptr_type) rsd8_tm_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) rsd8_tm_init,

   (ptr_type) gri_solid_poly_init,  /* Wall2d */
   (ptr_type) rsd8_tm_init,   
   (ptr_type) gri_solid_poly_init,
   (ptr_type) rsd8_tm_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) rsd8_tm_init,

   (ptr_type) gri_solid_poly_init,  /* Wall1d */
   (ptr_type) rsd8_tm_init,   
   (ptr_type) gri_solid_poly_init,
   (ptr_type) rsd8_tm_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) rsd8_tm_init,

   (ptr_type) gri_solid_poly_init,  /* Perspective -hscan */
   (ptr_type) rsd8_pm_init,   
   (ptr_type) gri_solid_poly_init,
   (ptr_type) rsd8_pm_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) rsd8_pm_init,

   (ptr_type) gri_solid_poly_init,  /* Perspective -vscan */
   (ptr_type) rsd8_pm_init,   
   (ptr_type) gri_solid_poly_init,
   (ptr_type) rsd8_pm_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) rsd8_pm_init,

/* BMT_TLUC8 */
   gr_null,    /* shaded translucent line */
   gr_null,    /* shaded translucent wire poly line */
   gr_null,    /* rgb vline */

   gr_null,    /* bitmap blitter */
   gr_null,    /* stencil clipped bitmap blitter */
   gr_null,    /* clut bitmap blitter */

   gr_null,    /* hflipped bitmap blitter */
   gr_null,    /* clut hflipped bitmap blitter */

   gr_null,    /* masked bitmap blitter */

   gr_null,    /* horizontal bitmap doubler */
   gr_null,    /* vertical bitmap doubler */
   gr_null,    /* horizontal and vertical bitmap doubler */

   gr_null,    /* blended horizontal bitmap doubler */
   gr_null,    /* blended vertical bitmap doubler */
   gr_null,    /* blended horizontal and vertical bitmap doubler */

   (ptr_type) gri_opaque_solid_scale_umap_init,   /* Scalers */
   (ptr_type) gri_trans_solid_scale_umap_init,
   (ptr_type) gri_opaque_solid_scale_umap_init,
   (ptr_type) gri_trans_solid_scale_umap_init,
   (ptr_type) gri_opaque_solid_scale_umap_init,
   (ptr_type) gri_trans_solid_scale_umap_init,

   gr_null,
   gr_null,

   gr_null,   /* Linear mappers */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   (ptr_type) gri_solid_poly_init,   /* Linear */
   (ptr_type) gri_trans_solid_lin_umap_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) gri_trans_solid_lin_umap_init,
   (ptr_type) gri_solid_poly_init,
   (ptr_type) gri_trans_solid_lin_umap_init,

   gr_null,   /* Floor */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Wall2d */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Wall1d */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Perspective -hscan */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null,

   gr_null,   /* Perspective -vscan */
   gr_null,
   gr_null,
   gr_null,
   gr_null,
   gr_null

