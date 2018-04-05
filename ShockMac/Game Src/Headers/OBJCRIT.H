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
#ifndef __OBJCRIT_H
#define __OBJCRIT_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/objcrit.h $
 * $Revision: 1.41 $
 * $Author: minman $
 * $Date: 1994/08/02 22:09:15 $
 *
 *
 */

// Includes
#include "objsim.h"
#include "objclass.h"

typedef struct {
//   COMMON_OBJSPEC_FIELDS;
   union {
		ObjID id;
		ObjSpecID headused;
	};
	union {
		ObjSpecID next;
		ObjSpecID headfree; 
	};
	ObjSpecID prev;
   fix des_heading, des_speed, urgency;
   short wait_frames;
   ushort flags;
   ulong attack_count;     // can attack when game time reaches this
   ubyte ai_mode;
   ubyte mood;
   ubyte orders;
   ubyte current_posture;
   char x1;
   char y1;
   char dest_x;                        // Current destination coordinates
   char dest_y;
   char pf_x;       // where we are currently pathfinding to (what our current step is)
   char pf_y;
   char path_id;    // what pathfinding track we are on
   char path_tries; // how many frames have we been trying to get to the
                    // next step on our pathfinding?
   ObjID loot1, loot2;                 // Some loot to get when we destroy critter
   // Note: Num frames is gotten from object properties
   fix sidestep;
} ObjCritter;

typedef struct {
//   COMMON_OBJSPEC_FIELDS;
   union {
		ObjID id;
		ObjSpecID headused;
	};
	union {
		ObjSpecID next;
		ObjSpecID headfree; 
	};
	ObjSpecID prev;
   fix des_heading, des_speed, urgency;
   short wait_frames;
   short base_time_interval;
   ulong attack_count;     // can attack when game time reaches this
   ubyte ai_mode;
   ubyte mood;
   ubyte orders;
   ubyte current_posture;
   char x1;
   char y1;
   char dest_x;                        // Current destination coordinate
   char dest_y;
   char old_x;    // the bin we were in prior to being in this bin
   char old_y;
   char last_x;   // the bin we were in last ai cycle
   char last_y;
   ObjID loot1, loot2;                 // Some loot to get when we destroy critter
   // Note: Num frames is gotten from object properties
} oldObjCritter;

// --------------------------
// Class typedefs

#define NUM_CRITTER_POSTURES          8

typedef struct _CritterAttack {
   int   damage_type;
   short damage_modifier;
   ubyte offense_value;
   ubyte penetration;
   ubyte attack_mass;
   short attack_velocity;
   ubyte accuracy;
   ubyte att_range;
   int speed;              // Wait this long between attacks .. in game_time units
   int slow_proj;          // what, if any, slow projectile we fire
} CritterAttack;

#define NUM_ALTERNATE_ATTACKS 2
#define MAX_CRITTER_VIEWS     8

typedef struct CritterProp {
   ubyte intelligence;
   CritterAttack attacks[NUM_ALTERNATE_ATTACKS];
   ubyte perception;        // each ai interval that player is seeable, this is percent of detection
   ubyte defense;
   ubyte proj_offset;      // slow projectile offset (y)
//   int speed;            
   int   flags;            // flying??, shield??, fixed point?, does it move??
   bool mirror;            // should it's views be mirrored?
   ubyte frames[NUM_CRITTER_POSTURES];           // number of animation frames.  
   ubyte anim_speed;
   ubyte attack_sound;     // play this when attacking.  -1 for no sound.
   ubyte near_sound;       // play when creature is nearby
   ubyte hurt_sound;       // play when damaged a large percentage
   ubyte death_sound;      // play when dying
   ubyte notice_sound;     // play when it notices the player
   int corpse;             // object triple of thing to put here when we die.
   ubyte views;            // number of views for multi-view postures
   ubyte alt_perc;         // percentage of using alternate attack
   ubyte disrupt_perc;     // chance of being disrupted if hit while attacking
   ubyte treasure_type;    // what kind of loot this critter carries.
   ubyte hit_effect;       // what kind of class of hit effects should we do
   ubyte fire_frame;       // what frame do we fire on
} CritterProp;

// ------------------
// Subclass typedefs

typedef struct MutantCritterProp {
   ubyte dummy;
} MutantCritterProp;

typedef struct RobotCritterProp {
   ubyte backup_weapon;
   ubyte metal_thickness;
} RobotCritterProp;

typedef struct CyborgCritterProp {
   short shield_energy;
} CyborgCritterProp;

#define NUM_VCOLORS  3
typedef struct CyberCritterProp {
   uchar vcolors[NUM_VCOLORS];
   uchar alt_vcolors[NUM_VCOLORS];
} CyberCritterProp;

#define EMPTY_STRUCTS

typedef struct RobobabeCritterProp {
#ifdef EMPTY_STRUCTS
   ubyte dummy;
#endif
} RobobabeCritterProp;

// Quantity defines - subclasses

#define NUM_MUTANT_CRITTER         9
#define NUM_ROBOT_CRITTER          12
#define NUM_CYBORG_CRITTER         7
#define NUM_CYBER_CRITTER          7
// Note that for our purposes, ROBOBABE = PLOT which might in itself tell you something
// significant about our game
#define NUM_ROBOBABE_CRITTER       2

#define NUM_CRITTER     (NUM_MUTANT_CRITTER + NUM_ROBOT_CRITTER + NUM_CYBORG_CRITTER + NUM_CYBER_CRITTER + NUM_ROBOBABE_CRITTER)

// Enumeration of subclasses
//

// Critter
#define CRITTER_SUBCLASS_MUTANT 0
#define CRITTER_SUBCLASS_ROBOT  1
#define CRITTER_SUBCLASS_CYBORG 2
#define CRITTER_SUBCLASS_CYBER  3
#define CRITTER_SUBCLASS_ROBOBABE       4

// Lots of posture stuff...

// view = 0-7 side views at angles
// view = 8 top
// view = 9 bottom

#define FRONT_VIEW      6

#define STANDING_CRITTER_POSTURE      0
#define MOVING_CRITTER_POSTURE        1
#define ATTACKING_CRITTER_POSTURE     2
#define ATTACK_REST_CRITTER_POSTURE   3
#define KNOCKBACK_CRITTER_POSTURE     4
#define DEATH_CRITTER_POSTURE         5
#define DISRUPT_CRITTER_POSTURE       6  
#define ATTACKING2_CRITTER_POSTURE    7

#define FIRST_FRONT_POSTURE           ATTACKING_CRITTER_POSTURE 
#define DEFAULT_CRITTER_POSTURE       STANDING_CRITTER_POSTURE

// single-view postures
#define CRITTER_ATTACK_BASE      RES_bmCritterAttack_0
#define CRITTER_ATTACK2_BASE     RES_bmCritterAttack2_0
#define CRITTER_ATTACK_REST_BASE RES_bmCritterAttackRest_0
#define CRITTER_DEATH_BASE       RES_bmCritterDeath_0
#define CRITTER_DISRUPT_BASE     RES_bmCritterDisrupt_0
#define CRITTER_KNOCKBACK_BASE   RES_bmCritterKnockback_0

// multi-view postures
#define CRITTER_MOVE_BASE    RES_bmCritterMovement_0
#define CRITTER_STAND_BASE   RES_bmCritterStanding_0

// Properties of subclasses
//

#ifdef __OBJSIM_SRC
CritterProp     CritterProps[NUM_CRITTER];
MutantCritterProp       MutantCritterProps[NUM_MUTANT_CRITTER];
RobotCritterProp        RobotCritterProps[NUM_ROBOT_CRITTER];
CyborgCritterProp       CyborgCritterProps[NUM_CYBORG_CRITTER];
CyberCritterProp        CyberCritterProps[NUM_CYBER_CRITTER];
RobobabeCritterProp     RobobabeCritterProps[NUM_ROBOBABE_CRITTER];
#else
extern CritterProp      CritterProps[NUM_CRITTER];
extern MutantCritterProp        MutantCritterProps[NUM_MUTANT_CRITTER];
extern RobotCritterProp RobotCritterProps[NUM_ROBOT_CRITTER];
extern CyborgCritterProp        CyborgCritterProps[NUM_CYBORG_CRITTER];
extern CyberCritterProp CyberCritterProps[NUM_CYBER_CRITTER];
extern RobobabeCritterProp      RobobabeCritterProps[NUM_ROBOBABE_CRITTER];
#endif

#ifdef __OBJSIM_SRC
ObjCritter objCritters[NUM_OBJECTS_CRITTER];
ObjCritter default_critter;
#else
extern ObjCritter objCritters[NUM_OBJECTS_CRITTER];
extern ObjCritter default_critter;
#endif

#define get_crit_posture(osid) (objCritters[osid].current_posture & 0xF)
#define set_crit_posture(osid,newpos) objCritters[osid].current_posture = (objCritters[osid].current_posture & 0xF0) + newpos

#define get_crit_view(oisd) (objCritters[osid].current_posture >> 8)
#define set_crit_view(osid,newview) objCritters[osid].current_posture = (newview << 8) + (objCritters[osid].current_posture & 0xF)

#endif // __OBJCRIT_H


