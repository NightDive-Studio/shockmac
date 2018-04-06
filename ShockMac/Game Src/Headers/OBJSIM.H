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
#ifndef __OBJSIM_H
#define __OBJSIM_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/objsim.h $
 * $Revision: 1.60 $
 * $Author: xemu $
 * $Date: 1994/09/06 23:47:57 $
 *
 *
 */

// Includes
#include "objects.h"

#define ID2SPEC(id)  (objs[(id)].specID)

#ifdef __OBJSIM_SRC
int ObjBaseArray[255];
int ClassBaseArray[16][16];
#else
extern int ObjBaseArray[255];
extern int ClassBaseArray[16][16];
#endif

#define OBJ_PLAYER_CAMERA  0
#define OBJ_STATIC_CAMERA  1
#define OBJ_CURRENT_CAMERA 2
#define OBJ_DYNAMIC_CAMERA 3

#define FIND_TRIPLE     0
#define FIND_SUBCLASS   1
#define FIND_CLASS      2
#define FIND_ALL        3

// Functions for getting the bitmap for a given critter on a specific frame, from a specific view
// macros for now -- this is basically a cheap interface to the resource system's LRU cache
#define get_critter_bitmap(id,triple,posture,frame,view) (lock_bitmap_from_ref(ref_from_critter_data((id),(triple),(posture),(frame),(view))))
#define release_critter_bitmap(triple,posture,frame,view) (RefUnlock(ref_from_critter_data((triple),(posture),(frame),(view))))

#define get_critter_bitmap_fast(id,triple,posture,frame,view,pref,panch) (lock_bitmap_from_ref_anchor(*(pref) = ref_from_critter_data((id),(triple),(posture),(frame),(view)),(panch)))	
#define release_critter_bitmap_fast(ref) (RefUnlock((ref)))

// macros for calling those other macros with only an object ID
#define get_critter_bitmap_obj(id,view) (get_critter_bitmap((id),ID2TRIP((id)),get_crit_posture(ID2SPEC(id)),objs[(id)].info.current_frame,(view)))
#define release_critter_bitmap_obj(id,view) (release_critter_bitmap(ID2TRIP((id)),get_crit_posture(ID2SPEC(id)),objs[(id)].info.current_frame,(view)))
#define get_critter_bitmap_obj_fast(id,view,pref,panch) (get_critter_bitmap_fast((id),ID2TRIP((id)),get_crit_posture(ID2SPEC(id)),objs[(id)].info.current_frame,(view),(pref),(panch)))	

#define DOOR_ID_BASE 2400
#define door_id(id) (DOOR_ID_BASE + CPTRIP(ID2TRIP(id)))

Ref obj_cache_ref(ObjID id);
#define get_obj_cache_bitmap(id,pref) (lock_bitmap_from_ref(*(pref) = obj_cache_ref(id)))
#define release_obj_cache_bitmap(ref) (RefUnlock((ref)))

// Animation
#define get_anim_bitmap(triple,frame) (obj_bitmaps[ObjProps[OPTRIP((triple))].bitmap_3d + (frame)])
#define get_anim_bitmap_obj(id) (get_anim_bitmap(ID2TRIP((id)),objAnimatings[ID2SPEC(id)].current_frame))
// keep starting index in lower two bytes, keep # of anim frames in next-to-top byte.
// wow, this has become gruesomely complex
// we will consider the possbility of just moving all the other flags around in order to
// steal that top bit, rather than this gruesome nightmare
#define BMAP_NUM_3D(x)  (((x) & 0x3FF) + (((x) & (0x8000)) >> 5))
#define FRAME_NUM_3D(x) (((x) & 0x7000) >> 12)
#define REPEAT_3D(x)    (((x) & 0x0800) >> 11)
#define ANIM_3D(x)      (((x) & 0x0400) >> 10)

// Textured Polygons
grs_bitmap *bitmap_from_tpoly_data(int tpdata, ubyte *scale, int *index, uchar *type, Ref *ref);

// Prototypes
errtype obj_init();
errtype obj_shutdown();
ObjID obj_create_base(int triple);
ObjID obj_create_clone(ObjID dna);
errtype obj_move_to_vel(ObjID id, ObjLoc *newloc, bool phys_tel, fix x_dot, fix y_dot, fix z_dot);
errtype obj_move_to(ObjID id, ObjLoc *newloc, bool phys_tel);
bool obj_destroy(ObjID id);
errtype obj_holocaust();
bool obj_holocaust_func(short keycode, ulong context, void* data);
errtype obj_load_properties();
errtype obj_create_player(ObjLoc *plr_loc);
Ref ref_from_critter_data(ObjID oid, int triple, byte posture, short frame, short view);
errtype obj_zero_unused();
errtype obj_model_hack(ObjID id, uchar *hack_x, uchar *hack_y, uchar *hack_z, uchar *hack_type);
bool obj_combat_destroy(ObjID id);
ObjID object_place(int triple, LGPoint square);
ushort obj_floor_compute(ObjID id, uchar flrh);
ushort obj_floor_height(ObjID id);
errtype obj_physics_refresh_area(short x, short y, bool use_floor);

ObjID physics_handle_to_id(physics_handle p);

#ifdef __OBJSIM_SRC
ObjID current_object = OBJ_NULL;
#else
extern ObjID current_object;
#endif

#endif  // __OBJSIM_H
