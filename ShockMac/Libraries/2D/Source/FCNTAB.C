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
 * $Source: r:/prj/lib/src/2d/RCS/fcntab.c $
 * $Revision: 1.4 $
 * $Author: kevin $
 * $Date: 1994/12/05 21:06:38 $
 *
 * Function table lists and globals.
 *
 * This file is part of the 2d library.
 */

#include "tabdrv.h"

void (**grd_function_table)();
grt_function_table *grd_function_fill_table;
grt_function_table *grd_function_table_list[] = {
   NULL,                 /* device driver-initialized by gr_set_mode */
   NULL,                 /* monochrome-not supported */
   (grt_function_table *) flat8_function_table, /* flat 8 canvas */
   NULL,                 /* flat 24-not supported */
   NULL,                 /* doubling, RSD8 canvas-not supported */
   NULL,                 /* translucent 8-not supported */
   NULL,                 /* span-obsolete */
   NULL                  /* generic-initialized by gr_force_generic */
};
