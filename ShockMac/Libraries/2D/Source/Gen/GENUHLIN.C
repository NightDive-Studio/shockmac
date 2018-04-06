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
 * $Source: n:/project/lib/src/2d/RCS/genuhlin.c $
 * $Revision: 1.1 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 02:35:29 $
*/

#include "pixfill.h"
#include "linfcn.h"

/* draw an unclipped horizontal line with integral coordinates.
   the correct fill is obtained via grd_pixel_fill,
   which in turn calls gr_set_upixel
 */

void gri_gen_uhline_fill (short x0, short y0, short x1, long c, long parm)
{
   short t;

   if (x0 > x1) {
      t = x0; x0 = x1; x1 = t;
   }
   while (x0 <= x1)
      grd_pixel_fill (c, parm, x0++, y0);
}

