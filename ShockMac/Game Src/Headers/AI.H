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
#ifndef __AI_H
#define __AI_H

/*
 * $Source: n:/project/cit/src/inc/RCS/ai.h $
 * $Revision: 1.22 $
 * $Author: xemu $
 * $Date: 1994/06/21 02:14:10 $
 *
 *
 */

// Includes
#include "objects.h"

// Defines
#define NUM_AI_MOODS       5
#define AI_MOOD_FRIENDLY   0
#define AI_MOOD_NEUTRAL    1
#define AI_MOOD_HOSTILE    2
#define AI_MOOD_ISOLATION  3
#define AI_MOOD_ATTACKING  4

#define NUM_AI_ORDERS      6
#define AI_ORDERS_GUARD    0     // hang out until player comes around
#define AI_ORDERS_ROAM     1     // wander about
#define AI_ORDERS_SLEEP    2     // do nothing ever until awakened
#define AI_ORDERS_PATROL   3     // back n forth between 2 points
#define AI_ORDERS_HIGHWAY  4     // follow invisible highway
#define AI_ORDERS_NOMOVE   5     // like guard, but will never move

#define AI_FLAG_NONE       0x00
#define AI_FLAG_FLYING     0x01  // we can fly
#define AI_FLAG_NOALERT    0x02  // don't speed up reaction when in combat
#define AI_FLAG_SMALL      0x04  // for musicai

#define SPREAD_DIST        2

#define SLOW_PROJECTILE_DURATION 1000
#define SLOW_PROJECTILE_GRAVITY  fix_make(0,0x0C00)

// Macros

// Prototypes

// External Functions:
// Let all the AI system spend time figuring out what to do.  Those AIs that know what they are doing,
// do it, others plan, the time load is hopefully distributed as nicely as possible
errtype ai_run(void);

// What do do when we are hit
errtype ai_critter_hit(ObjSpecID osid, short damage, bool tranq, bool stun);

// When we pretend that we're dead (pretend we're dead)
errtype ai_critter_die(ObjSpecID osid);
errtype ai_critter_really_dead(ObjSpecID osid);

// actually do attack
errtype ai_attack_player(ObjSpecID osid, char a);

// Change a critter's posture, and do appropriate
// things to other anim variables
errtype set_posture(ObjSpecID osid, ubyte new_pos);

// Looting.
errtype do_regular_loot(ObjSpecID source_critter, ObjID corpse);
errtype do_random_loot(ObjID corpse);

void ai_find_player(ObjID id);
void ai_critter_seen(void);
void ai_misses(ObjSpecID osid);


// Globals

#endif // __AI_H

