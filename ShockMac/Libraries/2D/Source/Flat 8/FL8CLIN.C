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
 * $Source: r:/prj/lib/src/2d/RCS/fl8clin.c $
 * $Revision: 1.7 $
 * $Author: kevin $
 * $Date: 1994/10/17 14:59:57 $
 *
 * Routine to draw an rgb shaded line to a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8clin.c $
 * Revision 1.7  1994/10/17  14:59:57  kevin
 * Use palette macros in preparation for switch to palette globals.
 * 
 * Revision 1.6  1994/06/11  01:24:08  lmfeeney
 * guts of the routine moved to fl8{c,s}lin.h, per fill type
 * line drawers are created by defining macros and including
 * this file
 * 
 * Revision 1.5  1994/05/06  18:18:38  lmfeeney
 * rewritten for greater accuracy and speed
 * 
 * Revision 1.4  1994/05/01  05:34:38  lmfeeney
 * rewritten using simple dda algorithm (+ bit twiddle hack) for greater
 * speed (20/30%) and improvement for e.g. diagonal lines
 * 
 * Revision 1.3  1993/10/19  09:50:19  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.2  1993/10/01  15:43:46  kaboom
 * Pared down include files to reduce dependencies.
 * 
 * Revision 1.1  1993/07/01  22:11:55  spaz
 * Initial revision
 */

#include <stdlib.h>
#include "fix.h"
#include "plytyp.h"
#include "cnvdat.h"
#include "rgb.h"
#include "scrdat.h"
#include "linfcn.h"



// MLA #pragma off (unreferenced)

#define fix_make_nof(x)           fix_make(x,0x0000)
#define macro_get_ipal(r,g,b)     (long) ((r>>19) &0x1f) | ((g>>14) & 0x3e0) | ((b>>9) & 0x7c00)

#undef macro_plot_rgb
#define macro_plot_rgb(x,p,i) \
   do { \
      p[x] = grd_ipal[i];\
   } while (0)

void gri_flat8_ucline_norm (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
#include "fl8clin.h"
}


#undef macro_plot_rgb
#define macro_plot_rgb(x,p,i) \
   do { \
      p[x] = (long) (((uchar*) parm)[(grd_ipal[i])]); \
   } while (0)

void gri_flat8_ucline_clut (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
#include "fl8clin.h"
}

#undef macro_plot_rgb
#define macro_plot_rgb(x,p,i) \
   do { \
      p[x] = p[x] ^ (grd_ipal[i]); \
   } while (0)

void gri_flat8_ucline_xor (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
#include "fl8clin.h"
}


/* punt */
#undef macro_plot_rgb
#define macro_plot_rgb(x,p,i) \
   do { \
      p[x] = (long) (((uchar*) parm)[(grd_ipal[i])]); \
   } while (0)

void gri_flat8_ucline_blend (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
#include "fl8clin.h"
}
