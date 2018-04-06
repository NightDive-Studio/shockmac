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
 * $Source: r:/prj/lib/src/2d/RCS/wire.h $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/08/04 09:46:20 $
 *
 * Declarations for wire frame primitives.
 *
 * This file is part of the 2d library.
 *
 * $Log: wire.h $
 * Revision 1.3  1994/08/04  09:46:20  kevin
 * Added new wire poly line functionality.
 * 
 * Revision 1.2  1993/10/19  10:31:09  kaboom
 * Updated arguments to be the same as solid polygon.
 * 
 * Revision 1.1  1993/06/02  16:35:23  kaboom
 * Initial revision
 */

#ifndef __WIRE_H
#define __WIRE_H
#include "plytyp.h"

extern void gr_wire_upoly(long c,int n,grs_vertex **vpl);
extern void gr_wire_poly(long c,int n,grs_vertex **vpl);
extern void gr_wire_ucpoly(int n,grs_vertex **vpl);
extern void gr_wire_cpoly(int n,grs_vertex **vpl);
#endif /* __WIRE_H */
