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
 * $Source: n:/project/lib/src/2d/RCS/grrect.h $
 * $Revision: 1.4 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 00:36:07 $
 *
 * Dispatch macros for rectangular/straight functions.
 *
 * This file is part of the 2d library.
 *
 * $Log: grrect.h $
 * Revision 1.4  1994/06/11  00:36:07  lmfeeney
 * lines removed from canvas table, #defines for backward compatibility
 * 
 * Revision 1.3  1993/10/19  10:21:35  kaboom
 * Now includes tabdat.h.
 * 
 * Revision 1.2  1993/10/08  01:16:07  kaboom
 * Changed quotes in #include lines to angle brackets for Watcom.
 * 
 * Revision 1.1  1993/04/29  18:36:43  kaboom
 * Initial revision
 */

#ifndef __GRRECT_H
#define __GRRECT_H
#include "icanvas.h"
#include "tabdat.h"
#include "ctxmac.h"
#include "grlin.h"

#define gr_clear \
   ((void (*)(long color))grd_canvas_table[DRAW_CLEAR])
#define gr_upoint \
   ((void (*)(short x,short y))grd_canvas_table[DRAW_UPOINT])
#define gr_point \
   ((int (*)(short x,short y))grd_canvas_table[DRAW_POINT])


/* The line routines have been removed from the canvas tables */

/* horizontal lines */

#define gr_uhline(x0,y0,x1) \
do {\
   grd_uhline_fill ((x0), (y0), (x1), gr_get_fcolor(), gr_get_fill_parm()); \
} while (0)

extern int gen_hline (short x0, short y0, short x1);

#define gr_hline gen_hline

/* vertical lines */

#define gr_uvline(x0,y0,y1) \
do {\
   grd_uvline_fill ((x0), (y0), (y1), gr_get_fcolor(), gr_get_fill_parm()); \
} while (0)

extern int gen_vline (short x0, short y0, short y1);

#define gr_vline gen_vline


#define gr_urect \
   ((void (*)(short x0,short y0,short x1,short y1))grd_canvas_table[DRAW_URECT])
#define gr_rect \
   ((int  (*)(short x0,short y0,short x1,short y1))grd_canvas_table[DRAW_RECT])
#define gr_ubox \
   ((void (*)(short x0,short y0,short x1,short y1))grd_canvas_table[DRAW_UBOX])
#define gr_box \
   ((int (*)(short x0,short y0,short x1,short y1))grd_canvas_table[DRAW_BOX])
#endif /* !__GRRECT_H */
