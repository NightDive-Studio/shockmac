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
 * $Source: r:/prj/lib/src/2d/RCS/fl8hlin.c $
 * $Revision: 1.8 $
 * $Author: lmfeeney $
 * $Date: 1994/08/12 01:09:33 $
 * 
 * Routines for horizontal drawing lines into a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8hlin.c $
 * Revision 1.8  1994/08/12  01:09:33  lmfeeney
 * get fill/solid from right place
 * 
 * Revision 1.7  1994/06/14  00:04:22  lmfeeney
 * fixed stupid error in xor lines
 * 
 * Revision 1.6  1994/06/11  01:46:09  lmfeeney
 * unclipped flat8 line drawers routines for each fill type
 * canvas values as parameters
 * 
 * Revision 1.5  1993/10/19  09:50:34  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.4  1993/10/08  01:15:17  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.3  1993/05/04  18:43:15  kaboom
 * Changed hline to be inclusive of its endpoints.
 * 
 * Revision 1.2  1993/03/02  19:38:32  kaboom
 * Changed not to draw rightmost pixel.
 * 
 * Revision 1.1  1993/02/22  14:44:24  kaboom
 * Initial revision
 */

#include <string.h>
#include "ctxmac.h"
#include "fill.h"
#include "cnvdat.h"
#include "linfcn.h"
#include "lg.h"

/* draw an unclipped horizontal line with integral coordinates. */

// MLA #pragma off (unreferenced)

/* clut should be in _ns everywhere, it doesn't need its own function */

void gri_flat8_uhline_ns (short x0, short y0, short x1, long c, long parm)
{
   uchar *p;
   short  t;

   if (x0 > x1) {
      t = x0; x0 = x1; x1 = t;
   }
   if (gr_get_fill_type() ==  FILL_SOLID)
     c =    (uchar) parm;
   p = grd_bm.bits + y0*grd_bm.row + x0;
   LG_memset (p, c, x1-x0+1);
 }

void gri_flat8_uhline_clut (short x0, short y0, short x1, long c, long parm)
{
   uchar *p;
   short  t;

   if (x0 > x1) {
      t = x0; x0 = x1; x1 = t;
   }

   c = (long) (((uchar *) parm) [c]);
   p = grd_bm.bits + y0*grd_bm.row + x0;
   LG_memset (p, c, x1-x0+1);
 }

void gri_flat8_uhline_xor (short x0, short y0, short x1, long c, long parm)
{
   uchar *p;
   short  t;

   if (x0 > x1) {
      t = x0; x0 = x1; x1 = t;
   }
  
   for ( p = grd_bm.bits + y0*grd_bm.row + x0; x0 <= x1; p++, x0++)
     *p = *p ^ c; 
 }

/* punt */
void gri_flat8_uhline_blend (short x0, short y0, short x1, long c, long parm)
{
   uchar *p;
   short  t;

   if (x0 > x1) {
      t = x0; x0 = x1; x1 = t;
   }
   p = grd_bm.bits + y0*grd_bm.row + x0;
   LG_memset (p, c, x1-x0+1);
 }
