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
 * $Source: n:/project/lib/src/2d/RCS/grcbm.h $
 * $Revision: 1.1 $
 * $Author: kevin $
 * $Date: 1994/03/15 13:09:57 $
 *
 * Dispatch macros for bitmap draw functions.
 *
 * This file is part of the 2d library.
 *
 * $Log: grcbm.h $
 * Revision 1.1  1994/03/15  13:09:57  kevin
 * Initial revision
 * 
 * Revision 1.5  1993/12/01  21:27:36  baf
 * Added macros for translucent/8 bitmaps.
 * 
 * Revision 1.4  1993/10/19  10:15:22  kaboom
 * Now includes tabdat.h.
 * 
 * Revision 1.3  1993/10/08  01:15:59  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.2  1993/09/02  20:10:45  kaboom
 * Added macros for flat 24 bitmap routines.
 * 
 * Revision 1.1  1993/04/29  18:35:50  kaboom
 * Initial revision
 */

#ifndef __GRCBM_H
#define __GRCBM_H
#include "icanvas.h"
#include "tabdat.h"

/* bitmap draw routines. */
#define gr_clut_ubitmap(bm,x,y,cl) \
   ((void (*)(grs_bitmap *_bm,short _x,short _y, uchar *_cl)) \
   grd_canvas_table[CLUT_DRAW_DEVICE_UBITMAP+2*((bm)->type)])(bm,x,y,cl)
#define gr_clut_bitmap(bm,x,y,cl) \
   ((int (*)(grs_bitmap *_bm,short _x,short _y, uchar *_cl)) \
   grd_canvas_table[CLUT_DRAW_DEVICE_BITMAP+2*((bm)->type)])(bm,x,y,cl)

#define gr_mono_clut_ubitmap \
   ((void (*)(grs_bitmap *bm,short x,short y,uchar *cl)) \
   grd_canvas_table[CLUT_DRAW_MONO_UBITMAP])
#define gr_mono_clut_bitmap \
   ((int (*)(grs_bitmap *bm,short x,short y,uchar *cl)) \
   grd_canvas_table[CLUT_DRAW_MONO_BITMAP])
#define gr_flat8_clut_ubitmap \
   ((void (*)(grs_bitmap *bm,short x,short y,uchar *cl)) \
   grd_canvas_table[CLUT_DRAW_FLAT8_UBITMAP])
#define gr_flat8_clut_bitmap \
   ((int (*)(grs_bitmap *bm,short x,short y,uchar *cl)) \
   grd_canvas_table[CLUT_DRAW_FLAT8_BITMAP])
#define gr_flat24_clut_ubitmap \
   ((void (*)(grs_bitmap *bm,short x,short y,uchar *cl)) \
   grd_canvas_table[CLUT_DRAW_FLAT24_UBITMAP])
#define gr_flat24_clut_bitmap \
   ((int (*)(grs_bitmap *bm,short x,short y,uchar *cl)) \
   grd_canvas_table[CLUT_DRAW_FLAT24_BITMAP])
#define gr_rsd8_clut_ubitmap \
   ((void (*)(grs_bitmap *bm,short x,short y,uchar *cl)) \
   grd_canvas_table[CLUT_DRAW_RSD8_UBITMAP])
#define gr_rsd8_clut_bitmap \
   ((int (*)(grs_bitmap *bm,short x,short y,uchar *cl)) \
   grd_canvas_table[CLUT_DRAW_RSD8_BITMAP])
#define gr_tluc8_clut_ubitmap \
   ((void (*)(grs_bitmap *bm,short x,short y,uchar *cl)) \
   grd_canvas_table[CLUT_DRAW_TLUC8_UBITMAP])
#define gr_tluc8_clut_bitmap \
   ((int (*)(grs_bitmap *bm,short x,short y,uchar *cl)) \
   grd_canvas_table[CLUT_DRAW_TLUC8_BITMAP])
#endif /* !__GRDBM_H */
