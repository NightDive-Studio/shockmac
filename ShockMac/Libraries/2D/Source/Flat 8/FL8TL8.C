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
 * $Source: n:/project/lib/src/2d/RCS/fl8tl8.c $
 * $Revision: 1.2 $
 * $Author: baf $
 * $Date: 1994/01/14 12:40:30 $
 * 
 * Routines for drawing flat 8 bitmaps into a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8tl8.c $
 * Revision 1.2  1994/01/14  12:40:30  baf
 * Lit translucency reform
 * 
 * Revision 1.1  1993/12/01  21:17:31  baf
 * Initial revision
 * 
 */

#include <string.h>
#include "bitmap.h"
#include "cnvdat.h"
#include "tluctab.h"
#include "fl8tf.h"

void flat8_tluc8_ubitmap (grs_bitmap *bm, short x, short y)
{
	uchar *src;
	uchar *dst;
	long 	w = bm->w;
	long 	h = bm->h;
	long 	i;
	long	grow = grd_bm.row;
	long  brow = bm->row;
	
	src = bm->bits;
	dst = grd_bm.bits + grow*y + x;
	
	if (bm->flags & BMF_TRANS)
	  while (h--) {
	     for (i=0; i<w; i++)
	        if (src[i]!=0) {
	           if (tluc8tab[src[i]] == NULL) dst[i]=src[i];
	           else dst[i] = tluc8tab [src[i]] [dst[i]];
	        }
	     src += brow;
	     dst += grow;
	  }
	else
	  while (h--) {
	     for (i=0; i<w; i++) {
	        if (tluc8tab[src[i]] == NULL) dst[i]=src[i];
	        else dst[i] = tluc8tab [src[i]] [dst[i]];
	     }
	     src += brow;
	     dst += grow;
	  }
}
