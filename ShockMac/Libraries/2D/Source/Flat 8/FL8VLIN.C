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
 * $Source: r:/prj/lib/src/2d/RCS/fl8vlin.c $
 * $Revision: 1.7 $
 * $Author: lmfeeney $
 * $Date: 1994/08/12 01:10:20 $
 * 
 * Routines for drawing vertical lines into a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8vlin.c $
 * Revision 1.7  1994/08/12  01:10:20  lmfeeney
 * get  fill/solid parm from right place
 * 
 * Revision 1.6  1994/06/11  01:44:53  lmfeeney
 * unclipped flat8 line drawer routines for each fill type
 * added canvas values as parameters
 * 
 * Revision 1.5  1993/10/19  09:51:05  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.4  1993/10/08  01:15:38  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.3  1993/05/04  18:44:04  kaboom
 * Changed vline to be inclusive of its endpoints.
 * 
 * Revision 1.2  1993/03/02  19:42:08  kaboom
 * Changed not to draw the bottommost pixel.
 * 
 * Revision 1.1  1993/02/22  14:44:38  kaboom
 * Initial revision
 */

#include "ctxmac.h"
#include "fill.h"
#include "cnvdat.h"
#include "linfcn.h"

/* unclipped vertical line with integral coordinates. */

// MLA #pragma off (unreferenced)


/* clut should be in _ns everywhere, it doesn't need its own function */

void gri_flat8_uvline_ns (short x0, short y0, short y1, long c, long parm)
{
	uchar *p;
	short  t;
	int		 grow = grd_bm.row;
	
	if (y0 > y1) {
	  t = y0; y0 = y1; y1 = t;
	}
	if (gr_get_fill_type() ==  FILL_SOLID) 
	 c = (uchar)parm; 
	
	p = grd_bm.bits + y0*grow + x0;
	for (; y0<=y1; y0++) {
	  *p = c;
	  p += grow;
	}
}

void gri_flat8_uvline_clut (short x0, short y0, short y1, long c, long parm)
{
	uchar *p;
	short  t;
	int		 grow = grd_bm.row;
	
	if (y0 > y1) {
	  t = y0; y0 = y1; y1 = t;
	}
	c = (long) (((uchar *) parm) [c]);
	p = grd_bm.bits + y0*grow + x0;
	for (; y0<=y1; y0++) {
	  *p = c;
	  p += grow;
	}
}

void gri_flat8_uvline_xor (short x0, short y0, short y1, long c, long parm)
{
	uchar *p;
	short  t;
	int		 grow = grd_bm.row;
	
	if (y0 > y1) {
	  t = y0; y0 = y1; y1 = t;
	}
	p = grd_bm.bits + y0*grow + x0;
	for (; y0<=y1; y0++) {
	  *p = *p ^ c;
	  p += grow;
	}
}

/* punt */
void gri_flat8_uvline_blend (short x0, short y0, short y1, long c, long parm)
{
	uchar *p;
	short  t;
	int		 grow = grd_bm.row;
	
	if (y0 > y1) {
	  t = y0; y0 = y1; y1 = t;
	}
	p = grd_bm.bits + y0*grow + x0;
	for (; y0<=y1; y0++) {
	  *p = c;
	  p += grow;
	}
}
