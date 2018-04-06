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
 * $Source: n:/project/lib/src/2d/RCS/grply.h $
 * $Revision: 1.2 $
 * $Author: baf $
 * $Date: 1993/11/23 17:52:47 $
 *
 * Dispatch macros for solid polygon routines.
 *
 * This file is part of the 2d library.
 *
 * $Log: grply.h $
 * Revision 1.2  1993/11/23  17:52:47  baf
 * Added tluc/8 interpolation
 * 
 * Revision 1.1  1993/10/19  10:20:53  kaboom
 * Initial revision
 * 
 */

#ifndef __GRPLY_H
#define __GRPLY_H
#include "icanvas.h"
#include "plytyp.h"
#include "tabdat.h"

#define gr_upoly \
   ((void (*)(long c,int n,grs_vertex **vpl)) \
   grd_canvas_table[FIX_UPOLY])
#define gr_poly \
   ((int  (*)(long c,int n,grs_vertex **vpl)) \
   grd_canvas_table[FIX_POLY])
#define gr_tluc8_upoly \
   ((void (*)(long c,int n,grs_vertex **vpl)) \
   grd_canvas_table[FIX_TLUC8_UPOLY])
#define gr_tluc8_poly \
   ((int  (*)(long c,int n,grs_vertex **vpl)) \
   grd_canvas_table[FIX_TLUC8_POLY])
#endif /* !__GRPLY_H */
