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
 * $Source: n:/project/lib/src/2d/RCS/fl8rect.c $
 * $Revision: 1.4 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:50:53 $
 * 
 * Routines for drawing rectangles into a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8rect.c $
 * Revision 1.4  1993/10/19  09:50:53  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.3  1993/10/08  01:15:29  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.2  1993/05/04  18:43:51  kaboom
 * Changed rectangle to omit the right and bottom edges.
 * 
 * Revision 1.1  1993/02/16  14:17:05  kaboom
 * Initial revision
 */

#include <string.h>
#include "cnvdat.h"
#include "flat8.h"
#include "lg.h"

void flat8_urect(short left, short top, short right, short bot)
{
	uchar 	*p;
	int 		w, h;
	int 		grow = grd_bm.row;
	long 		fcolor = grd_gc.fcolor;
	
	
	p = grd_bm.bits + top*grow + left;
	w = right-left;
	h = bot-top;
	
	while (h-- > 0) {
	  LG_memset(p, fcolor, w);
	  p += grow;
	}
}
