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
#ifndef __EFFECT_H
#define __EFFECT_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/effect.h $
 * $Revision: 1.35 $
 * $Author: xemu $
 * $Date: 1994/11/21 21:05:41 $
 *
 */

// Includes
#include "objects.h"
#include "objcrit.h"
#include "objgame.h"
#include "objwpn.h"

#define BLOOD_LIGHT     1
#define CAMERA_EXPL     2
#define TV_EXPL         3
#define SMOKE           4
#define PLNT_EXPL       5
#define BULL_HIT_WALL   6
#define BEAM_HIT_WALL   7
#define IMPACT          8
#define BULLET_ROBOT    9
#define BEAM_ROBOT_LT   10
#define BEAM_ROBOT_HVY  11
#define M_EXPL1         12
#define M_EXPL2         13
#define M_EXPL3         14
#define LG_EXPL         15
#define MAG_HIT         16
#define STUN_HIT        17
#define PLASMA_HIT      18
#define SMOKEY_EXPL     19
#define CRATE_EXPL      20
#define MNTR_EXPL2      21
#define GAS_EXPL        22
#define EMP_EXPL        23
#define CORPSE_HUM_EXPL 24
#define CORPSE_ROB_EXPL 25
#define EFFECT_NUMS     25

// should these be here - or is this just for special effects in the 3D??
#define SHIELD_HIT_EFFECT     40
#define SHIELD_GONE_EFFECT    41
#define BODY_HIT_EFFECT       42

#define CRIT_HIT_NUM       5
#define SEVERITIES         2
#define AMMO_TYPES         4
#define PROJ_TYPE          0
#define BEAM_TYPE          1
#define HAND_TYPE          2
#define GREN_TYPE          3
#define NON_CRITTER_EFFECT       (CRIT_HIT_NUM-1)
#define SPECIAL_TYPE       CRIT_HIT_NUM
 
extern ubyte effect_matrix[CRIT_HIT_NUM][AMMO_TYPES][SEVERITIES];

// info about the destruction of an object
// (a) should we play an effect?
// (b) should we destroy the object DURING the effect
// (c) should we play a sound effect?

#define EFFECT_VAL(x)            ((x) & 0x1F)
#define DESTROY_SOUND_EFFECT(x)  (((x) & 0x60) >> 5)
#define DESTROY_OBJ_EFFECT(x)    ((x) & 0x80)

// these are things about the state of the effects
// they are cleverly hidden in the instance data of the effect
// in start_frame

#define EFFECT_LIGHT_FLAG 0x01

#define EFFECT_LIGHT_MAP_SHIFT      3
#define EFFECT_LIGHT_MAP_MASK       0x78
#define EFFECT_DESTROY_OBJ_FLAG     0x80

#define EFFECT_LIGHT_MAP(x)         (((x) & EFFECT_LIGHT_MAP_MASK) >> EFFECT_LIGHT_MAP_SHIFT)
#define SET_EFFECT_LIGHT_MAP(x,y)   (x=(((x)&~(EFFECT_LIGHT_MAP_MASK))|(y << EFFECT_LIGHT_MAP_SHIFT)))
#define CLEAR_EFFECT_LIGHT_MAP(x)   ((x) &= ~(EFFECT_LIGHT_MAP_MASK))

#define EFFECT_DESTROY_OBJ(x)       ((x) & EFFECT_DESTROY_OBJ_FLAG)
#define SET_EFFECT_DESTROY_OBJ(x)   ((x) |= EFFECT_DESTROY_OBJ_FLAG)
#define CLEAR_EFFECT_DESTROY_OBJ(x) ((x) &= ~(EFFECT_DESTROY_OBJ_FLAG))

#define START_FRAME(x)       ((x) & ~(EFFECT_LIGHT_MAP_MASK | EFFECT_DESTROY_OBJ_FLAG))

// the effect location
#define EFFECT_LOC(id) (objs[id].info.make_info & 0x03) 
#define SET_EFFECT_LOC(id,x) (objs[id].info.make_info &= (~0x03 | x))
#define EFFECT_LEFT 01
#define EFFECT_RIGHT 02
#define EFFECT_CENTER 03

// the effect number
#define EFFECT_NUM(id) ((objs[id].info.make_info & 0x7C) >> 2)
#define SET_EFFECT_NUM(id,x) (objs[id].info.make_info &= (~0x7C | (x<<2)))

// show two effects????????
#define EFFECT_DUAL(id) (objs[id].info.make_info & 0x80)
#define SET_EFFECT_DUAL(id,x) (objs[id].info.make_info &= (x << 7))

// frame count
#define EFFECT_FRAME(id) ((objCritters[objs[id].specID].mood & 0x70) >> 4)
#define SET_EFFECT_FRAME(id,x) (objCritters[objs[id].specID].mood &= (~0x70 | x << 4))

// height's range is from 1-7 (one being the lowest, 3 the center)
#define EFFECT_HEIGHT(id) ((objCritters[objs[id].specID].orders & 0x70) >> 4)
#define SET_EFFECT_HEIGHT(id,x) (objCritters[objs[id].specID].orders &= (~0x70 | x << 4))

// scale's range is from 1-3 (1 being the smallest)
#define EFFECT_SCALE(id) (((objCritters[objs[id].specID].orders & 0x80) >> 6) | \
                           ((objCritters[objs[id].specID].mood & 0x80) >> 7))
#define SET_EFFECT_SCALE(id,x) do { objCritters[objs[id].specID].orders &= (0x7F | ((x&0x02)<<6)); \
                                    objCritters[objs[id].specID].mood   &= (0x7F | ((x&0x01)<<7));} while (0);

#define EFFECT_EIGHTH(id) ((objCritters[objs[id].specID].current_posture & 0xE0) >> 5)
#define SET_EFFECT_EIGHTH(id,x) (objCritters[objs[id].specID].current_posture &= (~0xE0 | (x << 5)))

#define EFFT2TRIP(num) ((num<=NUM_TRANSITORY_ANIMATING)?(BLOOD1_TRIPLE+num-1):(EXPLOSION1_TRIPLE+num-(NUM_TRANSITORY_ANIMATING+1)))

#define OBJ_LOC_TO_LIGHT_LOC(val) ((val+0x80) >> 8)

#define CRITTER_LAMP_MASK     0xF0
#define CRITTER_LAMP_SHIFT    4
#define CRITLIT(id)           (objCritters[objs[id].specID].current_posture)

#define CRITLOCX(id)          (objs[id].info.make_info)
#define SET_CRITLOCX(id,val)  (objs[id].info.make_info=val)
#define CRITLOCY(id)          (objCritters[objs[id].specID].ai_mode)
#define SET_CRITLOCY(id,val)  (objCritters[objs[id].specID].ai_mode=val)

#define CRITTER_LAMP(id) ((CRITLIT(id) & CRITTER_LAMP_MASK) >> CRITTER_LAMP_SHIFT)
#define SET_CRITTER_LAMP(id,lval) (CRITLIT(id) = ((CRITLIT(id)&(~CRITTER_LAMP_MASK))|(lval<<CRITTER_LAMP_SHIFT)))
#define CLEAR_CRITTER_LAMP(id) (CRITLIT(id) &= (~CRITTER_LAMP_MASK))

typedef void (*AnimlistCB)(ObjID id,void *user_data);

// do_special_effect
ObjID do_special_effect(ObjID owner, ubyte effect, ubyte start, ObjID obj, short location);
ObjID do_special_effect_location(ObjID owner, ubyte effect, ubyte start, ObjLoc *loc, short location);
void advance_animations(void);
errtype add_obj_to_animlist(ObjID id, bool repeat, bool reverse, bool cycle, short speed, int cb_id, void *user_data, short cbtype);
errtype remove_obj_from_animlist(ObjID id);
errtype animlist_clear();

#define MAX_ANIMLIST_SIZE  64

#define ANIMCB_REMOVE   1
#define ANIMCB_REPEAT   2
#define ANIMCB_CYCLE    3

#define ANIMFLAG_REPEAT    1
#define ANIMFLAG_REVERSE   2
#define ANIMFLAG_CYCLE     4

typedef struct {
   ObjID id;
   uchar flags;
   short cbtype;
   int callback;
   void *user_data;
   short speed;
} AnimListing;

#ifdef _EFFECT_SRC
short anim_counter = 0;
AnimListing  animlist[MAX_ANIMLIST_SIZE];
#else
extern short anim_counter;
extern AnimListing  animlist[MAX_ANIMLIST_SIZE];
#endif

#endif // __EFFECT_H


