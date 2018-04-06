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
#ifndef __GRENADES_H
#define __GRENADES_H

/*
 * $Source: n:/project/cit/src/inc/RCS/grenades.h $
 * $Revision: 1.17 $
 * $Author: minman $
 * $Date: 1994/06/20 22:59:39 $
 *
 */

// Includes
#include "objects.h"
#include "objclass.h"

// Defines

// type flags
#define     GREN_CONTACT_TYPE       0x01
#define     GREN_MOTION_TYPE        0x02
#define     GREN_TIMING_TYPE        0x04
#define     GREN_MINE_TYPE          0x08

// instance flags
#define     GREN_ACTIVE_FLAG        0x01
#define     GREN_DUD_FLAG           0x02
#define     GREN_MINE_STILL         0x04

typedef struct
{
   ushort   timestamp;
   ushort   type;
   ObjID    gren_id;
   ubyte    unique_id;
   ubyte    filler;
} GrenSchedEvent;

typedef struct
{
   fix      radius;
   fix      radius_change;
   int      damage_mod;
   int      damage_change;
   int      dtype;
   fix      knock_mass;
   ubyte    offense;
   ubyte    penet;
} ExplosionData;

#define SMALL_GAME_EXPL    0
#define MEDIUM_GAME_EXPL   1
#define LARGE_GAME_EXPL    2
#define GAME_EXPLS         3
extern ExplosionData game_explosions[GAME_EXPLS];

#define UNIQUE_LIMIT    255

// Prototypes

// Get the name for a particular grenade type.
char* get_grenade_name(int gtype, char* buf);

// this will activate the grenade - also set the timing stuff if it's a timing grenade
// give it the SpecID of the grenade
void activate_grenade(ObjSpecID osid);

// do_explosion()
// will explode the grenade with id, and it'll show a special effect if arg is TRUE
void do_explosion(ObjLoc loc, ObjID exclusion, ubyte special_effect, ExplosionData *attack_data);

//void do_explosion(ObjLoc loc, ObjID exclusion, ubyte special_effect, fix radius, fix radius_change, int damage_mod, int damage_change, int dtype, fix knock_mass, fix knock_speed, ubyte offense,ubyte penet);/
//void do_explosion(ObjID id, bool special_effect);

void do_grenade_explosion(ObjID id, bool special_effect);

void grenade_stopped(ObjID id);
void grenade_contact(ObjID id, int severity);

#define GRENADE_COLOR WHITE

#endif // __GRENADES_H

