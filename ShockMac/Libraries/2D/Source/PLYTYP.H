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
 * $Source: n:/project/lib/src/2d/RCS/plytyp.h $
 * $Revision: 1.2 $
 * $Author: kaboom $
 * $Date: 1993/10/19 10:24:09 $
 *
 * Polygon-related structures.
 *
 * This file is part of the 2d library.
 *
 * $Log: plytyp.h $
 * Revision 1.2  1993/10/19  10:24:09  kaboom
 * Includes fix.h for self-sufficency.
 * 
 * Revision 1.1  1993/10/02  01:12:47  kaboom
 * Initial revision
 */

#ifndef __PLYTYP
#define __PLYTYP
#include "fix.h"

/* format for vertex buffers. */
typedef struct {
   fix x, y;                  /* screen coordinates */
   fix u, v, w;               /* texture parameters/rgb */
   fix i;                     /* intensity */
} grs_vertex;
#endif /* !__PLYTYP */
