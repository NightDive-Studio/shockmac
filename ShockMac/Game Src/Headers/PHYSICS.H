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
#ifndef __PHYSICS_H
#define __PHYSICS_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/physics.h $
 * $Revision: 1.36 $
 * $Author: mahk $
 * $Date: 1994/09/06 08:44:53 $
 *
 *
 */

// Includes
#include "objects.h"
#include "dirac.h"

// Defines

typedef fix Physvec[6];
#define CONTROL_NO_CHANGE -127

#define MAX_PHYS_HANDLE MAX_OBJ

#define CHECK_OBJ_PH(oid) ((objs[oid].info.ph != -1) && (objs[oid].info.ph <= MAX_PHYS_HANDLE))

#define CSLOPE_SET(a,b,c) { terrain_info.cx = (a); terrain_info.cy = (b); terrain_info.cz = (c); }
#define FSLOPE_SET(a,b,c) { terrain_info.fx = (a); terrain_info.fy = (b); terrain_info.fz = (c); }
#define WGRAD_SET(a,b,c) { terrain_info.wx = (a); terrain_info.wy = (b); terrain_info.wz = (c); }
#define WGRAD_ADD(a,b,c) { terrain_info.wx += (a); terrain_info.wy += (b); terrain_info.wz += (c); }

#define NUM_EDMS_MODELS 5
#define EDMS_NONE    0
#define EDMS_ROBOT   1
#define EDMS_PELVIS  2
#define EDMS_JELLO   3
#define EDMS_DIRAC   4


// These are the magic edms cyber_space numbers.  
#define PELVIS_MODE_NORMAL  0
#define PELVIS_MODE_SKATES  1
#define PELVIS_MODE_CYBER   2

#define STANDARD_MASS   fix_make(1,0)
#define STANDARD_SIZE   (standard_robot.size)
#define STANDARD_GRAVITY   fix_make(4,0)

// Prototypes

#define CONTROL_BANKS 4
#define CONTROL_MAX_VAL 100
// why is this number 100? what is the point??

// Set the player motion controls, on a scale of  -100 to +100.  
// An arg of CONTROL_NO_CHANGE leaves the value unchanged. 
// There are CONTROL_BANKS banks of controls, which roughly average
// together.  
errtype physics_set_player_controls(int bank, byte xvel,byte yvel, byte zvel, byte xyrot, byte yzrot, byte xzrot);

// Set a single control, using the defined control numbers

#define CONTROL_XVEL    0 // x translation
#define CONTROL_YVEL    1 // y translation
#define CONTROL_ZVEL    2 // z translation
#define CONTROL_XYROT   3 // xy rotation
#define CONTROL_YZROT   4 // yz rotation
#define CONTROL_XZROT   5 // xz rotation

#define MOUSE_CONTROL_BANK 0
#define KEYBD_CONTROL_BANK 1
#define JOYST_CONTROL_BANK 2
#define INP6D_CONTROL_BANK 3
errtype physics_set_one_control(int bank, int num, byte val);
errtype physics_get_one_control(int bank, int num, byte* val);

// Run the physics system for one frame
errtype physics_run(void);

// Initialize EDMS, player, etc.
errtype physics_init(void);

// Set the gravity parameter of all objects to new_grav
errtype apply_gravity_to_objects(fix new_grav);

// Take an object, and moves it to a position and velocity relative to the 
// player.  returns true if it finds an appropriate place to put the object.
bool player_throw_object(ObjID id,  int x, int y, int lastx, int lasty, fix vel);

// Cause the player to assume one of three postures
#define POSTURE_STAND    0
#define POSTURE_STOOP    1
#define POSTURE_PRONE    2
#define NUM_POSTURES     3
errtype player_set_posture(ubyte new_posture);


// Lean the player.  Values are in a -100-+100 scale.  
// Hey kids, this don't exist no more.  set your self an 
// XZROT control if you want to lean sideways.
errtype player_set_lean(byte x, byte y);

// Plant the player's foot, turning directional controls into
// translational ones.  Unplants foot IFF planted is false
errtype player_plant_foot(bool planted);

// Set the player's eye position -100 to +100
void player_set_eye(byte eyecntl);

// Build the model given a state and object ID, and assign appropriate
// data into the object and do appropriate bookkeeping
errtype assemble_physics_object(ObjID id, State *pnew_state);

// Instantiators 
void instantiate_robot(int triple, Robot* r);
void instantiate_pelvis(int triple, Pelvis* r);
void instantiate_dirac(int triple, Dirac_frame* new_dirac);

errtype apply_gravity_to_one_object(ObjID oid, fix new_grav);

// Globals

#ifdef __PHYSICS_SRC
TerrainData terrain_info;
State standard_state;
#else
extern TerrainData terrain_info;
extern State standard_state;
extern Robot standard_robot;
#endif

#endif // __PHYSICS_H

