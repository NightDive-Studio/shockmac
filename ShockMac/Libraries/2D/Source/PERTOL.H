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
 * $Source: n:/project/lib/src/2d/RCS/pertol.h $
 * $Revision: 1.1 $
 * $Author: kevin $
 * $Date: 1994/01/16 12:01:31 $
 *
 * Structures, constants, and prototypes for
 * perspective detail setting procedures.
 * $Log: pertol.h $
 * Revision 1.1  1994/01/16  12:01:31  kevin
 * Initial revision
 * 
*/

#ifndef __PERTOL_H
#define __PERTOL_H

#include "fix.h"

typedef struct {
   ubyte ltol, wftol;
   fix cltol;
} gr_per_detail_level;

enum {
   GR_LOW_PER_DETAIL,
   GR_MEDIUM_PER_DETAIL,
   GR_HIGH_PER_DETAIL,
   GR_NUM_PER_DETAIL_LEVELS
};

extern void gr_set_per_tol(ubyte linear_tol, ubyte wall_floor_tol);
extern void gr_set_clut_lit_tol(fix clut_lit_tol);
extern void gr_set_per_detail_level(int detail_level);
extern void gr_set_per_detail_level_param
   (int linear_tol, int wall_floor_tol, fix clut_lit_tol, int detail_level);

#endif
