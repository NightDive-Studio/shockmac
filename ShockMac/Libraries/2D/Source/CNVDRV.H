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
 * $Source: r:/prj/lib/src/2d/RCS/cnvdrv.h $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/10/04 18:41:34 $
 *
 * Declarations for canvas tables.
 *
 * This file is part of the 2d library.
 *
 * $Log: cnvdrv.h $
 * Revision 1.3  1994/10/04  18:41:34  kevin
 * *** empty log message ***
 * 
 * Revision 1.2  1993/11/15  03:31:14  baf
 * Added generic canvas table.
 * 
 * Revision 1.1  1993/10/20  15:45:33  kaboom
 * Initial revision
 * 
 */

#ifndef __CNVDRV_H
#define __CNVDRV_H
extern void (*flat8_canvas_table[])();
extern void (*flat8d_canvas_table[])();
extern void (*modex_canvas_table[])();
extern void (*bank8_canvas_table[])();
extern void (*bank24_canvas_table[])();
extern void (*span_canvas_table[])();
extern void (*gen_canvas_table[])();
#endif
