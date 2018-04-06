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
 * $Source: n:/project/lib/src/2d/RCS/clplin.c $
 * $Revision: 1.4 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:50:06 $
 * 
 * Routines for clipping fixed-point lines to a rectangle.
 *
 * This file is part of the 2d library.
 *
 * $Log: clplin.c $
 * Revision 1.4  1993/10/19  09:50:06  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.3  1993/10/01  15:38:55  kaboom
 * Cleaned up code some; converted to using clpcon.h instead of clip.h.
 * 
 * Revision 1.2  1993/06/15  20:16:51  kaboom
 * Fixed clipper to deal with padded clipping region.
 * 
 * Revision 1.1  1993/02/22  14:41:22  kaboom
 * Initial revision
 * 
 ********************************************************************
 * Log from old clip.c:
 *
 * Revision 1.11  1993/02/05  16:27:54  matt
 * 2. Added parens to gr_clip_fix_line to fix the case where both points were
 *    off screen.
 * 3. Changed gr_clip_fix_line() to use new fix_mul_div() function.
 *
 * Revision 1.5  1992/11/12  13:31:58  kaboom
 * Fixed bug in fixed-point line clipper which was causing hangs.
 * Several of the clipping region values were being read as integers,
 * not fixed-point numbers.
 */ 

#include "grs.h"
#include "clpcon.h"
#include "cnvdat.h"
#include "clpfcn.h"

/* Returns clip code for fixed-point coordinates for the Cohen-Sutherland
   line clipper. */
int gr_clip_fix_code (fix x, fix y)
{
   int code = 0;

   if (x < grd_fix_clip.left)
      code |= CLIP_LEFT;
   else if (x > grd_fix_clip.right-fix_make(1,0))
      code |= CLIP_RIGHT;
   if (y < grd_fix_clip.top)
      code |= CLIP_TOP;
   else if (y > grd_fix_clip.bot-fix_make(1,0))
      code |= CLIP_BOT;

   return code;
}

/* fixed-point Cohen-Sutherland line clipper. */
int gr_clip_fix_line (fix *x0, fix *y0, fix *x1, fix *y1)
{
   int code0;                 /* clip code for (x0,y0) */
   int code1;                 /* code for (x1,y1) */
   int code;                  /* code for current point */
   fix dx;                    /* x distance */
   fix dy;                    /* y distance */
   fix *px;                   /* pointer to x current coordinate */
   fix *py;                   /*    " to current y */

   dx = *x1-*x0;
   dy = *y1-*y0;

   while (1) {
      /* get codes for endpoints. */
      code0 = gr_clip_fix_code (*x0, *y0);
      code1 = gr_clip_fix_code (*x1, *y1);

      if (code0==0 && code1==0)        /* check trivial accept */
         return CLIP_NONE;
      else if ((code0&code1) != 0)     /* check for trivial reject */
         return CLIP_ALL;

      /* set current code and px&py.  first, for point0, then when it's
         dealt with, point1. */
      if (code0 != 0) {
         px = x0;
         py = y0;
         code = code0;
      } else {
         px = x1;
         py = y1;
         code = code1;
      }

      /* check for left/right clip; compute intersection. */
      if (code & CLIP_LEFT) {
         *py += fix_mul_div (dy, grd_fix_clip.left-*px, dx);
         *px = grd_fix_clip.left;
      } else if (code & CLIP_RIGHT) {
         *py += fix_mul_div (dy, grd_fix_clip.right-fix_make(1,0)-*px, dx);
         *px = grd_fix_clip.right-fix_make(1,0);
      }
      /* check for top/bottom clip; compute intersection. */
      if (code & CLIP_TOP) {
         *px += fix_mul_div (dx, grd_fix_clip.top-*py, dy);
         *py = grd_fix_clip.top;
      } else if (code & CLIP_BOT) {
         *px += fix_mul_div (dx, grd_fix_clip.bot-fix_make(1,0)-*py, dy);
         *py = grd_fix_clip.bot-fix_make(1,0);
      }
   }
}
