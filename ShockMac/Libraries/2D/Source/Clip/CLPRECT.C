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
 * $Source: n:/project/lib/src/2d/RCS/clprect.c $
 * $Revision: 1.4 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:50:14 $
 * 
 * Routines for clipping rectangles to a rectangle.
 *
 * This file is part of the 2d library.
 *
 * $Log: clprect.c $
 * Revision 1.4  1993/10/19  09:50:14  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.3  1993/10/01  15:40:23  kaboom
 * Converted to include clpcon.h instead of clip.h.
 * 
 * Revision 1.2  1993/05/04  17:33:59  kaboom
 * Changed to deal with new padded clipping regions.
 * 
 * Revision 1.1  1993/02/22  14:41:56  kaboom
 * Initial revision
 */

#include "grs.h"
#include "clpcon.h"
#include "clpfcn.h"
#include "cnvdat.h"

/* clips the top, left, right, and bottom edges of a rectangle to the current
   clip rectangle.  pointers to the values are passed in so the values can be
   modified.  returns the clip code. */
int gr_clip_rect (short *left, short *top, short *right, short *bot)
{
   int code = CLIP_NONE;

   if (*right<=grd_clip.left || *left>=grd_clip.right ||
       *bot<=grd_clip.top  || *top>=grd_clip.bot)
      /* rect is completely clipped. */
      return CLIP_ALL;

   if (*left < grd_clip.left) {
      /* rect is off the left edge of the window. */
      *left = grd_clip.left;
      code |= CLIP_LEFT;
   }
   if (*right > grd_clip.right) {
      /* off the right edge of the window. */
      *right = grd_clip.right;
      code |= CLIP_RIGHT;
   }
   if (*top < grd_clip.top) {
      /* off the top of the window. */
      *top = grd_clip.top;
      code |= CLIP_TOP;
   }
   if (*bot > grd_clip.bot) {
      /* off the bottom of the window. */
      *bot = grd_clip.bot;
      code |= CLIP_BOT;
   }

   return code;
}
