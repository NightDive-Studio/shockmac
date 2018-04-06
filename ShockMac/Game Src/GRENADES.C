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
 * $Source: r:/prj/cit/src/RCS/grenades.c $
 * $Revision: 1.106 $
 * $Author: xemu $
 * $Date: 1994/11/25 18:18:45 $
 *
 */

#define __GRENADES_C

#include "grenades.h"
#include "effect.h"
#include "objwpn.h"
#include "objclass.h"
#include "objprop.h"
#include "gamestrn.h"
#include "cybrnd.h"
#include "damage.h"
#include "combat.h"
#include "colors.h"
#include "objsim.h"
#include "map.h"
#include "faketime.h"
#include "mainloop.h"
#include "gameobj.h"
#include "player.h"
#include "schedule.h"
#include "musicai.h"
#include "sfxlist.h"
#include "input.h"
#include "tools.h"
#include "otrip.h"
#include "screen.h"
#include "cybstrng.h"
#include "frprotox.h"
#include "frflags.h"
#include "physunit.h"
#include "trigger.h"    // for trap_sfx_func()

//----------------
//  Internal Prototypes
//----------------
void convert_grenade_to_explosion(ExplosionData *edata, int triple);
ubyte grenade_compute_damage(ObjID target, int wpn_triple, int power_level, ubyte *effect);
ObjID explosion_ray_cast_attack(ObjID gren_id, ObjID target, ObjLoc *gren_loc, fix radius, fix mass, fix speed);
void do_object_explosion(ObjID id);
bool activate_grenade_on_cursor(void);
void reactivate_mine(ObjID id);

ExplosionData game_explosions[GAME_EXPLS] =
{
//    RADIUS        RADIUS CHG      DMG  DCHG DTYPE           KNOCK_MASS       OFF  PENET
   {  fix_make(2,0), fix_make(1,0),  60,  50, EXPLOSION_FLAG, fix_make(250,0), 3,   50},
   {  fix_make(3,0), fix_make(1,0), 100,  80, EXPLOSION_FLAG, fix_make(300,0), 4,   65},
   {  fix_make(8,0), fix_make(3,0), 1000, 750,0x0103,         fix_make(1650,0),6,   35},
};


// grenade_counter is a unique identifier for grenades, because if one grenade blows up another grenade,
// the dead grenade should have no explosion. this is because the scheduler will check for that unique id, and if
// there is no grenade with the same ObjID AND unique id, then we have to believe that the grenade was
// destroyed.
static ubyte grenade_counter = 1;

// checks the first two to be equal = which means in_between
#define IN_BETWEEN_NOT_EQUAL(a,b,c) ((((a) <= (b)) && ((b) < (c))) || \
                            (((a) >= (b)) && ((b) > (c))))  

#define GRENADE_TIME_UNIT 10 // how many time-setting units in a second
#define GRENADE_BULLET_MASS fix_make(0,0x0100)
#define GRENADE_BULLET_SIZE fix_make(0,0x0500)

// --------------------------------------------------------
// convert_grenade_to_explosion()
//

void convert_grenade_to_explosion(ExplosionData *edata, int triple)
{
   int         ctrip = CPTRIP(triple);

   edata->radius         = fix_make(GrenadeProps[ctrip].radius,0);
   edata->radius_change  = fix_make(GrenadeProps[ctrip].radius_change, 0);
   edata->damage_mod     = GrenadeProps[ctrip].damage_modifier;
   edata->damage_change  = GrenadeProps[ctrip].damage_change;
   edata->dtype          = GrenadeProps[ctrip].damage_type;
   edata->knock_mass     = fix_make(GrenadeProps[ctrip].attack_mass,0) * 30;
   edata->offense        = GrenadeProps[ctrip].offense_value;
   edata->penet          = GrenadeProps[ctrip].penetration;
}

// ----------------------------------------------------------------------
// grenade_compute_damage()
//

ubyte grenade_compute_damage(ObjID target, int wpn_triple, int power_level, ubyte *effect)
{
   ubyte effect_class = (objs[target].obclass == CLASS_CRITTER) ? CritterProps[CPNUM(target)].hit_effect : NON_CRITTER_EFFECT;

   *effect = effect_matrix[effect_class][GREN_TYPE][0];

   return(compute_damage(target, GrenadeProps[CPTRIP(wpn_triple)].damage_type, GrenadeProps[CPTRIP(wpn_triple)].damage_modifier,
      GrenadeProps[CPTRIP(wpn_triple)].offense_value, GrenadeProps[CPTRIP(wpn_triple)].penetration, power_level, NULL, NULL, GREN_TYPE));
}

extern ObjID terrain_hit_exclusion;

// ----------------------------------------------------------------------
// explosion_ray_cast_attack()
//

ObjID explosion_ray_cast_attack(ObjID gren_id, ObjID target, ObjLoc *gren_loc, fix radius, fix mass, fix speed)
{
   Combat_Pt   source;
   Combat_Pt   old_source;
   Combat_Pt   dest;
   ObjID       affected_object = OBJ_NULL;
   Combat_Pt         vector;
   fix               dist;
   ubyte      floor_height;

   source.x = fix_from_obj_coord(gren_loc->x);
   source.y = fix_from_obj_coord(gren_loc->y);

   floor_height = obj_height_from_fix(fix_from_map_height
      (me_height_flr(MAP_GET_XY(OBJ_LOC_BIN_X(*gren_loc),OBJ_LOC_BIN_Y(*gren_loc)))));

   // move the ray cast up a bit if it's too close to ground
   source.z = fix_from_obj_height_val(gren_loc->z);
   if (floor_height >= (gren_loc->z-2))
      source.z += fix_from_obj_height_val(4);

   if (objs[target].info.ph != -1)
   {
      State             new_state;
      extern void get_phys_state(int ph, State *new_state, ObjID id);
      get_phys_state(objs[target].info.ph, &new_state,target);
      dest.x = new_state.X;   dest.y = new_state.Y;   dest.z = new_state.Z;
   }
   else
   {
      dest.x = fix_from_obj_coord(objs[target].loc.x);
      dest.y = fix_from_obj_coord(objs[target].loc.y);
      dest.z = fix_from_obj_height_val(objs[target].loc.z);

      // if it's a 3d model - move the target up by a quarter of its radius
      // since it's centered in the ground
      if (ObjProps[OPNUM(target)].render_type==1)
         dest.z += fix_make(ObjProps[OPNUM(target)].physics_xr,0)/(PHYSICS_RADIUS_UNIT<<1);
   }

   vector.x = dest.x - source.x;
   vector.y = dest.y - source.y;
   vector.z = dest.z - source.z;

   // normalize vector and convert it
   dist = fix_sqrt(fix_mul(vector.x, vector.x) + fix_mul(vector.y, vector.y) + fix_mul(vector.z, vector.z));

   vector.x = fix_div(vector.x, dist);
   vector.y = fix_div(vector.y, dist);
   vector.z = fix_div(vector.z, dist);

   old_source = source;
   terrain_hit_exclusion = gren_id;
   affected_object = ray_cast_vector(gren_id, &source, vector,
      mass, RAYCAST_ATTACK_SIZE, speed, radius);

   // this worries me - art
   dest.x -= (vector.x/6);
   dest.y -= (vector.y/6);
   dest.z -= (vector.z/6);

   // did we hit air (meaning nothing)
   // and because we've checked distance already - we must have hit the object
   // <INSERT BUG HERE>
   if ((source.x < 0) || (source.y < 0) || (source.z < 0))
   {
      affected_object=target;
   }
   // or did we hit after the object????
   else if (!IN_BETWEEN_NOT_EQUAL(old_source.x, source.x, dest.x) &&
            !IN_BETWEEN_NOT_EQUAL(old_source.y, source.y, dest.y) &&
            !IN_BETWEEN_NOT_EQUAL(old_source.z, source.z, dest.z))
   {
      affected_object=target;
   }

   return(affected_object);
}

#define EMP_VHOLD_AMT   380

// ----------------------------------------------------------------------
// do_explosion()
//

void do_explosion(ObjLoc loc, ObjID exclusion, ubyte special_effect, ExplosionData *edata)
{
   int   x = OBJ_LOC_BIN_X(loc);
   int   y = OBJ_LOC_BIN_Y(loc);                                            
   int   cx, cy;
   int   cradius = fix_int(edata->radius);
   fix      deltax, deltay,deltaz;
   fix      radius_squared = fix_mul(edata->radius, edata->radius);
   // we're dividing by two - to account later for lack of precision
   fix      max_damage = fix_make((edata->damage_mod>>2), 0);
   ObjRefID current_ref;
   ObjID    current_id;
   bool     no_effect;
   ubyte    affect;
   int      damage;
   extern ObjID damage_sound_id;
   extern char damage_sound_fx;

   if (special_effect)
   {
      fix   dist2player;
      deltax = fix_abs(OBJ_LOC_VAL_TO_FIX(objs[PLAYER_OBJ].loc.x-loc.x));
      deltay = fix_abs(OBJ_LOC_VAL_TO_FIX(objs[PLAYER_OBJ].loc.y-loc.y));
      deltaz = fix_from_obj_height_val(objs[PLAYER_OBJ].loc.z) - fix_from_obj_height_val(loc.z);
      dist2player = fix_fast_pyth_dist(fix_fast_pyth_dist(deltax,deltay),deltaz);

      do_special_effect_location(exclusion, special_effect, 0xFF, &loc, 0);

      if (dist2player < fix_mul(edata->radius_change,edata->radius_change))
      {
         if (ID2TRIP(exclusion)==EMP_G_TRIPLE)
            trap_sfx_func(0,0,4,EMP_VHOLD_AMT);
         else
         {
            fr_global_mod_flag(FR_SOLIDFR_SLDCLR, FR_SOLIDFR_MASK);
            fr_solidfr_color=GRENADE_COLOR;
         }
      }
   }

   for (cx=x-cradius; cx < (x+cradius); cx++)
   {
      // check dimensions of x
      if (cx < 0) continue;
      else if (cx >= MAP_XSIZE) break;

      for (cy = y-cradius; cy <(y+cradius); cy++)
      {
         // check dimensions of y
         if (cy < 0)  continue;
         else if (cy >= MAP_XSIZE) break;

         // get the objrefs for this square
         current_ref = MAP_GET_XY(cx, cy)->objRef;
         while (current_ref != OBJ_REF_NULL)
         {
            no_effect = FALSE;
            affect = 0;
            current_id = objRefs[current_ref].obj;

            if (current_id == OBJ_NULL)
            {
               break;
            }

            // make sure we're not going to affect ourselves
            // also, make sure that the ref points to the object in the same square
            // (since objects can have more than one objRef)

            if (current_id == exclusion)
            {
               current_ref = objRefs[current_ref].next;
               continue;
            }
            else if ((OBJ_LOC_BIN_X(objs[objRefs[current_ref].obj].loc) != objRefs[current_ref].state.bin.sq.x) ||
                     (OBJ_LOC_BIN_Y(objs[objRefs[current_ref].obj].loc) != objRefs[current_ref].state.bin.sq.y))
            {
               no_effect = TRUE;
            }
            else
            {
               // is object we're trying to affect already destroyed???
               no_effect = is_obj_destroyed(current_id);

               // make sure that the grenade can affect the object before doing lots of
               // heinous computations - also remember the affect value
               if (!no_effect)
                  affect = object_affect(current_id, edata->dtype);
            }

            if (exclusion != OBJ_NULL)
            {
               // have a random chance of not chaining explosions
               if ((objs[exclusion].obclass != CLASS_GRENADE) && (objs[current_id].obclass != CLASS_CRITTER))
               {
                  if (!RndRange(&grenade_rnd, 0, 1))
                     no_effect = TRUE;
               }
            }

            // only do damage, if no_effect is FALSE, and affect is non-zero
            if (!no_effect && affect)
            {
               ObjID    affected_object;
               ObjLoc   hit_loc = loc;
               fix      dist_squared;

               // get the distance of object to explosion
               deltax = fix_abs(OBJ_LOC_VAL_TO_FIX(objs[current_id].loc.x - loc.x));
               deltay = fix_abs(OBJ_LOC_VAL_TO_FIX(objs[current_id].loc.y - loc.y));
               deltaz = fix_from_obj_height_val(objs[current_id].loc.z) - fix_from_obj_height_val(loc.z);

               // get the distance squared and check with radius squared. (saves us a square 
               // root if object is not within radius).
               dist_squared = fix_mul(deltax, deltax) + fix_mul(deltay, deltay) + fix_mul(deltaz,deltaz);

               if (dist_squared < radius_squared)
               {
                  // let's do the raycast to see if we can hit the object
                  affected_object = explosion_ray_cast_attack(exclusion, current_id, &hit_loc,
                     edata->radius, edata->knock_mass, NO_RAYCAST_KICKBACK_SPEED);

                  // let's check the object for valid attack
                  if ((affected_object != OBJ_NULL) && (affected_object != PLAYER_OBJ))
                  {
                     // saves time in dealing with damage!
                     if (!objs[affected_object].info.current_hp)
                     {
                        affected_object = NULL;
                     }
                  }

                  if (affected_object == current_id)
                  {
                     fix   ratio;
                     fix   obj_dist;
                     fix   damage_fix;
                     int   percent_damage;
                     ubyte effect;
                     ubyte effect_class;

                     // divide by 2 to take into account - fix's lack of precision
                     int   dmg = edata->damage_mod >> 2;
                     int   dmgc = edata->damage_change >> 2;

                     // let's get the object's distance from explosion!
                     obj_dist = fix_fast_pyth_dist(fix_fast_pyth_dist(deltax,deltay),deltaz);

                     // are we within inner radius???
                     if (obj_dist < edata->radius_change)
                     {
                        ratio = fix_div(obj_dist, edata->radius_change);
                        damage_fix = max_damage - (ratio * (dmg-dmgc));
                     }
                     else
                     {
                        ratio = fix_div((obj_dist-edata->radius_change), (edata->radius - edata->radius_change));
                        damage_fix = (fix_make(1,0) - ratio) * dmgc;
                     }
                     percent_damage = fix_int(fix_div(damage_fix, max_damage)*100);

                     effect_class = (objs[current_id].obclass == CLASS_CRITTER) ? CritterProps[CPNUM(current_id)].hit_effect : NON_CRITTER_EFFECT;
                     effect = effect_matrix[effect_class][GREN_TYPE][0];

                     // okay let's figure out the damage done by the grenade 
                     // let's check if we're dealing with
                     // a. combat difficulty - 0
                     // b. not the player
                     // c. toughness not equal to 3
                     // if we meet all three of these conditions, damage the object fully

                     if (!player_struct.difficulty[COMBAT_DIFF_INDEX] && (current_id != PLAYER_OBJ))
                        damage = (ObjProps[OPNUM(current_id)].toughness!=3) ? objs[current_id].info.current_hp : 0;
                     else
                        damage = compute_damage(current_id, edata->dtype, edata->damage_mod, edata->offense, edata->penet, percent_damage, NULL, NULL, GREN_TYPE);

                     if (damage>0)
                     {
                        bool explosion_affected;
                        ubyte flags;

                        explosion_affected = FALSE;
                        if (objs[current_id].obclass == CLASS_GRENADE)
                        {
                           ubyte    chaining;
                           int      targ_triple;
                        
                           // check if this grenade will be affected by blast 
                           targ_triple = MAKETRIP(objs[current_id].obclass, objs[current_id].subclass, objs[current_id].info.type);
                        
                           chaining = GrenadeProps[CPTRIP(targ_triple)].touchiness + (percent_damage/15) +
                                       RndRange(&grenade_rnd, 0, 6) - 3;

                           if (chaining > 7)
                           {
                              explosion_affected = TRUE;
                           }
                        }

                        flags = 0;

                        // if the grenade went off in the player's hand, the shield does nothing
                        // and you take double damage cause you are a bobo.
                        if (current_id == PLAYER_OBJ)
                        {
                           if (ID2TRIP(exclusion)==GAS_G_TRIPLE) {
                              damage = (damage*2)/(2+player_struct.hardwarez[CPTRIP(ENV_HARD_TRIPLE)]);
                           }
                           if ((loc.x == objs[current_id].loc.x) && (loc.y==objs[current_id].loc.y))
                           {
                              message_info(get_temp_string(REF_STR_HoldingGrenade)); //"Holding a live grenade = Double damage\n");

                              // shield does no damage
                              flags = NO_SHIELD_ABSORBTION;

                              // do double damage
                              damage <<= 1;
                           }
                        }


                        // need to check that the target is not a grenade that we set off because
                        // of a chain explosion
                        if (explosion_affected)
                        {
                           objGrenades[objs[current_id].specID].unique_id = 0;   // get rid of timer - if grenade was timer
                           ADD_DESTROYED_OBJECT(current_id);
                        }
                        else
                        {
                           bool do_effect = FALSE;
                           ubyte destroy = ObjProps[OPNUM(current_id)].destroy_effect;
                           if ((objs[current_id].info.current_hp <= damage) && DESTROY_OBJ_EFFECT(destroy))
                           {
                              objs[current_id].info.current_hp=0;
                              do_effect = TRUE;
                           }
                           else if(damage_object(current_id, damage, edata->dtype,flags))
                              do_effect = (EFFECT_VAL(destroy) != 0);

                           if (do_effect)
                           {
                              ObjLoc      loc = objs[current_id].loc;
                              fix   deltax = OBJ_LOC_VAL_TO_FIX(objs[PLAYER_OBJ].loc.x-loc.x);
                              fix   deltay = OBJ_LOC_VAL_TO_FIX(objs[PLAYER_OBJ].loc.y-loc.y);
                              fix   dist = fix_fast_pyth_dist(deltax, deltay)<<2;

                              if (ObjProps[OPNUM(current_id)].physics_model != 2)
                                 loc.z += obj_height_from_fix(fix_make(ObjProps[OPNUM(current_id)].physics_xr,0) / PHYSICS_RADIUS_UNIT);

                              // move explosion towards player
                              loc.x += obj_coord_from_fix(fix_div(deltax, dist));
                              loc.y += obj_coord_from_fix(fix_div(deltay, dist));

                              do_special_effect_location(current_id, destroy, 0xFF, &loc, 0);
                           }
                        }
                     }
                  }
               }
            }

            current_ref = objRefs[current_ref].next;
         }
      }
   }
   if (damage_sound_fx != -1)
   {
      play_digi_fx_obj(damage_sound_fx,1,damage_sound_id);
   }
   damage_sound_fx = -1;
}

// -----------------------------------------------------------------
// do_grenade_explosion()
//
// how accurate should we make the explosion location??
// IMPORTANT: YOU MUST CALL destroy_destroyed_objects OUTSIDE OF PROCEDURE - NOT IN ANY OBJ LOOPS
// after calling do_explosion
//

void do_grenade_explosion(ObjID id, bool special_effect)
{
   ObjID          grenade_location_id;
   ObjLoc         gren_loc;
   ExplosionData  edata;
   int            triple;
   bool           in_hand = (id == object_on_cursor);
   ubyte          effect = (special_effect) ? (ObjProps[OPNUM(id)].destroy_effect&0x7F) : 0;

   // let's get the triple
   triple = MAKETRIP(objs[id].obclass, objs[id].subclass, objs[id].info.type);

   // secondly, make sure that we were given a grenade
   if (objs[id].obclass != CLASS_GRENADE)
   {
      return;
   }

   // is the grenade already a dud????
   if (objGrenades[objs[id].specID].flags & GREN_DUD_FLAG)
      return;

   // since we're activating it - set it to be a dud - we'll decide if it explodes later
   objGrenades[objs[id].specID].flags |= (GREN_DUD_FLAG);

   grenade_location_id = (in_hand) ? PLAYER_OBJ : id;
   // get rid of grenade cursor bitmap
   if (in_hand)
      pop_cursor_object();

   gren_loc = objs[grenade_location_id].loc;

   convert_grenade_to_explosion(&edata, triple);

   // Special earthshaker hack
   if (ID2TRIP(id) == EARTH_G_TRIPLE)
   {
      extern short fr_sfx_time;
      fr_global_mod_flag(FR_SFX_SHAKE, FR_SFX_MASK);
      fr_sfx_time = CIT_CYCLE << 1;
   }

   // Play sound effect
   play_digi_fx_obj(SFX_EXPLOSION_1,1,grenade_location_id);

   // actually destory the grenade (may want to go away if we want delayed grenade explosions) - minman
   // do the actual explosion
   do_explosion(gren_loc, id, effect, &edata);

   if (ID2TRIP(id) == EARTH_G_TRIPLE)
      play_digi_fx(SFX_RUMBLE,1);
}

// --------------------------------------------------------
// do_object_explosion()
//

void do_object_explosion(ObjID id)
{
   ObjLoc      loc = objs[id].loc;
   ExplosionData  edata;

   convert_grenade_to_explosion(&edata, OBJ_G_TRIPLE);

   // Check to see whether or not there is cool special stuff to do when this thing
   // gets destroyed.  obj_combat_destroy returns whether or not to go ahead and
   // continue the destruction process
   if (obj_combat_destroy(id))
   {
      ADD_DESTROYED_OBJECT(id);
   }

   // do some explosion here
   do_explosion(loc, id, 0, &edata); 
}

// -------------------------------------------------
// get_grenade_name()
//

#define NAMEBUFSZ 50

char* get_grenade_name(int gtype, char* buf)
{
   int triple= nth_after_triple(MAKETRIP(CLASS_GRENADE,0,0),gtype);

   get_object_short_name(triple,buf,NAMEBUFSZ);

   return buf;
}

// --------------------------------------------------------
// activate_grenade()
//

void activate_grenade(ObjSpecID osid)
{
   ObjID             id;
   int               triple;
   int               type;
   ubyte             deviation;
   int               tdev;
   int               time;
   ubyte             n;
   GrenSchedEvent    new_event;

   id = objGrenades[osid].id;

   if (id != OBJ_NULL)
   {
      type = objs[id].info.type;
      triple = MAKETRIP(objs[id].obclass, objs[id].subclass, type);
      objGrenades[osid].flags = GREN_ACTIVE_FLAG;
      play_digi_fx(SFX_GRENADE_ARM,1);

      if (GrenadeProps[CPNUM(id)].flags & GREN_TIMING_TYPE)
      {
         // activate the grenade

         // add deviation for the grenade

         deviation = TimedGrenadeProps[SCTRIP(triple)].timing_deviation;
         tdev = RndRange(&grenade_rnd, 0, (deviation*2));
         n = get_nth_from_triple(triple);
         time = (player_struct.grenades_time_setting[n] + tdev - deviation);

         // schedule the grenade explosion
         new_event.timestamp = TICKS2TSTAMP(player_struct.game_time + (CIT_CYCLE*time)/GRENADE_TIME_UNIT);
         new_event.type = GRENADE_SCHED_EVENT;
         new_event.gren_id = id;
         new_event.unique_id = grenade_counter;
         {
            errtype err = schedule_event(&global_fullmap->sched[MAP_SCHEDULE_GAMETIME],(SchedEvent *) &new_event);
         }
         objGrenades[osid].timestamp = new_event.timestamp;
      }
      objGrenades[osid].unique_id = grenade_counter;

      // let's increment the grenade counter
      grenade_counter = (grenade_counter == UNIQUE_LIMIT) ? 1 : grenade_counter+1;
   }
}

bool activate_grenade_on_cursor(void)
{
   ObjID oc=object_on_cursor;

   if(oc==NULL || objs[oc].obclass!=CLASS_GRENADE)
      return FALSE;

   if(objGrenades[objs[oc].specID].flags & GREN_ACTIVE_FLAG)
      return TRUE;

   // push and pop object cursor to update its live-ness display
   pop_cursor_object();
   activate_grenade(objs[oc].specID);
   push_cursor_object(oc);
   return TRUE;
}

#define PHYSICS_WAIT 3

// --------------------------------------------------------------------------
// reactivate_mine()
//
// this is so the grenade will contact with the player after the grenade has
// left the player's body.

void reactivate_mine(ObjID id)
{
   ObjSpecID         osid = objs[id].specID;
   GrenSchedEvent    new_event;

   if (id != OBJ_NULL)
   {
      // schedule the grenade reactivation
      new_event.timestamp = TICKS2TSTAMP(player_struct.game_time + (CIT_CYCLE*PHYSICS_WAIT)/GRENADE_TIME_UNIT);
      new_event.type = GRENADE_SCHED_EVENT;
      new_event.gren_id = id;
      new_event.unique_id = grenade_counter;
      {
         errtype err = schedule_event(&global_fullmap->sched[MAP_SCHEDULE_GAMETIME],(SchedEvent *) &new_event);
      }
      objGrenades[osid].timestamp = new_event.timestamp;
      objGrenades[osid].unique_id = grenade_counter;
      grenade_counter = (grenade_counter == UNIQUE_LIMIT) ? 1 : grenade_counter+1;
   }
}

// ------------------------------------------------------
// grenade_contact()
//
// called when grenade contact something

void grenade_contact(ObjID id, int)
{
   if (is_obj_destroyed(id))
   {
      return;
   }

    if ((GrenadeProps[CPNUM(id)].flags & GREN_CONTACT_TYPE) &&
       (objGrenades[objs[id].specID].flags & GREN_ACTIVE_FLAG))
   {
      ADD_DESTROYED_OBJECT(id);
   }
   else if ((GrenadeProps[CPNUM(id)].flags & GREN_MINE_TYPE) &&
       (objGrenades[objs[id].specID].flags & GREN_ACTIVE_FLAG))
   {
      if (!(objGrenades[objs[id].specID].flags & GREN_MINE_STILL))
      {
         // loook - don't explode me - i ain't still yet
         return;
      }   
      ADD_DESTROYED_OBJECT(id);
   }
}

