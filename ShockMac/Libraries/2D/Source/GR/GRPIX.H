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
 * $Source: r:/prj/lib/src/2d/RCS/grpix.h $
 * $Revision: 1.7 $
 * $Author: kevin $
 * $Date: 1994/11/12 02:22:57 $
 *
 * Dispatch macros for pixel functions.
 *
 * This file is part of the 2d library.
 *
 * $Log: grpix.h $
 * Revision 1.7  1994/11/12  02:22:57  kevin
 * added gr_set_pixel_interrupt() #define.
 * 
 * Revision 1.6  1994/08/16  15:34:57  kevin
 * Added gr_fill_upixel declaration.
 * 
 * Revision 1.5  1993/10/19  10:18:47  kaboom
 * Now includes tabdat.h.
 * 
 * Revision 1.4  1993/10/08  01:16:06  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.3  1993/09/02  20:11:19  kaboom
 * Updated index names to XXX_PIXEL8.
 * 
 * Revision 1.2  1993/05/03  16:50:09  kaboom
 * Changed pixel macros to use grd_pixel_table instead of grd_canvas_table.
 * 
 * Revision 1.1  1993/04/29  18:36:36  kaboom
 * Initial revision
 */

#ifndef __GRPIX_H
#define __GRPIX_H
#include "icanvas.h"
#include "ifcn.h"
#include "tabdat.h"

#define gr_set_upixel \
   ((void (*)(long color, short x, short y))grd_pixel_table[SET_UPIXEL8])
#define gr_set_pixel \
   ((int (*)(long color, short x, short y))grd_pixel_table[SET_PIXEL8])

#define gr_set_upixel_interrupt \
   ((void (*)(long color, short x, short y))grd_pixel_table[SET_UPIXEL8_INTERRUPT])
#define gr_set_pixel_interrupt \
   ((int (*)(long color, short x, short y))grd_pixel_table[SET_PIXEL8_INTERRUPT])

extern int gen_fill_pixel(long color, short x, short y);

#define gr_fill_upixel \
   ((void (*)(long color, short x, short y))grd_function_table[GRC_PIXEL])
#define gr_fill_pixel gen_fill_pixel

#define gr_get_upixel \
   ((long (*)(short x, short y))grd_pixel_table[GET_UPIXEL8])
#define gr_get_pixel \
   ((long (*)(short x, short y))grd_pixel_table[GET_PIXEL8])
#endif /* !__GRPIX_H */
