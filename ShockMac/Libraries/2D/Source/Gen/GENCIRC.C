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
#include "grd.h"
#include "genel.h"
#include "general.h"

/* This is rather lame, uses aspect ratio to determine parameters to
 * the elipse drawer.  That would be fine, except that it makes the
 * definition of r somewhat arbitrary, since it has to be in some unit
 * in some mode.  For compatibility (sort of), we say 320x200 x-pixel's.
 * This leads to some ugly rounding, but the circles are good enough.
 */

void gen_int_ucircle (short x0, short y0, short r)
{
  
  fix a, b, ratio;

  /* scale from 320x200 x-pixels */
  ratio = fix_div (((grd_cap->w)<<16), (320<<16));
  a = fix_mul((r<<16), ratio);

  /* calculate equivalent b */
  b = fix_div (a,(grd_cap->aspect));

  gr_int_uelipse (x0, y0, fix_fint(a), fix_fint(b));

  return;
}

int gen_int_circle (short x0, short y0, short r) 
{
  int c;
  
  fix a, b, ratio;
  
  /* scale from 320x200 x-pixels */
  ratio = fix_div (((grd_cap->w)<<16), (320<<16));
  a = fix_mul((r<<16), ratio);

  /* calculate equivalent b */
  b = fix_div (a,(grd_cap->aspect));
  
  c = gr_int_elipse (x0, y0, fix_fint(a), fix_fint(b));

  return c;
}
