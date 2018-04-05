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
#ifndef __OBJWPN_H
#define __OBJWPN_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/objwpn.h $
 * $Revision: 1.33 $
 * $Author: minman $
 * $Date: 1994/07/26 00:50:45 $
 *
 *
 */

// Includes
#include "objclass.h"

// Instance Data
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
	ubyte    ammo_type;
	ubyte    ammo_count;
} ObjGun;

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
} ObjAmmo;

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
   ObjID    owner;
   int      bullet_triple;
   int      duration;
//   char     power;
   ObjLoc   p1, p2, p3;         
} ObjPhysics;

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
   ubyte unique_id;
   ubyte walls_hit;
   short flags;
   short timestamp;
} ObjGrenade;

// Class typedefs

#define COMBAT_DATA_FIELDS \
   short damage_modifier;  \
   ubyte offense_value;    \
   int   damage_type;      \
   ubyte penetration

typedef struct _GunProp {
   ubyte fire_rate;
   ubyte useable_ammo_type;
} GunProp;

typedef struct _AmmoProp {
   COMBAT_DATA_FIELDS;
   ubyte cartridge_size;
   ubyte bullet_mass;
   short bullet_speed;
   ubyte range;
   ubyte recoil_force;
} AmmoProp;

typedef struct _PhysicsProp {
   ubyte flags;
} PhysicsProp;

typedef struct _GrenadeProp {
   COMBAT_DATA_FIELDS;
   ubyte touchiness;
   ubyte radius;
   ubyte radius_change;
   ubyte damage_change;
   ubyte attack_mass;
   short flags;            // Does it spew shrapnel? Can timer be set? Can it stick to wall?
} GrenadeProp;

// Subclass typedefs

#define EMPTY_STRUCTS

typedef struct _PistolGunProp {
#ifdef EMPTY_STRUCTS
   ubyte dummy;
#endif
} PistolGunProp;

typedef struct _AutoGunProp {
#ifdef EMPTY_STRUCTS
   ubyte dummy;
#endif
} AutoGunProp;

typedef struct _SpecialGunProp {
   COMBAT_DATA_FIELDS;
   ubyte speed;
   int   proj_triple;
   ubyte attack_mass;
   short attack_speed;
} SpecialGunProp;

typedef struct _HandtohandGunProp {
   COMBAT_DATA_FIELDS;
   ubyte energy_use;
   ubyte attack_mass;
   ubyte attack_range;
   short attack_speed;
} HandtohandGunProp;

typedef struct _BeamGunProp {
   COMBAT_DATA_FIELDS;
   ubyte max_charge;
   ubyte attack_mass;
   ubyte attack_range;
   short attack_speed;
} BeamGunProp;

typedef struct _BeamprojGunProp {
   COMBAT_DATA_FIELDS;
   ubyte max_charge;
   ubyte attack_mass;
   short attack_speed;
   ubyte speed;
   int   proj_triple;
   ubyte flags;
} BeamprojGunProp;

typedef struct _PistolAmmoProp {
#ifdef EMPTY_STRUCTS
   ubyte dummy;
#endif
} PistolAmmoProp;

typedef struct _NeedleAmmoProp {
#ifdef EMPTY_STRUCTS
   ubyte dummy;
#endif
} NeedleAmmoProp;

typedef struct _MagnumAmmoProp {
#ifdef EMPTY_STRUCTS
   ubyte dummy;
#endif
} MagnumAmmoProp;

typedef struct _RifleAmmoProp {
#ifdef EMPTY_STRUCTS
   ubyte dummy;
#endif
} RifleAmmoProp;

typedef struct _FlechetteAmmoProp {
#ifdef EMPTY_STRUCTS
   ubyte dummy;
#endif
} FlechetteAmmoProp;

typedef struct _AutoAmmoProp {
#ifdef EMPTY_STRUCTS
   ubyte dummy;
#endif
} AutoAmmoProp;

typedef struct _ProjAmmoProp {
#ifdef EMPTY_STRUCTS
   ubyte dummy;
#endif
} ProjAmmoProp;

typedef struct _TracerPhysicsProp {
   short xcoords[4];
   short ycoords[4];
   ubyte zcoords[4];
} TracerPhysicsProp;

#define NUM_SLOW_VCOLORS   6
typedef struct _SlowPhysicsProp {
   uchar vcolors[NUM_SLOW_VCOLORS];
} SlowPhysicsProp;

typedef struct _CameraPhysicsProp {
#ifdef EMPTY_STRUCTS
   ubyte dummy;
#endif
} CameraPhysicsProp;

typedef struct _DirectGrenadeProp {
#ifdef EMPTY_STRUCTS
   ubyte dummy;
#endif
} DirectGrenadeProp;

typedef struct _TimedGrenadeProp {
   ubyte min_time_set;
   ubyte max_time_set;
   ubyte timing_deviation;
} TimedGrenadeProp;
 
// Gun
#define NUM_PISTOL_GUN	      5
#define NUM_AUTO_GUN	         2
#define NUM_SPECIAL_GUN	      2
#define NUM_HANDTOHAND_GUN    2
#define NUM_BEAM_GUN	         3
#define NUM_BEAMPROJ_GUN	   2

// Ammo
#define NUM_PISTOL_AMMO	      2
#define NUM_NEEDLE_AMMO	      2
#define NUM_MAGNUM_AMMO	      3
#define NUM_RIFLE_AMMO	      2
#define NUM_FLECHETTE_AMMO	   2      
#define NUM_AUTO_AMMO	      2      
#define NUM_PROJ_AMMO	      2      

// Physics
#define NUM_TRACER_PHYSICS    6
#define NUM_SLOW_PHYSICS      16
#define NUM_CAMERA_PHYSICS    2

// Grenade
#define NUM_DIRECT_GRENADE    5
#define NUM_TIMED_GRENADE     3

#define NUM_GUN	(NUM_PISTOL_GUN + NUM_AUTO_GUN + NUM_SPECIAL_GUN + NUM_HANDTOHAND_GUN + NUM_BEAM_GUN + NUM_BEAMPROJ_GUN)

#define NUM_AMMO	(NUM_PISTOL_AMMO + NUM_NEEDLE_AMMO + NUM_MAGNUM_AMMO + NUM_RIFLE_AMMO + NUM_FLECHETTE_AMMO + NUM_AUTO_AMMO + NUM_PROJ_AMMO)

#define NUM_PHYSICS (NUM_TRACER_PHYSICS + NUM_SLOW_PHYSICS + NUM_CAMERA_PHYSICS)

#define NUM_GRENADE	(NUM_DIRECT_GRENADE + NUM_TIMED_GRENADE)

// Gun
#define GUN_SUBCLASS_PISTOL 0
#define GUN_SUBCLASS_AUTO 1
#define GUN_SUBCLASS_SPECIAL 2
#define GUN_SUBCLASS_HANDTOHAND 3
#define GUN_SUBCLASS_BEAM 4
#define GUN_SUBCLASS_BEAMPROJ 5

// Ammo
#define AMMO_SUBCLASS_PISTOL	   0
#define AMMO_SUBCLASS_NEEDLE	   1
#define AMMO_SUBCLASS_MAGNUM     2
#define AMMO_SUBCLASS_RIFLE	   3
#define AMMO_SUBCLASS_FLECHETTE	4
#define AMMO_SUBCLASS_AUTO	      5
#define AMMO_SUBCLASS_PROJ       6

// Physics
#define PHYSICS_SUBCLASS_TRACER 0
#define PHYSICS_SUBCLASS_SLOW 1
#define PHYSICS_SUBCLASS_CAMERA 2

// Grenade
#define GRENADE_SUBCLASS_DIRECT 0
#define GRENADE_SUBCLASS_TIMED 1

#ifdef STRANGE_EFFICIOMATRON_WAY
#define GunBase         (&PropsArray)
#define GunProps        ((GunProp *)GunBase)
#define PistolBase      (GunBase+(sizeof(GunProp)* NUM_GUN))
#define PistolProps     ((PistolGunProp *)PistolBase)
#define AutoBase        (PistolBase + (sizeof(PistolGunProp * NUM_PISTOL_GUN)))	
#define AutoProps       ((AutoGunProp *)AutoBase)			
#define SpecialBase        (AutoBase + (sizeof(AutoGunProp * NUM_AUTO_GUN)))	
#define SpecialProps       ((SpecialGunProp *)SpecialBase)			
#define HandtohandBase        (SpecialBase + (sizeof(SpecialGunProp * NUM_SPECIAL_GUN)))	
#define HandtohandProps       ((HandtohandGunProp *)HandtohandBase)			
#define BeamBase        (HandtohandBase + (sizeof(HandtohandGunProp * NUM_HANDTOHAND_GUN)))	
#define BeamProps       ((BeamGunProp *)BeamBase)			
#define BeamprojBase        (BeamBase + (sizeof(BeamGunProp * NUM_BEAM_GUN)))	
#define BeamprojProps       ((BeamprojGunProp *)BeamprojBase)			

#define AmmoBase        (BeamprojBase + sizeof(BeamprojGunProp * NUM_BEAMPROJ_GUN))
#define AmmoProps       ((AmmoProp *)
#endif

#ifdef __OBJSIM_SRC
GunProp	GunProps[NUM_GUN];
PistolGunProp	PistolGunProps[NUM_PISTOL_GUN];
AutoGunProp	AutoGunProps[NUM_AUTO_GUN];
SpecialGunProp	SpecialGunProps[NUM_SPECIAL_GUN];
HandtohandGunProp	HandtohandGunProps[NUM_HANDTOHAND_GUN];
BeamGunProp	BeamGunProps[NUM_BEAM_GUN];
BeamprojGunProp	BeamprojGunProps[NUM_BEAMPROJ_GUN];

AmmoProp	AmmoProps[NUM_AMMO];
PistolAmmoProp	PistolAmmoProps[NUM_PISTOL_AMMO];
NeedleAmmoProp	NeedleAmmoProps[NUM_NEEDLE_AMMO];
MagnumAmmoProp	MagnumAmmoProps[NUM_MAGNUM_AMMO];
RifleAmmoProp	RifleAmmoProps[NUM_RIFLE_AMMO];
FlechetteAmmoProp	FlechetteAmmoProps[NUM_FLECHETTE_AMMO];
AutoAmmoProp	AutoAmmoProps[NUM_AUTO_AMMO];
ProjAmmoProp	ProjAmmoProps[NUM_PROJ_AMMO];

PhysicsProp	PhysicsProps[NUM_PHYSICS];
TracerPhysicsProp TracerPhysicsProps[NUM_TRACER_PHYSICS];
SlowPhysicsProp SlowPhysicsProps[NUM_SLOW_PHYSICS];
CameraPhysicsProp CameraPhysicsProps[NUM_CAMERA_PHYSICS];
GrenadeProp	GrenadeProps[NUM_GRENADE];
DirectGrenadeProp	DirectGrenadeProps[NUM_DIRECT_GRENADE];
TimedGrenadeProp	TimedGrenadeProps[NUM_TIMED_GRENADE];
#else
extern GunProp	GunProps[NUM_GUN];
extern PistolGunProp	PistolGunProps[NUM_PISTOL_GUN];
extern AutoGunProp	AutoGunProps[NUM_AUTO_GUN];
extern SpecialGunProp	SpecialGunProps[NUM_SPECIAL_GUN];
extern HandtohandGunProp	HandtohandGunProps[NUM_HANDTOHAND_GUN];
extern BeamGunProp	BeamGunProps[NUM_BEAM_GUN];
extern BeamprojGunProp	BeamprojGunProps[NUM_BEAMPROJ_GUN];
extern AmmoProp	AmmoProps[NUM_AMMO];
extern PistolAmmoProp	PistolAmmoProps[NUM_PISTOL_AMMO];
extern NeedleAmmoProp	NeedleAmmoProps[NUM_NEEDLE_AMMO];
extern MagnumAmmoProp	MagnumAmmoProps[NUM_MAGNUM_AMMO];
extern RifleAmmoProp	RifleAmmoProps[NUM_RIFLE_AMMO];
extern FlechetteAmmoProp	FlechetteAmmoProps[NUM_FLECHETTE_AMMO];
extern AutoAmmoProp	AutoAmmoProps[NUM_AUTO_AMMO];
extern ProjAmmoProp	ProjAmmoProps[NUM_PROJ_AMMO];
extern PhysicsProp	PhysicsProps[NUM_PHYSICS];
extern TracerPhysicsProp TracerPhysicsProps[NUM_TRACER_PHYSICS];
extern SlowPhysicsProp SlowPhysicsProps[NUM_SLOW_PHYSICS];
extern CameraPhysicsProp CameraPhysicsProps[NUM_CAMERA_PHYSICS];
extern GrenadeProp	GrenadeProps[NUM_GRENADE];
extern DirectGrenadeProp	DirectGrenadeProps[NUM_DIRECT_GRENADE];
extern TimedGrenadeProp	TimedGrenadeProps[NUM_TIMED_GRENADE];
#endif

#ifdef __OBJSIM_SRC
ObjGun objGuns[NUM_OBJECTS_GUN];
ObjAmmo objAmmos[NUM_OBJECTS_AMMO];
ObjPhysics objPhysicss[NUM_OBJECTS_PHYSICS];
ObjGrenade objGrenades[NUM_OBJECTS_GRENADE];
ObjGun default_gun;
ObjAmmo default_ammo;
ObjPhysics default_physics;
ObjGrenade default_grenade;
#else
extern ObjGun objGuns[NUM_OBJECTS_GUN];
extern ObjAmmo objAmmos[NUM_OBJECTS_AMMO];
extern ObjPhysics objPhysicss[NUM_OBJECTS_PHYSICS];
extern ObjGrenade objGrenades[NUM_OBJECTS_GRENADE];
extern ObjGun default_gun;
extern ObjAmmo default_ammo;
extern ObjPhysics default_physics;
extern ObjGrenade default_grenade;
#endif

#endif // __OBJWPN_H





