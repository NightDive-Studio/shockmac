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
 * $Source: r:/prj/lib/src/2d/RCS/grstate.h $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/10/19 16:22:22 $
 *
 * Declarations for state push/pop.
 *
 * This file is part of the 2d library.
 *
 * $Log: grstate.h $
 * Revision 1.2  1994/10/19  16:22:22  kevin
 * Switch to canvas specific state.
 * 
 * Revision 1.1  1994/10/18  13:34:13  kevin
 * Initial revision
 * 
 * 
 */

#ifndef GRSTATE_H
#define GRSTATE_H

#include "icanvas.h"
#include "tabdat.h"

#define gr_push_state \
   ((int (*)())grd_pixel_table[PUSH_STATE])
#define gr_pop_state \
   ((int (*)())grd_pixel_table[POP_STATE])

#endif

