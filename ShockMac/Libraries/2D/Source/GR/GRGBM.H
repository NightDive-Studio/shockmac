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
 * $Source: n:/project/lib/src/2d/RCS/grgbm.h $
 * $Revision: 1.2 $
 * $Author: kaboom $
 * $Date: 1993/10/08 01:16:01 $
 *
 * Dispatch macros for get bitmap functions.
 *
 * This file is part of the 2d library.
 *
 * $Log: grgbm.h $
 * Revision 1.2  1993/10/08  01:16:01  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.1  1993/04/29  18:36:18  kaboom
 * Initial revision
 * 
 */

#ifndef __GRGBM_H
#define __GRGBM_H
#include "icanvas.h"

/* bitmap get routines. */
#define gr_get_ubitmap(bm,x,y) \
   ((void (*)(grs_bitmap *_bm,short _x,short _y)) \
   grd_canvas_table[GET_DEVICE_UBITMAP+2*((bm)->type)])(bm,x,y)
#define gr_get_bitmap(bm,x,y) \
   ((int (*)(grs_bitmap *_bm,short _x,short _y)) \
   grd_canvas_table[GET_DEVICE_BITMAP+2*((bm)->type)])(bm,x,y)

#define gr_get_mono_ubitmap \
   ((void (*)(grs_bitmap *bm,short x,short y)) \
   grd_canvas_table[GET_MONO_UBITMAP])
#define gr_get_mono_bitmap \
   ((int (*)(grs_bitmap *bm,short x,short y)) \
   grd_canvas_table[GET_MONO_BITMAP])
#define gr_get_flat8_ubitmap \
   ((void (*)(grs_bitmap *bm,short x,short y)) \
   grd_canvas_table[GET_FLAT8_UBITMAP])
#define gr_get_flat8_bitmap \
   ((int (*)(grs_bitmap *bm,short x,short y)) \
   grd_canvas_table[GET_FLAT8_BITMAP])
#define gr_get_rsd8_ubitmap \
   ((void (*)(grs_bitmap *bm,short x,short y)) \
   grd_canvas_table[GET_RSD8_UBITMAP]
#define gr_get_rsd8_bitmap \
   ((int (*)(grs_bitmap *bm,short x,short y)) \
   grd_canvas_table[GET_RSD8_BITMAP])
#endif /* !__GRGBM_H */
