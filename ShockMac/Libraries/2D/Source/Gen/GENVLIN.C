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
 * $Source: n:/project/lib/src/2d/RCS/genvlin.c $
 * $Revision: 1.6 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 02:27:59 $
 *
 * Generic routines for drawing vertical lines.
 *
 * This file is part of the 2d library.
 *
 * $Log: genvlin.c $
 * Revision 1.6  1994/06/11  02:27:59  lmfeeney
 * moved unclipped line drawer, two versions of clipped line drawer,
 * one for each i\f - do clipping inline, then call unclipped drawer
 * handles all fill types
 * 
 * Revision 1.5  1993/10/19  09:51:35  kaboom
 * Replaced #include "grd.h" with new headers split from grd.h.
 * 
 * Revision 1.4  1993/10/02  01:17:36  kaboom
 * Changed include of clip.h to include of clpcon.h and/or clpfcn.h.
 * 
 * Revision 1.3  1993/04/29  18:40:59  kaboom
 * Changed include of gr.h to smaller more specific grxxx.h.
 * 
 * Revision 1.2  1993/03/02  19:46:13  kaboom
 * Changed to not draw the bottommost pixel.
 * 
 * Revision 1.1  1993/02/25  23:12:01  kaboom
 * Initial revision
 */

#include "grd.h"
#include "ctxmac.h"
#include "clpcon.h"
#include "clpltab.h"
#include "grlin.h"
#include "general.h"

/* clipped vertical line with integral coordinates.  returns clip
   code. */

int gen_vline (short x0, short y0, short y1)
{
   int r;
  
   r = grd_vline_clip_fill (x0, y0, y1, gr_get_fcolor(), gr_get_fill_parm());
  
   return r;

}

int gri_vline_clip_fill (short x0, short y0, short y1, long c, long parm)
{
   short t;
   int r = CLIP_NONE;

   /* the clip code needs to be buried in here so that this can 
      be called from an interrupt handle !
    */

   if (y0 > y1) {
      t = y0; y0 = y1; y1 = t;
   }

   if (x0<grd_clip.left || x0>=grd_clip.right ||
       y1<grd_clip.top || y0>=grd_clip.bot)
     return CLIP_ALL;	 

   if (y0 < grd_clip.top) {
      r |= CLIP_TOP;
      y0 = grd_clip.top;
   }
   if (y1 >= grd_clip.bot) {
      r |= CLIP_BOT;
      y1 = grd_clip.bot-1;
   }
   
   grd_uvline_fill (x0, y0, y1, c, parm);

   return r;
}
