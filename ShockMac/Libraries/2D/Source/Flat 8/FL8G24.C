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
 * $Source: r:/prj/lib/src/2d/RCS/fl8g24.c $
 * $Revision: 1.4 $
 * $Author: kevin $
 * $Date: 1994/10/17 14:59:58 $
 *
 * Routines for reading 24-bit pixels from a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8g24.c $
 * Revision 1.4  1994/10/17  14:59:58  kevin
 * Use palette macros in preparation for switch to palette globals.
 * 
 * Revision 1.3  1993/10/19  09:50:22  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.2  1993/10/01  15:44:52  kaboom
 * Converted to include clpcon.h instead of clip.h.
 * 
 * Revision 1.1  1993/09/02  20:03:41  kaboom
 * Initial revision
 */

#include "clpcon.h"
#include "cnvdat.h"
#include "scrdat.h"
#include "flat8.h"

/* set an unclipped pixel in bank-switched memory. */
long flat8_get_upixel24 (short x, short y)
{
   uchar *p;
   long *r;
   int i;

   p = grd_canvas->bm.bits + grd_canvas->bm.row*y + x;
   i = *p;
   r = (long *)(grd_pal+3*i);
   return *r & 0x00ffffff;
}

/* set a clipped pixel in bank-switched memory.  return the clip code. */
long flat8_get_pixel24 (short x, short y)
{
   uchar *p;
   long *r;
   int i;

   if (x<grd_clip.left || x>=grd_clip.right ||
       y<grd_clip.top  || y>=grd_clip.bot)
      return CLIP_ALL;
   p = grd_canvas->bm.bits + grd_canvas->bm.row*y + x;
   i = *p;
   r = (long *)(grd_pal+3*i);
   return *r & 0x00ffffff;
}
