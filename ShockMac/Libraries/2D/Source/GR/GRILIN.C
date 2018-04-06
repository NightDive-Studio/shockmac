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
 * $Source: n:/project/lib/src/2d/RCS/grilin.c $
 * $Revision: 1.4 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 02:29:21 $
 *
 * Integral line drawers.  Converts integers to fixed-point and calls
 * normal line drawers.
 *
 * This file is part of the 2d library.
 *
 * $Log: grilin.c $
 * Revision 1.4  1994/06/11  02:29:21  lmfeeney
 * provides both interfaces to unclipped line drawer, unclipped
 * drawer moved
 * 
 * Revision 1.3  1993/10/08  01:16:03  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.2  1993/04/29  18:49:38  kaboom
 * Changed include of old gr.h to new grrend.h.
 * 
 * Revision 1.1  1993/03/29  18:44:53  kaboom
 * Initial revision
 */

#include "fix.h"
#include "clpcon.h"
#include "plytyp.h"
#include "lintyp.h"
#include "clpltyp.h"
#include "clpltab.h"
#include "ctxmac.h"
#include "grlin.h"

// prototypes
int gr_int_line (short x0, short y0, short x1, short y1);


int gr_int_line (short x0, short y0, short x1, short y1)
{
   int r;
   grs_vertex v0, v1;

   v0.x = x0;  v0.y = y0;      /* we don't need no stinking type checking */
   v1.x = x1;  v1.y = y1;
   
   r = grd_iline_clip_fill (gr_get_fcolor(), gr_get_fill_parm(), &v0, &v1);
   
   return r;
}

int gri_iline_clip_fill (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
   int r;
   grs_vertex u0, u1;
  
   u0.x = fix_make(v0->x, 32768); u0.y = fix_make(v0->y, 32768);
   u1.x = fix_make(v1->x, 32768); u1.y = fix_make(v1->y, 32768); 
   
   r = gri_line_clip (&u0, &u1);

   if (r != CLIP_ALL) 
      grd_uline_fill (c, parm, &u0, &u1);

   return r;
}
   
