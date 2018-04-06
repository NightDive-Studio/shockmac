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
 * $Source: n:/project/lib/src/2d/RCS/genrect.c $
 * $Revision: 1.5 $
 * $Author: kaboom $
 * $Date: 1993/10/02 01:17:30 $
 *
 * Generic filled rectangle routines.
 *
 * $Log: genrect.c $
 * Revision 1.5  1993/10/02  01:17:30  kaboom
 * Changed include of clip.h to include of clpcon.h and/or clpfcn.h.
 * 
 * Revision 1.4  1993/05/04  18:46:01  kaboom
 * Changed rectangle to omit its right and bottom edges.
 * 
 * Revision 1.3  1993/04/29  18:40:45  kaboom
 * Changed include of gr.h to smaller more specific grxxx.h.
 * 
 * Revision 1.2  1993/02/22  14:48:34  kaboom
 * Changed name of gr_clip_int_rect() to gr_clip_rect().
 * 
 * Revision 1.1  1993/02/16  15:59:53  kaboom
 * Initial revision
 * 
 ********************************************************************
 * Log from old general.c:
 * Revision 1.5  1992/11/19  02:35:32  kaboom
 * Fixed bug in gen_rect which would try to draw rectangles that were
 * completely clipped.  
 */

#include "grs.h"
#include "grd.h"
#include "clpcon.h"
#include "clpfcn.h"
#include "grrect.h"
#include "general.h"

/* draw an unclipped, filled rectangle with edges as given.  do this
   by making repeated calls to the installed unclipped hline drawer. */
void gen_urect (short left, short top, short right, short bot)
{
   while (top < bot)
      gr_uhline (left, top++, right-1);
}

/* draw a clipped, filled rectangle.  clip, then chain to the installed
   unclipped rectangle drawer.  returns clip code. */
int gen_rect (short left, short top, short right, short bot)
{
   int r;

   r = gr_clip_rect (&left, &top, &right, &bot);
   if (r != CLIP_ALL)
      gr_urect (left, top, right, bot);
   return r;
}
