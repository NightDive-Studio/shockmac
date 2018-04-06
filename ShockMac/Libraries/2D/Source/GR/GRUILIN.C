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
 *  $Source: n:/project/lib/src/2d/RCS/gruilin.c $
 *  $Revision: 1.1 $
 *  $Author: lmfeeney $
 *  $Date: 1994/06/11 02:37:08 $
 */

#include "fix.h"
#include "clpcon.h"
#include "plytyp.h"
#include "lintyp.h"
#include "clpltyp.h"
#include "ctxmac.h"
#include "grlin.h"
#include "linfcn.h"

/* This just converts to fix-point, then calls the current uline
   drawer, so this OK for all canvases, fill modes, etc.  In general,
   you would prefer to take advantage of nice integer special cases to
   avoid shifts, but this is probably overkill. 
*/

void gri_all_uiline_fill (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
   grs_vertex u0, u1;
     
   u0.x = fix_make(v0->x, 32768); u0.y = fix_make(v0->y, 32768);
   u1.x = fix_make(v1->x, 32768); u1.y = fix_make(v1->y, 32768); 

   grd_uline_fill (c, parm, &u0, &u1);
}

