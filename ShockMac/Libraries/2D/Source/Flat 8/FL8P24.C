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
 * $Source: r:/prj/lib/src/2d/RCS/fl8p24.c $
 * $Revision: 1.5 $
 * $Author: kevin $
 * $Date: 1994/10/17 14:59:59 $
 *
 * Routines for drawing 24-bit pixels onto a flat 8 canvas.
 *
 * This file is part of the 2d library.
 */

#include "clpcon.h"
#include "cnvdat.h"
#include "scrdat.h"
#include "rgb.h"
#include "flat8.h"

/* set an unclipped pixel in bank-switched memory.  draws 8-8-8 long
   rgb c at (x,y). */
void flat8_set_upixel24(long c,short x,short y)
{
   uchar *p;
   int i;

   i = gr_index_lrgb(c);
   p = grd_bm.bits + grd_bm.row*y + x;
   *p = grd_ipal[i];
}

/* set a clipped pixel in bank-switched memory.  draws an 8-8-8 long
   rgb c at (x,y).  return the clip code. */
int flat8_set_pixel24(long c, short x, short y)
{
   uchar *p;
   int i;

   if (x<grd_clip.left || x>grd_clip.right ||
       y<grd_clip.top || y>grd_clip.bot)
      return CLIP_ALL;
   i = gr_index_lrgb(c);
   p = grd_canvas->bm.bits + grd_canvas->bm.row*y + x;
   *p = grd_ipal[i];

   return CLIP_NONE;
}
