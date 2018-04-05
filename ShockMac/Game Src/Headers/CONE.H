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
#ifndef __CONE_H
#define __CONE_H

/*
 * $Source: n:/project/cit/src/inc/RCS/cone.h $
 * $Revision: 1.8 $
 * $Author: dc $
 * $Date: 1994/01/02 17:15:58 $
 *
 * $Log: cone.h $
 * Revision 1.8  1994/01/02  17:15:58  dc
 * indoor terrain renderer
 * 
 * Revision 1.7  1993/11/16  16:26:05  minman
 * got rid of testing prototypes so there's not
 * a depnedency on this file from menus.c
 * 
 * Revision 1.6  1993/11/16  16:22:43  minman
 * redid find_view_area prototype
 * 
 * Revision 1.5  1993/10/19  19:40:08  minman
 * added simple_cone_clip_pass
 * 
 * Revision 1.4  1993/09/02  23:07:14  xemu
 * angle me baby
 * 
 * Revision 1.3  1993/07/01  01:03:52  minman
 * added cone test
 * 
 * Revision 1.2  1993/06/24  01:58:49  minman
 * find_view_area takes a radius argument now
 * 
 * Revision 1.1  1993/06/17  20:13:19  minman
 * Initial revision
 * 
 *
 */

// Includes

// finds the view area polygon - modifies first argument to become
// an array of fix points that represents the view area
// polygon is in clockwise order
// x,y order - *count is the number of points in the polygon.
// return TRUE if it's a valid polygon (not one or two points)

bool find_view_area(fix *cone_list, fix fix_floor, fix fix_roof, int *count, fix radius);

// run the cone clip and render it.
void simple_cone_clip_pass(void);

#ifndef __CONE_SRC
extern fix   span_lines[8];
extern byte  span_index[2];
extern fix   span_intersect[4];
#endif

#endif // __CONE_H
