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
 * $Source: r:/prj/lib/src/2d/RCS/genpix.c $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/11/12 02:21:29 $
 *
 * Generic routines for drawing a clipped pixel.
 *
 * This file is part of the 2d library.
 *
 */

#include "clpcon.h"
#include "cnvdat.h"
#include "grpix.h"
#include "general.h"

void *grd_fill_upixel_func();

/* understands fill type. */
int gen_fill_pixel (long color, short x, short y)
{
   if (x<grd_clip.left || x>=grd_clip.right || y<grd_clip.top || y>=grd_clip.bot)
      return CLIP_ALL;

   gr_fill_upixel(color,x,y);
   return CLIP_NONE;
}

/* ignores fill type. */
int gen_set_pixel (long color, short x, short y)
{
   if (x<grd_clip.left || x>=grd_clip.right || y<grd_clip.top || y>=grd_clip.bot)
      return CLIP_ALL;

   gr_set_upixel(color,x,y);
   return CLIP_NONE;
}

int gen_set_pixel_interrupt(long color, short x, short y)
{
   if (x<grd_clip.left || x>=grd_clip.right || y<grd_clip.top || y>=grd_clip.bot)
      return CLIP_ALL;

   gr_set_upixel_interrupt(color,x,y);
   return CLIP_NONE;
}
