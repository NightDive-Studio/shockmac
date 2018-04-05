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
 * $Source: n:/project/lib/src/2d/RCS/grp24.h $
 * $Revision: 1.3 $
 * $Author: kaboom $
 * $Date: 1993/10/19 10:18:46 $
 *
 * Dispatch macros for 24-bit pixel functions.
 *
 * This file is part of the 2d library.
 *
 * $Log: grp24.h $
 * Revision 1.3  1993/10/19  10:18:46  kaboom
 * Now includes tabdat.h.
 * 
 * Revision 1.2  1993/10/08  01:16:05  kaboom
 * Changed quotes in #include lines to angle brackets for Watcom.
 * 
 * Revision 1.1  1993/09/02  20:11:35  kaboom
 * Initial revision
 */

#ifndef __GRP24_H
#define __GRP24_H
#include "icanvas.h"
#include "tabdat.h"

#define gr_set_upixel24 \
   ((void (*)(long color,short x,short y))grd_pixel_table[SET_UPIXEL24])
#define gr_set_pixel24 \
   ((int (*)(long color,short x,short y))grd_pixel_table[SET_PIXEL24])
#define gr_get_upixel24 \
   ((long (*)(short x,short y))grd_pixel_table[GET_UPIXEL24])
#define gr_get_pixel24 \
   ((long (*)(short x,short y))grd_pixel_table[GET_PIXEL24])
#endif /* !__GRP24_H */

