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
 * $Source: r:/prj/lib/src/2d/RCS/wirpoly.c $
 * $Revision: 1.4 $
 * $Author: kevin $
 * $Date: 1994/08/04 09:56:12 $
 *
 * Routines to draw wire polys.
 *
 * This file is part of the 2d library.
 *
 */

#include "clpcon.h"
#include "clpltab.h"
#include "grlin.h"
#include "grrend.h"
#include "plytyp.h"
#include "wire.h"

void gr_wire_upoly(long c,int n, grs_vertex **vpl) {
   int i;

   for (i=1;i<n;i++)
      gr_wire_poly_uline(c,vpl[i-1],vpl[i]);
   gr_wire_poly_uline(c,vpl[n-1],vpl[0]);
}

void gr_wire_poly(long c,int n, grs_vertex **vpl) {
   int i;
   grs_vertex v0,v1;

   for (i=1;i<n;i++) {
      v0.x=vpl[i-1]->x,v0.y=vpl[i-1]->y;
      v1.x=vpl[i]->x,v1.y=vpl[i]->y;
      if (gri_line_clip(&v0,&v1)!=CLIP_ALL)
         gr_wire_poly_uline(c,&v0,&v1);
   }
   v0.x=vpl[n-1]->x,v0.y=vpl[n-1]->y;
   v1.x=vpl[0]->x,v1.y=vpl[0]->y;
   if (gri_line_clip(&v0,&v1)!=CLIP_ALL)
      gr_wire_poly_uline(c,&v0,&v1);
}

void gr_wire_ucpoly(int n, grs_vertex **vpl) {
   int i;

   for (i=1;i<n;i++)
      gr_wire_poly_ucline(vpl[i-1],vpl[i]);
   gr_wire_poly_ucline(vpl[n-1],vpl[0]);
}

void gr_wire_cpoly(int n, grs_vertex **vpl) {
   int i;

   for (i=1;i<n;i++)
      gr_wire_poly_cline(vpl[i-1],vpl[i]);
   gr_wire_poly_cline(vpl[n-1],vpl[0]);
}
