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
 * $Source: r:/prj/cit/src/RCS/newai.c $
 * $Revision: 1.43 $
 * $Author: xemu $
 * $Date: 1994/10/24 23:15:00 $
 *
 */

#include <stdlib.h>

#include "ai.h"
#include "cyber.h"
#include "player.h"
#include "pathfind.h"
#include "otrip.h"
#include "objects.h"
#include "objprop.h"
#include "objgame.h"
#include "objcrit.h"
#include "objsim.h"
#include "objbit.h"
#include "faketime.h"
#include "musicai.h"
#include "tilename.h"
#include "ice.h"
#include "tools.h"
#include "trigger.h"
#include "map.h"
#include "mapflags.h"
#include "damage.h"
#include "combat.h"
#include "physics.h"
#include "safeedms.h"
#include "aiflags.h"
#include "doorparm.h"
#include "diffq.h"
#include "visible.h"


// SOME YUMMY DEFINES!
#define AI_EDMS


// A bunch of visibility related factors.
// These should probably wind up in a quickbox?
#define BASE_VISIBILITY 			178
#define SHIELD_VISIBLE_FACTOR		52
#define LIGHT_VISIBLE_FACTOR		75
#define LANTERN_VISIBLE_FACTOR   80
#define GUN_VISIBLE_FACTOR			168
#define UNTRACK_VISIBLE_FACTOR	44
#define CROUCH_VISIBLE_FACTOR		16
#define ZDIFF_VISIBLE_FACTOR		1
#define DIST_VISIBLE_FACTOR      3
#define BEHIND_VISIBLE_FACTOR    128

#define MEDIAN_PERCEPTION			64

#define DEFAULT_SPEED   fix_make(0xD,0)

// Number of ticks where, if we haven't seen the player in that long, 
// we forget where he is.
#define AI_ATTENTION_SPAN CIT_CYCLE * 10

extern ObjLoc last_known_loc;
ulong time_last_seen;
bool priority_check;
extern bool door_moving(ObjID id,bool dir);
extern errtype set_posture_movesafe(ObjSpecID osid, ubyte new_pos);

short compute_base_visibility();
errtype run_evil_otto(ObjID id, int dist);
errtype run_cspace_ice();
errtype ai_spot_player(ObjID id, bool *raycast_success);
bool do_physics_stupidity(ObjID id, int big_dist);
void check_attitude_adjustment(ObjID id, ObjSpecID osid,int big_dist,bool raycast_success);
void load_combat_art(int cp_num);
errtype run_combat_ai(ObjID id, bool raycast_success);
errtype do_stealth_stuff(ObjID id, short base_vis, bool *raycast_success, fix dist);
void set_des_heading(ObjID id, ObjSpecID osid, fix targ_x, fix targ_y, fixang *angdiff, fixang *target_ang);
errtype follow_pathfinding(ObjID id, ObjSpecID osid);
LGPoint ai_patrol_func(ObjID id, ObjSpecID osid);
LGPoint ai_highway_func(ObjID id, ObjSpecID osid);
LGPoint ai_roam_func(ObjID id, ObjSpecID osid);
LGPoint ai_none_func(ObjID, ObjSpecID osid);
errtype run_peaceful_ai(ObjID id, int big_dist);

// Run all the ICEs, deal with their agitation, etc.  Boy, this could probably
// be a lot smarter than iterating through all objects, like having the
// agitated objects flag themselves as such when they become agitated, etc.

#define SHODAN_AVATAR_HOSAGE_DISTANCE  0xA0
#define AGITATED_ICE_DIST     49
#define ICE_INTERVAL          (CIT_CYCLE >> 1)

// chance vs 0xFF of firing on a given half-second
uchar ice_fire_chances[] = { 0x40, 0x80, 0xD0, 0xF0 };
ulong run_ice_time = 0;

errtype run_cspace_ice()
{
   ObjID id;
   int dx,dy,dist;
   extern errtype ai_fire_special(ObjID src, ObjID target, int proj_triple, ObjLoc src_loc, ObjLoc target_loc, uchar a, int duration);

   // Look for hostile ICEs, which closely resemble creatures
   // of course, only do so if we be in cspace

   // I'm agitated.  Yeah, agitated.  -- Devo
   FORALLOBJS(id)
   {
      // Are we an agitated ice encrusted thing?  Is it our stochastic time as determined by Lord Chaos?
      if (ICE_ICE_BABY(id) && ICE_AGIT(id) && ((rand()&0xFF) < ice_fire_chances[ICE_LEVEL(id)]))
      {
         dx = PLAYER_BIN_X - OBJ_LOC_BIN_X(objs[id].loc);
         dy = PLAYER_BIN_Y - OBJ_LOC_BIN_Y(objs[id].loc);
         dist = dx * dx + dy * dy;

         // Are we close enough to the player to care? 
         if (dist < AGITATED_ICE_DIST)
         {
            // Lob a slow projectile off at that wacky player.  Boy, we're perfectly statically accurate.
            ai_fire_special(id, PLAYER_OBJ, CYBERBOLT_TRIPLE, objs[id].loc, objs[PLAYER_OBJ].loc, ICE_LEVEL(id), SLOW_PROJECTILE_DURATION);
         }
      }
   }
   run_ice_time = player_struct.game_time + ICE_INTERVAL;
   return(OK);
}


// Compute and return the player's basic visibility for this frame
short compute_base_visibility()
{
   MapElem *pme;
   short visibility;

	// Update detection variables	
   if (!cspace_decoy_obj)
   {
	   if (time_last_seen > player_struct.game_time + AI_ATTENTION_SPAN)
	      last_known_loc.x = 255;

	   //compute basic visibility

      pme = MAP_GET_XY(PLAYER_BIN_X,PLAYER_BIN_Y);

	   // Then we roll against our perception, after figuring in
	   // tons of factors...
	   // base visibility
	   visibility = BASE_VISIBILITY;

	   // More visible if shields up (can't check yet)

	   // More visible if standing in light
	   visibility += (LIGHT_VISIBLE_FACTOR /
						   max(1,(
						   max(0,me_light_flr(pme) - me_templight_flr(pme)) +
						   max(0,me_light_ceil(pme) - me_templight_ceil(pme))) / 2));
      visibility += LANTERN_VISIBLE_FACTOR * player_struct.light_value;         

	   // More visible if fired gun recently (this may want to be exponentially decaying
      if (player_struct.last_fire)  // make sure we've actually fired a weapon
   	   visibility += ((GUN_VISIBLE_FACTOR * CIT_CYCLE) / (player_struct.game_time - player_struct.last_fire));

	   // Less visible if critters have lost track of ya
	   if (last_known_loc.x == 255)
		   visibility -= UNTRACK_VISIBLE_FACTOR;

      // Some things that don't affect you in cspace
      if (!global_fullmap->cyber)
      {
	      // Less visible if crouching or crawling
	      visibility -= CROUCH_VISIBLE_FACTOR * player_struct.posture;
      }
   }
   return(visibility);
}

// id is evil otto, deal accordingly (moving, hosing, etc.)
errtype run_evil_otto(ObjID id, int dist)
{
   // Are we close enough to totally hose the player?
   if (dist < SHODAN_AVATAR_HOSAGE_DISTANCE)
   {
      damage_player(15, 0x7, NO_SHIELD_ABSORBTION);
//      attack_object(PLAYER_OBJ, CritterProps[cp_num].attacks[0].damage_type, CritterProps[cp_num].attacks[0].damage_modifier,
//         CritterProps[cp_num].attacks[0].offense_value, CritterProps[cp_num].attacks[0].penetration, 0, 100, NULL, 0, 0, NULL);
   }
   else
   {
      fix xvec,yvec,fdist;
      int dx,dy;
      ObjLoc newloc = objs[id].loc;
      // Move the AVAMATAR OF SHODAN (Evil Otto) closer to the player 
      // first, get a normalized vector
      dx = PLAYER_BIN_X - OBJ_LOC_BIN_X(objs[id].loc);
      dy = PLAYER_BIN_Y - OBJ_LOC_BIN_Y(objs[id].loc);
      xvec = fix_from_obj_coord(dx);
      yvec = fix_from_obj_coord(dy);
      fdist = fix_fast_pyth_dist(xvec,yvec);
      xvec = fix_div(xvec,fdist) >> 2;
      yvec = fix_div(yvec,fdist) >> 2;

      // then, move otto along it
      newloc.x = obj_coord_from_fix(xvec + fix_from_obj_coord(newloc.x));
      newloc.y = obj_coord_from_fix(yvec + fix_from_obj_coord(newloc.y));
      newloc.z = objs[PLAYER_OBJ].loc.z;
//      Warning(("AVATAR_SHODAN at 0x%x, 0x%x (dist = 0x%x)\n",newloc.x,newloc.y,dist));
      obj_move_to(id,&newloc,FALSE);
   }
   return(OK);
}


// Does appropriate destruction & reconstitution of physics part
// of said critter.
// Returns whether or not we should continue to think
// about this particular creature.
short ignore_distance[] = {6, 10} ;

bool do_physics_stupidity(ObjID id, int big_dist)
{
   ObjSpecID osid = objs[id].specID;
   int dist = big_dist >> 8;
   int use_dist;
   fix there_yet;

#ifdef DISTANCE_AI_KILL
   // don't phys-kill anything that is currently pathfinding or in combat mode (it'll leave combat mode 
   // after a while anyways)
   if ((objCritters[osid].path_id != -1) ||
       (objCritters[osid].mood == AI_MOOD_HOSTILE) || (objCritters[osid].mood == AI_MOOD_ATTACKING))
   {
      if (!EDMS_frere_jaques(objs[id].info.ph))
      {
//         Spew(DSRC_PHYSICS_Sleeper, ("obj id %x, ph = %d(0x%x) is PF or attack but asleep!\n",id,objs[id].info.ph,objs[id].info.ph));
//         Spew(DSRC_PHYSICS_Sleeper, ("obj id %x, head = %x, spd = %x, urg = %x\n",id,objCritters[osid].des_heading,
//            objCritters[osid].des_speed, objCritters[osid].urgency));
      }
      return(TRUE);
   }

   use_dist = ignore_distance[global_fullmap->cyber];

   // This really needs to deal with cameras!
   if (dist > use_dist)
   {
      // What, we've gone too far away?  Well, then set us to zero so EDMS sleeps us nicely
      if (CHECK_OBJ_PH(id))
         safe_EDMS_ai_control_robot(objs[id].info.ph,0,0,0,0,&there_yet,0);
//      else if (!(get_crit_posture(osid) == DEATH_CRITTER_POSTURE))
//         Warning(("hey, trying to sleep id %x without no physics handle (ph = %x)!\n",id,objs[id].info.ph));
//      if (EDMS_frere_jaques(objs[id].info.ph))
//         Spew(DSRC_PHYSICS_Sleeper, ("obj id %x, ph = %d(0x%x) is too far but awake!\n",id,objs[id].info.ph,objs[id].info.ph));
      return(FALSE);
   }
#endif
   if ((!EDMS_frere_jaques(objs[id].info.ph)) && (objCritters[osid].des_speed != 0))
   {
//      Spew(DSRC_PHYSICS_Sleeper, ("obj id %x, ph = %d(0x%x) is in range but asleep!\n",id,objs[id].info.ph,objs[id].info.ph));
//      Spew(DSRC_PHYSICS_Sleeper, ("obj id %x, head = %x (vs %x), spd = %x, urg = %x\n",id,objCritters[osid].des_heading,
//         objCritters[osid].des_speed, objCritters[osid].urgency));   
   }
   return(TRUE);
}

// If our current pathfind is greater than REPATHFIND_DIST away from reality,
// punt and repathfind 
#define REPATHFIND_DIST 0x4
errtype ai_spot_player(ObjID id, bool *raycast_success)
{
   ObjSpecID osid = objs[id].specID;
   ObjCritter *pcrit = &objCritters[osid];

	time_last_seen = player_struct.game_time;
   *raycast_success = TRUE;
   ai_find_player(id);

   // If not friendly, trigger the combat music
   if ((pcrit->mood != AI_MOOD_FRIENDLY) && (!ai_critter_sleeping(osid)))
      ai_critter_seen();

   // Aha!  We have seen the player, so if we are of a sort to get ticked off,
   // then let's do so.
   if (QUESTVAR_GET(COMBAT_DIFF_QVAR) > 0)
   {
      if ((pcrit->mood == AI_MOOD_NEUTRAL) || (pcrit->mood == AI_MOOD_ISOLATION))
      {
         // punt our old, probably irrelevant pathfind
         if (pcrit->path_id != -1)
         {
            delete_path(pcrit->path_id);
            pcrit->path_id = -1;
         }
      }
      else if (pcrit->path_id != -1)
      {
         // if the difference between our current path's destination and the actual
         // location of the player is too great, punt the old one.
         if (long_fast_pyth_dist((last_known_loc.x>>8) - paths[pcrit->path_id].dest.x, 
               (last_known_loc.y>>8) - paths[pcrit->path_id].dest.y) > REPATHFIND_DIST)
         {
            pcrit->path_id = -1;
            delete_path(pcrit->path_id);
         }
      }
      if ((pcrit->mood == AI_MOOD_NEUTRAL) || (pcrit->mood == AI_MOOD_ISOLATION))
      {
         if (pcrit->path_id != -1)
            delete_path(pcrit->path_id);
         pcrit->path_id = -1;
         pcrit->mood = AI_MOOD_HOSTILE;
         pcrit->flags |= AI_FLAG_CHASING;
         // and play our "notice" sound effect
         play_digi_fx_obj(CritterProps[CPNUM(id)].notice_sound, 1,id);
      }
   }
   return(OK);
}


// Do appropriate stuff for critter object id to try and find the
// player
errtype do_stealth_stuff(ObjID id, short base_vis, bool *raycast_success, fix dist)
{
   short use_vis;
   State plr_state, our_state;
   fixang ang_diff, real_ang;
   int r;
   ObjSpecID osid = objs[id].specID;
   ObjCritter *pcrit = &objCritters[osid];

   if (cspace_decoy_obj)
   {
      time_last_seen = player_struct.game_time;
      *raycast_success = TRUE;
      last_known_loc = objs[cspace_decoy_obj].loc;
   }
   else
   {
		// Where's Waldo?
		// We only get to look when we're getting to run AI on ourselves.


      // If we are "chasing" the player (via our flags)
      // Then we don't have to actually spot the player to know his current location
      // although we don't count as having "spotted" the player (so that we still time out if we haven't
      // found the player in ATTENTION_SPAN amount of time).
      // Even if this is true, we do the whole visibility rigamarole so that if we DO see the player,
      // then we reset our attention span.
      if (pcrit->flags & AI_FLAG_CHASING)
         last_known_loc = objs[PLAYER_OBJ].loc;

      // We do all the stealth rolling first, so that we can cut out raycasting whenever possible

      // We should also filter out cases where player is behind der robotenhausen
      use_vis = base_vis;

		// Less visible if on far Z to searcher
      if (!global_fullmap->cyber)
      {
   		use_vis -= ZDIFF_VISIBLE_FACTOR * abs(objs[PLAYER_OBJ].loc.z - objs[id].loc.z);

         // Less visible in far away in normal coords
         use_vis -= DIST_VISIBLE_FACTOR * fix_int(dist);
      }

      safe_EDMS_get_state(objs[PLAYER_OBJ].info.ph, &plr_state);
//      if (!CHECK_OBJ_PH(id))
//         Warning(("Ack! 0x%x ph == %d (osid = 0x%x) sanity = %d!\n",pcrit->id,objs[pcrit->id].info.ph,
//            osid,EDMS_sanity_check()));
//      else
      {
         safe_EDMS_get_state(objs[pcrit->id].info.ph, &our_state);
         ang_diff = point_in_view_arc(plr_state.X, plr_state.Y, our_state.X, our_state.Y,
            0x4000 - fixang_from_phys_angle(phys_angle_from_obj(objs[id].loc.h)),
            &real_ang);
         use_vis -= BEHIND_VISIBLE_FACTOR * (ang_diff / 0x8000);
      }

		// Now factor in our own perception skill.
		use_vis += CritterProps[CPNUM(id)].perception - MEDIAN_PERCEPTION;

      if (use_vis > 0)
      {

         r = rand()%255;
   		if (r < use_vis)
			{
				if (ray_cast_objects(id, PLAYER_OBJ,
					VISIBLE_MASS, VISIBLE_SIZE, VISIBLE_SPEED,
					VISIBLE_RANGE) == PLAYER_OBJ)
               ai_spot_player(id,raycast_success);
			}
      }
   }
   return(OK);
}

void set_des_heading(ObjID id, ObjSpecID osid, fix targ_x, fix targ_y, fixang *angdiff, fixang *target_ang)
{
   State current_state;

#ifdef AI_EDMS
   safe_EDMS_get_state(objs[id].info.ph, &current_state);
   *angdiff = point_in_view_arc(targ_x, targ_y, current_state.X, current_state.Y,
         0x4000 - fixang_from_phys_angle(phys_angle_from_obj(objs[objCritters[osid].id].loc.h)), target_ang);
   objCritters[osid].des_heading = fixang_to_fixrad(*target_ang);
#else
   objCritters[osid].des_heading = 0;
#endif
}

// Continue along the pathfinding path, grabbing new steps as
// necessary when reaching old steps.
#define MAX_PATH_TRIES  25

char dir_table[3][3] =
{ { 2, 2, 2, },
  { 3, 0, 1, },
  { 0, 0, 0, },
};

errtype follow_pathfinding(ObjID id, ObjSpecID osid)
{
   LGPoint sq,csq;
   char steps_left, path_id,newdir;
   ObjID open_me = OBJ_NULL;

   path_id = objCritters[osid].path_id;

   // If our pathfind request ain't been filled yet, don't do anything
   if (paths[path_id].num_steps == -1)
   {
      return(OK);
   }

   // See whether or not we've gone further ahead on our path than
   // we expected to.  Hmm, if this is too slow we could probably keep track
   // of our last location and only do this operation if that's changed.
   sq.x = OBJ_LOC_BIN_X(objs[id].loc);
   sq.y = OBJ_LOC_BIN_Y(objs[id].loc);
   if (check_path_cutting(sq, objCritters[osid].path_id))
   {
      objCritters[osid].pf_x = sq.x;
      objCritters[osid].pf_y = sq.y;
   }

   if (paths[path_id].num_steps == 0)
   {
      // If our path has been deleted from out from underneath us, stop
      // trying to follow it...
      delete_path(objCritters[osid].path_id);
      objCritters[osid].path_id = -1;
      return(OK);
   }

   if ((objCritters[osid].pf_x == -1) ||
       (paths[path_id].curr_step == -1) || 
       ((OBJ_LOC_BIN_X(objs[id].loc) == objCritters[osid].pf_x) &&
        (OBJ_LOC_BIN_Y(objs[id].loc) == objCritters[osid].pf_y))) 
   {
      // We're where we want to be, so lets get the next step!
      objCritters[osid].path_tries = 0;
      newdir = next_step_on_path(path_id, &sq, &steps_left);
      if (steps_left == -1)
      {
         objCritters[osid].path_id = -1;
      }
      else
      {
         // Plug the next step into our local state
         objCritters[osid].pf_x = sq.x;
         objCritters[osid].pf_y = sq.y;
      }
   }
   else
   {
      // Keep on truckin' towards our old location
//      char ft1[50],ft2[50],ft3[50];
      fixang angdiff, target_ang;

      if (objCritters[osid].path_tries++ > MAX_PATH_TRIES)
      {
         // Okay, clearly something has rendered our path invalid,
         // since we ain't having no success in getting there.  Let's punt.
         // We could make this keep trying and resubmit a PF request.  Should we?
         delete_path(objCritters[osid].path_id);
         objCritters[osid].path_id = -1;
      }
      // Is there a door in the way?
      // goddamn, this is a stupid way of doing things....argh!
      if (!(((ObjProps[OPNUM(id)].flags & CLASS_FLAGS) >> CLASS_FLAGS_SHF) & CRITTER_NODOOR_OBJPROP_FLAG))
      {
            sq.x = objCritters[osid].pf_x;
            sq.y = objCritters[osid].pf_y;
            csq.x = OBJ_LOC_BIN_X(objs[id].loc);
            csq.y = OBJ_LOC_BIN_Y(objs[id].loc);
            if ((abs(csq.x - sq.x) < 2) && (abs(csq.y - sq.y) < 2))
               pf_obj_doors(MAP_GET_XY(csq.x, csq.y), MAP_GET_XY(sq.x,sq.y), dir_table[sq.y - csq.y + 1][sq.x - csq.x + 1], &open_me); 
      //      Warning(("open_me = %x, dt = %d from (%x,%x) to (%x,%x)!\n",open_me,dir_table[sq.y - csq.y + 1][sq.x - csq.x + 1],
      //         csq.x,csq.y,sq.x,sq.y));
            if ((open_me != OBJ_NULL) && (DOOR_CLOSED(open_me)) && !(door_moving(open_me,FALSE)))
            {
               bool use_door(ObjID id, uchar in_inv, ObjID cursor_obj);
               use_door(open_me,0x2,OBJ_NULL);
            }
      }
      set_des_heading(id,osid, fix_make(objCritters[osid].pf_x, 0x8000),
         fix_make(objCritters[osid].pf_y, 0x8000), &angdiff, &target_ang);

#ifdef WACKY_SPEED_REDUCTION
      if (paths[path_id].num_steps < 2)
      {
         objCritters[osid].des_speed = DEFAULT_SPEED >> 3;
         Warning(("speed slowing due to distance!\n"));
      }
      else
#endif
      if (objCritters[osid].des_speed == 0)
         objCritters[osid].des_speed = DEFAULT_SPEED;

#ifdef SPEED_QUARTERING
      // Quarter speed if we are mostly turning and are going fast
      if ((angdiff > 0x2000) && (objCritters[osid].des_speed > MAX_TURNING_SPEED))
      {
         objCritters[osid].des_speed = objCritters[osid].des_speed >> 2;
      }
#endif
   }
   return(OK);
}

char ai_ranges;


// Are we legal to attack right now?  If so, slam us into ATTACKING,
// otherwise slam us into hostile.  Make appropriate adjustments so that
// we are actively looking for the player in either case.

// Hey Rocky, watch me pull this constant out of my butt!
#define SHORT_RANGE_Z   0xA0
void check_attitude_adjustment(ObjID id, ObjSpecID osid,int big_dist,bool raycast_success)
{
   char i;
   short dist = big_dist >> 8;
   int cp_num = CPNUM(id);
   uchar care_mask = 0;

   ai_ranges = 0;

   // Frankly, if we have no clue where the player is then
   // don't bother trying to find him or anything.... in fact
   // we go back to being NEUTRAL, I think.  Although we will
   // continue on our current pathfinding in hopes of reacquiring 
   // the player
   if (last_known_loc.x == 255)
   {
      objCritters[osid].mood = AI_MOOD_NEUTRAL;
      objCritters[osid].flags &= ~AI_FLAG_CHASING;
      set_posture(osid, STANDING_CRITTER_POSTURE);
      return;
   }

   // Check ranges
   for (i=0; i < 2; i++)
   {
      short rng;
      rng = CritterProps[cp_num].attacks[i].att_range;
      if (dist <= rng)
      {
         // If we are a "short range" attack, then check z before trying
         if (rng <= 2)
         {
            if ((abs(objs[id].loc.z - objs[PLAYER_OBJ].loc.z) << SLOPE_SHIFT_U) < SHORT_RANGE_Z)
               ai_ranges |= 1 << i;
         }
         else
            ai_ranges |= 1 << i;
      }
   }

   // If we have no chance of doing a given attack, then don't worry about it's range
   care_mask = 0;
   if (CritterProps[cp_num].alt_perc == 0)
      care_mask = 0x1;
   else if (CritterProps[cp_num].alt_perc == 0xFF)
      care_mask = 0x2;
   else
      care_mask = 0x3;

   if ((care_mask & ai_ranges) != care_mask)
   {
      // If we aren't in range of both weapons, get closer
      objs[id].info.inst_flags |= CLASS_INST_FLAG2;
   }
   if (ai_ranges & care_mask)
   {
      fixang angdiff, target_ang;
      // If we're in range of all wpns, stop trying to get closer
      // but do keep trying to face the player.  Note that normally the 
      // "face the player" part is dealt with by the pathfinder, hopefully,
      // and so will blast out our des_heading set here.  We have to do the work
      // anyways here in order to figure out wheher or not the critter is facing
      // the player
      set_des_heading(id,osid, fix_from_obj_coord(last_known_loc.x), fix_from_obj_coord(last_known_loc.y),
         &angdiff, &target_ang);
      if (angdiff < 0x2000) 
      {
#ifdef AI_EDMS
         if (raycast_success || 
            (ray_cast_objects(id, PLAYER_OBJ, VISIBLE_MASS, VISIBLE_SIZE, VISIBLE_SPEED, VISIBLE_RANGE) == PLAYER_OBJ))
         {
            raycast_success = TRUE;
            objCritters[osid].mood = AI_MOOD_ATTACKING;
#ifdef ANNOYING_COMBAT_SPEW
            Spew(DSRC_AI_Combat, ("id %x Spotted player, attacking!\n"));
#endif
         }
         else
#endif
         {
            objCritters[osid].mood = AI_MOOD_HOSTILE;
            set_posture_movesafe(osid, STANDING_CRITTER_POSTURE);
#ifdef ANNOYING_COMBAT_SPEW
            Spew(DSRC_AI_Combat, ("id %x failed raycast!\n"));
#endif
         }
      }
      else
      {
         objCritters[osid].mood = AI_MOOD_HOSTILE;
         set_posture_movesafe(osid, STANDING_CRITTER_POSTURE);
#ifdef ANNOYING_COMBAT_SPEW
         Spew(DSRC_AI_Combat, ("id %x failed angcheck, angdiff = %x\n",angdiff));
#endif
      }
   }
}

void load_combat_art(int cp_num)
{
	extern Id posture_bases[];
   char p;
   if (ResPtr(posture_bases[ATTACKING_CRITTER_POSTURE] + cp_num) == NULL)
   {
      // Suspend time during loading of combat art
      ulong old_ticks = *tmd_ticks;
      extern ulong last_real_time;
      for (p = ATTACKING_CRITTER_POSTURE; p <= ATTACKING2_CRITTER_POSTURE; p++)
      {
         if (p != KNOCKBACK_CRITTER_POSTURE)
         {
            ResLock(posture_bases[p] + cp_num);
            ResUnlock(posture_bases[p] + cp_num);
         }
      }
      last_real_time += *tmd_ticks - old_ticks;
   }
}   

// copied in ai.c
#define DEFAULT_URGENCY fix_make(0x30,0)

// Run the AI for a combat-worthy critter, either looking actively for
// a nearby player, or actually shooting at such
// This really needs to have gnosis of:
// -- beelining for player when close enough & appropriate
// -- sidestepping intelligently, using cover and such (we sidestep very stupidly now)
errtype run_combat_ai(ObjID id, bool raycast_success)
{
   ObjSpecID osid = objs[id].specID;
   ObjCritter *pcrit = &objCritters[osid];
   LGPoint dest,source;
   int cp_num;

   // Sidestep stupidly
//   pcrit->sidestep = fix_make(rand()%200 - 100, 0);

   cp_num = CPNUM(id);
   if (pcrit->mood == AI_MOOD_ATTACKING) 
   {
      // Don't bother if we're already trying to attack
      if ((get_crit_posture(osid) != ATTACKING_CRITTER_POSTURE) && (get_crit_posture(osid) != ATTACKING2_CRITTER_POSTURE))
      {
         if (ai_ranges)
         {
            if (pcrit->attack_count < player_struct.game_time)
            {
               char posture;
               extern bool music_on;

               load_combat_art(cp_num);
               if (!(ai_ranges & 0x1))
                  posture = ATTACKING2_CRITTER_POSTURE;
               else if (!(ai_ranges & 0x2))
                  posture = ATTACKING_CRITTER_POSTURE;
               else
                  posture = (rand()%255 < CritterProps[cp_num].alt_perc) ? ATTACKING2_CRITTER_POSTURE : ATTACKING_CRITTER_POSTURE;
               set_posture(osid, posture);
               if (music_on) mai_attack();
               // Set how long before we get to attack again!
               pcrit->attack_count = (posture==ATTACKING_CRITTER_POSTURE) ?
                  player_struct.game_time + CritterProps[cp_num].attacks[0].speed :
                  player_struct.game_time + CritterProps[cp_num].attacks[1].speed;
            }
         }
         else
            // If we are out of range, stop being all attack-like in one's posturing
            set_posture(osid,STANDING_CRITTER_POSTURE);
      }
   }

   if (raycast_success)
   {
      // jeepers!  that's the player!
      fixang diffang, targang;

      if (pcrit->orders == AI_ORDERS_NOMOVE)
         pcrit->sidestep = 0;
      else
         pcrit->sidestep = fix_make(rand()%400 - 200, 0);

      delete_path(pcrit->path_id);
      pcrit->path_id = -1;
      set_des_heading(id,osid,fix_make(OBJ_LOC_BIN_X(objs[PLAYER_OBJ].loc),0x8000), 
         fix_make(OBJ_LOC_BIN_Y(objs[PLAYER_OBJ].loc),0x8000), &diffang, &targang);

      // If we're out of range or keep missing, run hell-bent towards the player
      // otherwise take careful potshots
      if ((ai_ranges) && (!(objs[id].info.inst_flags & CLASS_INST_FLAG2)) &&
          (QUESTVAR_GET(COMBAT_DIFF_QVAR) < 3) && (ID2TRIP(id) != AUTOBOMB_TRIPLE))
      {
         objCritters[osid].des_speed = DEFAULT_SPEED >> 3;
         objCritters[osid].urgency = DEFAULT_URGENCY >> 1;
      }
      else
      {
         objCritters[osid].des_speed = DEFAULT_SPEED;    
         objCritters[osid].urgency = DEFAULT_URGENCY << 1;
      }
   }
   else
   {
      objCritters[osid].sidestep = 0;
      objCritters[osid].des_speed = DEFAULT_SPEED;
      if (pcrit->path_id != -1)
      {
         follow_pathfinding(id,osid);
      }
      else
      {
         // Do our damnedest to get closer to the player
         // For now this is a pathfind, it should gain knowledge of beelining soon
         if ((objs[id].info.inst_flags & CLASS_INST_FLAG2) || (pcrit->mood == AI_MOOD_HOSTILE))
         {
            source.x = OBJ_LOC_BIN_X(objs[id].loc);
            source.y = OBJ_LOC_BIN_Y(objs[id].loc);
            dest.x = OBJ_LOC_BIN_X(objs[PLAYER_OBJ].loc);
            dest.y = OBJ_LOC_BIN_Y(objs[PLAYER_OBJ].loc);
            pcrit->path_id = request_pathfind(source,dest,objs[PLAYER_OBJ].loc.z,objs[id].loc.z,TRUE);
   //         mprintf("combat id %x pathfind request = 0x%x!\n",id,pcrit->path_id);

            // if the path we want don't exist, stochastically go back to being neutral
            check_requests(TRUE);
            if ((paths[pcrit->path_id].num_steps == 0) && ((rand() & 0xFF) < 0x30))
            {
               objCritters[osid].mood = AI_MOOD_NEUTRAL;
               objCritters[osid].flags &= ~AI_FLAG_CHASING;
               set_posture(osid, STANDING_CRITTER_POSTURE);
            }
         }
      }
   }
   return(OK);
}


LGPoint ai_patrol_func(ObjID, ObjSpecID osid)
{
   char temp_x, temp_y;
   ObjCritter *pcrit = &objCritters[osid];
   LGPoint dest;

   // Swap destination
   temp_x = pcrit->dest_x;
   temp_y = pcrit->dest_y;
   dest.x = pcrit->dest_x = pcrit->x1;
   dest.y = pcrit->dest_y = pcrit->y1;
   pcrit->x1 = temp_x;
   pcrit->y1 = temp_y;
   return(dest);
}

LGPoint ai_highway_func(ObjID, ObjSpecID osid)
{
   LGPoint dest = {-1,-1};
   ObjID curr_id;
   int param, interface_param;

   curr_id = objCritters[osid].loot2;
   if (objs[curr_id].obclass != CLASS_TRAP)
   {
      return(dest);
   }
   switch(objCritters[osid].x1)
   {
      case 0: param = objTraps[objs[curr_id].specID].p1; break;   
      case 1: param = objTraps[objs[curr_id].specID].p2; break;
      case 2: param = objTraps[objs[curr_id].specID].p3; break;
   }

   // Secret usage in highway functions
   if ((objTraps[objs[curr_id].specID].p4 & 0xFFFF) != 0)
   {
      interface_param = objTraps[objs[curr_id].specID].p4 >> 16;
      switch (objTraps[objs[curr_id].specID].p4 & 0xFFFF)
      {
         case 1: do_multi_stuff(interface_param);  break;
      }
   }
   if ((param != OBJ_NULL) && (objs[param].active) && (objs[param].obclass == CLASS_TRAP))
   {
      dest.x = OBJ_LOC_BIN_X(objs[param].loc);
      dest.y = OBJ_LOC_BIN_Y(objs[param].loc);
      objCritters[osid].dest_x = dest.x;
      objCritters[osid].dest_y = dest.y;
      objCritters[osid].loot2 = param;
   }
   return(dest);
}

#define DEFAULT_BROWNIAN_DIST 5
#define FIND_ROAM_TRIES 25

LGPoint ai_roam_func(ObjID id, ObjSpecID osid)
{
   LGPoint dest,source;
   char tries=0;
   bool okay;
   short bd;

   source.x = OBJ_LOC_BIN_X(objs[id].loc);
   source.y = OBJ_LOC_BIN_Y(objs[id].loc);

   // reset number of tries
   objCritters[osid].x1 = 0;

   bd = objCritters[osid].y1 ? objCritters[osid].y1 : DEFAULT_BROWNIAN_DIST;
   okay = FALSE;
   while (!okay && (tries < FIND_ROAM_TRIES))
   {
      dest.x = source.x + rand()%bd - (bd/2);
      dest.y = source.y + rand()%bd - (bd/2);
      if (me_tiletype(MAP_GET_XY(dest.x,dest.y)) == TILE_OPEN)
         okay = TRUE;
      tries++;
   }
   return(dest);
}

LGPoint ai_none_func(ObjID, ObjSpecID osid)
{
   LGPoint goof = {-1,-1};
   objCritters[osid].urgency = 0;
   objCritters[osid].sidestep = 0;
   objCritters[osid].des_speed = 0;
   return(goof);
}

// Run the AI for a non-combat critter, carrying out SHODANs will in
// some other way
LGPoint (*ai_order_funcs[])(ObjSpecID, ObjID) = { ai_none_func, ai_roam_func, ai_none_func, ai_patrol_func, ai_highway_func, ai_none_func };

#define MAX_ROAM_PATH   32
#define MAX_ROAM_TRIES  255

#define TRANQ_RAND_MASK    0xFFF
#define TRANQ_RAND_LEVEL   5
#define CONFUSE_RAND_MASK  0xFFF
#define CONFUSE_RAND_LEVEL 5

errtype run_peaceful_ai(ObjID id, int big_dist)
{
   ObjSpecID osid = objs[id].specID;
   LGPoint source,dest;
   bool do_path = TRUE;

   objCritters[osid].sidestep = 0;
   if (objCritters[osid].path_id != -1)
   {
      // Follow our pathfinding...

      // If we have too far to go, and we aren't very specific about
      // what it is we are doing, try again
      if ((objCritters[osid].orders == AI_ORDERS_ROAM) || (objCritters[osid].flags & AI_FLAG_CONFUSED))
      {
         objCritters[osid].x1++;

         // if confused we are more impatient to find new locations
         if (objCritters[osid].flags & AI_FLAG_CONFUSED)
            objCritters[osid].x1 += 3;

         // stop this particular wandering if the path we just chose is too long or if we've been at it for too long.
         if (((objCritters[osid].x1 == 0) && (path_length(objCritters[osid].path_id) > MAX_ROAM_PATH)) ||
           (objCritters[osid].x1 > MAX_ROAM_TRIES))
         {
//            Spew(DSRC_AI_Pathfind, ("punting roam path!\n"));
            delete_path(objCritters[osid].path_id);
            objCritters[osid].path_id = -1;
            do_path = FALSE;
         }
      }
      if (do_path)
         follow_pathfinding(id,osid);
   }
   else
   {
      // Hmm, we should make a new pathfinding request
      // But punt out if we are too far away from the player
      if (do_physics_stupidity(id, big_dist))
      {
         source.x = OBJ_LOC_BIN_X(objs[id].loc);
         source.y = OBJ_LOC_BIN_Y(objs[id].loc);
         if (ai_order_funcs[objCritters[osid].orders] != NULL)
         {
            dest = ai_order_funcs[objCritters[osid].orders](id,osid);
            if (dest.x != -1)
            {
               objCritters[osid].path_id = request_pathfind(source,dest,0,objs[id].loc.z,FALSE);
//               mprintf("peaceful id %x pathfind request = 0x%x!\n",id,objCritters[osid].path_id);
//               Spew(DSRC_AI_Path, ("peace(%x): id %x getting new path (%d), from %x,%x to %x,%x\n",
//                  big_dist,id,objCritters[osid].path_id,
//                  PT_UNWRAP(source),PT_UNWRAP(dest)));
            }
         }
      }
//      else
//         Spew(DSRC_AI_Combat, ("combat skipping id %x due to distance %x\n",id,big_dist));
   }
   if ((objCritters[osid].flags & AI_FLAG_CONFUSED) && ((rand() & CONFUSE_RAND_MASK) < CONFUSE_RAND_LEVEL))
   {
      set_crit_posture(osid,MOVING_CRITTER_POSTURE);
      objCritters[osid].flags &= ~AI_FLAG_CONFUSED;
   }
   return(OK);
}

#define COMBAT_FRAMES   2
#define DEFAULT_FRAMES  13

errtype ai_run()
{
   ObjSpecID osid;
   ObjID id;
   short visibility;
   bool raycast_success;
   int dist;
   char mood;
#ifdef PLAYTEST
   short crit_count = 0;
#endif
   extern bool physics_running;
   extern ObjID shodan_avatar_id;
   extern bool ai_on;
   extern errtype apply_EDMS_controls(ObjSpecID osid);

#ifndef GAMEONLY
   // Punt out if no physics or no ai
   if (!physics_running)
      return(OK);
#endif

   check_requests(FALSE);

   // Check ICE agitation
   if ((global_fullmap->cyber) && (run_ice_time < player_struct.game_time))
      run_cspace_ice();

   visibility = compute_base_visibility();

   // Cycle through all the critters
   priority_check = FALSE;
   osid = objCritters[0].id;
   while (osid != OBJ_SPEC_NULL)
   {
      if ((!CHECK_OBJ_PH(objCritters[osid].id)) && (objCritters[osid].id != shodan_avatar_id))
      {
         goto ai_loop_end;
      }
      if ((objCritters[osid].orders == AI_ORDERS_SLEEP) ||
          (get_crit_posture(osid) == DEATH_CRITTER_POSTURE))
         goto ai_loop_end;
      else if (objCritters[osid].flags & AI_FLAG_TRANQ)
      {
         if ((rand() & TRANQ_RAND_MASK) < TRANQ_RAND_LEVEL)
         {
            objCritters[osid].flags &= ~AI_FLAG_TRANQ;
            if (CritterProps[CPNUM(objCritters[osid].id)].flags & AI_FLAG_FLYING)
               apply_gravity_to_one_object(objCritters[osid].id,0);
         }
         goto ai_loop_end;
      }

      // Do some book-keeping
      id = objCritters[osid].id;
      if (id == PLAYER_OBJ)
         goto ai_loop_end;
      raycast_success = FALSE;

      dist = long_fast_pyth_dist(objs[id].loc.x - objs[PLAYER_OBJ].loc.x,
         objs[id].loc.y - objs[PLAYER_OBJ].loc.y);
      if (global_fullmap->cyber && (id == shodan_avatar_id))
      {
         run_evil_otto(id,dist);
         goto ai_loop_end;
      }
      if (!do_physics_stupidity(id,dist))
      {
         goto ai_loop_end;
      }

      objCritters[osid].wait_frames--;

      // Tell EDMS what to do with us.
#ifdef AI_EDMS
      apply_EDMS_controls(osid);
#endif

      // If it is our turn to get a bigger share of the 
      // computron pie, then let's crank.
      if (objCritters[osid].wait_frames <= 0)
      {
         do_stealth_stuff(id,visibility, &raycast_success, dist);
         mood = objCritters[osid].mood;
         if (((mood == AI_MOOD_HOSTILE) || (mood == AI_MOOD_ATTACKING)) && !(objCritters[osid].flags & AI_FLAG_CONFUSED))
         {
            check_attitude_adjustment(id,osid,dist,raycast_success);
            run_combat_ai(id,raycast_success);
            objCritters[osid].wait_frames = COMBAT_FRAMES;
         }
         else
         {
//            if (objCritters[osid].flags & AI_FLAG_CONFUSED)
//               Spew(DSRC_AI_Hacks, ("critter %x, is confused! flags = %x\n",id,objCritters[osid].flags));
            run_peaceful_ai(id,dist);
#ifdef USE_DIST_OVERRIDE_FOR_DEFAULT_FRAMES
            objCritters[osid].wait_frames = min(DEFAULT_FRAMES, dist >> 8);
//            if ((dist >> 8) < DEFAULT_FRAMES)
//               Spew(DSRC_AI_AI, ("using %d frames for id %x instead of %d\n",dist >> 8, id, DEFAULT_FRAMES));
#else
            objCritters[osid].wait_frames = DEFAULT_FRAMES;
#endif
         }
         // a bit o' random deviation...
         objCritters[osid].wait_frames += (*tmd_ticks & 0x2);
      }

ai_loop_end:
      osid = objCritters[osid].next;
   }
   if (priority_check)
      check_requests(TRUE);
   return(OK);
}
