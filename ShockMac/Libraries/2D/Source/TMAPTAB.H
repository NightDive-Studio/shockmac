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
 * $Source: r:/prj/lib/src/2d/RCS/tmaptab.h $
 * $Revision: 1.5 $
 * $Author: kevin $
 * $Date: 1994/08/16 15:39:59 $
 *
 * Declarations for texture mapping inner loop
 * table list globals and lists.
 *
 * This file is part of the 2d library.
 * 
*/

#ifndef __TMAPTAB_H
#define __TMAPTAB_H

#include "tabdat.h"
#define grd_tmap_init_table grd_function_table
#define grd_tmap_hscan_init_table grd_tmap_init_table
#define grd_tmap_vscan_init_table (grd_tmap_init_table+6)

#endif /* !__TMAPTAB_H */


