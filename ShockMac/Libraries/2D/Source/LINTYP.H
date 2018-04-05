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
 *  $Source: r:/prj/lib/src/2d/RCS/lintyp.h $
 *  $Revision: 1.3 $
 *  $Author: kevin $
 *  $Date: 1994/09/06 02:28:40 $
 */

#ifndef __LINTYP_H
#define __LINTYP_H

#include "plytyp.h"
#include "line.h"
#include "fill.h"


/* The function pointer is actutally a ptr to void, because it contains 
   two different kinds of pointers.  Don't use a union since I want to 
   be able to initialize as automatic.

   LINES, ILINES, ULINES and CLINES take a vertex interface (fix pt coord)
   
   For these, the conversion to vertex is cheap relative to the overall
   cost and needed for rgb and i values.  (ILINES is converted, it does
   fix-pt internally.)

   HLINES and VLINES take an xy interface   (short coord)
   
   For these, these take short args and are used for speed and 
   for lots of calulations on the arglements (e.g. menues).
   Unfortunately, they now take 5 params (3 points and 2 parms).
   
   Fill parm last, hpoing compiler notices it's not always used.

*/

typedef
   void *grt_uline_fill;

typedef
   void (*grt_uline_fill_v) (long, long, grs_vertex *, grs_vertex *);

typedef
   void (*grt_uline_fill_xy) (short, short, short, long, long);

typedef
   void (*grt_wire_poly_uline) (long, long, grs_vertex *, grs_vertex *);

typedef
   void (*grt_wire_poly_ucline) (long, long, grs_vertex *, grs_vertex *);

typedef 
    grt_uline_fill grt_uline_fill_table[GRD_FILL_TYPES][GRD_LINE_TYPES]; 

#define grt_wire_poly_usline grt_wire_poly_ucline;

extern grt_uline_fill *grd_uline_fill_vector;

#endif
