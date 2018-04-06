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
#ifndef __TRIGGER_H
#define __TRIGGER_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/trigger.h $
 * $Revision: 1.28 $
 * $Author: xemu $
 * $Date: 1994/11/21 06:19:41 $
 *
 *
 */

// Includes
#include "objects.h"

// Defines
#define ENTRY_TRIGGER_TYPE              0
#define NULL_TRIGGER_TYPE               1
#define FLOOR_TRIGGER_TYPE              2
#define PLAYER_DEATH_TRIGGER_TYPE       3
#define DEATHWATCH_TRIGGER_TYPE         4
#define AREA_ENTRY_TRIGGER_TYPE         5
#define AREA_CONTINUOUS_TRIGGER_TYPE    6



// Typedefs
typedef struct
{
   ushort timestamp;
   ushort type;
   ObjID target_id;
   ObjID source_id;
} TrapSchedEvent;

#define HEIGHT_STEP_TIME 3
#define HEIGHT_TIME_UNIT 10

#define NUM_HEIGHT_SEMAPHORS 16
#define MAX_HSEM_KEY 63

typedef struct
{
   ushort timestamp;
   ushort type;
   char semaphor;
   char key;
   char steps_remaining;
   char sfx_code;
} HeightSchedEvent;

// sfx_codes -- 
// 0x1 for no terrain sound

typedef struct {
   uchar x;
   uchar y;
   uchar floor:1;
   uchar key:7;
   char inuse;
} height_semaphor;

typedef struct _EmailSchedEvent
{
   ushort timestamp;
   ushort type;
   short datamunge;
} EmailSchedEvent;

// Prototypes

// Somewhere in the city, an object has been destroyed.  Was it a
// destroy trigger?  YOU be the judge.
errtype trigger_check_destroyed(ObjID id);

// Trap/Trigger identified by id might have been set off -- player
// just entered it's square.  Deal appropriately.
errtype location_trigger_activate(ObjID id);

// Trap/Trigger identified by id should actually go off.
// return value is whether or not the trap beneath the trigger
// actually went off. use_message is a pointer to a boolean
// to set if the trap utilizes the message line.
bool trap_activate(ObjID id, bool *use_message);

#define is_trap(id) (objs[(id)].class == CLASS_TRAP)


// Use these functions to directly access trap-like functions
errtype trap_teleport_func(int targ_x, int targ_y, int targ_z, int targlevel);
errtype trap_scheduler_func(int p1, int p2, int p3, int p4);
errtype trap_lighting_func(bool floor, int p1, int p2, int p3, int p4);
errtype trap_damage_func(int p1, int p2, int p3, int p4);
errtype trap_create_obj_func(int p1, int p2, int p3, int p4);
errtype trap_questbit_func(int p1, int p2, int p3, int p4);
errtype trap_cutscene_func(int p1, int p2, int p3, int p4);
errtype trap_terrain_func(int p1, int p2, int p3, int p4);
errtype trap_sfx_func(int p1, int p2, int p3, int p4);

errtype check_deathwatch_triggers(ObjID id, bool really_dead);
errtype check_entrance_triggers(uchar old_x, uchar old_y, uchar new_x, uchar new_y);

errtype do_multi_stuff(ObjID id);

// Globals

extern char *trapname_strings[];

#endif // __TRIGGER_H

