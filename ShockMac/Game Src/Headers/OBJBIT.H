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
#ifndef __OBJBIT_H
#define __OBJBIT_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/objbit.h $
 * $Revision: 1.21 $
 * $Author: xemu $
 * $Date: 1994/08/22 03:03:51 $
 *
 */

// Includes

// OBJECT PROPERTIES FLAGS 

#define     INVENTORY_GENERAL       0x0001
#define     INVENTORY_GENERAL_SHF   0
#define     EDMS_PRESERVE           0x0002
#define     EDMS_PRESERVE_SHF       1   
#define     INVENT_USEMODE          0x000C
#define     INVENT_USEMODE_SHF      2
#define     OBJECT_USE_NOCURSOR        0x0010
#define     OBJECT_USE_NOCURSOR_SHIFT  4

// RENDER_BLOCK is true if the thing can block the renderer
// currently only used for doors, where instance bit need be checked
// as well.
#define     RENDER_BLOCK            0x0020
#define     RENDER_BLOCK_SHF        5

// LIGHT_TYPE is for how it should be lit. 
// 0 -- normal, simple lighting applies
// 1 -- use complicated lighting
// 2 -- never apply lighting
// 3 -- consult instance bit to determine whether to light
#define		LIGHT_TYPE					0x00C0
#define		LIGHT_TYPE_SHF				6

// TERRAIN_OBJECT
// 0 - ignore for terrain
// 1 - wall-like terrain
// 2 - complex terrain
// 3 - unused
#define     TERRAIN_OBJECT          0x0300
#define     TERRAIN_OBJECT_SHF      8

// MY_IM_LARGE
// Doubles the size of the bitmap when it renders, relative to the source art
#define     MY_IM_LARGE             0x0400
#define     MY_IM_LARGE_SHF         10

// terrain will special case terrain damage if this bit is set
#define     SPCL_TERR_DMG           0x0800
#define     SPCL_TERR_DMG_SHT       11

// Class-specific flags go here. 
#define     CLASS_FLAGS             0x7000
#define     CLASS_FLAGS_SHF         12

// If this bit is set, that class of object is considered "useless", and unless
// it is specially preserved via USEFUL_FLAG (instance flag) it can be destroyed
// to make room for others.
#define     USELESS_FLAG            0x8000
#define     USELESS_FLAG_SHF        15


// INSTANCE DATA FLAGS
#define HUDOBJ_INST_FLAG      0x01 // This is defined redundantly in hudobj.h
#define RENDER_BLOCK_FLAG     0x02 // whether we are blocking the renderer
#define UNLIT_FLAG				0x04 // Don't light up my life
#define INDESTRUCT_FLAG       0x08 // We can't be stopped, but we must be stopped
#define USEFUL_FLAG           0x10 // Don't punt us to make room for others
#define OLH_INST_FLAG         0x20 // have we interacted with this yet?
#define CLASS_INST_FLAG2      0x40 // Class-specific stuff
#define CLASS_INST_FLAG       0x80 // Class-specific stuff


// Notes on class specific instance data (flags)
// Fixtures: used for whether to zoom to mfd on use
// Critters: CLASS_INST_FLAG used to denote "loner"
//           CLASS_INST_FLAG2 used to denote that the critter wants to get closer.   
// Containers: CLASS_INST_FLAG used to indicate that the container is "freshly" dead
//                and needs to have loot placed on it when searched.
// Smallstuffs: corpses use their CLASS_INST_FLAG like Containers.
// Doors: Both CLASS_INST_FLAGs are used as secret identifier codes to make sure that
//    we get the right autoclose events.

// Notes on class specific property flagss
// Bigstuff & Smallstuff: CLASS_FLAG 0x1000 used to denote data1 is 1-2 ObjIDs
//		to be "used" ala a splitter trap
#define STUFF_OBJUSE_FLAG	0x1

// Critters:  
// CLASS_FLAG 0x1000 used to indicate that creature is incapable of movement.
// CLASS_FLAG 0x2000 used to indicate that creature is unable to open doors.
#define CRITTER_NOMOVE_OBJPROP_FLAG   0x1
#define CRITTER_NODOOR_OBJPROP_FLAG   0x2

#endif // __OBJBIT_H



