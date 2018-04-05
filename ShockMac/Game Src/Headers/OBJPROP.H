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
#ifndef __OBJPROP_H
#define __OBJPROP_H

/*
 * $Source: n:/project/cit/src/inc/RCS/objprop.h $
 * $Revision: 1.33 $
 * $Author: xemu $
 * $Date: 1994/06/01 01:39:20 $
 *
 */

// Includes
#include "objwpn.h"
#include "objwarez.h"
#include "objstuff.h"
#include "objgame.h"
#include "objcrit.h"

// The overall object properties typedef
typedef struct ObjProp {
   int   mass;
   short hit_points;
   ubyte armor;
   ubyte render_type;
   ubyte physics_model;
   ubyte hardness;
   ubyte pep; 
   ubyte physics_xr;
   ubyte physics_y;
   ubyte physics_z;
   int   resistances;
   ubyte defense_value;
   ubyte toughness;
   short flags;
   short mfd_id;
   short bitmap_3d;
   ubyte destroy_effect;
} ObjProp;

// Overall

#define NUM_OBJECT (NUM_GUN + NUM_AMMO + NUM_PHYSICS + NUM_GRENADE + NUM_DRUG + NUM_HARDWARE + NUM_SOFTWARE + NUM_BIGSTUFF + NUM_SMALLSTUFF + NUM_FIXTURE + NUM_DOOR + NUM_ANIMATING + NUM_TRAP + NUM_CONTAINER + NUM_CRITTER)

// This is "extra" in the sense of "fewer"
#define EXTRA_OBJ_ANIMS    -600

// Number of subclasses for each classes
//

#define NUM_SC_GUN         6
#define NUM_SC_AMMO        7
#define NUM_SC_PHYSICS     3
#define NUM_SC_GRENADE     2
#define NUM_SC_DRUG        1
#define NUM_SC_HARDWARE    2
#define NUM_SC_SOFTWARE    5
#define NUM_SC_BIGSTUFF    8
#define NUM_SC_SMALLSTUFF  8
#define NUM_SC_FIXTURE     6
#define NUM_SC_DOOR        5
#define NUM_SC_ANIMATING   3
#define NUM_SC_TRAP        3
#define NUM_SC_CONTAINER   7
#define NUM_SC_CRITTER     5

#define NUM_SUBCLASSES  NUM_SC_GUN + NUM_SC_AMMO + NUM_SC_PHYSICS + NUM_SC_GRENADE + NUM_SC_DRUG + NUM_SC_HARDWARE + NUM_SC_SOFTWARE + NUM_SC_BIGSTUFF + NUM_SC_SMALLSTUFF + NUM_SC_FIXTURE + NUM_SC_DOOR + NUM_SC_ANIMATING + NUM_SC_TRAP + NUM_SC_CONTAINER + NUM_SC_CRITTER 

#ifdef __OBJSIM_SRC
uchar num_subclasses[NUM_CLASSES] = {
   NUM_SC_GUN,
   NUM_SC_AMMO,
   NUM_SC_PHYSICS,
   NUM_SC_GRENADE,
   NUM_SC_DRUG,
   NUM_SC_HARDWARE,
   NUM_SC_SOFTWARE,
   NUM_SC_BIGSTUFF,
   NUM_SC_SMALLSTUFF,
   NUM_SC_FIXTURE,
   NUM_SC_DOOR,
   NUM_SC_ANIMATING,
   NUM_SC_TRAP,
   NUM_SC_CONTAINER,
   NUM_SC_CRITTER
};
#else
extern uchar num_subclasses[NUM_CLASSES];
#endif

#ifdef __OBJSIM_SRC
ObjProp ObjProps[NUM_OBJECT];
#else
extern ObjProp ObjProps[NUM_OBJECT];
#endif

#endif	// __OBJPROP_H
