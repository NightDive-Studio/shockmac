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
 * $Header: n:/project/lib/src/h/RCS/base.h 1.2 1993/09/28 01:12:42 kaboom Exp $
 *
 * Freefall Base header file
 *
 * $Log: base.h $
 * Revision 1.2  1993/09/28  01:12:42  kaboom
 * Converted #include "xxx" to #include <xxx> for watcom.
 * 
 * Revision 1.1  1993/04/16  23:41:10  mahk
 * Initial revision
 * 
 * Revision 1.1  1993/03/22  15:16:00  xemu
 * Initial revision
 * 
 * Revision 1.4  1993/03/02  18:42:42  rex
 * Moved Id and Ref out
 * 
 * Revision 1.3  1992/12/15  13:17:33  rex
 * Included types.h, removed redundant stuff
 * 
 * Revision 1.1  1992/08/31  17:00:22  unknown
 * Initial revision
 */

#ifndef BASE_H
#define BASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <conio.h>
#include <assert.h>

#include <types.h>

typedef struct {
   short x;
   short y;
} Point;

// macros to make comparing Points easier
#define PointsEqual(p1,p2) (!memcmp(&(p1),&(p2),sizeof(Point)))
#define PointSetNull(p) do {(p).x = -1; (p).y = -1;} while (0);
#define PointCheckNull(p) ((p).x == -1 && (p).y == -1)

typedef struct {
   Point ul;
   Point lr;
} Rect;

#endif
