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
 * $Source: r:/prj/cit/src/inc/RCS/frquad.h $
 * $Revision: 1.2 $
 * $Author: dc $
 * $Date: 1994/09/05 06:43:05 $
 *
 * Citadel Renderer
 *  quadrant defines and layout, flags, etc...
 */

/* note: 9 is unused
 *
 *     1 B 3
 *   0       2
 *   8   D   A
 *   4       6
 *     5 C 7
 */

#define QUAD_N_BASE 0
#define QUAD_S_BASE 4
#define QUAD_A_BASE 8
#define QUAD_X_OFF  2
#define QUAD_D_OFF  1
#define QUAD_CENTER 0xB

/*
 *     1 0 2
 *   B       4
 *   9   C   3
 *   A       5
 *     8 6 7
 */

#define QUAD2_BASE       0
#define QUAD2_RIGHT_FORK 1
#define QUAD2_LEFT_FORK  2
#define QUAD2_DELTA      3
#define QUAD2_CENTER   0xC

// masks for quadrant/octant free facing check
#define FMK_NW         (1<<0)
#define FMK_EW         (1<<1)
#define FMK_SW         (1<<2)
#define FMK_WW         (1<<3)

#define FMK_INT_NW     (1<<(4+0))
#define FMK_INT_EW     (1<<(4+1))
#define FMK_INT_SW     (1<<(4+2))
#define FMK_INT_WW     (1<<(4+3))
