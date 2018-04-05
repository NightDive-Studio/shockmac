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
 *  $Source: r:/prj/lib/src/2d/RCS/clpltab.h $
 *  $Revision: 1.2 $
 *  $Author: kevin $
 *  $Date: 1994/08/04 09:45:09 $
 */

/* External defintions for clipped line drawers and line clippers.
   
   In general, there would be a table of clipped line drawers.  Each
   clipped line drawer would call the appropriate line clipper and
   unclipped line drawer in whatever way it needed to for the kind of
   clip.  

   NB: The vlin drawer can't easily call a clipper, since it is
   compiled to be called from an interrupt.  It inlines the routine.
   The hlin drawer also inlines it's clipper, since the fn call 
   overhead is significant.

   (A stecil clipped line drawer might interleave clip and unclipped
   line draw calls, for example.)
*/

#ifndef __CLPLTAB_H
#define __CLPLTAB_H

#include "plytyp.h"

/* functions living in the vector */

extern int gri_line_clip_fill (long, long, grs_vertex *, grs_vertex *);
extern int gri_iline_clip_fill (long, long, grs_vertex *, grs_vertex *);
extern int gri_cline_clip_fill (long c, long parm, grs_vertex *v0, grs_vertex *v1);
extern int gri_sline_clip_fill (long c, long parm, grs_vertex *v0, grs_vertex *v1);
extern int gri_hline_clip_fill (short, short, short, long, long);
extern int gri_vline_clip_fill (short, short, short, long, long);

extern int gri_wire_poly_line_clip_fill (long c, long parm, grs_vertex *v0, grs_vertex *v1);
extern int gri_wire_poly_sline_clip_fill (long c, long parm, grs_vertex *v0, grs_vertex *v1);
extern int gri_wire_poly_cline_clip_fill (long c, long parm, grs_vertex *v0, grs_vertex *v1);

/* actual clippers */
extern int gri_line_clip (grs_vertex *, grs_vertex *);
extern int gri_cline_clip (grs_vertex *, grs_vertex *);
extern int gri_sline_clip (grs_vertex *, grs_vertex *);

/* these are implemented, but are not used */
extern int gri_hline_clip (short *, short *, short *);
extern int gri_vline_clip (short *, short *, short *);

#endif
