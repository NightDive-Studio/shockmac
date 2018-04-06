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
#ifndef __OBJWAREZ_H
#define __OBJWAREZ_H

/*
 * $Source: n:/project/cit/src/inc/RCS/objwarez.h $
 * $Revision: 1.12 $
 * $Author: xemu $
 * $Date: 1994/04/23 09:18:56 $
 *
 */

// Includes
#include "objclass.h"

// Instance Stuff
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
} ObjDrug;

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
	ubyte version;
} ObjHardware;

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
	ubyte version;
	short data_munge;
} ObjSoftware;

#define SOFTWARE_SECURITY(specid) ((objSoftwares[(specid)].data_munge & 0xF000) >> 12)
#define SOFTWARE_CONTENTS(specid) (objSoftwares[(specid)].data_munge & 0x0FFF)
#define SOFTWARE_SET_MUNGE(specid,sec,cont) (objSoftwares[(specid)].data_munge = ((sec) << 12) & (cont))

// Class Typedefs

typedef struct DrugProp {
   ubyte intensity;
   ubyte delay;
   ubyte duration;
   int   effect;
   int   side_effect;      // should we do a big case statement - therefore this is not needed
   int   after_effect;
   short flags;            // cyberspace?
} DrugProp;

typedef struct HardwareProp {
   short flags;               // activated, damaged??
} HardwareProp;

typedef struct SoftwareProp {
   short flags;               // none right now.
} SoftwareProp;

// Subclass Typedefs
typedef struct StatsDrugProp {
   short effectiveness;
   ubyte sound_effect_num;  // or whatever
   int   duration;
} StatsDrugProp;

typedef struct GoggleHardwareProp {
   ubyte dummy;
} GoggleHardwareProp;

typedef struct HardwareHardwareProp {
   short target_flag;
} HardwareHardwareProp;

typedef struct OffenseSoftwareProp {
   ubyte damage;
} OffenseSoftwareProp;

typedef struct DefenseSoftwareProp {
   ubyte dummy;
} DefenseSoftwareProp;

typedef struct OneshotSoftwareProp {
   ubyte dummy;
} OneshotSoftwareProp;

typedef struct MiscSoftwareProp {
   ubyte dummy;
} MiscSoftwareProp;

typedef struct DataSoftwareProp {
   ubyte dummy;
} DataSoftwareProp;

// Drug
#define NUM_STATS_DRUG	        7

// Hardware
#define NUM_GOGGLE_HARDWARE	5
#define NUM_HARDWARE_HARDWARE	10

// Software
#define NUM_OFFENSE_SOFTWARE	7
#define NUM_DEFENSE_SOFTWARE	3
#define NUM_ONESHOT_SOFTWARE	4
#define NUM_MISC_SOFTWARE	   5
#define NUM_DATA_SOFTWARE     3

// Class count
#define NUM_DRUG	(NUM_STATS_DRUG)
#define NUM_HARDWARE	(NUM_GOGGLE_HARDWARE + NUM_HARDWARE_HARDWARE)
#define NUM_SOFTWARE	(NUM_OFFENSE_SOFTWARE + NUM_DEFENSE_SOFTWARE + NUM_ONESHOT_SOFTWARE + NUM_MISC_SOFTWARE + NUM_DATA_SOFTWARE)

// Enumeration of subclasses
//

// Drug
#define DRUG_SUBCLASS_STATS	         0

// Hardware
#define HARDWARE_SUBCLASS_GOGGLE	      0
#define HARDWARE_SUBCLASS_HARDWARE	   1

// Software
#define SOFTWARE_SUBCLASS_OFFENSE	   0
#define SOFTWARE_SUBCLASS_DEFENSE	   1
#define SOFTWARE_SUBCLASS_ONESHOT	   2
#define SOFTWARE_SUBCLASS_MISC         3
#define SOFTWARE_SUBCLASS_DATA         4

#ifdef __OBJSIM_SRC
DrugProp	DrugProps[NUM_DRUG];
StatsDrugProp	StatsDrugProps[NUM_STATS_DRUG];
HardwareProp	HardwareProps[NUM_HARDWARE];
GoggleHardwareProp	GoggleHardwareProps[NUM_GOGGLE_HARDWARE];
HardwareHardwareProp	HardwareHardwareProps[NUM_HARDWARE_HARDWARE];
SoftwareProp	SoftwareProps[NUM_SOFTWARE];
OffenseSoftwareProp	OffenseSoftwareProps[NUM_OFFENSE_SOFTWARE];
DefenseSoftwareProp	DefenseSoftwareProps[NUM_DEFENSE_SOFTWARE];
OneshotSoftwareProp	OneshotSoftwareProps[NUM_ONESHOT_SOFTWARE];
MiscSoftwareProp	MiscSoftwareProps[NUM_MISC_SOFTWARE];
DataSoftwareProp	DataSoftwareProps[NUM_DATA_SOFTWARE];
#else
extern DrugProp	DrugProps[NUM_DRUG];
extern StatsDrugProp	StatsDrugProps[NUM_STATS_DRUG];
extern HardwareProp	HardwareProps[NUM_HARDWARE];
extern GoggleHardwareProp	GoggleHardwareProps[NUM_GOGGLE_HARDWARE];
extern HardwareHardwareProp	HardwareHardwareProps[NUM_HARDWARE_HARDWARE];
extern SoftwareProp	SoftwareProps[NUM_SOFTWARE];
extern OffenseSoftwareProp	OffenseSoftwareProps[NUM_OFFENSE_SOFTWARE];
extern DefenseSoftwareProp	DefenseSoftwareProps[NUM_DEFENSE_SOFTWARE];
extern OneshotSoftwareProp	OneshotSoftwareProps[NUM_ONESHOT_SOFTWARE];
extern MiscSoftwareProp	MiscSoftwareProps[NUM_MISC_SOFTWARE];
extern DataSoftwareProp	DataSoftwareProps[NUM_DATA_SOFTWARE];
#endif

#ifdef __OBJSIM_SRC
ObjDrug objDrugs[NUM_OBJECTS_DRUG];
ObjHardware objHardwares[NUM_OBJECTS_HARDWARE];
ObjSoftware objSoftwares[NUM_OBJECTS_SOFTWARE];
ObjDrug default_drug;
ObjHardware default_hardware;
ObjSoftware default_software;
#else
extern ObjDrug objDrugs[NUM_OBJECTS_DRUG];
extern ObjHardware objHardwares[NUM_OBJECTS_HARDWARE];
extern ObjSoftware objSoftwares[NUM_OBJECTS_SOFTWARE];
extern ObjDrug default_drug;
extern ObjHardware default_hardware;
extern ObjSoftware default_software;
#endif

#endif // __OBJWAREZ_H



