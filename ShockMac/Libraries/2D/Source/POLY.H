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
 * $Source: r:/prj/lib/src/2d/RCS/poly.h $
 * $Revision: 1.5 $
 * $Author: kevin $
 * $Date: 1994/08/16 15:37:24 $
 *
 * Constants for polygon scanning.
 *
 * This file is part of the 2d library.
 *
 * $Log: poly.h $
 * Revision 1.5  1994/08/16  15:37:24  kevin
 * Added tluc8 loop type.
 * 
 * Revision 1.4  1994/07/21  16:35:36  kevin
 * Added GRL_SOLID loop flag.
 * 
 * Revision 1.3  1994/07/18  17:07:53  kevin
 * Added clut loop flag.
 * 
 * Revision 1.2  1993/10/02  01:03:14  kaboom
 * Added inner loop id constants.
 * 
 * Revision 1.1  1993/08/10  19:18:19  kaboom
 * Initial revision
 */

#ifndef __PLYCON
#define __PLYCON
#define GRD_POLY_VERTS 128             /* maximum polygon vertices */
#define GRL_OPAQUE 0
#define GRL_TRANS 1
#define GRL_LOG2 2
#define GRL_CLUT 4
#define GRL_SOLID 8
#define GRL_TLUC8 16
#endif /* !__PLYCON */
