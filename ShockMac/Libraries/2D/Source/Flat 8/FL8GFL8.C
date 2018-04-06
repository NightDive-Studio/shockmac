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
 * $Source: n:/project/lib/src/2d/RCS/fl8gfl8.c $
 * $Revision: 1.3 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:50:23 $
 * 
 * Routines for reading flat 8 bitmaps from a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8gfl8.c $
 * Revision 1.3  1993/10/19  09:50:23  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.2  1993/10/08  01:15:14  kaboom
 * Changed quotes in #include lines to angle brackets for Watcom.
 * 
 * Revision 1.1  1993/02/16  14:15:10  kaboom
 * Initial revision
 */

#include <string.h>
#include "cnvdat.h"
#include "flat8.h"
#include "lg.h"

void flat8_get_flat8_ubitmap (grs_bitmap *bm, short x, short y)
{
	uchar *src;
	uchar *dst;
	short h = bm->h;
	short w = bm->w;
	ushort brow = bm->row;
	ushort grow = grd_bm.row;
	
	src = grd_bm.bits + grow*y + x;
	dst = bm->bits;
	while (h--) {
	  LG_memmove (dst, src, w);
	  src += grow;
	  dst += brow;
	}
}
