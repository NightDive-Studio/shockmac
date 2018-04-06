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
 * $Source: r:/prj/cit/src/RCS/damage.c $
 * $Revision: 1.206 $
 * $Author: xemu $
 * $Date: 1994/10/27 04:56:12 $
 */

#include <stdlib.h>

#include "Shock.h"
#include "MoviePlay.h"

#include "effect.h"
#include "newmfd.h"
#include "damage.h"
#include "objwpn.h"
#include "objects.h"
#include "objsim.h"
#include "objprop.h"
#include "weapons.h"
#include "player.h"
#include "combat.h"
#include "cybrnd.h"
#include "mainloop.h"
#include "trigger.h"
#include "musicai.h"
#include "sfxlist.h"
#include "ai.h"
#include "frprotox.h"
#include "frflags.h"
#include "gamerend.h"
#include "hkeyfunc.h"
#include "gameloop.h"
#include "target.h"
#include "objbit.h"
#include "physunit.h"
#include "hud.h"
#include "faketime.h"
#include "otrip.h"
#include "grenades.h"
#include "softdef.h"
#include "aiflags.h"
#include "input.h"
#include "mapflags.h"
#include "wares.h"
#include "drugs.h"
#include "diffq.h"
#include "hudobj.h"
#include "amap.h"
#include "target.h"
#include "status.h"

#define SQUARE(x)                ((x) * (x))
#define PLAYER_DEFENSE_VALUE     4
#define MAX_DAMAGE               2500

extern void    set_dmg_percentage(int which, ubyte percent);

//----------------
//  Internal Prototypes
//----------------
int random_bell_modifier(bool attack_on_player);
int randomize_damage(int damage,bool attack_on_player);
int armor_absorption(int raw_damage, int obj_triple, ubyte penetrate);
int shield_absorb_damage(int damage, ubyte dtype, byte shield_absorb, ubyte shield_threshold);
bool kill_player(void);
void regenerate_player(void);
void player_dies();
ubyte damage_player(int damage, ubyte dtype, ubyte flags);
void slow_proj_hit(ObjID id, ObjID victim);
void critter_hit_effect(ObjID target, ubyte effect,Combat_Pt location, int damage, int max_damage);


// -------------------------------------------
// destroy_destroyed_objects()
//

void destroy_destroyed_objects(void)
{
   int      i, j;
   bool     change_target = FALSE;
   bool     dupe;
   ObjID    id;

   if (destroyed_obj_count != 0)
   {
      for (i=0; i<destroyed_obj_count;i++)
      {
         // Look through the rest of the list for duplicates
         // if so, don't delete us now since we'll get what's coming
         // to us later...
         // Yeah, yeah, so this is an N-squared algorithm, but hopefully the size of
         // the list is pretty damn short...
         dupe = FALSE;
         id = destroyed_ids[i];

         for (j=i+1; j < destroyed_obj_count; j++)
         {
            if (id == destroyed_ids[j])
            {
               dupe = TRUE;
               break;
            }
         }
         if (!dupe)
         {
            {
               // check if the physics object contributed to the lighting of the map
               if (objs[id].obclass== CLASS_PHYSICS)
               {
                  ObjSpecID osid = objs[id].specID;
                  if (objPhysicss[osid].p2.x)
                  {
                     MapElem *mmp;
                     mmp=MAP_GET_XY(OBJ_LOC_TO_LIGHT_LOC(objPhysicss[osid].p1.x), OBJ_LOC_TO_LIGHT_LOC(objPhysicss[osid].p1.y));
                     me_rend3_set(mmp, me_bits_rend3(mmp)-1);
                     objPhysicss[osid].p2.x = 0;
                  }
               }
               else if (objs[id].obclass == CLASS_GRENADE)
               {
                  do_grenade_explosion(id, TRUE);
               }

               // If we destroyed a creature which was being targeted, we
               // need to switch the current target
               if (destroyed_ids[i] == player_struct.curr_target)
                  change_target = TRUE;

               obj_destroy(destroyed_ids[i]);

               if (change_target)
               {
                  toggle_current_target();
                  change_target = FALSE;
               }
            }
         }


         // object_dead();
      }

      chg_set_flg(DEMOVIEW_UPDATE);

      // "clear" the list
      destroyed_obj_count = 0;
   }
}

// -------------------------------------------
// is_obj_destroyed()
//

bool is_obj_destroyed(ObjID id)
{
   int   i;
   bool  found = FALSE;

   for (i=0; i<destroyed_obj_count;i++)
   {
      if (destroyed_ids[i] == id)
      {
         found = TRUE;
         break;
      }
   }
   return(found);
}

// ---------------------------------------------
// random_bell_modifier()
//

int random_bell_modifier(bool attack_on_player)
{
   int   rtotal;
   int   i;
   int   rval;
   int   retval;
   ubyte   dies, die_value, handicap;
   ubyte  difficulty = (global_fullmap->cyber) ?
      player_struct.difficulty[CYBER_DIFF_INDEX] : player_struct.difficulty[COMBAT_DIFF_INDEX];

   if (attack_on_player)
      difficulty = (difficulty==0) ? 3 : 4-difficulty;

   switch (difficulty)
   {
      case(0): dies = 2; die_value = 50; handicap = 15;
                  break;
      case(1): dies = 3; die_value = 33; handicap = 6;
                  break;
      case(2): dies = 5; die_value = 20; handicap = 0;
                  break;
      case(3): dies = 8; die_value = 12; handicap = 0;
                  break;
   }

   rtotal = handicap;
   for (i=0;i<dies; i++)
   {
      rval = RndRange(&damage_rnd, 0, die_value);
      rtotal += rval;
   }
 
   if (rtotal <= 2)        retval = -12;     // 00-02
   else if (rtotal <= 8)   retval = -8;      // 03-08
   else if (rtotal <= 16)  retval = -6;      // 09-16
   else if (rtotal <= 28)  retval = -3;      // 17-28
   else if (rtotal <= 40)  retval = -1;      // 29-40
   else if (rtotal <= 60)  retval =  0;      // 41-60
   else if (rtotal <= 72)  retval =  1;      // 61-72
   else if (rtotal <= 84)  retval =  3;      // 73-84
   else if (rtotal <= 92)  retval =  6;      // 85-92
   else if (rtotal <= 98)  retval =  8;      // 93-98
   else                    retval = 12;      // 99-100

   return(retval);
}

// -------------------------------------------
// randomize_damage()
//

int randomize_damage(int damage,bool attack_on_player)
{
   int        dtotal;
   ubyte      iterations;
   int        i;
   ubyte  difficulty = (global_fullmap->cyber) ?
      player_struct.difficulty[CYBER_DIFF_INDEX] : player_struct.difficulty[COMBAT_DIFF_INDEX];
   
   dtotal = damage/2;
   iterations = damage/8;

   // if damage div 8 is non-zero add an extra iteration
   // yes, we might have a value that's a little bigger than normal
   if (damage % 8)
      iterations++;

   for (i=0; i<iterations;i++)
      dtotal += RndRange(&damage_rnd, 1, 7);

   // if we're playing on difficulty 3 - reduce damage by a third
   if (!attack_on_player && (difficulty == 3))
      dtotal = (dtotal << 1) / 3;

   return(dtotal);
}

// ---------------------------------------------
// object_affect()
//
// there has been a hit, now we check how much an object is affected
//

ubyte object_affect(ObjID target_id, short dtype)
{
   ubyte affected = 0;
   int   resis;

   if (!target_id)
   {
      return 0;
   }

   resis = ObjProps[OPNUM(target_id)].resistances;
   affected = (resis & dtype & DAMAGE_TYPE_FIELD) ? 1 : 0 ;
   if (affected)
   {
      // are the super damage the same, and we have a non-zero primary damage
      if ((SUPER_DAMAGE(resis) == SUPER_DAMAGE(dtype)) && (SUPER_DAMAGE(resis)))
      {
         ubyte  difficulty = (global_fullmap->cyber) ?
            player_struct.difficulty[CYBER_DIFF_INDEX] : player_struct.difficulty[COMBAT_DIFF_INDEX];

         // don't do as much super damage if we're on combat diff 3
         affected = (difficulty==3) ? 3 : 4;
      }
      // are the primary damage the same, and we have a non-zero primary damage
      else if ((PRIMARY_DAMAGE(resis) == PRIMARY_DAMAGE(dtype)) && (PRIMARY_DAMAGE(resis)))
      {
         affected = 2;
      }
   }
//   mprintf("attack on %d, affect : %d\n", target_id, affected);
   return(affected);
}

// -----------------------------------------------------------------------------------------
// armor_absorption()
//

int armor_absorption(int raw_damage, int obj_triple, ubyte penetrate)
{
   short       real_penetration = (((short) penetrate * (90 + RndRange(&damage_rnd, 0, 20)))/100);
   int         damage;

   damage = (ObjProps[OPTRIP(obj_triple)].armor - real_penetration);
   damage = (damage > 0) ? raw_damage - damage : raw_damage;

   if (damage < 0)
      damage = 0;

   return(damage);
}

// some globals
uchar sound_hurt_threshold = 10;
uchar static_pain_time = 64;
uchar static_pain_base = 30;
uchar static_pain_delta = 30;
uchar shield_blowout_threshold = 15;

short fr_solidfr_time;
short fr_sfx_time;


// this is the voodoo threshold for shields, so that we have a reference 
// to do effects with. (static)
#define VOODOO_SHIELD_THRESHOLD     100

// -------------------------------------------------------------------------------------------
// shield_absorb_damage()
//
// returns damage to the player after shields

short shield_absorb_perc = 0;
bool shield_used;

int shield_absorb_damage(int damage, ubyte, byte shield_absorb, ubyte shield_threshold)
{
   ubyte          shield_drain=0;

   if ((damage|player_struct.hit_points)==0)
      return 0;     // 0 hp is already dead, eh?, 0 damage no matter

   if (damage > shield_threshold)
   {
      // absorption rate will be given a +/- 8 percentage for variation - unless we're at zero already
      shield_absorb = (shield_absorb) ? shield_absorb + RndRange(&damage_rnd, 0, 16) - 8 : 0;

      if (shield_absorb>0)
      {
         // figure out how much damage is absorbed by shields
         // hey - it's even a percentage
         // WHY - make it 0-255, dont divide needlessly, arghgqghgghdsghgfdgfjfghdfgj
         shield_drain = (damage * shield_absorb)/100;
         // let's absorb the damage... now!
         damage -= shield_drain;
      }
      else
         shield_absorb = 0;
   }
   else
   {
      if (shield_absorb)
      {
         // if we're below threshold, we've absorbed all damage...
         shield_absorb = 100;
         shield_drain = damage;
      }
      else
         shield_drain = 0;
   }
   
   // Hud me baby
   // oh,and sound too.

   if (shield_absorb > 0)
   {
      shield_absorb_perc = shield_absorb;
      hud_set_time(HUD_SHIELD, CIT_CYCLE);

      // do the sound if we're not in cyberspace
      if (!global_fullmap->cyber)
            (shield_absorb > 80) ? play_digi_fx(SFX_SHIELD_2,1) : play_digi_fx(SFX_SHIELD_1,1);
   }
   else if (((rand() & 0x1A) < damage) && !global_fullmap->cyber)
      play_digi_fx(SFX_PLAYER_HURT,1);

   // let's make sure that we don't go over the 100% shield flash
   if (shield_drain > VOODOO_SHIELD_THRESHOLD)
      shield_drain = VOODOO_SHIELD_THRESHOLD;

   // let's reuse the shield_drain variable
   // let's get the percentage absorbed (w.r.t. the VOODOO_SHIELD_THRESHOLD)
   shield_drain = (ubyte) (((int) shield_drain << 8) / VOODOO_SHIELD_THRESHOLD);

   if ((shield_used = (shield_drain > 0)) == TRUE)
      set_dmg_percentage(DMG_SHIELD,shield_drain);

   return(damage);
}


bool alternate_death = FALSE;
extern Boolean	gPlayingGame;
extern Boolean	gDeadPlayerQuit;

// kill_player()
// kills the player, checks for traps and stuff, so on
// returns true if player is really dead dead dead
bool kill_player(void)
{
   ObjSpecID osid;
   bool quick_death = TRUE;
   bool dummy;
   extern bool clear_player_data;

   // Look for a player death trigger.  If so, do it.
   // If not, play appropriate dying cutscene.
   osid = objTraps[0].id;
   while (osid != OBJ_SPEC_NULL)
   {
      if (ID2TRIP(objTraps[osid].id)==PLRDETH_TRIG_TRIPLE)
         if (trap_activate(objTraps[osid].id, &dummy))
            quick_death = FALSE;
      osid = objTraps[osid].next;
   }

   // if we died not from a trap - then we should clear the player data
#ifdef TEST_REBIRTH
   clear_player_data = FALSE;
   return FALSE;
#else
   clear_player_data = (quick_death | alternate_death);

   if (quick_death)
   {
      amap_reset();
     secret_render_fx=0;
     gDeadPlayerQuit = TRUE;
	 gPlayingGame = FALSE;															// Hop out of the game loop.
   }

   return(quick_death | alternate_death);
#endif
}

void regenerate_player(void)
{
   extern void wear_off_drug(int i);
   extern void regenetron_door_hack(void);
   int i;
   for (i = 0; i < NUM_DAMAGE_TYPES; i++)
      player_struct.hit_points_lost[i] = 0;
   hud_unset(HUD_RADPOISON|HUD_BIOPOISON);
   player_struct.curr_target = OBJ_NULL;
   player_struct.fatigue = 0;

   // clear physics state
   player_struct.posture = 0;
   player_struct.foot_planted=0;
   player_struct.leanx = 0;
   player_struct.leany = 0;
   player_struct.eye_pos = 0;

   for (i = 0; i < NUM_DRUGZ; i++)
      wear_off_drug(i);
   regenetron_door_hack();
}

// -----------------------------------------------------------------------------------------
// damage_player()
//
// returns 0 if player is still alive
// returns 1 if player is dead

#define CSHIELD_THRESHOLDS(lev)     0

#define MAX_CSHIELD_ABSORB    120
#define NUM_CSHIELD_LEVELS    10
#define CSHIELD_ABSORB_RATES(lev) ((lev) == 0) ? 0 : (MAX_CSHIELD_ABSORB / (NUM_CSHIELD_LEVELS - (lev)))

#define MAX_FATIGUE  10000
#define DEATH_TICKS  CIT_CYCLE

ulong player_death_time = 0;

// Something has caused the player to become a fatality
// typically this is damage, but can be delayed-death due to craze
void player_dies()
{
   extern void physics_zero_all_controls();
   extern void clear_digi_fx();
   extern short inventory_page;
#ifdef AUDIOLOGS
   extern char secret_pending_hack;
   secret_pending_hack = 0;
#endif

	// we should play funky death music
	mai_player_death();
    reset_input_system();
	chg_set_sta(GL_CHG_2);  // disable the input system

   // clear off hud & prep for funky regen FX
//   hud_unset(HUD_ALL);
   physics_zero_all_controls();

   // clear edms_state
   LG_memset(player_struct.edms_state, 0, sizeof(fix)*12);

   // reset the inventory page
   inventory_page = 0;

   // hey no more things to do to player while he's dead
   // it's really a secret - don't do things to player
   // while there are cool secret_render_fx going on
   player_struct.dead = TRUE;
   set_dmg_percentage(DMG_BLOOD, 100); // 100 is an arbitrary number - we're trying to get it to do red static
	secret_render_fx=DYING_REND_SFX;

   clear_digi_fx();
   player_struct.num_deaths++;
}

// -------------------------------------------------------
// damage_player()
//

#define DAMAGE_DIFFICULTY ((global_fullmap->cyber) ? 3 : 0)

ubyte damage_player(int damage, ubyte dtype, ubyte flags)
{
   ubyte *cur_hp;
   short rawval;
   bool dead = 0, damage_dealt=FALSE;
   char dlev, dmg_type=DMG_BLOOD;

   if (secret_render_fx > 0)
      return 0;

   if ((damage <= 0) || (player_struct.hit_points==0))     // 0 hp is already dead, eh?
      return 0;

   if (global_fullmap->cyber && !player_struct.difficulty[CYBER_DIFF_INDEX])
      return 0;

   cur_hp= (global_fullmap->cyber) ?
      &player_struct.cspace_hp : &player_struct.hit_points;

   shield_used = FALSE;

   // check if shields should play a role
   if ((dtype==RADIATION_TYPE)||(dtype==BIO_TYPE))
      dmg_type=DMG_RAD;
   else if (!(flags & NO_SHIELD_ABSORBTION))
   {
      byte absorb_rate, thresh_val;

      // compensate for difficulty level
      dlev = player_struct.difficulty[DAMAGE_DIFFICULTY];
      if (dlev!=3)
         damage>>=(2-dlev);
      if (global_fullmap->cyber)
      {
         if (player_struct.softs.defense[SOFTWARE_CSHIELD])
         {
		      absorb_rate=CSHIELD_ABSORB_RATES(player_struct.softs.defense[SOFTWARE_CSHIELD]);
            thresh_val =CSHIELD_THRESHOLDS(player_struct.softs.defense[SOFTWARE_CSHIELD]);
         }
         else
            absorb_rate = thresh_val = 0;
      }
      else
      {
         absorb_rate=(byte)player_struct.shield_absorb_rate;
         thresh_val =player_struct.shield_threshold;
      }
      damage = shield_absorb_damage(damage,dtype,absorb_rate,thresh_val);
   }

   if (damage<=0)
      return 0;

#ifdef WACKY_STATIC_USAGE
   // Play digi FX should go in here when we have appropriate SFX
   if ((!global_fullmap->cyber) && (damage > static_pain_base + rand()%static_pain_delta))
   {
      extern char static_density, static_color, static_grouping;
      // Turn on fullscreen static & turn off any SFX that might be otherwise going on.
      fr_global_mod_flag(FR_SOLIDFR_STATIC, FR_SOLIDFR_MASK|FR_SFX_MASK);
      fr_solidfr_time = (static_pain_time);
      play_digi_fx(SFX_STATIC, -1);
   }
#endif

   // did we take more damage than hit points?? - eeeegggads! we're dead
   if ((*cur_hp)<=damage)
   {
      damage_dealt=TRUE;
      {
		   if (global_fullmap->cyber)
		   {  // No matter what, the player can't be killed in one shot.....it takes at least two shots
            player_struct.cspace_hp = (player_struct.cspace_hp == 1) ? 0 : 1;
	      }
         else // normal (non-cyberspace) damage - player's dead dead dead
		   {
	         if (*cur_hp>0)
		      {
#ifdef CRAZE_NODEATH
               if ((player_struct.drug_status[DRUG_LSD] > 0) && (QUESTVAR_GET(COMBAT_DIFF_QVAR) < 3))
                  *cur_hp = 1;
               else
#endif 
               {
                  *cur_hp = 0;
                  dead = TRUE;

                  // if we're carrying an object - let's drop it here!
                  if (object_on_cursor != OBJ_NULL)
                  {
                     obj_move_to(object_on_cursor,&objs[PLAYER_OBJ].loc,TRUE);
                     pop_cursor_object();
                  }

                  // signal for the effects that happen with death
                  player_dies();
               }
	   	   }
	 	   }
      }
   }
   if (!damage_dealt)
   {
      extern int mai_damage_sum;

      *cur_hp -= damage;
	   mai_damage_sum += damage;
   }

   if (*cur_hp == 0)
   	rawval = damage<<8;
   else
     rawval = ((damage<<8) / (*cur_hp));      // what is this minmax3/20xff thing?
                                             							// it's my voodoo - minman
   if (!shield_used)
      set_dmg_percentage(dmg_type,(ubyte) min(max(rawval,((damage*3)/2)),0x00FF));

   // makes sure gamescreen knows that it should be updated
   chg_set_flg(VITALS_UPDATE);
   return(dead);
}   

// --------------------------------------------------
// damage_object()
//
// return 0 - if object is still alive after damage
// return 1 - if object has been destroyed
// 
// also does the appropriate texture map change if
// object is destroyed???????

ubyte damage_object(ObjID target_id, int damage, int dtype, ubyte flags)
{
   int   obclass = objs[target_id].obclass;
   int   dead = 0;
   bool  tranq=FALSE;
   bool  stun= FALSE;
   short target_hp = ObjProps[OPNUM(target_id)].hit_points;

   // If we've already been destroyed, or don't care, thendon't bother us.
   if ((objs[target_id].info.inst_flags & INDESTRUCT_FLAG) || 
         ((target_id != player_struct.rep) && (objs[target_id].info.current_hp == 0)))
      return(0);

   // let the player get his/her own special treatment
   if (target_id == PLAYER_OBJ)
      dead = damage_player(damage, (ubyte) PRIMARY_DAMAGE(dtype), flags);
   else
   {
      // are we still alive - then do special stuff that we only care about if
      // we're still alive, makes too much sense
      if (objs[target_id].info.current_hp > damage)
      {
         // damage object - but it's not dead yet.
         dead = 0;
         if (obclass == CLASS_CRITTER)
         {
            int   pct = (450L * damage)/objs[target_id].info.current_hp;
            tranq = dtype & TRANQ_FLAG;
            stun  = (flags & STUN_ATTACK);

            if (tranq)
            {
               tranq = FALSE;
               // okay tranq - only if we've done damage, and we're lucky and the damage we're doing
               // is a decent amount of the remaining life
               if (damage)
                  tranq = (RndRange(&damage_rnd, 0, 100) < pct);
            }
            else if (stun)
            {
               if (objs[target_id].subclass == CRITTER_SUBCLASS_ROBOT)
                  stun = FALSE;
               else
                  stun = (RndRange(&damage_rnd, 0, 100) < pct);
            }
            ai_critter_hit(objs[target_id].specID, damage,tranq, stun);
         }
         // get rid of the hit points
         objs[target_id].info.current_hp -= damage;
      }
      else
      {
         objs[target_id].info.current_hp = 0;
         dead = 1;

         // Check to see whether or not there is cool special stuff to do when this thing
         // gets destroyed.  obj_combat_destroy returns whether or not to go ahead and
         // continue the destruction process
         if (obj_combat_destroy(target_id))
            ADD_DESTROYED_OBJECT(target_id);

         if (DESTROY_SOUND_EFFECT(ObjProps[OPNUM(target_id)].destroy_effect))
         {
            extern ObjID damage_sound_id;
            extern char damage_sound_fx;

            damage_sound_fx = SFX_CPU_EXPLODE;
            damage_sound_id = target_id;
         }
      }

      if ((obclass == CLASS_CRITTER) && !global_fullmap->cyber)
      {
         ubyte seriousness=0;
         extern void hud_report_damage(ObjID target, byte seriousness);

         // marc's desired code
         if (stun)
            seriousness=7;
         else if (tranq)
            seriousness=6;
         else if (!object_affect(target_id, dtype))
            seriousness=5;
         else if (damage > target_hp)
            seriousness=4;
         else if (damage > (target_hp * 4)/5)
            seriousness=3;
         else if (damage > target_hp/5)
            seriousness=2;
         else if (damage > 0)
            seriousness=1;

         hud_report_damage(target_id,seriousness);
      }

      // If we damaged the currently targeted creature, let mfds know...
      if ((target_id == player_struct.curr_target) && !global_fullmap->cyber)
         mfd_notify_func(MFD_TARGET_FUNC, MFD_TARGET_SLOT, FALSE, MFD_ACTIVE, TRUE);

   }
   return(dead);
}

// -----------------------------------------------
// simple_damage_object() takes damage of a particular type with particular flags, 
// and applies it to the object if it is vulnerable to the type.
// returns whether the object was destroyed.

bool simple_damage_object(ObjID target, int damage, ubyte dtype, ubyte flags)
{
   if (object_affect(target,1 << (dtype-1) ))
      return damage_object(target,damage,dtype,flags);
   return FALSE;
}

#define FIRST_ENRG_PROJ_TYPE     7
#define DAMAGE_PROJ_DISTANCE     (fix_make(0,0x6000))

// OBJ_NULL victim means terrain
void slow_proj_hit(ObjID id, ObjID victim)
{
   Combat_Pt      origin;
   ObjLoc      loc = objs[id].loc;
   ObjRefID    current_ref;
   ObjID       current_id;
   ubyte       affect;
   ubyte       dtype;
   ubyte       proj_power;
   ubyte       special_effect = EFFECT_VAL(ObjProps[OPNUM(id)].destroy_effect);
   int         a;
   int         weapon_triple;

   current_ref = MAP_GET_XY(OBJ_LOC_BIN_X(objs[id].loc), OBJ_LOC_BIN_Y(objs[id].loc))->objRef;

   if (objPhysicss[objs[id].specID].owner != PLAYER_OBJ)
   {
      if (special_effect)
         do_special_effect_location(id, special_effect, 0xFF, &loc, 0);
      return;
   }

   a = objPhysicss[objs[id].specID].bullet_triple;
   if (objs[id].info.type >= FIRST_ENRG_PROJ_TYPE)
   {
      weapon_triple = MAKETRIP(CLASS_GUN, GUN_SUBCLASS_BEAMPROJ, TRIP2TY(a));
      proj_power = TRIP2CL(a);
      dtype = BeamprojGunProps[SCTRIP(weapon_triple)].damage_type;
   }
   else
   {
      weapon_triple = MAKETRIP(CLASS_GUN, GUN_SUBCLASS_SPECIAL, TRIP2TY(a));
      proj_power = 100;
      dtype = SpecialGunProps[SCTRIP(weapon_triple)].damage_type;
   }
   if (weapon_triple == RAILGUN_TRIPLE)
   {
      do_explosion(loc, id, 0, &(game_explosions[1]));
      play_digi_fx_obj(SFX_EXPLOSION_1,1,id);
   }
   else if(victim == OBJ_NULL)
   {
      while (current_ref != OBJ_REF_NULL)
      {
         current_id = objRefs[current_ref].obj;

         if (current_id != id)
         {
            affect=object_affect(current_id, dtype);
            if (affect)
            {
               fix      dist,deltax,deltay,deltaz;

               deltax = fix_from_obj_coord(objs[id].loc.x-objs[current_id].loc.x);
               deltay = fix_from_obj_coord(objs[id].loc.y-objs[current_id].loc.y);
               deltaz = fix_from_obj_height_val(objs[id].loc.z-objs[current_id].loc.z);
               dist = fix_mul(deltax,deltax)+fix_mul(deltay,deltay)+fix_mul(deltaz,deltaz);

               if (dist < DAMAGE_PROJ_DISTANCE)
               {

                  origin.x = fix_make(-1,0);
                  origin.y = fix_make(-1,0);
                  origin.z = fix_make(-1,0);

                  player_attack_object(current_id,weapon_triple,proj_power,origin);
               }
            }
         }
         current_ref = objRefs[current_ref].next;
      }
   }
   else
   {
      origin.x = fix_from_obj_coord(loc.x);
      origin.y = fix_from_obj_coord(loc.y);
      origin.z = fix_from_obj_height(loc.z);

      player_attack_object(victim, weapon_triple, proj_power, origin);
   }
   if (special_effect)
      do_special_effect_location(id, special_effect, 0xFF, &loc, 0);
}

// returns whether it is being killed...
bool special_terrain_hit(ObjID cobjid)
{
   if (is_obj_destroyed(cobjid))
      return TRUE;

   if (objs[cobjid].obclass==CLASS_GRENADE)
   {
      if (objGrenades[objs[cobjid].specID].flags&GREN_ACTIVE_FLAG)
         ADD_DESTROYED_OBJECT(cobjid);
      else
      {
         return FALSE;     // dead grenades stay alive
         // in the sense that they are not live, but should remain physics live, see
      }
   }
   else if (objs[cobjid].obclass == CLASS_PHYSICS)
   {
      // only one terrain hit per turn!
      if (objPhysicss[objs[cobjid].specID].p3.x)
         return FALSE;
      else
         objPhysicss[objs[cobjid].specID].p3.x = 3;

      slow_proj_hit(cobjid,OBJ_NULL);
      EDMS_obey_collisions(objs[cobjid].info.ph);

      if (PhysicsProps[CPNUM(cobjid)].flags & PROJ_PRESERVE_WALL)
         return FALSE;

      // Signal a miss to our controller
      ai_misses(objs[objPhysicss[objs[cobjid].specID].owner].specID);
   }
   ADD_DESTROYED_OBJECT(cobjid);
   return TRUE;
}

#define DMG_THRESH  0x60000
#define HACK_THRESH 0x1800
#define SPCL_THRESH 0x80


// HEY COMMENTED OUT PROCEDURE
#ifdef CALLS_WERENT_SLOW
bool terrain_damage_object(physics_handle ph, fix raw_damage)
{
   bool dead = FALSE;
   ObjID target = physics_handle_to_id(ph);

   if (ObjProps[OPNUM(cobjid)].flags & SPCL_TERR_DMG)
   {
      if (raw_damage>SPCL_THRESH)
      {
         objs[target].info.current_hp = 0;
         ADD_DESTROYED_OBJECT(target);
         dead = TRUE;
      }
   }
   else
      dead = simple_damage_object(target,(raw_damage-HACK_THRESH)>>10,EXPLOSION_FLAG,NO_SHIELD_ABSORBTION);

   return(dead);
}
#endif

// ------------------------------
// compute_damage()
//
// computes damage to be inflicted on target
//
// target         ObjID of object that will be damaged
// damage_type    damage type of the attack       example : energy, explosion, physical, needle, etc...
// damage_mod     raw value of damage inflicted
// offense        offensive value of the attack
// penet          penetration value of the attack
// power_level    percentage of damage to be inflicted
// *effect        returns the effect number to be played
// *effect_row    a pointer to the effect row 

int compute_damage(ObjID target,int damage_type,int damage_mod,ubyte offense,ubyte penet,int power_level,ubyte *effect,ubyte *effect_row, ubyte attack_effect_type)
{
   int      damage = 0;
   int      delta;
   int      modifier;
   ubyte    affect;

   // AFFECTIVENESS
   //
   affect = object_affect(target, damage_type);
   if (affect)
   {
      damage = (damage_mod * power_level * affect) / 100;

      damage = armor_absorption(damage, ID2TRIP(target), penet);

      if (!global_fullmap->cyber)
      {
         // Compute the CRITICAL HIT affector
         //
         delta = random_bell_modifier((target == PLAYER_OBJ));
         modifier = (target == PLAYER_OBJ) ? (offense + delta - PLAYER_DEFENSE_VALUE) :
                     ((offense - ObjProps[OPNUM(target)].defense_value) + delta);

         if (modifier < -3)
            damage /= SQUARE(modifier+3);    // plus because it's negative
         else if ((modifier > 3) && (ObjProps[OPNUM(target)].defense_value != 0xFF))
         {
            // we are going to do critical damage, but we must check that the defense value
            // isn't 0xFF (meaning it's invulnerable to critical hits).

            // just put an upper bound to be safe.
            if (modifier > 12)
               modifier = 12;
            damage = (damage * modifier)/3;
         }
      }

      // TOUGHNESS
      if (ObjProps[OPNUM(target)].toughness!=3)
  	      damage>>=ObjProps[OPNUM(target)].toughness;
      else
         damage=0;

      if (damage < 0)
      {
         damage = 0;
      }
      else
      {
         damage = randomize_damage(damage,target==PLAYER_OBJ);

         // bound to realistic max
         if (damage > MAX_DAMAGE)
            damage = MAX_DAMAGE;
      }

      // take either the normal effect, unless we did lots of damage - then play bigger one
      if ((effect_row != NULL) && (attack_effect_type != SPECIAL_TYPE) && (effect != NULL) && !global_fullmap->cyber) 
      {
         // check to see if we did damage - if so - do appropriate hit effect
         // otherwise do a wall hit effect - MEANS NO DAMAGE/EFFECT
         if (damage)
            *effect = (damage < (damage_mod*7)/6) ? *(effect_row) : *(effect_row+1);
         else
            *effect = effect_matrix[NON_CRITTER_EFFECT][attack_effect_type][0];
      }
      else if (effect != NULL)
         *effect = 0;
   }
   else
   {
      // we didn't affect - so do the no effect one!
      if (effect)
	      *effect = (!global_fullmap->cyber) ? effect_matrix[NON_CRITTER_EFFECT][attack_effect_type][0] : 0;
   }

   return(damage);
}

// --------------------------------------------------------------
// critter_hit_effect()
//
void critter_hit_effect(ObjID target, ubyte effect,Combat_Pt location, int damage, int max_damage)
{
   fix      radius, height;
   byte      ht;
   ObjLoc   loc = objs[target].loc;

   // temporary - to hit effect_center - will take care of later
   SET_EFFECT_LOC(target, EFFECT_CENTER);

   SET_EFFECT_NUM(target, effect);
   SET_EFFECT_FRAME(target, 0);

   radius = fix_make(ObjProps[OPNUM(target)].physics_xr,0)/(PHYSICS_RADIUS_UNIT*4);

   height = fix_from_obj_height_val(loc.z);
   ht = ((height - location.z)/radius) + 4;
   if (ht < 1)
      ht = 1;
   else if (ht > 7)
      ht = 7;

   SET_EFFECT_HEIGHT(target, ht);

   if (damage < (max_damage/3))
   {
      SET_EFFECT_DUAL(target, 0);
      SET_EFFECT_SCALE(target, 1);
   }
   else if (damage < max_damage)
   {
      SET_EFFECT_DUAL(target, 0);
      SET_EFFECT_SCALE(target, 2);
   }
   else
   {
      SET_EFFECT_DUAL(target, 1);
      SET_EFFECT_SCALE(target, 3);
   }
}

// ---------------------------------------------------------------------------
// get_damage_estimate()
//
// Returns a number from DAMAGE_MIN to DAMAGE_MAX indicating how wounded
// a creature is.

int get_damage_estimate(ObjSpecID osid)
{
   ObjID    id = objCritters[osid].id;
   int      triple = ID2TRIP(id);

   return(DAMAGE_MAX -
      ((objs[id].info.current_hp*DAMAGE_MAX)/
        ObjProps[OPTRIP(triple)].hit_points));
}

// -------------------------------------------------------
// attack_object()
//
// target         ObjID of object that will be damaged
// damage_type    damage type of the attack       example : energy, explosion, physical, needle, etc...
// damage_mod     raw value of damage inflicted
// offense        offensive value of the attack
// penet          penetration value of the attack
// flags          flags for the attack          (currently just to see if player's shields absorb damage)
// power_level    percentage of damage to be inflicted
// *effect        returns the effect number to be played
// *effect_row    a pointer to the effect row 
// 
// returns whether target died

ubyte attack_object(ObjID target, int damage_type,int damage_mod, ubyte offense, ubyte penet, ubyte flags, int power_level, ubyte *effect_row, ubyte *effect, ubyte attack_effect_type, int *damage_inflicted)
{
   int    damage;
   char   diff;

   if (effect)
	   *effect = 0;
   // check to see if we have a valid target 
   if (target == OBJ_NULL)
      return(0);
   else if (is_obj_destroyed(target))
      return(0);
   // the next is same as dead!
   else if ((objs[target].info.current_hp==0) && DESTROY_OBJ_EFFECT(ObjProps[OPNUM(target)].destroy_effect))
      return(0);

   // get the difficulty level
   diff = (global_fullmap->cyber) ? player_struct.difficulty[CYBER_DIFF_INDEX] : player_struct.difficulty[COMBAT_DIFF_INDEX];

   // look combat difficult 0 setting - KILL OBJECTS IN ONE SHOT!!! - if object's toughness isn't 3
   if ((ObjProps[OPNUM(target)].toughness!=3) && !diff && (target != PLAYER_OBJ))
   {
      damage = objs[target].info.current_hp;
      if (effect)
	      *effect = (effect_row) ? *(effect_row+1) : 0;
   }
   else
   {
#ifdef SELFRUN       // we do max damage if we're in self run
      damage = ((objs[target].obclass == CLASS_CRITTER) && (target != PLAYER_OBJ)) ? 0xFF : 0; 
#else
      damage = compute_damage(target, damage_type, damage_mod, offense, penet, power_level, effect, effect_row, attack_effect_type);
#endif
   }

   if (damage_inflicted)
      *damage_inflicted=damage;

   // okay let's check if we're destroying an object that has a flagged destroy effect
   // if the high bit is flagged then we will destroy the object during the animation
   if ((objs[target].info.current_hp <= damage) && DESTROY_OBJ_EFFECT(ObjProps[OPNUM(target)].destroy_effect))
   {
      objs[target].info.current_hp=0;
      if (effect)
	      *effect = ObjProps[OPNUM(target)].destroy_effect;
      return(TRUE);
   }
   return(damage_object(target, damage, damage_type, flags));
}

// -------------------------------------------------------------
// player_attack_object()
//

#define CRAZE_DAMAGE_MOD 2

ubyte player_attack_object(ObjID target, int wpn_triple, int power_level, Combat_Pt origin)
{
   ubyte offense;
   int   damage_mod;
   int   wpn_class = TRIP2CL(wpn_triple);
   int   dtype;
   int   damage_inflicted;
   int   prop_val;
   ubyte penet;
   ubyte effect;
   ubyte *effect_row;
   ubyte attack_effect_type;
   ubyte special_effect = 0;
   ubyte flags = 0;
   ObjID effect_id = OBJ_NULL;
   bool  dead = FALSE;
   bool  new_loc = FALSE;
   ObjLoc            loc;
   ubyte effect_class = (objs[target].obclass == CLASS_CRITTER) ? CritterProps[CPNUM(target)].hit_effect : NON_CRITTER_EFFECT;

   // Special targeting ware hack
   if ((objs[target].obclass == CLASS_CRITTER) && 
       (get_player_ware_version(WARE_HARD,HARDWARE_TARGET) > 3) && (player_struct.curr_target == OBJ_NULL)
       && (!global_fullmap->cyber))
   {
      select_current_target(target,FALSE);
   }

   switch (wpn_class)
   {
      case (CLASS_GUN):    // Beam weapon is the only gun with damage type
         switch (TRIP2SC(wpn_triple))
         {
            case (GUN_SUBCLASS_HANDTOHAND):
               prop_val = SCTRIP(wpn_triple);
               damage_mod = HandtohandGunProps[prop_val].damage_modifier;
               offense = HandtohandGunProps[prop_val].offense_value;
               if (player_struct.drug_status[DRUG_LSD] > 0)
               {
                  damage_mod <<= CRAZE_DAMAGE_MOD;
                  offense += CRAZE_DAMAGE_MOD;
               }
               dtype = HandtohandGunProps[prop_val].damage_type;
               penet = HandtohandGunProps[prop_val].penetration;
               attack_effect_type = HAND_TYPE;
            break;
            case (GUN_SUBCLASS_BEAM):
               prop_val = SCTRIP(wpn_triple);
               damage_mod = BeamGunProps[prop_val].damage_modifier;
               offense = BeamGunProps[prop_val].offense_value;
               dtype = BeamGunProps[prop_val].damage_type;
               penet = BeamGunProps[prop_val].penetration;
               attack_effect_type = BEAM_TYPE;
            break;
            case (GUN_SUBCLASS_SPECIAL):
               prop_val = SCTRIP(wpn_triple);
               damage_mod = SpecialGunProps[prop_val].damage_modifier;
               offense = SpecialGunProps[prop_val].offense_value;
               dtype = SpecialGunProps[prop_val].damage_type;
               penet = SpecialGunProps[prop_val].penetration;
               attack_effect_type = SPECIAL_TYPE;
               special_effect = EFFECT_VAL(ObjProps[OPTRIP(SpecialGunProps[prop_val].proj_triple)].destroy_effect);
               break;
            case (GUN_SUBCLASS_BEAMPROJ):
               prop_val = SCTRIP(wpn_triple);
               damage_mod = BeamprojGunProps[prop_val].damage_modifier;
               offense = BeamprojGunProps[prop_val].offense_value;
               dtype = BeamprojGunProps[prop_val].damage_type;
               penet = BeamprojGunProps[prop_val].penetration;
               attack_effect_type = SPECIAL_TYPE;
               special_effect = EFFECT_VAL(ObjProps[OPTRIP(BeamprojGunProps[prop_val].proj_triple)].destroy_effect);
               if (BeamprojGunProps[prop_val].flags & 0x02)
                  flags |= STUN_ATTACK;
               break;
         }
         break;
      case (CLASS_AMMO):
         damage_mod = AmmoProps[CPTRIP(wpn_triple)].damage_modifier;
         offense = AmmoProps[CPTRIP(wpn_triple)].offense_value;
         dtype = AmmoProps[CPTRIP(wpn_triple)].damage_type; 
         penet = AmmoProps[CPTRIP(wpn_triple)].penetration;
         attack_effect_type = PROJ_TYPE;
         break;
      case (CLASS_GRENADE):
         damage_mod = GrenadeProps[CPTRIP(wpn_triple)].damage_modifier;
         offense = GrenadeProps[CPTRIP(wpn_triple)].offense_value;
         dtype = GrenadeProps[CPTRIP(wpn_triple)].damage_type;
         penet = GrenadeProps[CPTRIP(wpn_triple)].penetration;
         attack_effect_type = GREN_TYPE;
         break;
      default:
         return(0);
         break;
   }
   effect_row = effect_matrix[effect_class][attack_effect_type];
   dead = attack_object(target,dtype,damage_mod,offense,penet,flags,power_level, effect_row, &effect, attack_effect_type, &damage_inflicted);

   // this is for a slow projectile spang - not for objects exploding
   if (attack_effect_type == SPECIAL_TYPE)
      effect = 0; //special_effect;

   if (dead)
   {
      ubyte    old_effect = effect;

      // check to see if we're suppose to play an animation if object is destroyed....
      if (EFFECT_VAL(ObjProps[OPNUM(target)].destroy_effect))
         effect = ObjProps[OPNUM(target)].destroy_effect;

      if (old_effect != effect)
      {
         new_loc = TRUE;
         loc = objs[target].loc;
      }
   }

   if (effect != 0)
   {
//      if (objs[target].obclass == CLASS_CRITTER)
//         critter_hit_effect(target, effect, origin, damage_inflicted, damage_mod);
//#ifdef REMOVE_OLD_EFFECT
//      else
//#endif
      {
         fix   deltax, deltay,dist;

         // if it's a 3-d model - let's get the right place
         if (ObjProps[OPNUM(target)].render_type==1)
         {
            loc = objs[target].loc;

            if (ObjProps[OPNUM(target)].physics_xr > 20)
            {
               // let's randomize the hit by a bit if object is big enough
               loc.x += (RndRange(&damage_rnd, 0, 0x40)-0x20);
               loc.y += (RndRange(&damage_rnd, 0, 0x40)-0x20);
               loc.z += ((RndRange(&damage_rnd, 0, 0x10)-0x08) +
                  obj_height_from_fix(fix_make(ObjProps[OPNUM(target)].physics_xr,0) / PHYSICS_RADIUS_UNIT));
            }

            // and in the end - let's bring the effect up to the center of the object
         }
         else if (!new_loc)
         {
            // if we've been given bad info - let's just go on
            if ((origin.x < 0) || (fix_int(origin.y) > 64))
            {
               return(dead);
            }

            loc.x = obj_coord_from_fix(origin.x);
            loc.y = obj_coord_from_fix(origin.y);
            loc.z = obj_height_from_fix(origin.z);
         }

         deltax = OBJ_LOC_VAL_TO_FIX(objs[PLAYER_OBJ].loc.x-loc.x);
         deltay = OBJ_LOC_VAL_TO_FIX(objs[PLAYER_OBJ].loc.y-loc.y);
         dist = fix_fast_pyth_dist(deltax, deltay)<<2;

         // move explosion towards player
         loc.x += obj_coord_from_fix(fix_div(deltax, dist));
         loc.y += obj_coord_from_fix(fix_div(deltay, dist));

         effect_id=do_special_effect_location(target, effect, 0xFF, &loc, 0);
      }
   }

   if (attack_effect_type==BEAM_TYPE)
   {
      extern ObjID beam_effect_id;
      if (effect_id!=OBJ_NULL)
      {
         beam_effect_id = effect_id;
         hudobj_set_id(beam_effect_id, TRUE);
      }
   }

   return(dead);
}
