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
#include "gamescr.h"

#define MAX_SHODAN_LEVEL      120
// a little less than a half-second per level
#define SHODAN_TIME_SHIFT     7
#define SHODAN_CONQUER_REF              REF_IMG_bmSHODANEndgame
#define SHODAN_FULLSCRN_CONQUER_REF     REF_IMG_bmSHODANEndgameFull
#define SHODAN_BITMASK_SIZE   (320 * 200)
#define SHODAN_INTERVAL    CIT_CYCLE >> 4

#define LOWER_SHODAN_X (full_game_3d) ? 15  : 5 
#define UPPER_SHODAN_X (full_game_3d) ? 305 : 263
#define LOWER_SHODAN_Y (full_game_3d) ? 15  : 5
#define UPPER_SHODAN_Y (full_game_3d) ? 185 : 108

#define SHODAN_CONQUER_GET(arr, i) (arr[i >> 3] & (1 << (i & 0x7)))
#define SHODAN_CONQUER_SET(arr, i) (arr[i >> 3] |= (1 << (i & 0x7)))
#define SHODAN_CONQUER_UNSET(arr, i) (arr[i >> 3] &= ~(1 << (i & 0x7)))
