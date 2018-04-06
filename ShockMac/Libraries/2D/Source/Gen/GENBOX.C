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
 * $Source: n:/project/lib/src/2d/RCS/genbox.c $
 * $Revision: 1.9 $
 * $Author: rex $
 * $Date: 1994/04/29 14:28:25 $
 *
 * Generic box (unfilled rectangle) routines.
 */

#include "grs.h"
#include "grd.h"
#include "clpcon.h"
#include "grrect.h"
#include "general.h"

/* draw an unclipped, unfilled rectangle.  does 2 hlines & 2 vlines. */
void gen_ubox(short left, short top, short right, short bot)
{
   if (left<=(right-2))
      gr_uhline(left, top, right-2);
   if (top<=(bot-2))
      gr_uvline(right-1, top, bot-2);
   if ((left+1)<=(right-1))
      gr_uhline(left+1, bot-1, right-1);
   if ((top+1)<=(bot-1))
      gr_uvline(left, top+1, bot-1);
}

/* draw a clipped, unfilled rectangle.  does 2 clipped hlines and 2 clipped
   vlines.  returns clip code. */
int gen_box(short left, short top, short right, short bot)
{
   int code = CLIP_NONE;

   if (left<=(right-2))
      code |= gr_hline(left, top, right-2);
   if (top<=(bot-2))
      code |= gr_vline(right-1, top, bot-2);
   if ((left+1)<=(right-1))
      code |= gr_hline(left+1, bot-1, right-1);
   if ((top+1)<=(bot-1))
      code |= gr_vline(left, top+1, bot-1);

   return code;
}
