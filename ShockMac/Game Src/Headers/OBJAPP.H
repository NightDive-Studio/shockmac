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
#ifndef __OBJAPP_H
#define __OBJAPP_H

/*
** $Header: r:/prj/cit/src/inc/RCS/objapp.h 1.25 1994/08/30 07:15:21 xemu Exp $
 * 
*/

//////////////////////////////
//
// An ObjClass is an enum encompassing all the different classes
// in the world.  Be sure to set NUM_CLASSES and CLASS_FIRST
// correctly.
//
// An ObjRefState specifies the location of an ObjRef.
//
// It is made up of two parts:
//
//   - ObjRefStateBin is used to choose where to put an ObjRef
//     in the world-wide data structure.  All ObjRefs with the
//     same ObjRefStateBin are part of the same chain.
//
//   - ObjRefStateInfo is extra information associated with an
//     ObjRef's location.  For example, in Freefall we specify
//     here what rendering triangles an object overlaps.  If
//     you have no such information in your application, then
//     #define NO_OBJ_REF_STATE_INFO.
//
// There must be a specific ObjRefStateBin which is "null".
// For example, it is used to terminate lists of ObjRefStateBins.
// The ObjRefStateBinSetNull and -CheckNull macros respectively
// set a bin to be null and check if it is null.
//
// An ObjLoc specifies the location of an Obj.
// An ObjInfo contains any extra information that you need to have associated
// with each and every object.
//
// #define HASH_OBJECTS if you want ObjRef chains to be hashed
// by location.  You must then define the number of entries, and
// how many of those entries are accessible by the hashing function.
// (I recommend keeping these in the ratio 2:1).  You must also write
// a macro (or function if you prefer) that computes a number n such that
// OBJ_HASH_HEAD_ENTRIES_START <= n < OBJ_HASH_ENTRIES, given an
// ObjRefStateBin.
//
// If you do not #define HASH_OBJECTS, you must write a macro that provides
// the head of the ObjRef chain for a given bin.  (You are responsible for
// keeping a 2-dimensional array of bins, or whatever.)  This must be a macro
// so that the object system can take the address of the result.

////////////////////////////// HERE IS THE STUFF YOU MUST CHANGE
//                          //  
//                          ////////////////////////////////

//#include <map.h>

// #define HASH_OBJECTS
#define NO_OBJ_REF_STATE_INFO

// enumeration of classes
// ## INSERT NEW CLASS HERE

typedef enum ObjClass {
   CLASS_GUN,
   CLASS_AMMO,
   CLASS_PHYSICS,
   CLASS_GRENADE,
   CLASS_DRUG,
   CLASS_HARDWARE,
   CLASS_SOFTWARE,
   CLASS_BIGSTUFF,
   CLASS_SMALLSTUFF,
   CLASS_FIXTURE,
   CLASS_DOOR,
   CLASS_ANIMATING,
   CLASS_TRAP,
   CLASS_CONTAINER,
   CLASS_CRITTER,
	NUM_CLASSES,
	CLASS_FIRST = CLASS_GUN
} ObjClass;

// The total number of objects in the game, and of each type
// ## INSERT NEW CLASS HERE
//

#define NUM_OBJECTS				        872
#define NUM_OBJECTS_GUN               16
#define NUM_OBJECTS_AMMO              32
#define NUM_OBJECTS_PHYSICS           32
#define NUM_OBJECTS_GRENADE           32
#define NUM_OBJECTS_DRUG              32
#define NUM_OBJECTS_HARDWARE          8
#define NUM_OBJECTS_SOFTWARE          16
#define NUM_OBJECTS_BIGSTUFF          176
#define NUM_OBJECTS_SMALLSTUFF        128
#define NUM_OBJECTS_FIXTURE           64
#define NUM_OBJECTS_DOOR              64
#define NUM_OBJECTS_ANIMATING         32
#define NUM_OBJECTS_TRAP              160
#define NUM_OBJECTS_CONTAINER         64
#define NUM_OBJECTS_CRITTER           64

// THe total number of references of objects
#define NUM_REF_OBJECTS		1600

// i hate cpp, no sizeof() in #if, so we have to do this
#define SIZEOF_AN_OBJREFSTATEBIN 4
typedef struct {
	LGPoint sq;
} ObjRefStateBin;

#define OBJREF_SQ(ori) (objRefs[ori].state.bin.sq)

// i hate cpp, no sizeof() in #if, so we have to do this
#define SIZEOF_AN_OBJREFSTATEINFO 1
typedef struct {
	uchar flags;
} ObjRefStateInfo;

#define ObjRefStateBinSetNull(bin) PointSetNull((bin).sq)
#define ObjRefStateBinCheckNull(bin) (PointCheckNull((bin).sq))

// i hate cpp, no sizeof() in #if, so we have to do this
#define SIZEOF_AN_OBJLOC 8
typedef struct {
	ushort x, y;								// high 8 bits: what square      low 8 bits: where within square
   ubyte z;	
   ubyte p, h, b;
} ObjLoc;

#define OBJ_LOC_BIN_X(oloc) ((oloc).x >> 8)
#define OBJ_LOC_BIN_Y(oloc) ((oloc).y >> 8)
#define OBJ_LOC_FINE_X(oloc) ((ushort)((oloc).x & 0xFF))
#define OBJ_LOC_FINE_Y(oloc) ((ushort)((oloc).y & 0xFF))
#ifdef SAFE_FIX
#define OBJ_LOC_VAL_TO_FIX(value) (fix_make((value >> 8), ((value & 0xFF) << 8)))
#else
#define OBJ_LOC_VAL_TO_FIX(value) (((fix)value)<<8)				
#endif

typedef struct {
	char ph;
   byte type;
   short current_hp;       
   ubyte make_info;                    // maker, as in Zortech MK III laser rifle or whatever
   ubyte current_frame;                // animdata
   ubyte time_remainder;               // animdata
   uchar inst_flags;                   //  flags for instance data.  right now 0x01 is used by Mahk's render tricks
} ObjInfo;

typedef struct {
	int ph;
   byte type;
   short current_hp;       
   ubyte make_info;                    // maker, as in Zortech MK III laser rifle or whatever
   ubyte current_frame;                // animdata
   ubyte time_remainder;               // animdata
   uchar inst_flags;                   //  flags for instance data.  right now 0x01 is used by Mahk's render tricks
} old_ObjInfo;

#ifdef HASH_OBJECTS
# define OBJ_HASH_ENTRIES 512
# define OBJ_HASH_HEAD_ENTRIES 256
# define OBJ_HASH_HEAD_ENTRIES_START (OBJ_HASH_ENTRIES - OBJ_HASH_HEAD_ENTRIES)
# define OBJ_HASH_FUNC(bin) ((((((bin).sq.x) << 2) + ((bin).sq.y)) & (OBJ_HASH_HEAD_ENTRIES - 1)) + OBJ_HASH_HEAD_ENTRIES_START)
# define ObjRefHead(bin) (objHashTable[ObjGetHashElem((bin),FALSE)].ref) /* don't change this */
#else
# define ObjRefHead(bin) (MAP_GET_XY((bin).sq.x,(bin).sq.y))->objRef
#endif

//                          ////////////////////////////////
//                          //  
////////////////////////////// WASN'T THAT EASY?

typedef struct {
	ObjRefStateBin bin;
} ObjRefState;

typedef struct {
	ObjRefStateBin bin;
	ObjRefStateInfo info;
} oldObjRefState;

// The following macros perform simple comparing and copying operations.
// If your structures are immensely complicated, you can turn them into
// functions.  It will slow things down, though.

// isnt it neat that you cant do sizeof(ObjRefStateBin) in a #if
// i love cpp with an unholy, inhuman, and altogether pathetic way

#if (SIZEOF_AN_OBJREFSTATEBIN==4)
#define ObjRefStateBinEqual(bin1,bin2) (*((int *)(&bin1))==*((int *)(&bin2)))
#elif (SIZEOF_AN_OBJREFSTATEBIN==2)
#define ObjRefStateBinEqual(bin1,bin2) (*((short *)(&bin1))==*((short *)(&bin2)))
#elif (SIZEOF_AN_OBJREFSTATEBIN==1)
#define ObjRefStateBinEqual(bin1,bin2) (*((char *)(&bin1))==*((char *)(&bin2)))
#else
#define ObjRefStateBinEqual(bin1,bin2) (!memcmp(&(bin1),&(bin2),sizeof(ObjRefStateBin)))
#endif
#define ObjRefStateBinCopy(srcbin, dstbin) do {dstbin = srcbin;} while (0)

#ifndef NO_OBJ_REF_STATE_INFO
# if (SIZEOF_AN_OBJREFSTATEINFO==4)
# define ObjRefStateInfoEqual(info1,info2) (*((int *)(&info1))==*((int *)(&info2)))
# elif (SIZEOF_AN_OBJREFSTATEINFO==2)
# define ObjRefStateInfoEqual(info1,info2) (*((short *)(&info1))==*((short *)(&info2)))
# elif (SIZEOF_AN_OBJREFSTATEINFO==1)
# define ObjRefStateInfoEqual(info1,info2) (*((char *)(&info1))==*((char *)(&info2)))
# else
# define ObjRefStateInfoEqual(info1,info2) (!memcmp(&(info1),&(info2),sizeof(ObjRefStateInfo)))
# endif
# define ObjRefStateInfoCopy(srcinfo, dstinfo) do {dstinfo = srcinfo;} while (0)
#endif

#if (SIZEOF_AN_OBJLOC==4)
#define ObjLocEqual(bin1, bin2)        (*((int *)(&bin1))==*((int *)(&bin2))) 
#elif (SIZEOF_AN_OBJLOC==2)
#define ObjLocEqual(bin1, bin2)        (*((short *)(&bin1))==*((short *)(&bin2)))
#elif (SIZEOF_AN_OBJLOC==1)
#define ObjLocEqual(bin1, bin2)        (*((char *)(&bin1))==*((char *)(&bin2)))
#else
#define ObjLocEqual(bin1, bin2)        (!memcmp(&(bin1),&(bin2),sizeof(ObjLoc)))
#endif
#define ObjLocCopy(srcbin, dstbin)     do {dstbin = srcbin;} while (0)

////////////////////////////// MORE STUFF YOU MUST CHANGE
//                          //
//                          ////////////////////////////////
//
// You can turn some of the following macros into functions, if they get complicated.
//
// These are all for the use of the debugging system.  They should print
// a user-friendly representation of the appropriate structure into str,
// without a trailing newline.

#define ObjRefStateSprint(str,refstate)
#define ObjRefStateBinSprint(str,bin)
#define ObjRefStateInfoSprint(str,info)
#define ObjLocSprint(str,loc)
#define ObjInfoSprint(str,info)

//                          ////////////////////////////////
//                          //  
////////////////////////////// END OF STUFF YOU MUST CHANGE

////////////////////////////////////////////////////////////
//
// Here are prototypes of a few functions you should define in
// objapp.c.  Any macros above that you decided to turn into functions
// should also be defined in objapp.c.
//

void ObjInfoInit (ObjInfo *info);

//////////////////////////////
//
// This should initialize the following iterator.  Nice name, huh?

void ObjRefStateBinIteratorInit (void);

//////////////////////////////
//
// After ObjRefStateBinIteratorInit () has been called, calling this
// should put a new valid ObjRefStateBin in bin every time it is called.
// It returns FALSE if it has already returned all valid bins (in which
// case the value of bin is undefined); otherwise it returns TRUE, of
// course.
//
// It can use static variables; the iterator is guaranteed to be active
// only once at a time.

bool ObjRefStateBinIterator (ObjRefStateBin *bin);

#endif //OBJAPP_H
