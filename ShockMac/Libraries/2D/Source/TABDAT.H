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
 * $Source: r:/prj/lib/src/2d/RCS/tabdat.h $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/08/16 15:38:58 $
 *
 * Declarations for function tables.
 *
 * This file is part of the 2d library.
 *
 * $Log: tabdat.h $
 * Revision 1.2  1994/08/16  15:38:58  kevin
 * new function table declataions.
 * 
 * Revision 1.1  1993/10/19  10:30:49  kaboom
 * Initial revision
 * 
 */

#ifndef __TABDAT_H
#define __TABDAT_H
extern void (**grd_pixel_table)();
extern void (**grd_device_table)();
extern void (**grd_canvas_table)();
extern void (**grd_function_table)();
#endif /* !__TABDAT_H */
