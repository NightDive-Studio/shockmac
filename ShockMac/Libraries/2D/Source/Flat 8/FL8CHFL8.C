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
 * $Source: n:/project/lib/src/2d/RCS/fl8chfl8.c $
 * $Revision: 1.4 $
 * $Author: kevin $
 * $Date: 1993/10/26 02:23:49 $
 * $Log: fl8chfl8.c $
 * Revision 1.4  1993/10/26  02:23:49  kevin
 * Use default clut if passed cl=NULL.
 * 
 * Revision 1.3  1993/10/19  09:50:17  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.2  1993/10/08  01:15:07  kaboom
 * Changed quotes in #include lines to angle brackets for Watcom.
 * 
 * Revision 1.1  1993/10/06  14:39:25  kevin
 * Initial revision
 */

#include "cnvdat.h"
#include "scrmac.h"
#include "flat8.h"

/* draw an unclipped, horizontally flipped flat 8 bitmap to a flat 8
   canvas through a color lookup table (clut). */
void flat8_clut_hflip_flat8_ubitmap (grs_bitmap *bm, short x, short y,uchar *cl)
{
	short w;                   /* bitmap width */
	short h;                   /* height */
	uchar *src;                /* pointer into source bitmap */
	uchar *dst;                /* pointer into canvas memory */
	ushort	row;
	
	if (cl == NULL) cl=gr_get_clut();
	h = bm->h;
	src = bm->bits;
	dst = grd_bm.bits + y*grd_bm.row + x+bm->w-1;
	while (h--) {
	w = bm->w;
	while (w--)
	 *dst-- = cl[*src++];
	src += bm->row-bm->w;
	dst += grd_bm.row+bm->w;
	}
}
