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
 * $Source: r:/prj/lib/src/2d/RCS/fl8lin.c $
 * $Revision: 1.11 $
 * $Author: lmfeeney $
 * $Date: 1994/08/12 01:09:59 $
 *
 * Routines for drawing fixed-point lines onto a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8lin.c $
 * Revision 1.11  1994/08/12  01:09:59  lmfeeney
 * get  fill/solid parm from right place
 * 
 * Revision 1.10  1994/06/11  01:22:44  lmfeeney
 * guts of the routine moved to fl8{c,s}lin.h, per fill type
 * line drawers are created by defining macros and including
 * this file
 * 
 * Revision 1.9  1994/05/06  18:18:58  lmfeeney
 * rewritten for greater accuracy
 * 
 * Revision 1.8  1993/12/15  11:25:22  kaboom
 * Fixed up problems with not including endpoints and to match up with
 * new polygon scanner.
 * 
 * Revision 1.7  1993/10/19  09:50:35  kaboom
 * Replaced #include "grd.h" with new headers split from grd.h.
 * 
 * Revision 1.6  1993/10/08  01:15:18  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.5  1993/06/23  04:56:07  kaboom
 * Put in checks for single-pixel horizontal and vertical lines.
 * 
 * Revision 1.4  1993/06/22  15:14:38  kaboom
 * Now checks to see if final span is empty.
 * 
 * Revision 1.3  1993/06/16  18:15:24  kaboom
 * Removed foolish mprintfs accidentally left in.
 * 
 * Revision 1.2  1993/06/16  01:59:53  kaboom
 * Fixed gradual precision error.  Last span now explicitly set to x1
 * instead of adding m_inv.
 * 
 * Revision 1.1  1993/03/02  20:33:50  kaboom
 * Initial revision
 */

#include <string.h>
#include "fix.h"
#include "plytyp.h"
#include "ctxmac.h"
#include "fill.h"
#include "cnvdat.h"
#include "scrdat.h"
#include "linfcn.h"
#include "lg.h"

/* This particular mess implements the fix_uline for each of the
   five fill types.  The main driver (essentially the code in the
   the 'original' fl8lin.c) is now a code fragment in fl8lin.h

   For each function, four macros are (re)defined - flat8_pixel_fill_xf
   and flat8_pixel_fill_xi - which set the pixel value in the case of
   x fix-point and x known to be integer, and flat8_pixel_fill_row, which
   allows us to retain the speed hack for a (nearly) horizontal line.  For 
   fill types which are indep of the pixel value (norm, solid, clut), the 
   color is set only once in flat8_pixel_fill_init.

   Note that each macro is referenced 7 times (the line drawer has
   lots of dx <=> dy type cases).  This makes blend come out huge.

   None of these macros take arguments, instead they rely on secret
   gnosis of the variable names in fl8lin.h 
*/


/* not all line fill functions use all their parameters */
// MLA #pragma off (unreferenced)

/* same for norm, solid and clut */

#undef  flat8_pixel_fill_xf 
#define flat8_pixel_fill_xf \
do { \
   p[fix_fint(x0)] = c; \
} while (0)

#undef  flat8_pixel_fill_xi
#define flat8_pixel_fill_xi \
do { \
   p[x0] = c; \
} while (0)

#undef flat8_pixel_fill_row 
#define flat8_pixel_fill_row \
do {LG_memset(p+x0,c,x1-x0+1);} \
while(0)

#undef flat8_pixel_fill_init 
#define flat8_pixel_fill_init \
do { \
   if (gr_get_fill_type() ==  FILL_SOLID) \
     c = (uchar)parm; \
} while (0)

/* norm */

void  gri_flat8_uline_ns (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
#include "fl8lin.h"
}


/* clut */

#undef flat8_pixel_fill_init
#define flat8_pixel_fill_init \
do { \
   c = (long) (((uchar*)parm)[c]); \
} while (0)

void  gri_flat8_uline_clut (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
#include "fl8lin.h"
}

/* xor */

#undef  flat8_pixel_fill_xf 
#define flat8_pixel_fill_xf \
do { \
   p[fix_fint(x0)] = c ^ p[fix_fint(x0)]; \
} while (0)

#undef  flat8_pixel_fill_xi
#define flat8_pixel_fill_xi \
do { \
   p[x0] = c ^ p[x0]; \
} while (0)
  
#undef flat8_pixel_fill_row 
#define flat8_pixel_fill_row \
do { \
   while (x0 < x1) { \
     flat8_pixel_fill_xi; \
     x0++; \
   } \
} while (0)

void  gri_flat8_uline_xor (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
#include "fl8lin.h"
}

/* blend -- maybe we should just swallow the function call */

#define QMASK 0x3fc7f8ff
/* convert red in a glomped rgb to a fixed point */
#define rtof(b) ( ((b)&0x3ff)<<12)
/* convert green in a glomped rgb to a fixed point */
#define gtof(b) ( ((b)&0x1ff800)<<1)
/* convert blue in a glomped rgb to a fixed point */
#define btof(b) ( ((b)&0xffc00000)>>10)

#undef  flat8_pixel_fill_xf 
#define flat8_pixel_fill_xf \
   do { \
      uchar *k; \
      grs_rgb prev; \
      grs_rgb lg_new; \
      fix r1, g1, b1; \
\
      prev = grd_bpal [p[fix_fint(x0)]]; \
      lg_new = grd_bpal [c]; \
\
      r1 = fix_mul(rtof(lg_new),(fix) parm) + fix_mul(rtof(prev),FIX_UNIT-(fix) parm); \
      g1 = fix_mul(gtof(lg_new),(fix) parm) + fix_mul(gtof(prev),FIX_UNIT-(fix) parm); \
      b1 = fix_mul(btof(lg_new),(fix) parm) + fix_mul(btof(prev),FIX_UNIT-(fix) parm); \
      \
      k = grd_ipal; \
      k += (r1>>17)&0x1f; \
      k += (g1>>12)&0x3e0; \
      k += (b1>>7)&0x7c00; \
      p[fix_fint(x0)] = *k; \
    } while (0)

#undef  flat8_pixel_fill_xi
#define flat8_pixel_fill_xi \
   do { \
      uchar *k; \
      grs_rgb prev; \
      grs_rgb lg_new; \
      fix r1, g1, b1; \
\
      prev = grd_bpal [p[x0]]; \
      lg_new = grd_bpal [c]; \
\
      r1 = fix_mul(rtof(lg_new),(fix) parm) + fix_mul(rtof(prev),FIX_UNIT- (fix) parm); \
      g1 = fix_mul(gtof(lg_new),(fix) parm) + fix_mul(gtof(prev),FIX_UNIT- (fix) parm); \
      b1 = fix_mul(btof(lg_new),(fix) parm) + fix_mul(btof(prev),FIX_UNIT- (fix) parm); \
      \
      k = grd_ipal; \
      k += (r1>>17)&0x1f; \
      k += (g1>>12)&0x3e0; \
      k += (b1>>7)&0x7c00; \
      p[x0] = *k; \
    } while (0)


#undef flat8_pixel_fill_row 
#define flat8_pixel_fill_row \
do { \
   while (x0 < x1) { \
     flat8_pixel_fill_xi; \
     x0++; \
   } \
} while (0)

#undef flat8_pixel_fill_init 
#define flat8_pixel_fill_init \
do { \
   ; \
} while (0)

void  gri_flat8_uline_blend (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
#include "fl8lin.h"
}



