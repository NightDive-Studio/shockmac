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
#ifndef __COMBAT_H
#define __COMBAT_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/combat.h $
 * $Revision: 1.21 $
 * $Author: minman $
 * $Date: 1994/07/21 01:45:51 $
 *
 *
 */

// Includes
#include "objects.h"

#define RAYCAST_ATTACK_SIZE         (fix_make(0,0x0400))
#define NO_RAYCAST_KICKBACK_SPEED   (fix_make(0, 0x0100))

// all temporary stuff
typedef struct {
   fix   x;
   fix   y;
   fix   z;
} Combat_Pt;

typedef struct {
   fix            dx;
   fix            dy;
   fix            dz;
   Combat_Pt      origin;
   fix            mass;
   fix            size;
   fix            speed;
   fix            range;
   physics_handle exclusion;
} Combat_Ray;

// ******* RAYCAST FUNCTIONS *************
// the following ray_cast_* functions do the same thing, but allow for different types of input
// returns the first object hit, and returns OBJ_NULL, if ray does not hit any object

// does a ray cast from object src to the ObjLoc dest
ObjID ray_cast_attack(ObjID src, ObjLoc dest, 
         fix bullet_mass, fix bullet_size, fix bullet_speed, fix bullet_range);

// does a ray cast between point src to point dest
ObjID ray_cast_points(ObjID exclusion, Combat_Pt src, Combat_Pt dest,
         fix bullet_mass, fix bullet_size, fix bullet_speed, fix bullet_range);

// does a ray cast from point src in the direction of vector
// returns hit location at src
ObjID ray_cast_vector(ObjID exclusion, Combat_Pt *src, Combat_Pt vector,
         fix bullet_mass, fix bullet_size, fix bullet_speed, fix bullet_range);

// does a ray cast from object src to object dest
ObjID ray_cast_objects(ObjID src, ObjID dest,
         fix bullet_mass, fix bullet_size, fix bullet_speed, fix bullet_range);

// given the point on the 3d view window, returns the vector in that direction to
// be used for ray_tracing
void find_fire_vector(LGPoint *pt, Combat_Pt *vector);

#endif // __COMBAT_H
