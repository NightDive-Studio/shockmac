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
 * $Source: r:/prj/lib/src/2d/RCS/grhbm.h $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/11/11 16:48:10 $
 *
 * Dispatch macros for horizontal bitmap flip routines.
 *
 * This file is part of the 2d library.
 *
 * $Log: grhbm.h $
 * Revision 1.3  1994/11/11  16:48:10  kevin
 * added hflip_in_place primitive.
 * 
 * Revision 1.2  1993/10/08  01:16:02  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.1  1993/06/06  15:13:10  kaboom
 * Initial revision
 * 
 */

#ifndef __GRHBM_H
#define __GRHBM_H
#include "grs.h"
#include "icanvas.h"

extern void gr_hflip_in_place(grs_bitmap *bm);

#define gr_hflip_ubitmap(bm,x,y) \
   ((void (*)(grs_bitmap *_bm,short _x,short _y)) \
   grd_canvas_table[HFLIP_DEVICE_UBITMAP+2*((bm)->type)])(bm,x,y)
#define gr_hflip_bitmap(bm,x,y) \
   ((void (*)(grs_bitmap *_bm,short _x,short _y)) \
   grd_canvas_table[HFLIP_DEVICE_BITMAP+2*((bm)->type)])(bm,x,y)

#define gr_hflip_flat8_ubitmap \
   ((void (*)(grs_bitmap *bm,short x,short y)) \
   grd_canvas_table[HFLIP_FLAT8_UBITMAP])
#define gr_hflip_flat8_bitmap \
   ((void (*)(grs_bitmap *bm,short x,short y)) \
   grd_canvas_table[HFLIP_FLAT8_BITMAP])
#endif /* !__GRHBM_H */
