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
 * $Source: r:/prj/lib/src/2d/RCS/gencwlin.c $
 * $Revision: 1.1 $
 * $Author: kevin $
 * $Date: 1994/08/04 09:53:32 $
 *
 * Routines to clip and draw wire poly lines.
 *
 * This file is part of the 2d library.
 *
 */

#include "clpcon.h"
#include "clpltab.h"
#include "grlin.h"
#include "plytyp.h"

int gri_wire_poly_cline_clip_fill (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
   int r;
   grs_vertex u0, u1;

   /* save inputs (don't really need whole struct) */
   u0 = *v0;   
   u1 = *v1;

   r = gri_cline_clip (&u0, &u1);

   if (r != CLIP_ALL) 
     grd_wire_poly_ucline_fill (c, parm, &u0, &u1);

   return r;
}

int gri_wire_poly_line_clip_fill (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
   int r;
   grs_vertex u0, u1;

   /* save inputs (don't really need whole struct) */
   u0 = *v0;   
   u1 = *v1;

   r = gri_line_clip (&u0, &u1);

   if (r != CLIP_ALL) 
     grd_wire_poly_uline_fill (c, parm, &u0, &u1);

   return r;
}
