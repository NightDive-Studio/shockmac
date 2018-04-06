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
#ifndef __SCHEDULE_H
#define __SCHEDULE_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/schedule.h $
 * $Revision: 1.10 $
 * $Author: mahk $
 * $Date: 1994/07/11 14:08:14 $
 *
 */

// Includes
#include "schedtyp.h"

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
   DER SCHEDUMIFIERINEN 
   -----------------------------------------
   The game has a number of schedules, probably one
   in the map and another in the player struct.
   A schedule is a queue of events.  Each event has
   an event type, a timestamp, and some satellite data.
   When a schedule runs, all events whose timestamp is earlier
   than the current time are dispatched to the event handler
   for that type.  The event handlers are stored in a global
   function array indexed on event type.
   Each schedule can have a different notion of time;
   although typically we will schedule on player_struct.game_time.
   
*/

// -----------------------------
// DEFINES
// -------
#define SCHED_DATASIZ 4

typedef struct _sched_event
{
   ushort timestamp;
   ushort type; 
   char data[SCHED_DATASIZ];
} SchedEvent;

typedef void (*SchedHandler)(Schedule* s, SchedEvent* ev);

/// --------------------------------
/// TIMESTAMP CONSTRUCTORS

// Convert a tmd_ticks time to a scheduler timestamp
// When building an event for a schedule that uses
// player_struct.game_time or .real_time, use these
// macros to get scheduler stamps.  
#define TICKS2TSTAMP(t) ((ushort)(((t) >> 4) & 0xFFFF))
#define TSTAMP2TICKS(t) ((t) << 4)

#define NULL_SCHED_EVENT         0
#define GRENADE_SCHED_EVENT      1
#define EXPLOSION_SCHED_EVENT    2
#define DOOR_SCHED_EVENT         3
#define TRAP_SCHED_EVENT         4
#define EXPOSE_SCHED_EVENT       5
#define FLOOR_SCHED_EVENT        6
#define CEIL_SCHED_EVENT         7
#define LIGHT_SCHED_EVENT        8
#define BARK_SCHED_EVENT         9
#define EMAIL_SCHED_EVENT        10

typedef struct _expose_data
{
   byte damage;
   ubyte type;
   ubyte tsecs;
   ubyte count;
} SchedExposeData; 

// Prototypes
errtype schedule_init(Schedule* s,int size, bool grow);
// Initialize a schedule.  If grow is true, the schedule
// will realloc memory when it needs space for more events. 
// size is the number of events the shedule can hold.

errtype schedule_free(Schedule* s);
// Free a schedule.

errtype schedule_event(Schedule* s, SchedEvent* ev);
// Add an event to the specified schedule.  

errtype schedule_run(Schedule* s, ushort time);
// Run the schedule to the specified time, dispatching all events
// which are scheduled for earlier than time.  

errtype schedule_reset(Schedule* s);
// Removes all events from the schedule without dispatching them.


void run_schedules(void);
// Runs all loaded schedules.



// Globals

#endif // __SCHEDULE_H

