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
 *  $Source: r:/prj/lib/src/2d/RCS/grlin.h $
 *  $Revision: 1.2 $
 *  $Author: kevin $
 *  $Date: 1994/08/04 09:46:07 $
 */

#ifndef __GRLIN_H
#define __GRLIN_H

#include "plytyp.h"
#include "line.h"
#include "lintyp.h"
#include "clpltyp.h"

#define grd_uline_fill  ((grt_uline_fill_v)  (grd_uline_fill_vector[GR_LINE])) 
#define grd_uiline_fill ((grt_uline_fill_v)  (grd_uline_fill_vector[GR_ILINE]))
#define grd_uhline_fill ((grt_uline_fill_xy) (grd_uline_fill_vector[GR_HLINE]))
#define grd_uvline_fill ((grt_uline_fill_xy) (grd_uline_fill_vector[GR_VLINE]))
#define grd_usline_fill ((grt_uline_fill_v) (grd_uline_fill_vector[GR_SLINE])) 
#define grd_ucline_fill ((grt_uline_fill_v) (grd_uline_fill_vector[GR_CLINE])) 
#define grd_wire_poly_uline_fill ((grt_wire_poly_uline) (grd_uline_fill_vector[GR_WIRE_POLY_LINE])) 
#define grd_wire_poly_usline_fill ((grt_wire_poly_usline) (grd_uline_fill_vector[GR_WIRE_POLY_SLINE])) 
#define grd_wire_poly_ucline_fill ((grt_wire_poly_ucline) (grd_uline_fill_vector[GR_WIRE_POLY_CLINE])) 

/* these should become table definitions */

#define grd_line_clip_fill   ((grt_line_clip_fill_v)  (grd_line_clip_fill_vector[GR_LINE])) 
#define grd_iline_clip_fill  ((grt_line_clip_fill_v)  (grd_line_clip_fill_vector[GR_ILINE]))
#define grd_hline_clip_fill  ((grt_line_clip_fill_xy) (grd_line_clip_fill_vector[GR_HLINE]))
#define grd_vline_clip_fill  ((grt_line_clip_fill_xy) (grd_line_clip_fill_vector[GR_VLINE]))
#define grd_sline_clip_fill  ((grt_line_clip_fill_v) (grd_line_clip_fill_vector[GR_SLINE])) 
#define grd_cline_clip_fill  ((grt_line_clip_fill_v) (grd_line_clip_fill_vector[GR_CLINE])) 
#define grd_wire_poly_line_clip_fill ((grt_wire_poly_uline) (grd_line_clip_fill_vector[GR_WIRE_POLY_LINE])) 
#define grd_wire_poly_sline_clip_fill ((grt_wire_poly_usline) (grd_line_clip_fill_vector[GR_WIRE_POLY_SLINE])) 
#define grd_wire_poly_cline_clip_fill ((grt_wire_poly_ucline) (grd_line_clip_fill_vector[GR_WIRE_POLY_CLINE])) 

#endif
