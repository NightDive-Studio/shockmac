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
 * $Source: n:/project/lib/src/2d/RCS/grbm.h $
 * $Revision: 1.4 $
 * $Author: kaboom $
 * $Date: 1993/10/19 10:15:22 $
 *
 * Dispatch macros for utility functions.
 *
 * This file is part of the 2d library.
 *
 * $Log: grbm.h $
 * Revision 1.4  1993/10/19  10:15:22  kaboom
 * Now includes tabdat.h.
 * 
 * Revision 1.3  1993/10/08  01:15:55  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.2  1993/06/03  15:10:15  kaboom
 * Now uses the grd_pixel_table to call bitmap utility functions.
 * 
 * Revision 1.1  1993/04/29  18:35:01  kaboom
 * Initial revision
 */

#ifndef __GRBM_H
#define __GRBM_H
#include "icanvas.h"
#include "tabdat.h"

#define gr_calc_row(w) \
   ((short (*)(short _w)) grd_pixel_table[CALC_ROW])(w)
#define gr_sub_bitmap(bm, x, y, w, h) \
   ((grs_bitmap *(*)(grs_bitmap *_bm,short _x,short _y,short _w,short _h)) \
   grd_pixel_table[SUB_BITMAP])(bm, x, y, w, h)
#endif /* !__GRBM_H */
