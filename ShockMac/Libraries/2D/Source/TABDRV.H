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
 * $Source: r:/prj/lib/src/2d/RCS/tabdrv.h $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/10/04 18:44:24 $
 *
 * Declarations for function tables.
 *
 * This file is part of the 2d library.
 *
 * $Log: tabdrv.h $
 * Revision 1.2  1994/10/04  18:44:24  kevin
 * Added #define for doubling canvas in case we might someday use it.
 * 
 * Revision 1.1  1994/08/16  15:39:30  kevin
 * Initial revision
 * 
 */

#ifndef __TABDRV_H
#define __TABDRV_H

#include "bitmap.h"
#include "fill.h"
#include "ifcn.h"

typedef void (*grt_function_table[GRD_FILL_TYPES][GRD_FUNCS*REAL_BMT_TYPES])(); 

extern grt_function_table gen_function_table;
extern grt_function_table flat8_function_table;
extern grt_function_table flat8d_function_table;
extern grt_function_table modex_function_table;
extern grt_function_table bank8_function_table;
extern grt_function_table bank24_function_table;

#endif /* __TABDRV_H */
