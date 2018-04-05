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
 * $Source: r:/prj/cit/src/RCS/ai.c $
 * $Revision: 1.167 $
 * $Author: xemu $
 * $Date: 1994/10/18 19:50:51 $
 */

#define __AI_SRC

#include <stdlib.h>

#include "ai.h"
#include "aiflags.h"
#include "objects.h"
#include "objsim.h"
#include "objcrit.h"
#include "objwpn.h"
#include "physics.h"
#include "faketime.h"
#include "player.h"
#include "damage.h"
#include "diffq.h"
#include "combat.h"
#include "grenades.h"
#include "musicai.h"
#include "otrip.h"
#include "objprop.h"
#include "objbit.h"
#include "mapflags.h"
#include "tilename.h"
#include "mainloop.h"
#include "physunit.h"
#include "tools.h"
#include "gamestrn.h"
#include "safeedms.h"
#include "treasure.h"
#include "fullscrn.h"
#include "screen.h"
#include "sfxlist.h"

#include "ice.h"
#include "cyber.h"


#define AI_EDMS

//errtype ai_fire_slow_projectile(ObjID src, int proj_triple, ObjLoc src_loc, ObjLoc target_loc, uchar a, int duration);
//errtype ai_throw_grenade(ObjID src, int proj_triple, ObjLoc src_loc, ObjLoc target_loc);
errtype ai_fire_special(ObjID src, ObjID target, int proj_triple, ObjLoc src_loc, ObjLoc target_loc, uchar a, int duration);

#define SLOW_PROJECTILE_DURATION 1000
#define SLOW_PROJECTILE_GRAVITY  fix_make(0,0x0C00)
#define SLOW_PROJECTILE_SPEED    fix_make(5,0)
#define ATTACK_GRENADE_GRAVITY   fix_make(0,0x8000)
#define ATTACK_GRENADE_SPEED     fix_make(7,0)

// tolerance to completion of an EDMS-driven AI maneuver
#define AI_COMPLETE_TOLERANCE fix_make(0, 0x5fff)

// Number of frames beyond which, if we haven't seen the player,
// we stop trying to shoot at 'im.
#define UNSEEN_FIRE_THRESHOLD 10		 	

fix there_yet;
ObjLoc last_known_loc;

// -----------
//  PROTOTYPES
// -----------
errtype set_posture_safe(ObjSpecID osid, ubyte new_pos);
errtype set_posture_movesafe(ObjSpecID osid, ubyte new_pos);
errtype clear_critter_controls(ObjSpecID osid);
errtype apply_EDMS_controls(ObjSpecID osid);
errtype roll_on_dnd_treasure_tables(int *pcont, char treasure_type);


#define AI_HEAD_HIT_CHANCE 0x40
void ai_find_player(ObjID id)
{
   State st;
   extern void state_to_objloc(State *s, ObjLoc *l);
   extern void get_phys_state(int ph, State *new_state, ObjID id);
   
   if (global_fullmap->cyber)
      last_known_loc = objs[PLAYER_OBJ].loc;
   else
   {
      bool slow_proj = FALSE;
      CritterProp cp = CritterProps[CPNUM(id)];
      // Some of the time they shoot at yer head, the other times at yer body.
      // Unless they have a slow projectile, in which case they always shoot at yer head.
      if ((cp.attacks[0].slow_proj != 0) || ((cp.alt_perc > 0) && (cp.attacks[1].slow_proj)))
         slow_proj = TRUE;
      if ((slow_proj) || ((rand() & 0xFF) < AI_HEAD_HIT_CHANCE) || (global_fullmap->cyber))
         get_phys_state(objs[PLAYER_OBJ].info.ph, &st, PLAYER_OBJ);
//         EDMS_get_pelvic_viewpoint(objs[PLAYER_OBJ].info.ph, &st);
      else
         EDMS_get_state(objs[PLAYER_OBJ].info.ph, &st);
      state_to_objloc(&st, &last_known_loc);
//      Warning(("player loc = %x, %x, %x\n",last_known_loc.x,last_known_loc.y,last_known_loc.z));
   }
}

errtype set_posture(ObjSpecID osid, ubyte new_pos)
{
   if (new_pos != get_crit_posture(osid))
   {
      set_crit_posture(osid,new_pos);
      objs[objCritters[osid].id].info.current_frame = 0;
   }
   return(OK);
}

errtype set_posture_safe(ObjSpecID osid, ubyte new_pos)
{
   if ((get_crit_posture(osid) == STANDING_CRITTER_POSTURE) ||
      (get_crit_posture(osid) == MOVING_CRITTER_POSTURE))
      return(set_posture(osid,new_pos));
   return(OK);
}

errtype set_posture_movesafe(ObjSpecID osid, ubyte new_pos)
{
   if ((get_crit_posture(osid) != STANDING_CRITTER_POSTURE) &&
      (get_crit_posture(osid) != MOVING_CRITTER_POSTURE))
      return(set_posture(osid,new_pos));
   return(OK);
}

errtype clear_critter_controls(ObjSpecID osid)
{
   objCritters[osid].des_heading = 0;
   objCritters[osid].des_speed = 0;
   objCritters[osid].urgency = 0;
   objCritters[osid].sidestep = 0;
   return(OK);
}

// tolerance between "standing" and "walking" anims
// when all your dots is greater than this, you are considered moving
//#define MOVE_TOLERANCE  fix_make(0,0x0700)
fix move_tolerance = fix_make(0,0x0400);

// copied in newai.c
#define DEFAULT_URGENCY fix_make(0x30,0)

errtype apply_EDMS_controls(ObjSpecID osid)
{
#ifdef AI_EDMS
	State crit_state;
	ObjCritter *pcrit = &objCritters[osid];
	ObjID id = pcrit->id;
	fix curr_ai_dist = FIX_UNIT;
	fix use_speed, use_step;

	// Apply controls from last frame, see how close we came
	if (CHECK_OBJ_PH(id))
	{
		safe_EDMS_get_state(objs[id].info.ph, &crit_state);
		
		// Hmm, I wonder whether there is a faster way to do this.
		if (fix_abs(crit_state.X_dot) + fix_abs(crit_state.Y_dot) + fix_abs(crit_state.Z_dot) > move_tolerance)
		{
			if ((get_crit_posture(osid) == STANDING_CRITTER_POSTURE) || 
				 (get_crit_posture(osid) == ATTACK_REST_CRITTER_POSTURE))
			{
				set_posture_safe(osid, MOVING_CRITTER_POSTURE);
			}
		}
		else
		{
			set_posture_safe(osid, STANDING_CRITTER_POSTURE);
		}

		if ((pcrit->urgency == 0) && !(pcrit->flags & AI_FLAG_TRANQ))
			pcrit->urgency = DEFAULT_URGENCY;

		if (pcrit->orders == AI_ORDERS_NOMOVE)
		{
			use_speed = 0;
			use_step = 0;
		}
		else
		{   
			use_speed = pcrit->des_speed;
			use_step = pcrit->sidestep;
		}

		safe_EDMS_ai_control_robot(objs[id].info.ph, pcrit->des_heading,
													use_speed, pcrit->sidestep, pcrit->urgency, &there_yet, curr_ai_dist);
	} 
#endif
	return(OK);
}

#define SHODAN_AVATAR_HOSAGE_DISTANCE  0x4
#define AGITATED_ICE_DIST     100

// percentage of hits to take in one attack in order to play SFX
#define HURT_SOUND_THRESHOLD 25

// All critters within ANGER_RADIUS of a critter that takes damage
// become angry unless their loner bit is set.
#define ANGER_RADIUS	3

void ai_critter_seen(void)
{
   extern int mai_combat_length;
   mlimbs_combat = player_struct.game_time + mai_combat_length;
}


errtype ai_critter_hit(ObjSpecID osid, short damage, bool tranq, bool stun)
{
	char i,j;
   short r;
	char x1,x2,y1,y2;
	ObjRefID oref;
	ObjID	oid;
   char diff;

   // become unconfused & untranqed
   // woo hoo, watch me pull odds out of my butt
   if ((objCritters[osid].flags & (AI_FLAG_CONFUSED|AI_FLAG_TRANQ)) && ((rand() & 0xF) <= 6))
      objCritters[osid].flags &= ~(AI_FLAG_CONFUSED|AI_FLAG_TRANQ);

   // If we are truly asleep, let the enemy pound on us...
   if (ai_critter_sleeping(osid))
      return(OK);

   oid = objCritters[osid].id;
   switch (ID2TRIP(oid))
   {
      case ROBOBABE_TRIPLE:
         {
            extern uchar *shodan_bitmask;
            extern void shodan_phase_in(uchar *bitmask, short x, short y, short w, short h, short num, bool dir);
            shodan_phase_in(shodan_bitmask, 0, 0, FULL_VIEW_WIDTH, FULL_VIEW_HEIGHT, damage << 4 , FALSE);
         }
         break;
   }

	// Play a sound effect if hurt enough
   if ((damage * 100 / ObjProps[OPNUM(objCritters[osid].id)].hit_points) > HURT_SOUND_THRESHOLD)
      play_digi_fx_obj(CritterProps[CPNUM(objCritters[osid].id)].hurt_sound,1,objCritters[osid].id);

	// Depending on how disruptable we are, we might be disrupted
   // that means we have to restart our attack timer
   r = rand()%255;
//   Spew(DSRC_AI_Combat, ("r = %d, dp = %d + %d = %d\n",r,CritterProps[CPNUM(objCritters[osid].id)].disrupt_perc,damage >> 2,
//      CritterProps[CPNUM(objCritters[osid].id)].disrupt_perc + (damage >> 2)));
   if (r < CritterProps[CPNUM(objCritters[osid].id)].disrupt_perc + (damage >> (5 - QUESTVAR_GET(COMBAT_DIFF_QVAR))))
   {
      set_posture(osid, DISRUPT_CRITTER_POSTURE);
      objCritters[osid].attack_count = player_struct.game_time + CritterProps[CPNUM(objCritters[osid].id)].attacks[0].speed;
   }

	// And boy, are we ticked.
   objCritters[osid].mood = AI_MOOD_HOSTILE;

   // We get to butt in line, and we know exactly where the player is...
   ai_find_player(oid);
   objCritters[osid].wait_frames = 0;

   if (tranq)
   {
      objCritters[osid].flags |= AI_FLAG_TRANQ;
      objCritters[osid].des_speed = 0;
      objCritters[osid].urgency = 0;
      objCritters[osid].sidestep = 0;
      apply_gravity_to_one_object(oid, STANDARD_GRAVITY);
      if (objs[oid].info.ph != -1)
      {
         EDMS_control_robot(objs[oid].info.ph,fix_make(0,0x0010),0,0);
      }
      if (get_crit_posture(osid) >= ATTACKING_CRITTER_POSTURE)
      {
         set_crit_posture(osid, STANDING_CRITTER_POSTURE);
         objs[oid].info.current_frame = 0;
      }
   }
   if (stun)
      objCritters[osid].flags |= AI_FLAG_CONFUSED;

   diff = QUESTVAR_GET((global_fullmap->cyber) ? CYBER_DIFF_QVAR : COMBAT_DIFF_QVAR);
   if (diff > 0)
   {
	   // Hey, we'll get our buddies in on the matter too...
	   // We anger all critters within ANGER_RADIUS who are the same
	   // loner-ness as ourselves.
	   // The radius someday might want to be objprop dependant rather 
	   // than a constant.
      // Oh, and we also don't have any effect on critters with a mood of ISOLATION

	   oid = objCritters[osid].id;
	   x1 = OBJ_LOC_BIN_X(objs[oid].loc) - ANGER_RADIUS;
	   x2 = x1 + (2 * ANGER_RADIUS);
	   y1 = OBJ_LOC_BIN_Y(objs[oid].loc) - ANGER_RADIUS;
	   y2 = y1 + (2 * ANGER_RADIUS);

	   for (j=y1; j<=y2; j++)
	   {
		   for (i=x1; i<=x2; i++)
		   {
			   oref = me_objref(MAP_GET_XY(i,j));
			   while (oref != OBJ_REF_NULL)
			   {
				   oid = objRefs[oref].obj;
				   if ((objs[oid].ref = oref) && (objs[oid].obclass == CLASS_CRITTER))
				   {
                  if ((objCritters[objs[oid].specID].mood != AI_MOOD_ISOLATION) &&
                     (!ai_critter_sleeping(osid)))
                  {
					      // Only get upset if loner-ness matches our own.
					      if ((objs[oid].info.inst_flags & CLASS_INST_FLAG) ==
						      (objs[objCritters[osid].id].info.inst_flags & CLASS_INST_FLAG))
					      {
						      objCritters[objs[oid].specID].mood = AI_MOOD_HOSTILE;
					      }
                  }
				   }
				   oref = objRefs[oref].next;
			   }
		   }
	   }
   }
   return(OK);
}

errtype ai_autobomb_explode(ObjID id, ObjSpecID osid);

errtype ai_critter_die(ObjSpecID osid)
{
   extern ObjID damage_sound_id;
   extern char damage_sound_fx;
   ObjID id = objCritters[osid].id;

   if (ID2TRIP(id) == AUTOBOMB_TRIPLE)
      ai_autobomb_explode(id,osid);

   set_posture(osid, DEATH_CRITTER_POSTURE);
   if (CritterProps[CPNUM(id)].death_sound != 255)
   {
      damage_sound_fx = CritterProps[CPNUM(id)].death_sound;
      damage_sound_id = id;
   }
//      play_digi_fx_obj(CritterProps[CPNUM(id)].death_sound,1,id);

   // If we were flying, we ain't no more!
   if (CritterProps[CPNUM(id)].flags & AI_FLAG_FLYING)
      apply_gravity_to_one_object(id, STANDARD_GRAVITY);
   return(OK);
}



#define FIRST_CORPSE_TTYPE 11

int treasure_table[NUM_TREASURE_TYPES][NUM_TREASURE_SLOTS][NUM_TREASURE_ENTRIES] = 
{
   // No Treasure
   {  { 100, NOTHING_TRIPLE},
      { 0, 0},
      { 0, 0},
      { 0, 0},
      { 0, 0},
      { 0, 0},
      { 0, 0}  },
   // Humanoid (1)
   {  { 7, LSD_DRUG_TRIPLE},
      { 15, MEDI_DRUG_TRIPLE},
      { 13, BEV_CONT_TRIPLE},
      { 65, NOTHING_TRIPLE},
      { 0, 0},
      { 0, 0},
      { 0, 0}  },
   // Drone (2)
   {  { 14, SPAMMO_TRIPLE},
      { 8, TEFAMMO_TRIPLE},
      { 17, NNAMMO_TRIPLE}, 
      { 6, MEDI_DRUG_TRIPLE},
      { 5, FRAG_G_TRIPLE},
      { 50, NOTHING_TRIPLE},
      { 0, 0}  },
   // Assassin (3)
   {  { 40, SPAMMO_TRIPLE},
      { 15, NNAMMO_TRIPLE},
      { 8, TEFAMMO_TRIPLE},
      { 7, TNAMMO_TRIPLE},
      { 30, NOTHING_TRIPLE},
      { 0, 0},
      { 0, 0}  },
   // Warrior Cyborg (4)
   {  { 25, SPAMMO_TRIPLE},
      { 25, TEFAMMO_TRIPLE},
      { 15, HNAMMO_TRIPLE},
      { 5, SPLAMMO_TRIPLE},
      { 5, FRAG_G_TRIPLE},
      { 25, NOTHING_TRIPLE},
      { 0, 0}  },
   // Flier Bots (5)
   {  { 30, SPAMMO_TRIPLE},
      { 10, NNAMMO_TRIPLE},
      { 5, TEFAMMO_TRIPLE},
      { 5, HNAMMO_TRIPLE},
      { 50, NOTHING_TRIPLE},
      { 0, 0},
      { 0, 0}  },
   // Security 1 Bots (6)
   {  { 20, HNAMMO_TRIPLE},
      { 20, HTAMMO_TRIPLE},
      { 15, SPLAMMO_TRIPLE},
      { 15, MRAMMO_TRIPLE},
      { 10, TEFAMMO_TRIPLE},
      { 10, HSAMMO_TRIPLE},
      { 10, NOTHING_TRIPLE} },
   // Exec Bots (7)
   {  { 25, NOTHING_TRIPLE},
      { 25, SPLAMMO_TRIPLE},
      { 15, HTAMMO_TRIPLE},
      { 10, HNAMMO_TRIPLE},
      { 10, MRAMMO_TRIPLE},
      { 7, HSAMMO_TRIPLE},
      { 8, NOTHING_TRIPLE}  },
   // Cyborg Enforcer (8)
   {  { 10, EMP_G_TRIPLE},
      { 40, MRAMMO_TRIPLE},
      { 13, SLGAMMO_TRIPLE},
      { 12, BGAMMO_TRIPLE},
      { 8, MEDI_DRUG_TRIPLE},
      { 2, AIDKIT_TRIPLE},
      { 15, STAMINA_DRUG_TRIPLE}  },
   // Security II Bot (9)
   {  { 40, HTAMMO_TRIPLE},
      { 40, HSAMMO_TRIPLE},
      { 10, MRAMMO_TRIPLE},
      { 7, NOTHING_TRIPLE},
      { 3, PRAMMO_TRIPLE},  // no not impart prammo in favor of nothing
      { 0, 0},
      { 0, 0}  },
   // Elite Cyborg (10)
   {  { 15, RGAMMO_TRIPLE},
      { 20, BGAMMO_TRIPLE},
      { 20, HSAMMO_TRIPLE},
      { 11, MEDI_DRUG_TRIPLE },
      { 2, AIDKIT_TRIPLE },
      { 27, NOTHING_TRIPLE },
      { 5, PRAMMO_TRIPLE} }, // yeah, what he said
   // Standard Corpse (11)
   {  { 80, NOTHING_TRIPLE},
      { 5, HELMET_TRIPLE},
      { 5, BEV_CONT_TRIPLE},
      { 5, WRAPPER_TRIPLE},
      { 2, LSD_DRUG_TRIPLE},
      { 2, PHASER_TRIPLE},
      { 1, STAMINA_DRUG_TRIPLE}  },
   // loot-oriented corpse (12)
   {  { 10, SPAMMO_TRIPLE},
      { 10, STAMINA_DRUG_TRIPLE},
      { 5, BATTERY_TRIPLE},
      { 11, MEDI_DRUG_TRIPLE },
      { 64, NOTHING_TRIPLE},
      { 0, 0},
      { 0, 0}  },
   // electro-stuff treasure (maint & repair bots)
   {  { 5, EPICK_TRIPLE},
      { 15, BATTERY_TRIPLE},
      { 80, NOTHING_TRIPLE},
      { 0, 0},
      { 0, 0},
      { 0, 0},
      { 0, 0}  },
   // serv-bot treasure
   {  { 35, BEV_CONT_TRIPLE},
      { 5, MEDI_DRUG_TRIPLE},
      { 15, BEAKER_CONT_TRIPLE},
      { 15, FLASK_CONT_TRIPLE},
      { 12, BATTERY_TRIPLE},
      { 1, SKULL_TRIPLE},
      { 17, NOTHING_TRIPLE} },
};

errtype roll_on_dnd_treasure_tables(int *pcont, char treasure_type)
{
   char perc;
   char count = 0;
   bool give, done = FALSE;
   int chance, trip;

   perc = rand()%100;
   while (!done && (count < NUM_TREASURE_SLOTS))
   {
      give=FALSE;
      chance=treasure_table[treasure_type][count][0];
      trip=treasure_table[treasure_type][count][1];
      if (chance == 0)
         done = TRUE;
      else
      {
         if(TRIP2CL(trip)==CLASS_AMMO) {
            if(!player_struct.cartridges[get_nth_from_triple(trip)])
               give=TRUE;
         }
         if (give || perc < chance)
         {
            if (trip != NOTHING_TRIPLE)
            {
               if ((QUESTVAR_GET(COMBAT_DIFF_QVAR) <= 2) || ((rand() & 0xFF) < 0x80))
                  *pcont = obj_create_base(trip);
            }
            done = TRUE;
         }
         else
            perc -= chance;
      }
      count++;
   }
   return(OK);
}

// Distribute loot as appropriate.  If we go to loot being contained
// "in" corpses, this is the procedure to change.
errtype do_regular_loot(ObjSpecID source_critter, ObjID corpse)
{
   ObjID l1,l2=OBJ_NULL;
   ObjSpecID osid = objs[corpse].specID;

   l1 = objCritters[source_critter].loot1;
   if (objCritters[source_critter].orders != AI_ORDERS_HIGHWAY)
      l2 = objCritters[source_critter].loot2;

   if(l2!=OBJ_NULL&&objs[l2].active) {
      objContainers[osid].contents1=OBJ_NULL; // in case we had set it to a triple for later random generation
      objContainers[osid].contents2=l2;
      // unset freshness flag
      objs[corpse].info.inst_flags &= ~CLASS_INST_FLAG;
      if(objs[l2].info.current_hp==0)
         objs[corpse].info.current_hp=0;
   }

   if(l1!=OBJ_NULL&&objs[l1].active) {
      objContainers[osid].contents1=l1;
      // unset freshness flag
      objs[corpse].info.inst_flags &= ~CLASS_INST_FLAG;
      if(objs[l1].info.current_hp==0)
         objs[corpse].info.current_hp=0;
   }

   return(OK);
}

errtype do_random_loot(ObjID corpse)
{
   ObjSpecID osid = objs[corpse].specID;
   int *pc1, *pc2;
   uchar t_type;
//   char buf[80];

   if (( ((ID2TRIP(corpse) >= MUT_CORPSE1_TRIPLE) && (ID2TRIP(corpse) <= OTH_CORPSE8_TRIPLE)) ||
         ((ID2TRIP(corpse) >= CORPSE1_TRIPLE) && (ID2TRIP(corpse) <= CORPSE8_TRIPLE)))        
       && (objs[corpse].info.inst_flags & CLASS_INST_FLAG))
   {
      switch (objs[corpse].obclass)
      {
         case CLASS_CONTAINER:
            t_type = CritterProps[CPTRIP(objContainers[osid].contents1)].treasure_type;
            pc1 = &objContainers[osid].contents1;
            pc2 = &objContainers[osid].contents2;
            *pc1 = 0;
            *pc2 = 0;
            break;
         case CLASS_SMALLSTUFF:
            t_type = FIRST_CORPSE_TTYPE + objSmallstuffs[osid].cosmetic_value;
            pc1 = &objSmallstuffs[osid].data1;
            pc2 = &objSmallstuffs[osid].data2;
            break;
      }

      if (*pc1 == 0)
      {
         roll_on_dnd_treasure_tables(pc1,t_type);
         switch (QUESTVAR_GET(COMBAT_DIFF_QVAR))
         {
            case 0:
            case 1:
               if (*pc1 == 0)
                  roll_on_dnd_treasure_tables(pc1, t_type);
               break;
         }
      }
      if (*pc2 == 0)
      {
         roll_on_dnd_treasure_tables(pc2,t_type);
         switch (QUESTVAR_GET(COMBAT_DIFF_QVAR))
         {
            case 0:
            case 1:
               if (*pc2 == 0)
                  roll_on_dnd_treasure_tables(pc2, t_type);
               break;
         }
      }

      // unset the freshness bit
      objs[corpse].info.inst_flags &= ~CLASS_INST_FLAG;
   }
   return(OK);
}

errtype ai_critter_really_dead(ObjSpecID osid)
{
   int corpse_trip;
   char f;
   extern errtype obj_floor_func(ObjID id);

   corpse_trip = CritterProps[CPNUM(objCritters[osid].id)].corpse;
   if (corpse_trip != 0)
   {
      ObjID new_obj;
      new_obj = obj_create_base(corpse_trip);
      if (new_obj)
      {
         if (f = FRAME_NUM_3D(ObjProps[OPNUM(new_obj)].bitmap_3d))
            objs[new_obj].info.current_frame = rand()%(f+1);
         else
            objs[new_obj].info.current_frame = 0;
         obj_move_to(new_obj, &objs[objCritters[osid].id].loc, TRUE);
//         obj_floor_func(new_obj);

         if (objCritters[osid].flags & AI_FLAG_NOLOOT)
         {
            objContainers[objs[new_obj].specID].contents1 = OBJ_NULL;
            objs[new_obj].info.inst_flags &= ~CLASS_INST_FLAG;
         }
         else
         {
            // set our contents1 to the triple so that we know how to generate loot right later
            objContainers[objs[new_obj].specID].contents1 = ID2TRIP(objCritters[osid].id);

            // fresh kill, yum!
            objs[new_obj].info.inst_flags |= CLASS_INST_FLAG;
         }

         // if we have regular loot, however, do what we do
         do_regular_loot(osid, new_obj);
      }
   }
   return(OK);
}

bool pacifism_on;
#define AUTOBOMB_RANGE  fix_make(3,0)

void ai_misses(ObjSpecID osid)
{
   // We aren't hitting, so get closer sometimes
   if (rand()%4 == 1)
   {
      objs[objCritters[osid].id].info.inst_flags |= CLASS_INST_FLAG2;
      objCritters[osid].mood = AI_MOOD_HOSTILE;
   }
   else
      objs[objCritters[osid].id].info.inst_flags &= ~CLASS_INST_FLAG2;
}

errtype ai_autobomb_explode(ObjID id, ObjSpecID osid)
{
   CritterAttack ca;
   ExplosionData edata;
   extern void critter_light_world(ObjID id);
   
   ca = CritterProps[CPNUM(id)].attacks[0];

   edata.radius =  AUTOBOMB_RANGE;
   edata.radius_change = (AUTOBOMB_RANGE >> 1);
   edata.damage_mod = ca.damage_modifier;
   edata.damage_change = ca.damage_modifier >> 1;
   edata.dtype = ca.damage_type;
   edata.knock_mass = ca.attack_mass;
   edata.offense = ca.offense_value;
   edata.penet = ca.penetration;

   // Do an explosion!
   do_explosion(objs[id].loc, id, FALSE, &edata);
   play_digi_fx_obj(SFX_EXPLOSION_1,1,id);
   
   // Make us dying....
   set_posture(osid, DEATH_CRITTER_POSTURE);

   // make us light up the world
   critter_light_world(id);
   
   return(OK);
}

errtype ai_attack_player(ObjSpecID osid, char a)
{
   int wpnflags, wpnpower;
   ObjID hit_obj = OBJ_NULL;
   ObjID id = objCritters[osid].id;
	ObjLoc dest_loc;
   int cp_num;
   fix attack_mass;           // cause of new ray casting prototype - minman

   if (pacifism_on)
      return(OK);
   
   cp_num = CPNUM(objCritters[osid].id);

   // If we are an autobomb, don't attack as usual, instead go boom!
   switch (ID2TRIP(id))
   {
      case AUTOBOMB_TRIPLE:
         {
            return(ai_autobomb_explode(id,osid));
         }
         break;
   }

   wpnpower = 100; // Full strength attack!
   wpnflags = 0;   // Normal attack
   attack_mass = fix_make(CritterProps[cp_num].attacks[a].attack_mass,0) * 20;

#ifdef CRITTER_ALWAYS_ACCURATE
   hit_obj = ray_cast_objects(objCritters[osid].id, PLAYER_OBJ,
      attack_mass, fix_make(0,CritterProps[cp_num].attacks[a].attack_size),
                     fix_make(CritterProps[cp_num].attacks[a].attack_velocity, 0),
                     fix_make(CritterProps[cp_num].attacks[a].att_range, 0));
#else
	// If we have NO idea where the player is (all failed detection rolls)
	// then don't bother firing.						
	if (last_known_loc.x != 255)
	{
		short miss_amt = ((255 - CritterProps[cp_num].attacks[a].accuracy) - rand()%255);

      // Play sound effect
      play_digi_fx_obj(CritterProps[cp_num].attack_sound,1,objCritters[osid].id);
      objs[objCritters[osid].id].info.inst_flags |= UNLIT_FLAG;

		// Shoot at player's last known location.
		// Also, modify where we fire by our accuracy variable
		dest_loc = last_known_loc;

		if (miss_amt > 0)
		{
			// We've failed our accuracy roll, so let's perturb
			// our target by an amount proportional to the amount
			// that we missed by.
			dest_loc.x += (rand()%miss_amt - (miss_amt / 2));
			dest_loc.y += (rand()%miss_amt - (miss_amt / 2));
			dest_loc.z += rand()%miss_amt;
		}
      if (CritterProps[cp_num].attacks[a].slow_proj == 0)
      {
#ifdef PLAYTEST
         extern bool prevent_ray_spew;
         prevent_ray_spew = FALSE;
#endif
		   hit_obj = ray_cast_attack(objCritters[osid].id,
			   dest_loc,
			   attack_mass, RAYCAST_ATTACK_SIZE,
			   fix_make(CritterProps[cp_num].attacks[a].attack_velocity,0),
			   fix_make(CritterProps[cp_num].attacks[a].att_range,0));
#ifdef PLAYTEST
         prevent_ray_spew = TRUE;
#endif
         if (hit_obj != NULL)
         {
            attack_object(hit_obj, CritterProps[cp_num].attacks[a].damage_type, CritterProps[cp_num].attacks[a].damage_modifier,
               CritterProps[cp_num].attacks[a].offense_value, CritterProps[cp_num].attacks[a].penetration,
               wpnflags, wpnpower, NULL, 0, 0, NULL);

            objs[objCritters[osid].id].info.inst_flags &= ~CLASS_INST_FLAG2;
         }
         else
            ai_misses(osid);
      }
      else
      {
         ai_fire_special(objCritters[osid].id, PLAYER_OBJ, CritterProps[cp_num].attacks[a].slow_proj,
               objs[objCritters[osid].id].loc, dest_loc, a, SLOW_PROJECTILE_DURATION);
      }
	}
   else
   {
      objCritters[osid].mood = AI_MOOD_HOSTILE;
   }
#endif						
   return(OK);
}

#define SLOW_PROJ_RAY_MASS    (fix_make(0,0x1000))
#define SLOW_PROJ_RAY_SIZE    (fix_make(0,0x1800))
#define SLOW_PROJ_RAY_RANGE   (fix_make(20,0))
extern void get_phys_state(int ph, State *new_state, ObjID id);

errtype ai_fire_special(ObjID src, ObjID target, int proj_triple, ObjLoc src_loc, ObjLoc target_loc, uchar a, int duration)
{
   ObjID    proj_id;
   fix      xvel, yvel, zvel;
   fix      xdiff, ydiff, zdiff;
   fix      dist;
   fix      fire_speed;
   fixang   new_angle;
   ubyte    head;
   State    new_state;
   Robot          da_robot;
   extern void activate_grenade(ObjSpecID osid);

   if (!global_fullmap->cyber)
   {
      // let's attack only if we think we're going to hit our target
      if (ray_cast_attack(src, target_loc, SLOW_PROJ_RAY_MASS, SLOW_PROJ_RAY_SIZE,
               NO_RAYCAST_KICKBACK_SPEED, SLOW_PROJ_RAY_RANGE) != target)
         return(OK);
   }

   proj_id = obj_create_base(proj_triple);
   if (proj_id == OBJ_NULL)
   {
      Warning(("Could not create slow projectile!\n"));
      return(OK);
   }

   if (TRIP2CL(proj_triple) == CLASS_PHYSICS)
   {
      objPhysicss[objs[proj_id].specID].owner = src;
      objPhysicss[objs[proj_id].specID].bullet_triple = a;
      objPhysicss[objs[proj_id].specID].duration = player_struct.game_time + duration;
      fire_speed = SLOW_PROJECTILE_SPEED;
   }
   else
   {
      activate_grenade(objs[proj_id].specID);
      fire_speed = ATTACK_GRENADE_SPEED;
   }

#ifdef AI_EDMS

   if ((objs[src].obclass == CLASS_CRITTER) && (CritterProps[CPNUM(src)].proj_offset))
      src_loc.z += (CritterProps[CPNUM(src)].proj_offset >> (SLOPE_SHIFT_D-2));

   get_phys_state(objs[PLAYER_OBJ].info.ph, &new_state,PLAYER_OBJ);

 
   // Compute distance
   xdiff = fix_from_obj_coord(target_loc.x) - fix_from_obj_coord(src_loc.x);
   ydiff = fix_from_obj_coord(target_loc.y) - fix_from_obj_coord(src_loc.y);
   zdiff = new_state.Z - fix_from_obj_height_val(src_loc.z);
   dist = fix_fast_pyth_dist(xdiff,ydiff);

   if (global_fullmap->cyber)
   {
      // let's get heading
      new_angle = fix_atan2(ydiff, xdiff);
      head = (ubyte) obj_angle_from_fixang(new_angle); //(fix_div(new_angle, FIXANG_PI) >> 9);

      // let's start the work for pitch
      new_angle = fix_atan2(zdiff, dist);
//      pitch = (ubyte) (fix_div(new_angle, FIXANG_PI) >> 9);

      // shift coordinate frames for heading
      src_loc.h = (ubyte) ((320L-head) % 256);
      src_loc.p = obj_angle_from_fixang(new_angle); // pitch;
      src_loc.b = 0;
   }

   xvel = fix_mul_div(xdiff, fire_speed, dist);
   yvel = fix_mul_div(ydiff, fire_speed, dist);

   // LET'S HACK HACK HACK THE WAY GRENADES ARE THROWN!!!!
   // let's guess on the zvel!
   if (TRIP2CL(proj_triple) == CLASS_PHYSICS)
      zvel = fix_mul_div(zdiff, fire_speed, dist);
   else
      zvel = (zdiff < fix_make(0,0x4000)) ? fix_mul(fix_make(0,0x3800), dist) : fix_mul(zdiff, fix_make(4,0));

   obj_move_to_vel(proj_id, &src_loc, TRUE, xvel, yvel, zvel);
   EDMS_ignore_collisions(objs[src].info.ph, objs[proj_id].info.ph);
   if (TRIP2CL(proj_triple) == CLASS_PHYSICS)
      apply_gravity_to_one_object(proj_id, SLOW_PROJECTILE_GRAVITY);
   else
      apply_gravity_to_one_object(proj_id, ATTACK_GRENADE_GRAVITY);

   // don't ask me why i have to do this - but i do
   EDMS_get_robot_parameters(objs[proj_id].info.ph, &da_robot);
   da_robot.cyber_space = -1;
   EDMS_set_robot_parameters(objs[proj_id].info.ph, &da_robot);

#endif
   return(OK);
}

/* KLC - these don't do anything.
errtype ai_freeze_tag()
{
   return(OK);
}

errtype ai_time_passes(ulong *ticks_passed)
{
   return(OK);
}
*/