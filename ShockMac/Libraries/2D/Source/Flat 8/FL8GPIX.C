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
 * $Source: n:/project/lib/src/2d/RCS/fl8gpix.c $
 * $Revision: 1.3 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:50:32 $
 * 
 * Routines for reading pixels from a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8gpix.c $
 * Revision 1.3  1993/10/19  09:50:32  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.2  1993/10/08  01:15:15  kaboom
 * Changed quotes in #include lines to angle brackets for Watcom.
 * 
 * Revision 1.1  1993/02/16  14:15:28  kaboom
 * Initial revision
 */

#include "cnvdat.h"
#include "flat8.h"

/* returns value of the unclipped pixel at (x, y). */
long flat8_get_upixel (short x, short y)
{
   uchar *p;

   p = grd_bm.bits + grd_bm.row*y + x;
   return (long)*p;
}

/* returns value of the clipped pixel at (x, y).  returns -1 if pixel isn't
   in the window bounds. */
long flat8_get_pixel (short x, short y)
{
   uchar *p;

   if (x<grd_clip.left || x>grd_clip.right || y<grd_clip.top || y>grd_clip.bot)
      return -1;

   p = grd_bm.bits + grd_bm.row*y + x;
   return (long)*p;
}
