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
 * $Source: r:/prj/lib/src/2d/RCS/genov.c $
 * $Revision: 1.1 $
 * $Author: lmfeeney $
 * $Date: 1994/11/21 01:23:46 $
 */

#include "clpcon.h"
#include "cnvdat.h"
#include "grpix.h"
#include "grrect.h"
#include "fix.h"
#include "genov.h"


/* 
 *  x-y oriented filled-elipse drawer, mostly from Foley and VanDam,
 *  but the clipping especially could be made much more efficient
 *  
 *  the elipse drawer should be canvas specific and in the tables, but it would
 *  be better to wait until other 2d.h changes */

void gr_int_uoval (int x0, int y0, int a, int b)
{
  int x;
  int y;

  fix24 a_sq, b_sq;
  fix24 d1,d2,t1,t2;   

  a_sq = (a * a)<<8;
  b_sq = (b * b)<<8;

  x = 0;
  y = b;
  
  /* d1 = b_sq - a_sq*b + a_sq/4 */

  d1 = b_sq - fix24_mul(a_sq,(b<<8)) + fix24_div(a_sq,(4<<8));

  t1 = fix24_mul(a_sq,((y<<8)-128)); 
  t2 = fix24_mul(b_sq,((x+1)<<8));

  while (t1 > t2) {

    if (d1 < 0) {
      d1 += fix24_mul(b_sq,((x<<9)+(3<<8)));
      x++;
    }
    else {
      d1 += fix24_mul(b_sq,((x<<9)+(3<<8)));
      d1 += fix24_mul(a_sq,(((-y)<<9)+(2<<8)));
      x++; y--;
    }

    gr_uhline((short)(x0-x),(short)(y0+y),(short)(x0+x));
    gr_uhline((short)(x0-x),(short)(y0-y),(short)(x0+x));

    t1 = fix24_mul(a_sq,((y<<8)-128)); 
    t2 = fix24_mul(b_sq,((x+1)<<8));
  }
  
  t1 = fix24_mul(((x<<8)+128),((x<<8)+18));
  d2 = fix24_mul(t1,b_sq);

  t1 = fix24_mul(((y-1)<<8),((y-1)<<8));
  d2 += fix24_mul(t1,a_sq);

  t1 = fix24_mul(a_sq,b_sq);
  d2 -= t1;

  while (y > 0) {
    if (d2 < 0) {
      t1 = fix24_mul(((x<<9)+(2<<8)),b_sq);
      t2 = fix24_mul(((3<<8)-(y<<9)),a_sq);
      d2 = d2 + t1 + t2;
      x++; y--;
    }
    else {
      t2 = fix24_mul(((3<<8)-(y<<9)),a_sq);
      d2 += t2;
      y--;
    }
    gr_uhline((short)(x0-x),(short)(y0+y),(short)(x0+x));
    gr_uhline((short)(x0-x),(short)(y0-y),(short)(x0+x));
  }
}


int gr_int_oval (int x0, int y0, int a, int b)
{
  int x;
  int y;

  fix24 a_sq, b_sq;
  fix24 d1,d2,t1,t2;   

  /* trivial clipping */

  if (x0+a<=grd_clip.left || x0-a>grd_clip.right ||
      y0+b<=grd_clip.top || y0-b>grd_clip.bot)
    return CLIP_ALL;


  a_sq = (a * a)<<8;
  b_sq = (b * b)<<8;

  x = 0;
  y = b;
  
  /* d1 = b_sq - a_sq*b + a_sq/4 */

  d1 = b_sq - fix24_mul(a_sq,(b<<8)) + fix24_div(a_sq,(4<<8));

  t1 = fix24_mul(a_sq,((y<<8)-128)); 
  t2 = fix24_mul(b_sq,((x+1)<<8));

  while (t1 > t2) {

    if (d1 < 0) {
      d1 += fix24_mul(b_sq,((x<<9)+(3<<8)));
      x++;
    }
    else {
      d1 += fix24_mul(b_sq,((x<<9)+(3<<8)));
      d1 += fix24_mul(a_sq,(((-y)<<9)+(2<<8)));
      x++; y--;
    }

    gr_hline((short)(x0-x),(short)(y0-y),(short)(x0+x));
    gr_hline((short)(x0-x),(short)(y0+y),(short)(x0+x));

    t1 = fix24_mul(a_sq,((y<<8)-128)); 
    t2 = fix24_mul(b_sq,((x+1)<<8));
  }
  
  t1 = fix24_mul(((x<<8)+128),((x<<8)+128));
  d2 = fix24_mul(t1,b_sq);

  t1 = fix24_mul(((y-1)<<8),((y-1)<<8));
  d2 += fix24_mul(t1,a_sq);

  t1 = fix24_mul(a_sq,b_sq);
  d2 -= t1;

  while (y > 0) {
    if (d2 < 0) {
      t1 = fix24_mul(((x<<9)+(2<<8)),b_sq);
      t2 = fix24_mul(((3<<8)-(y<<9)),a_sq);
      d2 = d2 + t1 + t2;
      x++; y--;
    }
    else {
      t2 = fix24_mul(((3<<8)-(y<<9)),a_sq);
      d2 += t2;
      y--;
    }

    gr_hline((short)(x0-x),(short)(y0-y),(short)(x0+x));
    gr_hline((short)(x0-x),(short)(y0+y),(short)(x0+x));
  }

  /* could be more specific */
  return CLIP_NONE;
}

