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
 * $Source: r:/prj/lib/src/2d/RCS/fl8pix.c $
 * $Revision: 1.5 $
 * $Author: kevin $
 * $Date: 1994/08/16 13:14:01 $
 * 
 * Routines for drawing pixels into a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8pix.c $
 * Revision 1.5  1994/08/16  13:14:01  kevin
 * Added pixel primitives for all fill modes.
 * 
 * Revision 1.4  1993/10/19  09:50:49  kaboom
 * Replaced #include "grd.h" with new headers split from grd.h.
 * 
 * Revision 1.3  1993/10/01  16:00:45  kaboom
 * Converted to include clpcon.h instead of clip.h
 * 
 * Revision 1.2  1993/05/16  00:33:22  kaboom
 * Fixed clipped case to handle new padded clipping rectangle.
 * 
 * Revision 1.1  1993/02/16  14:16:16  kaboom
 * Initial revision
 */

#include "blend.h"
#include "cnvdat.h"
#include "fl8tf.h"

/* draws an unclipped pixel of the given color at (x, y) on the canvas. */
void flat8_set_upixel(long color, short x, short y)
{
   uchar *p;

   p = grd_bm.bits + grd_bm.row*y + x;
   *p = color;
}

void flat8_clut_set_upixel (long color, short x, short y)
{
   uchar *p;

   p = grd_bm.bits + grd_bm.row*y + x;
   *p = ((uchar *)grd_gc.fill_parm)[color];
}

void flat8_xor_set_upixel (long color, short x, short y)
{
   uchar *p;

   p = grd_bm.bits + grd_bm.row*y + x;
   *p = color ^ *p;
}

void flat8_blend_set_upixel (long color, short x, short y)
{
   uchar *p;

   p = grd_bm.bits + grd_bm.row*y + x;
   *p = gr_blend(color, *p, grd_gc.fill_parm);
}

// MLA #pragma off (unreferenced)
void flat8_solid_set_upixel (long color, short x, short y)
{
   uchar *p;

   p = grd_bm.bits + grd_bm.row*y + x;
   *p = (uchar )grd_gc.fill_parm;
}
// MLA #pragma on (unreferenced)
