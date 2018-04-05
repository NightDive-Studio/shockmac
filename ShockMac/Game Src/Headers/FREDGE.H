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
 * $Source: n:/project/cit/src/inc/RCS/fredge.h $
 * $Revision: 1.2 $
 * $Author: dc $
 * $Date: 1994/06/13 18:01:56 $
 */

// noway, no how, no nothing, null space
#define MEDGE_NO_TILE            0x0000

// a full wall, end of the line, no further can you go
#define MEDGE_NO_EGRESS          0x0001
#define MEDGE_FULL_WALL          0x0001

// no floor height difference
#define MEDGE_FLAT_CASE          0x0002

// various partial things
#define MEDGE_SMALL_STEP         0x0004
#define MEDGE_LARGE_STEP         0x0008
#define MEDGE_CLIFF_THING        0x0010

// secret bug cases for playtest
#define MEDGE_BOWTIE_CASE        0x1000
#define MEDGE_INTERNAL_CROSSING  0x2000

int get_edge_code(void *mp, int edge);
char *map_get_edge(void *mp, int edge, int ceil_p);

#ifndef __FRTERR_SRC
// left val, right val, "sum" val
extern char edge_vals[3];
#endif

