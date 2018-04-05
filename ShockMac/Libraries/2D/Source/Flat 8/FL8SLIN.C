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
 * $Source: n:/project/lib/src/2d/RCS/fl8slin.c $
 * $Revision: 1.5 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 01:24:10 $
 *
 * Routine to draw a gouraud shaded line to a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8slin.c $
 * Revision 1.5  1994/06/11  01:24:10  lmfeeney
 * guts of the routine moved to fl8{c,s}lin.h, per fill type
 * line drawers are created by defining macros and including
 * this file
 * 
 * Revision 1.4  1994/05/06  18:19:13  lmfeeney
 * rewritten for greater accuracy and speed
 * 
 * Revision 1.3  1993/10/19  09:50:58  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.2  1993/10/01  16:01:20  kaboom
 * Pared down includes to reduce dependencies.
 * 
 * Revision 1.1  1993/07/01  22:12:15  spaz
 * Initial revision
 */

#include <stdlib.h>
#include "cnvdat.h"
#include "plytyp.h"
#include "linfcn.h"

/* not all fill routines use all parms */
// MLA #pragma off (unreferenced)

#undef macro_plot_i
#define macro_plot_i(x,p,i) \
do { \
    p[x] = i; \
  } while (0)

void gri_flat8_usline_norm (long c, long parm, grs_vertex *v0,  grs_vertex *v1)
{
#include "fl8slin.h"
}

#undef macro_plot_i
#define macro_plot_i(x,p,i) \
do { \
    p[x] = (long) (((uchar *) parm)[i]); \
   } while (0)

void gri_flat8_usline_clut (long c, long parm, grs_vertex *v0,  grs_vertex *v1)
{
#include "fl8slin.h"
}

#undef macro_plot_i
#define macro_plot_i(x,p,i) \
do {\
    p[x] = p[x] ^ i; \
  } while (0)

void gri_flat8_usline_xor (long c, long parm, grs_vertex *v0,  grs_vertex *v1)
{
#include "fl8slin.h"
}

/* punt */

#undef macro_plot_i
#define macro_plot_i(x,p,i)  \
do { \
   p[x] = i; \
   } while (0)

void gri_flat8_usline_blend (long c, long parm, grs_vertex *v0,  grs_vertex *v1)
{
#include "fl8slin.h"
}
