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
#include "bitmap.h"
#include "cnvdat.h"
#include "tabdat.h"
#include "ifcn.h"
#include "grs.h"
#include "general.h"


void temp_flat8_ubitmap (grs_bitmap *bm, int x, int y)
{
   ((void (*)(grs_bitmap *_bm,int _x, int _y))
      grd_function_table[GRC_BITMAP+BMT_FLAT8*GRD_FUNCS])(bm, x, y);
}

void temp_flat8_bitmap (grs_bitmap *bm, int x, int y)
{
   ((void (*)(grs_bitmap *_bm,int _x, int _y, grs_stencil *_sten))
      grd_function_table[GRC_STENCIL_BITMAP+BMT_FLAT8*GRD_FUNCS])(bm, x, y, grd_clip.sten);
}


