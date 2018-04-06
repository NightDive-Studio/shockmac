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
 *  $Source: r:/prj/lib/src/2d/RCS/clpltab.c $
 *  $Revision: 1.2 $
 *  $Author: kevin $
 *  $Date: 1994/08/04 09:54:59 $
 */

#include "clpltyp.h"
#include "clpltab.h"
#include "grnull.h"
#include "line.h"
#include "plytyp.h"

grt_line_clip_fill grd_line_clip_fill_table [GRD_LINE_TYPES] = 
{
   gri_line_clip_fill,
   gri_iline_clip_fill,
   gri_hline_clip_fill,
   gri_vline_clip_fill,
   gri_sline_clip_fill,
   gri_cline_clip_fill,
   gri_wire_poly_line_clip_fill,
   gr_null,
   gri_wire_poly_cline_clip_fill
};

grt_line_clip_fill * grd_line_clip_fill_vector = grd_line_clip_fill_table;
