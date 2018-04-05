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
 * $Source: r:/prj/lib/src/2d/RCS/gente.h $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/08/16 15:34:31 $
 *
 * Texture mapping edge calculation functions.
 * Generic canvas.
 *
 */

/* polygon edges */
extern void gri_x_edge(grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side);
extern void gri_ix_edge(grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side);
extern void gri_rgbx_edge(grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side);

/* scaler edges */
extern void gri_scale_edge(grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side);

/* texture mapping edges */
extern void gri_uvx_edge(grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side);
extern void gri_uvix_edge(grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side);
extern void gri_uvwx_edge(grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side);
extern void gri_uviwx_edge(grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side);
extern void gri_uvwy_edge(grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side);
extern void gri_uviwy_edge(grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side);


