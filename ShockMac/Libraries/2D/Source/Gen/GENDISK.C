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
 * $Source: r:/prj/lib/src/2d/RCS/gendisk.c $
 * $Revision: 1.4 $
 * $Author: lmfeeney $
 * $Date: 1994/11/21 01:22:55 $
 *
 * Generic disk drawing routines.
 *
 * $Log: gendisk.c $
 * Revision 1.4  1994/11/21  01:22:55  lmfeeney
 * rewrote to determine use aspect ratio and draw appropriate oval
 * 
 * Revision 1.3  1993/10/08  01:15:39  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.2  1993/04/29  18:40:25  kaboom
 * Changed include of gr.h to smaller more specific grxxx.h.
 * 
 * Revision 1.1  1993/03/03  18:10:02  kaboom
 * Initial revision
 */

#include "grd.h"
#include "genov.h"
#include "general.h"

/* This is rather lame, uses aspect ratio to determine parameters to
 * the oval drawer.  That would be fine, except that it makes the
 * definition of r somewhat arbitrary, since it has to be in some unit
 * in some mode.  For compatibility (sort of), we say 320x200 x-pixel's.
 * This leads to some ugly rounding, but the circles are good enough.
 */

void gen_int_udisk (short x0, short y0, short r)
{
  fix a, b, ratio;

  /* scale from 320x200 x-pixels */
  ratio = fix_div (((grd_cap->w)<<16), (320<<16));
  a = fix_mul((r<<16), ratio);

  /* calculate equivalent b */
  b = fix_div (a,(grd_cap->aspect));

  gr_int_uoval (x0, y0, fix_fint(a), fix_fint(b));

  return;
}

/* this really should return a clip code */
void gen_int_disk (short x0, short y0, short r)
{
  int c;
  
  fix a, b, ratio;
  
  /* scale from 320x200 x-pixels */
  ratio = fix_div (((grd_cap->w)<<16), (320<<16));
  a = fix_mul((r<<16), ratio);

  /* calculate equivalent b */
  b = fix_div (a,(grd_cap->aspect));
  
  c = gr_int_oval (x0, y0, fix_fint(a), fix_fint(b));

  return;
}
