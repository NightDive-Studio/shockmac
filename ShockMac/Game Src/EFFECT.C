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
/*
 * $Source: r:/prj/cit/src/RCS/effect.c $
 * $Revision: 1.111 $
 * $Author: xemu $
 * $Date: 1994/11/21 21:06:39 $
 *
 */

#define _EFFECT_SRC

#include <string.h>

#include "objgame.h"
#include "effect.h"
#include "weapons.h"  // for handart stuff
#include "objprop.h"
#include "objwpn.h"
#include "objsim.h"
#include "faketime.h"
#include "gametime.h"
#include "damage.h"
#include "player.h"
#include "mainloop.h"
#include "ai.h"
#include "objsim.h"
#include "objbit.h"
#include "otrip.h"
#include "cybrnd.h"
#include "schedule.h"
#include "wares.h"
#include "textmaps.h"
#include "frparams.h"
#include "gamesys.h"
#include "hudobj.h"
#include "aiflags.h"
#include "mapflags.h"
#include "doorparm.h"


#define HANDART_SPEED      85

#define MAX_ANIMLIST_CALLBACKS   10
AnimlistCB animlist_callbacks[MAX_ANIMLIST_CALLBACKS];

ubyte effect_matrix[CRIT_HIT_NUM][AMMO_TYPES][SEVERITIES] = {
   {
      // soft mutants - blood
      {BLOOD_LIGHT, BLOOD_LIGHT},      // projectiles
      {BLOOD_LIGHT, BLOOD_LIGHT},      // beam
      {BLOOD_LIGHT, BLOOD_LIGHT},      // hand-to-hand
      {M_EXPL2, M_EXPL2}               // grenades
   },
   {
      // plant mutant
      {PLNT_EXPL, PLNT_EXPL},          // projectiles
      {PLNT_EXPL, PLNT_EXPL},          // beam
      {PLNT_EXPL, PLNT_EXPL},          // hand-to-hand
      {PLNT_EXPL, PLNT_EXPL}          // grenades
   },
   {
      // robot
      {BULLET_ROBOT,BULLET_ROBOT}, // projectile
      {BEAM_ROBOT_LT,BEAM_ROBOT_HVY},     // beam guns
      {IMPACT,IMPACT},                    // hand-to-hand               
      {M_EXPL2,M_EXPL2}                  // grenades
   },
   {
      // cyborgs
      {BLOOD_LIGHT,BLOOD_LIGHT},          // flechette bullets
      {BEAM_ROBOT_HVY,BEAM_ROBOT_HVY},     // beam guns
      {IMPACT,IMPACT},                    // hand-to-hand
      {M_EXPL2,M_EXPL2}
   },
   {
      // all other objects
      {BULL_HIT_WALL, BULL_HIT_WALL},
      {BEAM_HIT_WALL, BEAM_HIT_WALL},
      {IMPACT, IMPACT},
      {M_EXPL2, M_EXPL2}
   }
};

// External Prototypes
extern void do_object_explosion(ObjID id);

// Internal Prototypes
void critter_light_world(ObjID id);
void critter_unlight_world(ObjID id);
int anim_frames(ObjID id);
errtype increment_anim(ulong num_units);
bool anim_data_from_id(ObjID id, bool* reverse, bool* cycle);
void init_animlist(void);

// -----------------------------------------------------------------
// do_special_effect_location
//

ObjID beam_effect_id=OBJ_NULL;

ObjID do_special_effect_location(ObjID owner, ubyte effect, ubyte start, ObjLoc *loc, short )
{
	ObjID				new_id=OBJ_NULL;
	ObjSpecID			osid;
	int					triple = 0;

   // are we suppose to destroy the attached object?
   bool  special = DESTROY_OBJ_EFFECT(effect);

   // strip out the extra stuff
   effect = EFFECT_VAL(effect);

   if ((effect < 1) || (effect > EFFECT_NUMS))
      return (OBJ_NULL);

   triple = EFFT2TRIP(effect);
   if (triple)
   {
	   new_id = obj_create_base(triple);
      if (new_id == OBJ_NULL)
      {
         // make sure that if we're suppose to do an explosion - we do it!
         if (ExplosionAnimatingProps[SCTRIP(triple)].frame_explode && special)
            do_object_explosion(owner);
         return(OBJ_NULL);
      }
      osid = objs[new_id].specID;
	   obj_move_to(new_id, loc, TRUE);
	   if (start == 0xFF)
	      start = START_FRAME(objAnimatings[osid].start_frame);
      objAnimatings[osid].owner = owner;
	   objs[new_id].info.current_frame = start;
      if (TRIP2SC(triple) == ANIMATING_SUBCLASS_EXPLOSION)
      {
         if (ExplosionAnimatingProps[SCTRIP(triple)].frame_explode && special)
            SET_EFFECT_DESTROY_OBJ(objAnimatings[osid].start_frame);
      }
      if (AnimatingProps[CPNUM(new_id)].flags & EFFECT_LIGHT_FLAG)
      {
         MapElem  *mmp;
         ubyte    i;
         ubyte    x = OBJ_LOC_BIN_X(*loc);
         ubyte    y = OBJ_LOC_BIN_Y(*loc);
         ubyte    light_bits = 0;

         for (i=0; i<4;i++)
         {
            mmp=MAP_GET_XY(x+(i/2), y+(i%2));
            if (!me_bits_rend3(mmp))
            {
               me_rend3_set(mmp, me_bits_rend3(mmp)+1);
               light_bits |= (1 << i);
            }
         }
         SET_EFFECT_LIGHT_MAP(objAnimatings[osid].start_frame, light_bits);
      }
   }
   return(new_id);
}


// --------------------------------------------------------------
// do_special_effect()
//

ObjID do_special_effect(ObjID owner, ubyte effect, ubyte start, ObjID target_id, short location)
{
   ObjLoc   loc= objs[target_id].loc;
   
   return(do_special_effect_location(owner, effect, start, &loc, location));
}

void critter_light_world(ObjID id)
{
   int      j;
   ubyte    light_bits=0;
   ubyte    x = OBJ_LOC_BIN_X(objs[id].loc);
   ubyte    y = OBJ_LOC_BIN_Y(objs[id].loc);
   MapElem *mmp;

   for (j=0; j<4;j++)
   {
      mmp=MAP_GET_XY(x+(j/2), y+(j%2));
      if (!me_bits_rend3(mmp))
      {
         me_rend3_set(mmp, me_bits_rend3(mmp)+1);
         light_bits |= (1 << j);
      }
   }

   if (light_bits)
   {
      SET_CRITLOCX(id,x);
      SET_CRITLOCY(id,y);
      SET_CRITTER_LAMP(id, light_bits);
   }
}

void critter_unlight_world(ObjID id)
{
   ubyte    light_bits = CRITTER_LAMP(id);
   ubyte    x = CRITLOCX(id);
   ubyte    y = CRITLOCY(id);
   ubyte    j;
   MapElem *mmp;

   if (light_bits)
   {
      for (j=0; j<4;j++)
      {
         if (light_bits & (1 << j))
         {
            mmp=MAP_GET_XY(x+(j/2), y+(j%2));
            me_rend3_set(mmp, me_bits_rend3(mmp)-1);
         }
      }
      CLEAR_CRITTER_LAMP(id);
   }
}


#define DEFAULT_ANIMLIST_SPEED 128
// ---------------------------------------------------------
// anim_frames()
//

// in gameobj.c also
#define MAX_TELEPORT_FRAME    10
#define DIEGO_DEATH_BATTLE_LEVEL 8

int anim_frames(ObjID id)
{
   int retval = 1;
   RefTable *prt;

   switch(objs[id].obclass)
   {
      case CLASS_DOOR:
//		prt = ResReadRefTable(door_id(id));
		prt = (RefTable *)ResLock(door_id(id));
		retval = prt->numRefs;
//		ResFreeRefTable(prt);								
		ResUnlock(door_id(id));
	      break;
      default:
	      switch(objs[id].obclass)
	      {
	         case CLASS_BIGSTUFF:
	            retval = objBigstuffs[objs[id].specID].cosmetic_value;
	            if (retval == 0)
      		      retval = 1;
	            break;
	         case CLASS_SMALLSTUFF:
	            retval = objSmallstuffs[objs[id].specID].cosmetic_value;
	            if (retval == 0)
      		      retval = 4;
	            break;
            case CLASS_CRITTER:
               if ((ID2TRIP(id) == DIEGO_TRIPLE) && (get_crit_posture(objs[id].specID) == DEATH_CRITTER_POSTURE) &&
                  (player_struct.level != DIEGO_DEATH_BATTLE_LEVEL))
                  retval = MAX_TELEPORT_FRAME;
               break;
	         default:
	            retval = FRAME_NUM_3D(ObjProps[OPNUM(id)].bitmap_3d);
	            break;
	      }
         break;
   }
   return(retval);
}


// Light #defines
#define BRIGHT_LIGHT_FLASH 60L    // brightness of flash
#define LIGHT_DELTA 4           // time length of flash

extern void lamp_change_setting(byte offset);
extern ubyte energy_expulsion;
extern ubyte handart_count;
extern bool handart_flash;

#define DEFAULT_ANIMATION_SPEED  32

#ifdef USE_ANIMCRIT_DEFS
#define STANDARD_CRITTER_SPEED   fix_make(0,0x4000)
#define MIN_CRITTER_ANIM_SPEED   25
#define MAX_CRITTER_ANIM_SPEED   200
#define MIN_MOJO                 fix_make(0,0x1A00)
#else
fix standard_critter_speed = fix_make(0,0x5800);
fix min_mojo = fix_make(0,0x1a00);
int min_critter_anim_speed = 35;
int max_critter_anim_speed = 170;
int attacking_anim_speed = 45;
#endif

errtype increment_anim(ulong num_units)
{
   ObjSpecID      osid;
   ObjID          id;
   uchar          anim_rem[MAX_ANIMLIST_SIZE];
   uchar          cb_list[MAX_ANIMLIST_SIZE];   
   int            triple,num_frames;
   char curr_frames;
   int post,i;
   short rem_num=0, cb_num=0;
   int interval;
   ulong new_units;
   ulong hand_speed=HANDART_SPEED;
   ubyte old_handart;
   LightSchedEvent      new_event;
   extern bool anim_on;
#ifdef SPEW_ON
   char ft1[30];
#endif

   // *************************************************************
   //                   HAND ART 
   // *************************************************************

   if (handart_show > 1)
   {
      old_handart = handart_show;
      new_units = num_units + handart_remainder;
      if (handart_count != 2)
      {
         hand_speed = hand_speed/2;
      }
      handart_show += (new_units / hand_speed);
      handart_remainder = new_units % hand_speed;
      if (old_handart != handart_show)
         chg_set_flg(_current_3d_flag);
      
      // check to see if we're going to skip the fire frame, if so, don't skip it!
      // - this is so we definitely show the fire frame, otherwise it would look very goooooofy - minman

      if (handart_show > (handart_count&0x7F))
         handart_show = (handart_fire) ? 1 : 2;
   }
   if ((handart_show != 1) && handart_flash)
   {
      byte  light_val;
      ubyte slot = player_struct.actives[ACTIVE_WEAPON];
      extern byte gun_fire_offset;

      switch (player_struct.weapons[slot].type)
      {
         case (GUN_SUBCLASS_BEAM):
            if (energy_expulsion)
               light_val = (ubyte) ((BRIGHT_LIGHT_FLASH*energy_expulsion)/100);
            break;
         case (GUN_SUBCLASS_HANDTOHAND):
            light_val = 0;
            break;
         case (GUN_SUBCLASS_PISTOL):
            if (player_struct.weapons[slot].subtype==1)
            {
               light_val=0;
               break;
            }
         default:
            light_val = BRIGHT_LIGHT_FLASH;
            break;
      }

      if (light_val)
      {
         light_val += gun_fire_offset;

         new_event.timestamp = TICKS2TSTAMP(player_struct.game_time) + LIGHT_DELTA;
         new_event.type = LIGHT_SCHED_EVENT;
         new_event.light_value = light_val;
         new_event.previous = (player_struct.hardwarez_status[CPTRIP(LANTERN_HARD_TRIPLE)] & WARE_ON);
         {
            errtype err = schedule_event(&game_seconds_schedule,(SchedEvent *) &new_event);
         }
         lamp_change_setting(light_val);
         _frp_light_bits_set(LIGHT_BITS_CAM);
         handart_flash = FALSE;
      }
   }

   if (!anim_on)
      return(OK);

   // ****************************************************
   // Class Animating
   // ****************************************************

   osid = objAnimatings[0].id;
   while (osid != OBJ_SPEC_NULL)
   {
      short dest_frame;

      id = objAnimatings[osid].id;
      if (id == OBJ_NULL)
      {
         osid = OBJ_SPEC_NULL;
      }
      else
      {
         ubyte spd;
         int cptrip;

         triple = MAKETRIP(objs[id].obclass, objs[id].subclass, objs[id].info.type);
         new_units = num_units + objs[id].info.time_remainder;
         cptrip = CPTRIP(triple);
         spd = AnimatingProps[cptrip].speed;
         if (spd == 0)
         {
            spd = DEFAULT_ANIMATION_SPEED;
         }
         interval = new_units / spd;
         objs[id].info.time_remainder = new_units % spd;
         objs[id].info.current_frame += interval;
         if (objs[id].subclass == ANIMATING_SUBCLASS_EXPLOSION)
         {
	         if (EFFECT_DESTROY_OBJ(objAnimatings[osid].start_frame) &&
                (objs[id].info.current_frame >= ExplosionAnimatingProps[SCNUM(id)].frame_explode))
            {
               do_object_explosion(objAnimatings[osid].owner);
               CLEAR_EFFECT_DESTROY_OBJ(objAnimatings[osid].start_frame);
            }
         }
//         dest_frame = objAnimatings[osid].end_frame;
//         if (dest_frame==0)
         dest_frame = FRAME_NUM_3D(ObjProps[OPTRIP(triple)].bitmap_3d);
         if ((objs[id].info.current_frame != 255) && (objs[id].info.current_frame > dest_frame))
         {
            if (AnimatingProps[CPNUM(id)].flags & EFFECT_LIGHT_FLAG)
            {
               ubyte    i;
               ubyte    light_bits = EFFECT_LIGHT_MAP(objAnimatings[objs[id].specID].start_frame);
               ubyte    x = OBJ_LOC_BIN_X(objs[id].loc);
               ubyte    y = OBJ_LOC_BIN_Y(objs[id].loc);
               MapElem *mmp;

               for (i=0; i<4;i++)
               {
                  if (light_bits & (1 << i))
                  {
                     mmp=MAP_GET_XY(x+(i/2), y+(i%2));
                     me_rend3_set(mmp, me_bits_rend3(mmp)-1);
                  }
               }
               CLEAR_EFFECT_LIGHT_MAP(objAnimatings[objs[id].specID].start_frame);
            }

	         switch(objs[id].subclass)
	         {
	            case ANIMATING_SUBCLASS_TRANSITORY:
	            case ANIMATING_SUBCLASS_EXPLOSION:
	               ADD_DESTROYED_OBJECT(id);
                    if (id == beam_effect_id)
                    {
                       hudobj_set_id(id,FALSE);
                       beam_effect_id = OBJ_NULL;
                    }
	               break;
	            default:
 	               objs[id].info.current_frame = START_FRAME(objAnimatings[osid].start_frame);
	               break;
	         }
         }
         osid = objAnimatings[osid].next;
      }
   }

   // Objects on the animation list
   LG_memset(anim_rem,0,MAX_ANIMLIST_SIZE);
   LG_memset(cb_list,0,MAX_ANIMLIST_SIZE);
   for (i=0; i < anim_counter; i++)
   {
      id = animlist[i].id;
      num_frames = anim_frames(id);
      new_units = num_units + objs[id].info.time_remainder;
      interval = new_units / animlist[i].speed;
      objs[id].info.time_remainder = new_units % animlist[i].speed;
      if (animlist[i].flags  & ANIMFLAG_REVERSE)
      {
         switch (objs[id].obclass)
         {
            case CLASS_DOOR:
               if ((objs[id].info.current_frame - interval < DOOR_OPEN_FRAME) && (objs[id].info.current_frame >= DOOR_OPEN_FRAME))
               {
                  obj_physics_refresh_area(OBJ_LOC_BIN_X(objs[id].loc), OBJ_LOC_BIN_Y(objs[id].loc),TRUE);
               }
               break;
         }
         if (objs[id].info.current_frame < interval)
	      {
	         if (animlist[i].flags & ANIMFLAG_CYCLE)
	         {
	            if ((animlist[i].callback != 0) && (animlist[i].cbtype & ANIMCB_CYCLE))
                  cb_list[cb_num++] = i;
	            objs[id].info.current_frame = 0;
	            // turn around
	            animlist[i].flags &= ~ANIMFLAG_REVERSE;
	         }
	         else if (animlist[i].flags & ANIMFLAG_REPEAT)
	         {
	            if ((animlist[i].callback != 0) && (animlist[i].cbtype & ANIMCB_REPEAT))
                  cb_list[cb_num++] = i;
	            objs[id].info.current_frame = num_frames - 1;
	         }
	         else
	         {
	            objs[id].info.current_frame = 0;
	            anim_rem[rem_num++] = i;
               switch(objs[id].obclass)
               {
                  case CLASS_DOOR:
                     // if we are a kind of door that blocks the renderer, and we are closed, then
                     // set our instance bit so that the renderer can know about it.   
                     if (RENDER_BLOCK & ObjProps[OPNUM(id)].flags)
                        objs[id].info.inst_flags |= RENDER_BLOCK_FLAG;
                     break;
               }
	         }
         }     
	      else
	         objs[id].info.current_frame -= interval;
      }
      else
      {
         switch (objs[id].obclass)
         {
            case CLASS_DOOR:
               if (((objs[id].loc.p != 0) || (objs[id].loc.b != 0)) && 
                  (objs[id].info.current_frame < DOOR_OPEN_FRAME) && (objs[id].info.current_frame + interval >= DOOR_OPEN_FRAME))
               {
                  obj_physics_refresh_area(OBJ_LOC_BIN_X(objs[id].loc), OBJ_LOC_BIN_Y(objs[id].loc),TRUE);
               }
               break;
         }
	      objs[id].info.current_frame += interval;

	      if (objs[id].info.current_frame >= num_frames)
	      {
	         if (animlist[i].flags & ANIMFLAG_CYCLE)
	         {
	            if ((animlist[i].callback != 0) && (animlist[i].cbtype & ANIMCB_CYCLE))
                  cb_list[cb_num++] = i;
	            objs[id].info.current_frame = num_frames - 1;
	            // turn around
	            animlist[i].flags |= ANIMFLAG_REVERSE;
	         }
	         else if (animlist[i].flags & ANIMFLAG_REPEAT)
	         {
	            if ((animlist[i].callback != 0) && (animlist[i].cbtype & ANIMCB_REPEAT))
                  cb_list[cb_num++] = i;
	            objs[id].info.current_frame = 0;
	         }
	         else
	         {
	            objs[id].info.current_frame = num_frames - 1;
	            anim_rem[rem_num++] = i;
	         }
	      }
      }
   }

   for (i=0; i < cb_num; i++)
      animlist_callbacks[animlist[cb_list[i]].callback](animlist[cb_list[i]].id, animlist[cb_list[i]].user_data);
   for (i=0; i < rem_num; i++)
      remove_obj_from_animlist(animlist[anim_rem[i]].id);

   // Class Critter
   // iterate through all the critters, and update their frames...
   osid = objCritters[0].id;
   while (osid != OBJ_SPEC_NULL)
   {
      int cptripnum,asp;
      id = objCritters[osid].id;
      triple = MAKETRIP(objs[id].obclass, objs[id].subclass, objs[id].info.type);
      if ((triple == DIEGO_TRIPLE) && (get_crit_posture(objs[id].specID) == DEATH_CRITTER_POSTURE) &&
                  (player_struct.level != DIEGO_DEATH_BATTLE_LEVEL))
               {
                  osid = objCritters[osid].next;
                  continue;
               }
      if (EFFECT_LOC(id))
      {
//         ulong time = (player_struct.game_time & 0x03);
//         if ((time/32) != EFFECT_EIGHTH(id))
//         {
//            SET_EFFECT_FRAME(id,EFFECT_FRAME(id)+(((time/32)+8-EFFECT_EIGHTH(id))%8));
  //          if (EFFECT_FRAME(id) > FRAME_NUM_3D(ObjProps[OPTRIP(EFFT2TRIP(EFFECT_NUM(id)))].bitmap_3d))
 //              SET_EFFECT_LOC(id,0);
   //         SET_EFFECT_EIGHTH(id, (time/32));
    //     }
      }
      if ((id != PLAYER_OBJ) && 
         (!ai_critter_sleeping(osid) || (get_crit_posture(osid) == DEATH_CRITTER_POSTURE)))
      {
         new_units = num_units + objs[id].info.time_remainder;
         cptripnum = CPTRIP(triple);
         post = get_crit_posture(osid);
         asp = CritterProps[cptripnum].anim_speed;
         if (asp == 0)
         {
            asp = 1;
         }
         else
         {
            if ((post == MOVING_CRITTER_POSTURE) && (objs[id].info.ph != -1))
            {
               State s;
               fix pd;
#ifdef USE_PHYS_STATE
               extern void get_phys_state(int ph, State *new_state, ObjID id);
               get_phys_state(objs[id].info.ph, &s,id);
#else
               EDMS_get_state(objs[id].info.ph, &s);
#endif
               pd = fix_fast_pyth_dist(s.X_dot,s.Y_dot);
               if (pd > min_mojo)
               {
                  asp = fix_int(fix_mul_div(fix_make(asp,0), standard_critter_speed, pd));
                  if (asp < min_critter_anim_speed) 
                     asp = min_critter_anim_speed;
                  else if (asp > max_critter_anim_speed) 
                     asp = max_critter_anim_speed;
               }
            } 
            else if ((post == ATTACKING_CRITTER_POSTURE) || (post == ATTACKING2_CRITTER_POSTURE))
            {
               asp = attacking_anim_speed;
            }
         }
         interval = new_units/asp;
         objs[id].info.time_remainder = new_units % asp;
			objs[id].info.inst_flags &= ~(UNLIT_FLAG);

         if (CRITTER_LAMP(id) && (ID2TRIP(id) != AUTOBOMB_TRIPLE))
            critter_unlight_world(id);

         // Do attack if at right point in anim
         curr_frames = (objs[id].subclass == CRITTER_SUBCLASS_CYBER) ? 4 : CritterProps[CPTRIP(triple)].frames[post];
         if ((post == ATTACKING_CRITTER_POSTURE) || (post == ATTACKING2_CRITTER_POSTURE))
         {
            short att_frame;

            att_frame = (CritterProps[CPTRIP(triple)].fire_frame == 0) ?
               (curr_frames*3)/4 : CritterProps[CPTRIP(triple)].fire_frame;

	         // Only attack when first reaching or passing sancted attack frame
	         if ((objs[id].info.current_frame < att_frame) && (objs[id].info.current_frame + interval >= att_frame))
	         {
	            ai_attack_player(osid,(post == ATTACKING2_CRITTER_POSTURE));
					// make sure we see the right attack frame
					objs[id].info.current_frame = att_frame;
					if (((ObjProps[OPTRIP(triple)].flags & LIGHT_TYPE) >> LIGHT_TYPE_SHF) == 3)
               {
						objs[id].info.inst_flags |= UNLIT_FLAG;
                  if(!CRITTER_LAMP(id) && (ID2TRIP(id) != AUTOBOMB_TRIPLE))
                     critter_light_world(id);
               }
	         }
				else
					objs[id].info.current_frame += interval;
         }
			else
	         objs[id].info.current_frame += interval;

         // If past end of cycle, wrap around
         if (objs[id].info.current_frame >= curr_frames)
         {
	         // If dying, remove at end of cycle
	         if (post == DEATH_CRITTER_POSTURE)
	         {
	            objs[id].info.current_frame = curr_frames - 1;
	            ai_critter_really_dead(objs[id].specID);

               // turn off the darned autobomb before we kill it
               if (CRITTER_LAMP(id) && (ID2TRIP(id) == AUTOBOMB_TRIPLE))
                  critter_unlight_world(id);

	            ADD_DESTROYED_OBJECT(id);
	         }
	         else if ((post == ATTACKING_CRITTER_POSTURE) || (post == ATTACKING2_CRITTER_POSTURE) || 
                     (post == DISRUPT_CRITTER_POSTURE) || (post == KNOCKBACK_CRITTER_POSTURE))
	         {
	            set_posture(objs[id].specID, ATTACK_REST_CRITTER_POSTURE);
	         }
	         else
	         {
	            // Otherwise, loop around again
	            objs[id].info.current_frame = 0;
	            objs[id].info.time_remainder =0;
	         }
         }
      }

      // Go to next critter
      osid = objCritters[osid].next;
   }

   // Animating Textures
   // Start at 1, since 0 is the normal texture group
   for (i=1; i < NUM_ANIM_TEXTURE_GROUPS; i++)
   {
      if (animtextures[i].num_frames > 0)
      {
         new_units = num_units + animtextures[i].time_remainder;
         interval = new_units / animtextures[i].anim_speed;
         animtextures[i].time_remainder = new_units % animtextures[i].anim_speed;
         while (interval > 0)
         {
            if (animtextures[i].flags & ANIMTEXTURE_REVERSED)
            {
               // Currently, REVERSED implies CYCLE.  Maybe this should change
               // in the future.
               animtextures[i].current_frame--;
               if (animtextures[i].current_frame < 0)
               {
                  animtextures[i].flags &= ~(ANIMTEXTURE_REVERSED);
                  animtextures[i].current_frame = 0;
               }
            }
            else
            {
               animtextures[i].current_frame++;
               if (animtextures[i].current_frame >= animtextures[i].num_frames)
               {
                  if (animtextures[i].flags & ANIMTEXTURE_CYCLE)
                  {
                     animtextures[i].flags |= ANIMTEXTURE_REVERSED;
                     animtextures[i].current_frame = animtextures[i].num_frames - 1;
                  }
                  else
                  {
                     animtextures[i].current_frame = 0;
                  }
               }
            }
            interval--;
         }
      }
   }
   destroy_destroyed_objects();
   return(OK);
}

void advance_animations(void)
{
   ulong time_diff;

   time_diff = (player_struct.game_time - player_struct.last_anim_check);
   increment_anim(time_diff);
   player_struct.last_anim_check = player_struct.game_time;
}

// fills in requested types of information about an animating object,
// returning FALSE iff that object is not in the anim list.  Pass a
// NULL pointer about a piece of data if you don't want it.
bool anim_data_from_id(ObjID id, bool* reverse, bool* cycle)
{
   int i;

   for(i=0;i<anim_counter;i++) {
      if(animlist[i].id==id) {
         if(reverse)
            *reverse=(animlist[i].flags & ANIMFLAG_REVERSE)!=0;
         if(cycle)
            *cycle=(animlist[i].flags & ANIMFLAG_CYCLE)!=0;
         return TRUE;
      }
   }
   return FALSE;
}

#define CHECK_ANIM_SPEED
errtype add_obj_to_animlist(ObjID id, bool repeat, bool reverse, bool cycle, short speed, int cb_id, void *user_data, short cbtype)
{
   int i=0;
   bool replace_me = FALSE;
   int use_counter = anim_counter;
#ifdef CHECK_ANIM_SPEED
   char count=0;
#endif


   if (anim_counter == MAX_ANIMLIST_SIZE)
   {
      return(ERR_NOMEM);
   }

   for (i=0; i < anim_counter; i++)
   {
      if (animlist[i].id == id)
      {
   	   replace_me = TRUE;
   	   use_counter = i;
      }
   }
   animlist[use_counter].id = id;

   // Set flags
   animlist[use_counter].flags = 0;
   if (repeat)
      animlist[use_counter].flags |= ANIMFLAG_REPEAT;
   if (reverse)
      animlist[use_counter].flags |= ANIMFLAG_REVERSE;
   if (cycle)
      animlist[use_counter].flags |= ANIMFLAG_CYCLE;
   if (speed)
      animlist[use_counter].speed = speed;
   else
      animlist[use_counter].speed = DEFAULT_ANIMLIST_SPEED;
#ifdef CHECK_ANIM_SPEED
   // Hmm, there's probably a better way to check for power-of-2-ness
   for (i=0; i < 16; i++)
   {
      if (animlist[use_counter].speed & (1 << i))
         count++;
      if (count > 1)
      {
         break;
      }
   }
#endif

   animlist[use_counter].cbtype = cbtype;
   animlist[use_counter].callback = cb_id;
   animlist[use_counter].user_data = user_data;
   if (!replace_me)
      anim_counter++;

   objs[id].info.time_remainder = 0;
   return(OK);
}

errtype remove_obj_from_animlist(ObjID id)
{
   int i = 0;
   AnimlistCB cb = NULL;
   void *ud;

   for (i=0; i < anim_counter; i++)
   {
      if (animlist[i].id == id)
      {
	      if ((animlist[i].callback != 0) && (animlist[i].cbtype & ANIMCB_REMOVE))
	      {
	         cb = animlist_callbacks[animlist[i].callback];
	         ud = animlist[i].user_data;
	      }
	      anim_counter--;
	      animlist[i] = animlist[anim_counter];
	      if (cb != NULL)
	         cb(id,ud);
	      return(OK);
      }
   }
   return(ERR_NOEFFECT);
}

errtype animlist_clear()
{
   LG_memset(animlist, 0, sizeof(AnimListing) * MAX_ANIMLIST_SIZE);
   anim_counter = 0;
   return(OK);
}


void init_animlist(void)
{
   extern void diego_teleport_callback(ObjID id, void *user_data);
   extern void destroy_screen_callback_func(ObjID id, void *user_data);
   extern void unshodanizing_callback(ObjID id, void *user_data);
   extern void unmulti_anim_callback(ObjID id, void *user_data);
   extern void multi_anim_callback(ObjID id, void *user_data);
   extern void animate_callback_func(ObjID id, void *user_data);
   animlist_callbacks[1] = (AnimlistCB)diego_teleport_callback;
   animlist_callbacks[2] = (AnimlistCB)destroy_screen_callback_func;
   animlist_callbacks[3] = (AnimlistCB)unshodanizing_callback;
   animlist_callbacks[4] = (AnimlistCB)unmulti_anim_callback;
   animlist_callbacks[5] = (AnimlistCB)multi_anim_callback;
   animlist_callbacks[6] = (AnimlistCB)animate_callback_func;
}
