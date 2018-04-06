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
 * $Source: n:/project/lib/src/2d/RCS/genhlin.c $
 * $Revision: 1.6 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 02:27:08 $
 *
 * Generic routines for drawing horizontal lines.
 *
 * This file is part of the 2d library.
 *
 * $Log: genhlin.c $
 * Revision 1.6  1994/06/11  02:27:08  lmfeeney
 * moved unclipped line drawer, two versions of clipped line drawer, 
 * one for each i\f - do clipping inline, then call unclipped drawer
 * handles all fill types
 * 
 * Revision 1.5  1993/10/19  09:51:14  kaboom
 * Replaced #include "grd.h" with new headers split from grd.h.
 * 
 * Revision 1.4  1993/10/02  01:17:25  kaboom
 * Changed include of clip.h to include of clpcon.h and/or clpfcn.h.
 * 
 * Revision 1.3  1993/04/29  18:40:36  kaboom
 * Changed include of gr.h to smaller more specific grxxx.h.
 * 
 * Revision 1.2  1993/03/02  19:45:58  kaboom
 * Changed to not draw the rightmost pixel.
 * 
 * Revision 1.1  1993/02/25  23:10:59  kaboom
 * Initial revision
 */

#include "grd.h"
#include "ctxmac.h"
#include "clpcon.h"
#include "clpltab.h"
#include "grlin.h"
#include "general.h"
#include "linfcn.h"

/* draw a clipped horizontal line with integral coordinates.  returns a clip
   code. */

int gen_hline (short x0, short y0, short x1)
{
   int r;
  
   r = grd_hline_clip_fill (x0, y0, x1, gr_get_fcolor(), gr_get_fill_parm());
  
   return r;
}

int gri_hline_clip_fill (short x0, short y0, short x1, long c, long parm)
{
   int r = CLIP_NONE;
   short t;

   if (x0 > x1) {
      t = x0; x0 = x1; x1 = t;
   }
   if (y0<grd_clip.top || y0>=grd_clip.bot ||
       x1<grd_clip.left || x0>=grd_clip.right)
      return CLIP_ALL;			/* forget about return values */

   if (x0 < grd_clip.left) {
      r |= CLIP_LEFT;
      x0 = grd_clip.left;
   }
   if (x1 >= grd_clip.right) {
      r |= CLIP_RIGHT;
      x1 = grd_clip.right-1;
   }

   if (r != CLIP_ALL)
     grd_uhline_fill (x0, y0, x1, c, parm);

   return r;
}
