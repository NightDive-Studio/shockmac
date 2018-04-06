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
 * $Source: n:/project/lib/src/2d/RCS/grcply.h $
 * $Revision: 1.4 $
 * $Author: baf $
 * $Date: 1994/02/14 20:38:39 $
 *
 * Dispatch macros for color shaded polygon routines.
 *
 * This file is part of the 2d library.
 *
 * $Log: grcply.h $
 * Revision 1.4  1994/02/14  20:38:39  baf
 * Added dummy parameter to cpoly and spoly routines, for uniformity needed by 3D.
 * 
 * Revision 1.3  1993/11/29  20:27:39  baf
 * General revisions and tidying/up of translucency
 * and shading routines
 * 
 * Revision 1.2  1993/11/23  17:52:45  baf
 * Added tluc/8 interpolation
 * 
 * Revision 1.1  1993/10/19  10:16:55  kaboom
 * Initial revision
 * 
 */

#ifndef __GRCPLY_H
#define __GRCPLY_H
#include "icanvas.h"
#include "plytyp.h"
#include "tabdat.h"

#define gr_ucpoly \
   ((void (*)(long c, int n,grs_vertex **vpl))grd_canvas_table[FIX_UCPOLY])
#define gr_cpoly \
   ((int (*)(long c, int n,grs_vertex **vpl))grd_canvas_table[FIX_CPOLY])
#endif /* !__GRCPLY_H */
