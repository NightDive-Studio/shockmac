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
#ifndef __OBJVER_H
#define __OBJVER_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/objver.h $
 * $Revision: 1.67 $
 * $Author: xemu $
 * $Date: 1994/11/21 21:05:31 $
 *
 */


// v19 - v20 :  removed objRefs from the save format, added sidestep to critter data
// v20 - v21 :  switched over to individual resources rather than a compound resource, to cut memory req in 3
// v21 - v22 :  switch to pad-less objRefs
// v22 - v23 :  go from 12 byte to 10 byte objrefs
// v23 - v24 :  a massive sociopolitical upheaval where the empowered majority
//                of the Bigstuffs makes a daring coup, boldly seizing the resources
//                once held by its smaller brethren.  Most noticeably, the power of the Traps
//                and the Containers are severely cut back by clever political infighting.
// v24 - v25 : after the fallout of the big v23/24 conflict, the Bigstuffs further consolidate their
//                power, going to 176 instead of a mere 160.
// v25 - v26 : smaller objinfo structs, to save some memory
// v26 - v27 : the incredible shrinking objinfo continues, this time as physhandles lose 3
//             bytes without dieting or exercise.               
#define OBJECT_VERSION_NUMBER  ((int)27)

// v39 - v40 : eliminated bitmap_2d
// v40 - v41 : explosions have a frame_explode - and got rid of useless combat data
// v41 - v42 : oops - damage for attacks should be an int not a ubyte.....
// v42 - v43 : hey - critters throw grenades
// v43 - v44 : slow projectiles have a light flag now
// v44  - v45: critters with slow projectile attacks have y offsets
#define OBJPROP_FILENAME   "objprop.dat"
#define OBJPROP_VERSION_NUMBER   45

// 7  - 8  : Added automap strings
// 8  - 9  : automap strings, for real this time, honest
// 9  - 10 : added an EDMS_State for the player
// 10 - 11 : added pathfinding data
// 11 - 12 : animlist and h_sems added
// 12 - 13 : rev 2 of the animlist
#define MISC_SAVELOAD_VERSION_NUMBER 13

#endif // __OBJVER_H


