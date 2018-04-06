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
 * $Source: n:/project/lib/src/2d/RCS/fl8pnt.c $
 * $Revision: 1.4 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:50:51 $
 * 
 * Routines for drawing points into a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8pnt.c $
 * Revision 1.4  1993/10/19  09:50:51  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.3  1993/10/01  16:00:46  kaboom
 * Converted to include clpcon.h instead of clip.h
 * 
 * Revision 1.2  1993/05/16  00:33:03  kaboom
 * Fixed clipped case to handle new padded clipping rectangle.
 * 
 * Revision 1.1  1993/02/16  14:16:27  kaboom
 * Initial revision
 */

#include "clpcon.h"
#include "cnvdat.h"
#include "flat8.h"
 
void flat8_upoint(short x, short y)
{
   uchar *p;

   p = grd_bm.bits + y*grd_bm.row + x;
   *p = grd_gc.fcolor;
}

int flat8_point(short x, short y)
{
   uchar *p;

   if (x<grd_clip.left || x>=grd_clip.right ||
       y<grd_clip.top  || y>=grd_clip.bot)
      return CLIP_ALL;

   p = grd_bm.bits + grd_bm.row*y + x;
   *p = grd_gc.fcolor;
   return CLIP_NONE;
}
