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
#ifndef __OBJSTUFF_H
#define __OBJSTUFF_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/objstuff.h $
 * $Revision: 1.23 $
 * $Author: xemu $
 * $Date: 1994/07/09 00:08:58 $
 *
 *
 */

// Includes
#include "objclass.h"

// Instance typedefs
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
   short cosmetic_value;
   int data1;
   int data2;
} ObjBigstuff;

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
   short cosmetic_value;
   int data1;
   int data2;
} ObjSmallstuff;

// Class typedefs
typedef struct _BigstuffProp {
   int data;
} BigstuffProp;

typedef struct _SmallstuffProp {
   short uses_flags;
} SmallstuffProp;

// Subclass typedefs
typedef struct _ElectronicBigstuffProp {
   ubyte dummy;
} ElectronicBigstuffProp;

typedef struct _FurnishingBigstuffProp {
   ubyte dummy;
} FurnishingBigstuffProp;

typedef struct _OnthewallBigstuffProp {
   ubyte dummy;
} OnthewallBigstuffProp;

typedef struct _LightBigstuffProp {
   ubyte dummy;
} LightBigstuffProp ;

typedef struct _LabgearBigstuffProp {
   ubyte dummy;
} LabgearBigstuffProp ;

typedef struct _TechnoBigstuffProp {
   ubyte dummy;
} TechnoBigstuffProp ;

typedef struct _DecorBigstuffProp {
   ubyte dummy;
} DecorBigstuffProp ;

typedef struct _TerrainBigstuffProp {
   ubyte dummy;
} TerrainBigstuffProp ;

typedef struct _UselessSmallstuffProp {
   ubyte dummy;
} UselessSmallstuffProp ;

typedef struct _BrokenSmallstuffProp {
   ubyte dummy;
} BrokenSmallstuffProp ;

typedef struct _CorpselikeSmallstuffProp {
   ubyte dummy;
} CorpselikeSmallstuffProp ;

typedef struct _GearSmallstuffProp {
   ubyte dummy;
} GearSmallstuffProp ;

typedef struct _CardsSmallstuffProp {
   ubyte dummy;
} CardsSmallstuffProp ;

#define NUM_SMALLSTUFF_VCOLORS   6
typedef struct _CyberSmallstuffProp {
   uchar vcolors[NUM_SMALLSTUFF_VCOLORS];
} CyberSmallstuffProp ;

typedef struct _OnthewallSmallstuffProp {
   ubyte dummy;
} OnthewallSmallstuffProp ;

typedef struct _PlotSmallstuffProp {
   ObjID target;
} PlotSmallstuffProp ;

// Big Stuff
#define NUM_ELECTRONIC_BIGSTUFF 9
#define NUM_FURNISHING_BIGSTUFF 10
#define NUM_ONTHEWALL_BIGSTUFF 11
#define NUM_LIGHT_BIGSTUFF 4
#define NUM_LABGEAR_BIGSTUFF 9
#define NUM_TECHNO_BIGSTUFF 8
#define NUM_DECOR_BIGSTUFF 16
#define NUM_TERRAIN_BIGSTUFF 10

// Small Stuff
#define NUM_USELESS_SMALLSTUFF 8
#define NUM_BROKEN_SMALLSTUFF 10
#define NUM_CORPSELIKE_SMALLSTUFF 15
#define NUM_GEAR_SMALLSTUFF 6
#define NUM_CARDS_SMALLSTUFF 12
#define NUM_CYBER_SMALLSTUFF 12
#define NUM_ONTHEWALL_SMALLSTUFF 9
#define NUM_PLOT_SMALLSTUFF 8


#define NUM_BIGSTUFF (NUM_ELECTRONIC_BIGSTUFF + NUM_FURNISHING_BIGSTUFF + NUM_ONTHEWALL_BIGSTUFF + NUM_LIGHT_BIGSTUFF + NUM_LABGEAR_BIGSTUFF + \
   NUM_TECHNO_BIGSTUFF + NUM_DECOR_BIGSTUFF + NUM_TERRAIN_BIGSTUFF)

#define NUM_SMALLSTUFF (NUM_USELESS_SMALLSTUFF + NUM_BROKEN_SMALLSTUFF + NUM_CORPSELIKE_SMALLSTUFF + NUM_GEAR_SMALLSTUFF + NUM_CARDS_SMALLSTUFF + \
      NUM_CYBER_SMALLSTUFF + NUM_ONTHEWALL_SMALLSTUFF + NUM_PLOT_SMALLSTUFF)

// Enumeration of subclasses
// Furniture
#define BIGSTUFF_SUBCLASS_ELECTRONIC 0
#define BIGSTUFF_SUBCLASS_FURNISHING 1
#define BIGSTUFF_SUBCLASS_ONTHEWALL 2
#define BIGSTUFF_SUBCLASS_LIGHT 3
#define BIGSTUFF_SUBCLASS_LABGEAR 4
#define BIGSTUFF_SUBCLASS_TECHNO 5
#define BIGSTUFF_SUBCLASS_DECOR 6
#define BIGSTUFF_SUBCLASS_TERRAIN 7

// Stuff
#define SMALLSTUFF_SUBCLASS_USELESS 0
#define SMALLSTUFF_SUBCLASS_BROKEN 1
#define SMALLSTUFF_SUBCLASS_CORPSELIKE 2
#define SMALLSTUFF_SUBCLASS_GEAR 3
#define SMALLSTUFF_SUBCLASS_CARDS 4
#define SMALLSTUFF_SUBCLASS_CYBER 5
#define SMALLSTUFF_SUBCLASS_ONTHEWALL 6
#define SMALLSTUFF_SUBCLASS_PLOT 7

#ifdef __OBJSIM_SRC

BigstuffProp BigstuffProps[NUM_BIGSTUFF];
ElectronicBigstuffProp ElectronicBigstuffProps[NUM_ELECTRONIC_BIGSTUFF];
FurnishingBigstuffProp FurnishingBigstuffProps[NUM_FURNISHING_BIGSTUFF];
OnthewallBigstuffProp OnthewallBigstuffProps[NUM_ONTHEWALL_BIGSTUFF];
LightBigstuffProp LightBigstuffProps[NUM_LIGHT_BIGSTUFF];
LabgearBigstuffProp LabgearBigstuffProps[NUM_LABGEAR_BIGSTUFF];
TechnoBigstuffProp TechnoBigstuffProps[NUM_TECHNO_BIGSTUFF];
DecorBigstuffProp DecorBigstuffProps[NUM_DECOR_BIGSTUFF];
TerrainBigstuffProp TerrainBigstuffProps[NUM_TERRAIN_BIGSTUFF];

SmallstuffProp SmallstuffProps[NUM_SMALLSTUFF];
UselessSmallstuffProp UselessSmallstuffProps[NUM_USELESS_SMALLSTUFF];
BrokenSmallstuffProp BrokenSmallstuffProps[NUM_BROKEN_SMALLSTUFF];
CorpselikeSmallstuffProp CorpselikeSmallstuffProps[NUM_CORPSELIKE_SMALLSTUFF];
GearSmallstuffProp GearSmallstuffProps[NUM_GEAR_SMALLSTUFF];
CardsSmallstuffProp CardsSmallstuffProps[NUM_CARDS_SMALLSTUFF];
CyberSmallstuffProp CyberSmallstuffProps[NUM_CYBER_SMALLSTUFF];
OnthewallSmallstuffProp OnthewallSmallstuffProps[NUM_ONTHEWALL_SMALLSTUFF];
PlotSmallstuffProp PlotSmallstuffProps[NUM_PLOT_SMALLSTUFF];
#else
extern BigstuffProp BigstuffProps[NUM_BIGSTUFF];
extern ElectronicBigstuffProp ElectronicBigstuffProps[NUM_ELECTRONIC_BIGSTUFF];
extern FurnishingBigstuffProp FurnishingBigstuffProps[NUM_FURNISHING_BIGSTUFF];
extern OnthewallBigstuffProp OnthewallBigstuffProps[NUM_ONTHEWALL_BIGSTUFF];
extern LightBigstuffProp LightBigstuffProps[NUM_LIGHT_BIGSTUFF];
extern LabgearBigstuffProp LabgearBigstuffProps[NUM_LABGEAR_BIGSTUFF];
extern TechnoBigstuffProp TechnoBigstuffProps[NUM_TECHNO_BIGSTUFF];
extern DecorBigstuffProp DecorBigstuffProps[NUM_DECOR_BIGSTUFF];
extern TerrainBigstuffProp TerrainBigstuffProps[NUM_TERRAIN_BIGSTUFF];

extern SmallstuffProp SmallstuffProps[NUM_SMALLSTUFF];
extern UselessSmallstuffProp UselessSmallstuffProps[NUM_USELESS_SMALLSTUFF];
extern BrokenSmallstuffProp BrokenSmallstuffProps[NUM_BROKEN_SMALLSTUFF];
extern CorpselikeSmallstuffProp CorpselikeSmallstuffProps[NUM_CORPSELIKE_SMALLSTUFF];
extern GearSmallstuffProp GearSmallstuffProps[NUM_GEAR_SMALLSTUFF];
extern CardsSmallstuffProp CardsSmallstuffProps[NUM_CARDS_SMALLSTUFF];
extern CyberSmallstuffProp CyberSmallstuffProps[NUM_CYBER_SMALLSTUFF];
extern OnthewallSmallstuffProp OnthewallSmallstuffProps[NUM_ONTHEWALL_SMALLSTUFF];
extern PlotSmallstuffProp PlotSmallstuffProps[NUM_PLOT_SMALLSTUFF];

#endif

#ifdef __OBJSIM_SRC
ObjBigstuff objBigstuffs[NUM_OBJECTS_BIGSTUFF];
ObjSmallstuff objSmallstuffs[NUM_OBJECTS_SMALLSTUFF];
ObjBigstuff default_bigstuff;
ObjSmallstuff default_smallstuff;
#else
extern ObjBigstuff objBigstuffs[NUM_OBJECTS_BIGSTUFF];
extern ObjSmallstuff objSmallstuffs[NUM_OBJECTS_SMALLSTUFF];
extern ObjBigstuff default_bigstuff;
extern ObjSmallstuff default_smallstuff;
#endif

#endif // __OBJSTUFF_H


