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
 * $Source: n:/project/lib/src/2d/RCS/genlin.c $
 * $Revision: 1.13 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 02:26:12 $
 *
 * Generic routines for drawing fixed-point lines.
 *
 * This file is part of the 2d library.
 *
 * $Log: genlin.c $
 * Revision 1.13  1994/06/11  02:26:12  lmfeeney
 * moved unclipped drawer, now contains two versions of
 * clipped line drawer, one for each i\f - call clipper
 * and call unclipped line drawer
 * 
 * Revision 1.12  1994/05/06  18:19:37  lmfeeney
 * rewritten for greater accuracy
 * 
 * Revision 1.11  1993/12/15  11:25:52  kaboom
 * Fixed up problems with not including endpoints and matching up with
 * new polygon scanner.
 * 
 * Revision 1.10  1993/10/19  09:51:15  kaboom
 * Replaced #include "grd.h" with new headers split from grd.h.
 * 
 * Revision 1.9  1993/10/02  01:17:26  kaboom
 * Changed include of clip.h to include of clpcon.h and/or clpfcn.h.
 * 
 * Revision 1.8  1993/06/23  04:56:43  kaboom
 * Put in checks for single-pixel horizontal and vertical lines.
 * 
 * Revision 1.6  1993/06/22  15:14:56  kaboom
 * Now checks to see if final span is empty.
 * 
 * Revision 1.5  1993/06/16  02:00:39  kaboom
 * Fixed gradual precision error.  Last span now explicitly set to x1
 * instead of adding m_inv.
 * 
 * Revision 1.4  1993/04/29  18:40:33  kaboom
 * Changed include of gr.h to smaller more specific grxxx.h.
 * 
 * Revision 1.3  1993/03/03  17:58:48  kaboom
 * Fixed bugs where y was not initialized.
 * 
 * Revision 1.2  1993/03/02  20:34:11  kaboom
 * Changed algorithm to match polygon edge scanner.
 * 
 * Revision 1.1  1993/02/25  23:10:44  kaboom
 * Initial revision
 */

#include <stdlib.h>
#include "ctxmac.h"
#include "clpcon.h"
#include "clpltab.h"
#include "grlin.h"
#include "general.h"

/* draw a clipped fractional-precision line, call the fixed-point line
   drawer with the preferred (v0, v1, fill) interface 
 */

int gen_fix_line (fix x0, fix y0, fix x1, fix y1)
{
   int r;
   grs_vertex v0, v1;

   v0.x = x0; v0.y = y0; 
   v1.x = x1; v1.y = y1; 

   r = grd_line_clip_fill (gr_get_fcolor(), gr_get_fill_parm(), &v0, &v1);

   return r;
}

int gri_line_clip_fill (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
   int r;
   grs_vertex u0, u1;

   u0.x = v0->x; u0.y = v0->y; 
   u1.x = v1->x; u1.y = v1->y; 

   r = gri_line_clip (&u0, &u1);

   if (r != CLIP_ALL) 
     grd_uline_fill (c, parm, &u0, &u1);

   return r;
}

