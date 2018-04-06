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
 * $Source: n:/project/lib/src/2d/RCS/tmaps.h $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/02/26 22:44:02 $
 *
 * Texture mapping public structures.
 *
 * This file is part of the 2d library.
 *
 * $Log: tmaps.h $
 * Revision 1.2  1994/02/26  22:44:02  kevin
 * New stuff for new sloppy linear mappers.
 * 
 * Revision 1.1  1994/01/03  22:03:36  kevin
 * Initial revision
 * 
 * 
*/

/* Perspective mapper context structure. */
#include "grs.h"

#ifndef __TMAPS_H
#define __TMAPS_H

typedef struct {
   short tmap_type;
   short flags;
   uchar *clut;
} grs_tmap_info;

#define TMF_PER 1
#define TMF_CLUT 2
#define TMF_FLOOR 4
#define TMF_WALL 4

#endif /* !__TMAPS_H */


