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
#ifndef __OBJGAME_H 
#define __OBJGAME_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/objgame.h $
 * $Revision: 1.52 $
 * $Author: minman $
 * $Date: 1994/07/30 00:19:06 $
 *
 */

// Includes
#include "objclass.h"

// Instance Typedefs
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
   ubyte trap_type;
   ubyte destroy_count;
   uint comparator;
   uint p1,p2,p3,p4;
   short access_level;
} ObjFixture;

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
   short locked;     
   ubyte stringnum;
   ubyte cosmetic_value;
   ubyte access_level;
   ubyte autoclose_time;
   ObjID other_half;    
} ObjDoor;

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
   ubyte start_frame;
   ubyte end_frame;
   ObjID owner;
} ObjAnimating;

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
   ubyte trap_type;
   ubyte destroy_count;
   uint comparator;
   uint p1,p2,p3,p4;
} ObjTrap;

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
   int contents1;
   int contents2;
   ubyte dim_x;
   ubyte dim_y;
   ubyte dim_z;
   int data1;
} ObjContainer;

// Class typedefs
typedef struct FixtureProp {
   ubyte characteristics;
} FixtureProp;

typedef struct DoorProp {
   ubyte security_level;         // i.e difficulty to unlock
} DoorProp;

#define ANIM_FLAG_NONE     0
#define ANIM_FLAG_REPEAT   1
#define ANIM_FLAG_REVERSE  2

typedef struct AnimatingProp {
   ubyte    speed;
   ubyte    flags;
} AnimatingProp;

typedef struct TrapProp {
   ubyte dummy;
} TrapProp;

typedef struct ContainerProp {
   ObjID contents;           // obviously not the way to do it, but you get the idea
   ubyte num_contents;
} ContainerProp;

// Subclass typedefs
typedef struct ControlFixtureProp {
   ubyte dummy;
} ControlFixtureProp;

typedef struct ReceptacleFixtureProp {
   ubyte dummy;
} ReceptacleFixtureProp;

typedef struct TerminalFixtureProp {
   ubyte dummy;
} TerminalFixtureProp;

typedef struct PanelFixtureProp {
   ubyte dummy;
} PanelFixtureProp;

typedef struct VendingFixtureProp {
   ubyte dummy;
} VendingFixtureProp;

typedef struct CyberFixtureProp {
   ubyte dummy;
} CyberFixtureProp;

typedef struct _NormalDoorProp {
   ubyte dummy;
} NormalDoorProp;

typedef struct _DoorwaysDoorProp {
   ubyte dummy;
} DoorwaysDoorProp;

typedef struct _ForceDoorProp {
   ubyte dummy;
} ForceDoorProp;

typedef struct _ElevatorDoorProp {
   ubyte dummy;
} ElevatorDoorProp;

typedef struct _SpecialDoorProp {
   ubyte dummy;
} SpecialDoorProp;

typedef struct _ObjectsAnimatingProp {
   ubyte dummy;
} ObjectsAnimatingProp;

typedef struct _TransitoryAnimatingProp {
   ubyte dummy;
} TransitoryAnimatingProp;

typedef struct _ExplosionAnimatingProp {
   ubyte    frame_explode;
} ExplosionAnimatingProp;

typedef struct _TriggerTrapProp {
   ubyte dummy;
} TriggerTrapProp;

typedef struct _FeedbacksTrapProp {
   ubyte dummy;
} FeedbacksTrapProp;

typedef struct _SecretTrapProp {
   ubyte dummy;
} SecretTrapProp;

typedef struct _ActualContainerProp {
   ubyte dummy;
} ActualContainerProp;

typedef struct _WasteContainerProp {
   ubyte dummy;
} WasteContainerProp;

typedef struct _LiquidContainerProp {
   ubyte dummy;
} LiquidContainerProp;

typedef struct _MutantCorpseContainerProp {
   ubyte dummy;
} MutantCorpseContainerProp;

typedef struct _RobotCorpseContainerProp {
   ubyte dummy;
} RobotCorpseContainerProp;

typedef struct _CyborgCorpseContainerProp {
   ubyte dummy;
} CyborgCorpseContainerProp;

typedef struct _OtherCorpseContainerProp {
   ubyte dummy;
} OtherCorpseContainerProp;

// Quantity defines - subclasses
// Fixture
#define NUM_CONTROL_FIXTURE 9
#define NUM_RECEPTACLE_FIXTURE 7
#define NUM_TERMINAL_FIXTURE 3
#define NUM_PANEL_FIXTURE 11
#define NUM_VENDING_FIXTURE 2
#define NUM_CYBER_FIXTURE 3

// Door
#define NUM_NORMAL_DOOR 10
#define NUM_DOORWAYS_DOOR 9
#define NUM_FORCE_DOOR 7
#define NUM_ELEVATOR_DOOR 5
#define NUM_SPECIAL_DOOR 10

// Animating
#define NUM_OBJECT_ANIMATING 9
#define NUM_TRANSITORY_ANIMATING 11
#define NUM_EXPLOSION_ANIMATING 14

// Trap
#define NUM_TRIGGER_TRAP 13
#define NUM_FEEDBACKS_TRAP 1
#define NUM_SECRET_TRAP 5

// Container
#define NUM_ACTUAL_CONTAINER 3
#define NUM_WASTE_CONTAINER 3
#define NUM_LIQUID_CONTAINER 4
#define NUM_MUTANT_CORPSE_CONTAINER 8
#define NUM_ROBOT_CORPSE_CONTAINER 13
#define NUM_CYBORG_CORPSE_CONTAINER 7
#define NUM_OTHER_CORPSE_CONTAINER 8

#define NUM_FIXTURE	(NUM_CONTROL_FIXTURE + NUM_RECEPTACLE_FIXTURE + NUM_TERMINAL_FIXTURE + NUM_PANEL_FIXTURE + NUM_CYBER_FIXTURE + NUM_VENDING_FIXTURE)
#define NUM_DOOR	(NUM_NORMAL_DOOR + NUM_DOORWAYS_DOOR + NUM_FORCE_DOOR + NUM_ELEVATOR_DOOR + NUM_SPECIAL_DOOR)
#define NUM_ANIMATING	(NUM_OBJECT_ANIMATING + NUM_TRANSITORY_ANIMATING + NUM_EXPLOSION_ANIMATING)
#define NUM_TRAP	(NUM_TRIGGER_TRAP + NUM_FEEDBACKS_TRAP + NUM_SECRET_TRAP)
#define NUM_CONTAINER	(NUM_ACTUAL_CONTAINER + NUM_WASTE_CONTAINER + NUM_LIQUID_CONTAINER + NUM_MUTANT_CORPSE_CONTAINER + NUM_ROBOT_CORPSE_CONTAINER + NUM_CYBORG_CORPSE_CONTAINER + NUM_OTHER_CORPSE_CONTAINER) 

// Enumeration of subclasses
// Fixture
#define FIXTURE_SUBCLASS_CONTROL	0
#define FIXTURE_SUBCLASS_RECEPTACLE	1
#define FIXTURE_SUBCLASS_TERMINAL 2
#define FIXTURE_SUBCLASS_PANEL 3
#define FIXTURE_SUBCLASS_VENDING 4
#define FIXTURE_SUBCLASS_CYBER 5

// Door
#define DOOR_SUBCLASS_NORMAL 0
#define DOOR_SUBCLASS_DOORWAYS 1
#define DOOR_SUBCLASS_FORCE 2
#define DOOR_SUBCLASS_ELEVATOR 3
#define DOOR_SUBCLASS_SPECIAL 4

// Animating
#define ANIMATING_SUBCLASS_OBJECTS 0
#define ANIMATING_SUBCLASS_TRANSITORY 1
#define ANIMATING_SUBCLASS_EXPLOSION 2

// Trap
#define TRAP_SUBCLASS_TRIGGER	0
#define TRAP_SUBCLASS_FEEDBACKS 1
#define TRAP_SUBCLASS_SECRET 2

// Container
#define CONTAINER_SUBCLASS_ACTUAL 0
#define CONTAINER_SUBCLASS_WASTE 1
#define CONTAINER_SUBCLASS_LIQUID 2
#define CONTAINER_SUBCLASS_MUTANT_CORPSE 3
#define CONTAINER_SUBCLASS_ROBOT_CORPSE 4
#define CONTAINER_SUBCLASS_CYBORG_CORPSE 5
#define CONTAINER_SUBCLASS_OTHER_CORPSE 6 

#ifdef __OBJSIM_SRC
FixtureProp	FixtureProps[NUM_FIXTURE];
ControlFixtureProp	ControlFixtureProps[NUM_CONTROL_FIXTURE];
ReceptacleFixtureProp	ReceptacleFixtureProps[NUM_RECEPTACLE_FIXTURE];
TerminalFixtureProp	TerminalFixtureProps[NUM_TERMINAL_FIXTURE];
PanelFixtureProp	PanelFixtureProps[NUM_PANEL_FIXTURE];
VendingFixtureProp	VendingFixtureProps[NUM_VENDING_FIXTURE];
CyberFixtureProp	CyberFixtureProps[NUM_CYBER_FIXTURE];

DoorProp	DoorProps[NUM_DOOR];
NormalDoorProp NormalDoorProps[NUM_NORMAL_DOOR];
DoorwaysDoorProp DoorwaysDoorProps[NUM_DOORWAYS_DOOR];
ForceDoorProp ForceDoorProps[NUM_FORCE_DOOR];
ElevatorDoorProp ElevatorDoorProps[NUM_ELEVATOR_DOOR];
SpecialDoorProp SpecialDoorProps[NUM_SPECIAL_DOOR];

AnimatingProp	AnimatingProps[NUM_ANIMATING];
ObjectsAnimatingProp ObjectsAnimatingProps[NUM_OBJECT_ANIMATING];
TransitoryAnimatingProp TransitoryAnimatingProps[NUM_TRANSITORY_ANIMATING];
ExplosionAnimatingProp ExplosionAnimatingProps[NUM_EXPLOSION_ANIMATING];

TrapProp	TrapProps[NUM_TRAP];
TriggerTrapProp TriggerTrapProps[NUM_TRIGGER_TRAP];
FeedbacksTrapProp FeedbacksTrapProps[NUM_FEEDBACKS_TRAP];
SecretTrapProp SecretTrapProps[NUM_SECRET_TRAP];

ContainerProp	ContainerProps[NUM_CONTAINER];
ActualContainerProp ActualContainerProps[NUM_ACTUAL_CONTAINER];
WasteContainerProp WasteContainerProps[NUM_WASTE_CONTAINER];
LiquidContainerProp LiquidContainerProps[NUM_LIQUID_CONTAINER];
MutantCorpseContainerProp MutantCorpseContainerProps[NUM_MUTANT_CORPSE_CONTAINER];
RobotCorpseContainerProp RobotCorpseContainerProps[NUM_ROBOT_CORPSE_CONTAINER];
CyborgCorpseContainerProp CyborgCorpseContainerProps[NUM_CYBORG_CORPSE_CONTAINER];
OtherCorpseContainerProp OtherCorpseContainerProps[NUM_OTHER_CORPSE_CONTAINER];
#else
extern FixtureProp	FixtureProps[NUM_FIXTURE];
extern ControlFixtureProp	ControlFixtureProps[NUM_CONTROL_FIXTURE];
extern ReceptacleFixtureProp	ReceptacleFixtureProps[NUM_RECEPTACLE_FIXTURE];
extern TerminalFixtureProp	TerminalFixtureProps[NUM_TERMINAL_FIXTURE];
extern PanelFixtureProp	PanelFixtureProps[NUM_PANEL_FIXTURE];
extern VendingFixtureProp	VendingFixtureProps[NUM_VENDING_FIXTURE];
extern CyberFixtureProp	CyberFixtureProps[NUM_CYBER_FIXTURE];

extern DoorProp	DoorProps[NUM_DOOR];
extern NormalDoorProp NormalDoorProps[NUM_NORMAL_DOOR];
extern DoorwaysDoorProp DoorwaysDoorProps[NUM_DOORWAYS_DOOR];
extern ForceDoorProp ForceDoorProps[NUM_FORCE_DOOR];
extern ElevatorDoorProp ElevatorDoorProps[NUM_ELEVATOR_DOOR];
extern SpecialDoorProp SpecialDoorProps[NUM_SPECIAL_DOOR];

extern AnimatingProp	AnimatingProps[NUM_ANIMATING];
extern ObjectsAnimatingProp ObjectsAnimatingProps[NUM_OBJECT_ANIMATING];
extern TransitoryAnimatingProp TransitoryAnimatingProps[NUM_TRANSITORY_ANIMATING];
extern ExplosionAnimatingProp ExplosionAnimatingProps[NUM_EXPLOSION_ANIMATING];

extern TrapProp	TrapProps[NUM_TRAP];
extern TriggerTrapProp TriggerTrapProps[NUM_TRIGGER_TRAP];
extern FeedbacksTrapProp FeedbacksTrapProps[NUM_FEEDBACKS_TRAP];
extern SecretTrapProp SecretTrapProps[NUM_SECRET_TRAP];

extern ContainerProp	ContainerProps[NUM_CONTAINER];
extern ActualContainerProp ActualContainerProps[NUM_ACTUAL_CONTAINER];
extern WasteContainerProp WasteContainerProps[NUM_WASTE_CONTAINER];
extern LiquidContainerProp LiquidContainerProps[NUM_LIQUID_CONTAINER];
extern MutantCorpseContainerProp MutantCorpseContainerProps[NUM_MUTANT_CORPSE_CONTAINER];
extern RobotCorpseContainerProp RobotCorpseContainerProps[NUM_ROBOT_CORPSE_CONTAINER];
extern CyborgCorpseContainerProp CyborgCorpseContainerProps[NUM_CYBORG_CORPSE_CONTAINER];
extern OtherCorpseContainerProp OtherCorpseContainerProps[NUM_OTHER_CORPSE_CONTAINER];
#endif

#ifdef __OBJSIM_SRC
ObjFixture objFixtures[NUM_OBJECTS_FIXTURE];
ObjDoor objDoors[NUM_OBJECTS_DOOR];
ObjAnimating objAnimatings[NUM_OBJECTS_ANIMATING];
ObjTrap objTraps[NUM_OBJECTS_TRAP];
ObjContainer objContainers[NUM_OBJECTS_CONTAINER];
ObjFixture default_fixture;
ObjDoor default_door;
ObjAnimating default_animating;
ObjTrap default_trap;
ObjContainer default_container;
#else
extern ObjFixture objFixtures[NUM_OBJECTS_FIXTURE];
extern ObjDoor objDoors[NUM_OBJECTS_DOOR];
extern ObjAnimating objAnimatings[NUM_OBJECTS_ANIMATING];
extern ObjTrap objTraps[NUM_OBJECTS_TRAP];
extern ObjContainer objContainers[NUM_OBJECTS_CONTAINER];
extern ObjFixture default_fixture;
extern ObjDoor default_door;
extern ObjAnimating default_animating;
extern ObjTrap default_trap;
extern ObjContainer default_container;
#endif

#endif // __OBJGAME_H


