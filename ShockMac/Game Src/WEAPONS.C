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
 * $Source: r:/prj/cit/src/RCS/weapons.c $
 * $Revision: 1.186 $
 * $Author: xemu $
 * $Date: 1994/11/23 22:50:30 $
 *
 */

#define __WEAPONS_SRC

#include <string.h>

#include "weapons.h"
#include "damage.h"
#include "objclass.h"
#include "objprop.h"
#include "objwpn.h"
#include "player.h"
#include "gameloop.h"
#include "loops.h"
#include "handart.h"
#include "hand.h"
#include "objsim.h"
#include "objclass.h"
#include "gamestrn.h"
#include "newmfd.h"
#include "musicai.h"
#include "combat.h"
#include "sfxlist.h"
#include "tools.h"
#include "effect.h"
#include "status.h"
#include "schedule.h"
#include "wares.h"
#include "render.h"
#include "otrip.h"
#include "mainloop.h"
#include "screen.h"
#include "fullscrn.h"
#include "physics.h"
#include "physunit.h"
#include "cybrnd.h"
#include "softdef.h"
#include "colors.h"
#include "hud.h"
#include "cybstrng.h"
#include "frtypes.h"
#include "doorparm.h"
#include "gr2ss.h"
#ifdef STEREO_SUPPORT
#include <inp6d.h>
#include <i6dvideo.h>
#endif

#define MIN_ENERGY_WPN_THRESHOLD 20
#define FATIGUE_ACCURACY_RATIO   400

extern Boolean	DoubleSize;

// char ammo_type_letters[] = "stnths mphssb  "; 

bool muzzle_fire_light;
short mouse_attack_x = -1;
short mouse_attack_y = -1;

extern void weapon_mfd_for_reload(void);

//----------------
//  Internal Prototypes
//----------------
bool does_weapon_overload(int type, int subtype);
void weapon_properties(int triple, ubyte *damage_modifier, ubyte *offense);
ObjID do_effect_fix(ObjID owner, ubyte effect, ubyte start, Combat_Pt effect_point, short location);
ObjID do_wall_hit(Combat_Pt *hit_point, Combat_Pt vector, int triple, short mouse_x, short mouse_y, bool do_effect);
bool player_fire_handtohand(LGPoint *pos, ubyte slot, ObjID *what_hit,int gun_triple);
bool decrease_ammo(ubyte slot, int shots);
bool player_fire_projectile(LGPoint *pos, LGRegion *r, ubyte slot, int gun_triple);
bool weapon_energy_drain(weapon_slot *ws, ubyte charge, ubyte max_charge);
bool player_fire_energy(LGPoint *pos, ubyte slot, int gun_triple);
bool player_fire_slow_projectile_weapon(LGPoint *pos, ubyte slot, int gun_triple);
bool player_fire_energy_proj(LGPoint *pos, ubyte slot, int gun_triple);
bool fire_player_software(LGPoint *pos, LGRegion *r, bool pull);
void unload_current_weapon(void);
void check_temperature(weapon_slot *ws, bool clear);
bool reload_current_weapon(void);
bool reload_weapon_hotkey(short keycode, ulong context, void* data);
bool ready_to_draw_handart(void);


// -------------------------------------------------
// does_weapon_overload()
//
bool does_weapon_overload(int type, int )
{
   switch(type)
   {
      case (GUN_SUBCLASS_BEAM):
         return(TRUE);
         break;
      case (GUN_SUBCLASS_PISTOL):
      case (GUN_SUBCLASS_AUTO):
      case (GUN_SUBCLASS_SPECIAL):
      case (GUN_SUBCLASS_HANDTOHAND):
      case (GUN_SUBCLASS_BEAMPROJ):
      default:
         return(FALSE);
         break;
   }
}

// -----------------------------------------------------------------
// get_weapon_name()
//

char* get_weapon_name(int type, int subtype, char* buf)
{
   char  name[50];

   if ((type >= MAX_WEAPON_TYPE) || (subtype >= MAX_WEAPON_SUBTYPE))
   {
      return(NULL);
   }
   
   get_object_short_name(MAKETRIP(CLASS_GUN, type, subtype), name, 50);
   strcpy(buf, name);
   return buf;
}

// --------------------------------------------------------------
// get_weapon_long_name()
//

char* get_weapon_long_name(int type, int subtype, char *buf)
{
   char  name[50];

   if ((type >= MAX_WEAPON_TYPE) || (subtype >= MAX_WEAPON_SUBTYPE))
   {
      return(NULL);
   }
   
   get_object_long_name(MAKETRIP(CLASS_GUN, type, subtype), name, 50);
   strcpy(buf, name);

   return buf;
}

// ---------------------------------------------------------------------
// weapon_properties()
//

void weapon_properties(int triple, ubyte *damage_modifier, ubyte *offense)
{
   int   wpn_class = TRIP2CL(triple);

   switch (wpn_class)
   {
      case (CLASS_GUN):    // Beam weapon is the only gun with damage type
         *damage_modifier = BeamGunProps[SCTRIP(triple)].damage_modifier;
         *offense = BeamGunProps[SCTRIP(triple)].offense_value;
         break;
      case (CLASS_PHYSICS):
         *damage_modifier = AmmoProps[CPTRIP(triple)].damage_modifier;
         *offense = AmmoProps[CPTRIP(triple)].offense_value;
         break;
      case (CLASS_GRENADE):
         *damage_modifier = GrenadeProps[CPTRIP(triple)].damage_modifier;
         *offense = GrenadeProps[CPTRIP(triple)].offense_value;
         break;
   }
}

// -------------------------------------------------------------------------
// set_beam_weapon_max_charge()
//

void set_beam_weapon_max_charge(ubyte index, ubyte max_charge)
{
   if (max_charge < MIN_ENERGY_USE)
      max_charge=MIN_ENERGY_USE;
   player_struct.weapons[index].setting = max_charge;
   mfd_notify_func(MFD_WEAPON_FUNC,MFD_WEAPON_SLOT,FALSE,MFD_ACTIVE,TRUE);

   return;
}

// -------------------------------------------------------
// do_effect_fix()
//

ObjID do_effect_fix(ObjID owner, ubyte effect, ubyte start, Combat_Pt effect_point, short location)
{
   ObjLoc      loc;

   loc.x = obj_coord_from_fix(effect_point.x);
   loc.y = obj_coord_from_fix(effect_point.y);
   loc.z = obj_height_from_fix(effect_point.z);
   return(do_special_effect_location(owner, effect, start, &loc, location));
return OBJ_NULL;
}

// ---------------------------------------------------------------
//
// do_wall_hit()
//

ObjID do_wall_hit(Combat_Pt *hit_point, Combat_Pt, int triple, short mouse_x, short mouse_y, bool do_effect)
{
   ubyte effect = 0;
   ObjID id = OBJ_NULL;
   extern ushort fr_get_at_raw(frc *fr, int x, int y, bool again, bool transp);
   ObjID efft;

   // you know this should all be changed, so change it when
   // fr_get_at_raw_real is done
   id = fr_get_at_raw(_current_fr_context, mouse_x, mouse_y, FALSE, FALSE);
   if (id < 0)
      id = OBJ_NULL;
   else
   {
      ObjID obj_trans;

      obj_trans=fr_get_at_raw(_current_fr_context, mouse_x, mouse_y, FALSE, TRUE);

      if(objs[id].obclass==CLASS_DOOR)
      {
         if(!DOOR_REALLY_CLOSED(id))
            id=obj_trans;
      }
      else
         id=obj_trans;

      if(id<0)
         id=OBJ_NULL;
//      else if (ObjProps[OPNUM(target)].render_type!=1) // don't do this if the object isn't a 3d model
//      else if(objs[id].class==CLASS_CRITTER) // let edms raycast deal with hitting critters
      else if (objs[id].info.ph >= 0) // physics object can be dealt with the edms raycast system
         id=OBJ_NULL;
   }
   if (id == OBJ_NULL)
   {
      switch (TRIP2CL(triple))
      {
         case (CLASS_GUN):    // Beam weapon is the only gun with damage type
            effect = (TRIP2SC(triple) == GUN_SUBCLASS_HANDTOHAND) ? 0 : BEAM_HIT_WALL;
            break;
         case (CLASS_AMMO):
            effect = BULL_HIT_WALL;
            break;
         case (CLASS_GRENADE):
            effect = 0;
            break;
      }
      // make sure that we should do an effect, and the hit point is in the world
      if (do_effect && effect && (hit_point->x > 0) && (hit_point->y > 0) && (hit_point->z > 0))
      {
         efft = do_effect_fix(OBJ_NULL, effect, 0xFF, *hit_point, 0);

         // set the special beam weapon effect voodoo
         if (efft && (TRIP2CL(triple) == CLASS_GUN))
         {
            if (TRIP2SC(triple) == GUN_SUBCLASS_BEAM)
            {
               extern ObjID beam_effect_id;
               extern void hudobj_set_id(short id, bool val);

               beam_effect_id = efft;
               hudobj_set_id(beam_effect_id, TRUE);
            }
         }
      }
   }

   return(id);
}


#define NO_KNOCKBACK_MASS  fix_make(0,0x2000)
#define NO_KNOCKBACK_SPEED fix_make(1,0)

#define HAND2HANDX   (SCREEN_VIEW_X+(SCREEN_VIEW_WIDTH/2))
#define HAND2HANDY   (SCREEN_VIEW_Y+(SCREEN_VIEW_HEIGHT/2))
#define FULLHAND2HANDY   (FULL_VIEW_Y+((FULL_VIEW_HEIGHT*2)/3))
//#define HAND2HANDY2  (SCREEN_VIEW_Y+SCREEN_VIEW_HEIGHT-3)

#define HITLOCS   3

#define HAND_OFFSET   (fix_make(0, 0x4000))
#define HAND_Z_OFFSET   (fix_make(0, 0x3000))

#define HAND_X_DELTA    15

//Point hand_locations[HITLOCS] = {{HAND2HANDX-25,HAND2HANDY2},{HAND2HANDX+25,HAND2HANDY2},
//                                 {HAND2HANDX-25, HAND2HANDY}, {HAND2HANDX+25, HAND2HANDY}};

// ---------------------------------------------------------------------------
// player_fire_handtohand()
//

extern void get_phys_state(int ph, State *new_state, ObjID id);

bool player_fire_handtohand(LGPoint *, ubyte, ObjID *what_hit,int gun_triple)
{
   fix            attack_mass;
   fix            range;

   State          new_state;        // used to get physics state

   Combat_Pt      view_pt;          // used to calculate where to shot from
   Combat_Pt      body_pt;

   Combat_Pt      vector;
   Combat_Pt      origin;
   Combat_Pt      save_origin;
   Combat_Pt      hit_point;

   ObjID          target;
   LGPoint          hand_pos;
   extern ubyte   handart_count;
   byte           i;
   bool           dead;
   bool           hit_wall=FALSE;

   *what_hit = OBJ_NULL;

   // let's get the range of the weapon
   range = fix_make(HandtohandGunProps[SCTRIP(gun_triple)].attack_range,0);

   // get the viewpoint of the player
//   EDMS_get_pelvic_viewpoint(objs[PLAYER_OBJ].info.ph, &new_state);
   get_phys_state(objs[PLAYER_OBJ].info.ph, &new_state,PLAYER_OBJ);
   view_pt.x = new_state.X;   view_pt.y = new_state.Y;   view_pt.z = new_state.Z;

   // get the body point of the player
   EDMS_get_state(objs[PLAYER_OBJ].info.ph, &new_state);
   body_pt.x = new_state.X;   body_pt.y = new_state.Y;   body_pt.z = new_state.Z;

   // find the actual place to fire from
   origin.x = (view_pt.x+body_pt.x)/2;
   origin.y = (view_pt.y+body_pt.y)/2;
   origin.z = (view_pt.z+body_pt.z)/2;
   save_origin = origin;

   // let's iterate through the three attempts we're going to do
   for (i=(HITLOCS-1); i >= 0 ; i--)
   {
      // restore the origin of the attack - this is because 
      // raycast changes the origin
      origin = save_origin;

      // find the firing vector
      if (i)
         hand_pos.x = (i%2) ? (HAND2HANDX+HAND_X_DELTA) : (HAND2HANDX-HAND_X_DELTA);
      else
         hand_pos.x = HAND2HANDX;

      hand_pos.y = (full_game_3d) ? FULLHAND2HANDY : HAND2HANDY;
      if (DoubleSize)
         hand_pos.y = SCONV_Y(hand_pos.y) >> 1;
      else
         ss_point_convert(&(hand_pos.x),&(hand_pos.y),FALSE);
      find_fire_vector(&hand_pos, &vector);

      // shift the origin point for the last attack which is right 
      // in front of you
      if (!i)
         origin.z = view_pt.z;

      // do the actual raycast
      target = ray_cast_vector(PLAYER_OBJ, &origin, vector,
         NO_KNOCKBACK_MASS, RAYCAST_ATTACK_SIZE,
         NO_KNOCKBACK_SPEED, range);

      // check if we hit something on the wall
      if (target == OBJ_NULL)
      {
         target = do_wall_hit(&origin, vector, gun_triple, hand_pos.x, hand_pos.y, (i==0));

         // check the distance!
         if (target != OBJ_NULL)
         {
            fix   dist;
            fix   temp;
            temp = fix_from_obj_coord(objs[target].loc.x) - body_pt.x;
            dist = fix_mul(temp, temp);
            temp = fix_from_obj_coord(objs[target].loc.y) - body_pt.y;
            dist += fix_mul(temp, temp);
            temp = fix_from_obj_height(target) - body_pt.z;
            dist += fix_mul(temp, temp);
            if (dist > fix_mul(range,range))
               target = OBJ_NULL;
         }
      }

      // if we've either hit the wall or we've hit an object - remember we've hit something here
      if (((origin.x > 0) && (origin.y > 0)) || (target != OBJ_NULL))
      {
         hit_point = origin;

         if (target != OBJ_NULL)
            break;                  // exit out of loop!
         else
            hit_wall = TRUE;
      }
   }

   // so we've hit an object - let's do it damage
   if (target != OBJ_NULL)
   {
      ubyte    val = HandtohandGunProps[SCTRIP(gun_triple)].energy_use;
      int      use;
   
      if (val && !player_struct.energy)
      {
         use = 0;
         message_info(get_temp_string(REF_STR_NoEnergyWeapon));
      }
      else
         use = (val)?((drain_energy(val)*100L)/val):100;

      player_struct.num_hits++;
      dead=player_attack_object(target, gun_triple, use, origin);
      *what_hit=target;
   }

   // okay - if we've hit an object - let's do a raycast, do simulate the effect of hitting an object
   if (hit_wall || (target != OBJ_NULL))
   {
      // use the original origin
      origin=save_origin;

      // let's simulate now!
      attack_mass = fix_make(HandtohandGunProps[SCTRIP(gun_triple)].attack_mass, 0) * 20;
      ray_cast_vector(PLAYER_OBJ, &origin, vector,
         attack_mass, RAYCAST_ATTACK_SIZE,
         fix_make(HandtohandGunProps[SCTRIP(gun_triple)].attack_speed, 0),
         fix_make(HandtohandGunProps[SCTRIP(gun_triple)].attack_range, 0));
 
      // see - if we hit an object - the hitting of the object will do an effect
      // so if we hit a wall - let's do the DARN sound effect
      if (target == OBJ_NULL)
      {
         do_effect_fix(PLAYER_OBJ, IMPACT, 0xFF, hit_point, 0);

         switch (gun_triple)
         {
            case BATON_TRIPLE:
            	play_digi_fx(SFX_GUN_PIPE_HIT_METAL,1);
            	break;
            case LASERAPIER_TRIPLE:
            	play_digi_fx(SFX_GUN_LASEREPEE_HIT,1);
            	break;
         }
      }
   }
   else
   {
      switch (gun_triple)
      {
         case BATON_TRIPLE:
     		play_digi_fx(SFX_GUN_PIPE_MISS,1);
        	 	break;
         case LASERAPIER_TRIPLE:
         		play_digi_fx(SFX_GUN_LASEREPEE_MISS,1);
         		break;
      }
   }

   handart_count &= (~0x80);
   return(TRUE);
}

// -----------------------------------------------------------------
// decrease_ammo()
//
// returns FALSE if we don't have as many shots as we want to remove.
// else removes them and returns TRUE.

bool decrease_ammo(ubyte slot, int shots)
{
   if(player_struct.weapons[slot].ammo == 0) {
      weapon_mfd_for_reload();
      return FALSE;
   }
   
   player_struct.weapons[slot].ammo -= shots;

   if(player_struct.weapons[slot].ammo == 0)
      weapon_mfd_for_reload();

   return TRUE;
}

// ---------------------------------------------------------------------------
// player_fire_projectile
//
// decrements the current_magazine - returns false iff there is no more
// ammo - (make click noise, if this is true???)
// we need to set something in the "bullet" so that we know the "owner"
// so we can keep track of hits by the player.

//#define BRIGHT_LIGHT_FLASH 50

bool player_fire_projectile(LGPoint *pos, LGRegion *r, ubyte slot, int gun_triple)
{
   int            ammo_triple;
   int            ammo_subclass;
   fix            bullet_mass;
   Combat_Pt      vector;
   Combat_Pt      origin;
   ObjID          target;
   State          new_state;

   ammo_subclass = AMMOTYPE_SUBCLASS(GunProps[CPTRIP(gun_triple)].useable_ammo_type);
   ammo_triple = MAKETRIP(CLASS_AMMO,ammo_subclass,player_struct.weapons[slot].ammo_type);

   // decrement the ammo count - unless we're out of ammo
   if(!decrease_ammo(slot,1))
      return FALSE;

   // If we finished a cartridge, throw it away
//   if (player_struct.weapons[slot].ammo == 0)
//      player_struct.weapons[slot].ammo_type = EMPTY_WEAPON_SLOT;

   get_phys_state(objs[PLAYER_OBJ].info.ph, &new_state,PLAYER_OBJ);
   origin.x = new_state.X;   origin.y = new_state.Y;   origin.z = new_state.Z;

   find_fire_vector(pos, &vector);
   bullet_mass = fix_make(AmmoProps[CPTRIP(ammo_triple)].bullet_mass, 0) * 30;

   target = ray_cast_vector(PLAYER_OBJ, &origin, vector,
      bullet_mass, RAYCAST_ATTACK_SIZE,
      fix_make(AmmoProps[CPTRIP(ammo_triple)].bullet_speed, 0),
      fix_make(AmmoProps[CPTRIP(ammo_triple)].range, 0));
 
   // check if we hit something on the wall
   if (target == OBJ_NULL)
      target = do_wall_hit(&origin, vector, ammo_triple,pos->x, pos->y,TRUE);

   if (target != OBJ_NULL)
   {
      player_struct.num_hits++;
      player_attack_object(target, ammo_triple, 100, origin);
   }

   // Modify cursor position because of ammo's recoil (Done after shot is fired)
   randomize_cursor_pos(pos, r,
      AmmoProps[CPTRIP(ammo_triple)].recoil_force+player_struct.fatigue/FATIGUE_ACCURACY_RATIO);

   mfd_notify_func(MFD_WEAPON_FUNC, MFD_WEAPON_SLOT, FALSE, MFD_ACTIVE, FALSE);
   chg_set_flg(INVENTORY_UPDATE);
   return TRUE;
}

ubyte energy_expulsion = 0;
bool overload_beam = FALSE;

// ----------------------------------------------------------
// weapon_energy_drain()
//
// deals with heat/settings/charge of an energy weapon
//

bool weapon_energy_drain(weapon_slot *ws, ubyte charge, ubyte max_charge)
{
   ubyte energy_used;

   if (ws->heat >= OVERHEAT_THRESHOLD)
   {
      message_info(get_temp_string(REF_STR_GunTooHot));
      return(FALSE);
   }
   if (player_struct.energy < MIN_ENERGY_WPN_THRESHOLD)
   {
      message_info(get_temp_string(REF_STR_NoEnergyFireWeapon));
      return(FALSE);
   }

   // find out which is smaller, the charge setting, or the amount of "coolness" in the weapon
   charge = min(charge, (MAX_HEAT - ws->heat));
   if (OVERLOAD_VALUE(ws->setting))              // if we are overloaded - double it!
   {
      overload_beam = TRUE;
      charge = (MAX_HEAT * 2);
   }
   else
      overload_beam = FALSE;
   
   energy_used = (ubyte) ((((int) max_charge) * charge)/100);

   // energy_expulsion is used for lighting values
   energy_expulsion = charge;
   drain_energy(energy_used);

   // use up remaining energy if energy_used was more than energy left, and then
   //  find out what the setting would have been with the left over energy
   //  add the heat from the shot to the weapon
   if ((charge/2) > (0xFF - ws->heat))
   {
      charge = (0xFF - ws->heat)*2;
      ws->heat = 0xFF;
   }
   else
      ws->heat += (charge/2);

   return(TRUE);
}

// -------------------------------------------------------------------------------
// player_fire_energy() 
// 
// Fires the energy weapon in the specified slot returns true 
// iff the weapon actually got a shot off.

bool player_fire_energy(LGPoint *pos, ubyte slot, int gun_triple)
{
   Combat_Pt      vector;
   Combat_Pt      origin;
   ObjID          target;
   fix            bullet_mass;
   State          new_state;
   weapon_slot    *ws = &player_struct.weapons[slot];

   // get the charge setting
   ubyte          charge = OVERLOAD_VALUE(ws->setting) ? (MAX_HEAT) : ws->setting;

   if(!weapon_energy_drain(ws, charge, BeamGunProps[SCTRIP(gun_triple)].max_charge))
      return FALSE;

   chg_set_flg(VITALS_UPDATE);

   // do the ray cast here
   get_phys_state(objs[PLAYER_OBJ].info.ph, &new_state,PLAYER_OBJ);
   origin.x = new_state.X;   origin.y = new_state.Y;   origin.z = new_state.Z;
 
   find_fire_vector(pos, &vector);
   bullet_mass = fix_make(BeamGunProps[SCTRIP(gun_triple)].attack_mass, 0) * 30;

   target = ray_cast_vector(PLAYER_OBJ, &origin, vector,
      bullet_mass, RAYCAST_ATTACK_SIZE,
      fix_make(BeamGunProps[SCTRIP(gun_triple)].attack_speed, 0),
      fix_make(BeamGunProps[SCTRIP(gun_triple)].attack_range, 0));

   // did we hit something???
   if (target == OBJ_NULL)
      target = do_wall_hit(&origin, vector, gun_triple, pos->x, pos->y,TRUE);

   if (target != OBJ_NULL)
   {
      player_struct.num_hits++;
      player_attack_object(target, gun_triple, charge, origin);
   }

   // deal with overload
   OVERLOAD_RESET(ws->setting);

   mfd_force_update();  // need to do this because of OVERLOAD
   chg_set_flg(INVENTORY_UPDATE);

   return(TRUE);
}


#define SLOW_PROJECTILE_DURATION 1000
#define SLOW_PROJECTILE_GRAVITY  fix_make(0,0x0C00)
#define SLOW_PROJ_SPEED 6
#define PROJ_RAYCAST_RANGE    fix_make(0,0x6000)

// ---------------------------------------------------------------------------
// player_fire_slow_projectile
//

bool player_fire_slow_projectile(int proj_triple, int fire_triple,fix proj_mass, fix fire_spd, ubyte proj_speed, LGPoint *pos);

bool player_fire_slow_projectile_weapon(LGPoint *pos, ubyte slot, int gun_triple)
{
   ubyte          proj_speed = SpecialGunProps[SCTRIP(gun_triple)].speed;

   // decrement the ammo count - unless we're out of ammo
   decrease_ammo(slot,1);
   return(player_fire_slow_projectile(SpecialGunProps[SCTRIP(gun_triple)].proj_triple, gun_triple,
      fix_make(SpecialGunProps[SCTRIP(gun_triple)].attack_mass, 0) * 20,
      fix_make(SpecialGunProps[SCTRIP(gun_triple)].attack_speed, 0),
      proj_speed, pos ));
}


// here we actually generate the slow projectile and send it on it's way
// note that this actually takes in the relevant low-level data, so that 
// the source of the shot need not be a Weapon(tm) but can be a software, etc... -- Rob

extern ubyte old_head, old_pitch;

bool player_fire_slow_projectile(int proj_triple, int fire_triple,fix proj_mass, fix fire_spd, ubyte proj_speed, LGPoint *pos)
{
   Combat_Pt      vector;
   Combat_Pt      origin;
   State          new_state;
   ObjID          proj_id;
   ObjSpecID      osid;
   ObjLoc         loc;
   Robot          da_robot;
   LGPoint          new_pos;
   physics_handle obj_ph;

   fix            dist;
   ubyte          head;

   proj_id = obj_create_base(proj_triple);
   if (proj_id == OBJ_NULL)
   {
      return(OK);
   }
   osid = objs[proj_id].specID;

   objPhysicss[osid].owner = PLAYER_OBJ;
   objPhysicss[osid].bullet_triple = fire_triple;
   objPhysicss[osid].duration = player_struct.game_time + SLOW_PROJECTILE_DURATION;
   objPhysicss[osid].p3.x = objPhysicss[osid].p3.y = 0;

   new_pos = *pos;
   new_pos.y -= 4;
   if (global_fullmap->cyber)
   {
      // shift zero to middle of screen
      new_pos.x -= (((fauxrend_context *)_current_fr_context)->xtop
         + (((fauxrend_context *)_current_fr_context)->xwid >> 1));
      new_pos.y -= (((fauxrend_context *)_current_fr_context)->ytop
         + (((fauxrend_context *)_current_fr_context)->ywid >> 1));

      // shrink
      new_pos.x = (new_pos.x*3)/4;
      new_pos.y = (new_pos.y*3)/4;

      // shift back
      new_pos.x += (((fauxrend_context *)_current_fr_context)->xtop
         + (((fauxrend_context *)_current_fr_context)->xwid >> 1));
      new_pos.y += (((fauxrend_context *)_current_fr_context)->ytop
         + (((fauxrend_context *)_current_fr_context)->ywid >> 1));
   }

   find_fire_vector(&new_pos, &vector);

   get_phys_state(objs[PLAYER_OBJ].info.ph, &new_state,PLAYER_OBJ);
   // do the raycasting to have the kickback effect
   if (fire_spd && !global_fullmap->cyber)
   {
      origin.x = new_state.X;   origin.y = new_state.Y;   origin.z = new_state.Z;
      ray_cast_vector(PLAYER_OBJ, &origin, vector, proj_mass,
         RAYCAST_ATTACK_SIZE, fire_spd, PROJ_RAYCAST_RANGE);
   }

   // Add projectile object here and send it on its way.
   loc = objs[PLAYER_OBJ].loc;
   loc.x = obj_coord_from_fix(new_state.X);
   loc.y = obj_coord_from_fix(new_state.Y);
   loc.z = obj_height_from_fix(new_state.Z);

   // let's get the heading of the shot
   head = obj_angle_from_fixang(fix_atan2(vector.y, vector.x)); 
   loc.h = (ubyte) ((320L-head) % 256);

   // let's get the pitch
   dist = fix_fast_pyth_dist(vector.x, vector.y);
   loc.p = obj_angle_from_fixang(fix_atan2(vector.z, dist));

   // for slow projectiles - we don't care about the bank
   loc.b = 0; 

   // let's move it a little in front of the player
   if (global_fullmap->cyber)
   {
      new_pos.x = (((fauxrend_context *)_current_fr_context)->xwid >> 1)+ ((fauxrend_context *)_current_fr_context)->xtop;
      new_pos.y = (((fauxrend_context *)_current_fr_context)->ywid >> 1)+ ((fauxrend_context *)_current_fr_context)->ytop;
      find_fire_vector(&new_pos, &origin);
      loc.x += (obj_coord_from_fix(origin.x)/2);
      loc.y += (obj_coord_from_fix(origin.y)/2);
      loc.z += (obj_height_from_fix(origin.z)/2);
   }

   // get state gets the velocity - this from shamu the pirate!
   EDMS_get_state(objs[PLAYER_OBJ].info.ph, &new_state); 
                                                                                                    
   vector.x = (vector.x * proj_speed) + (obj_coord_from_fix(new_state.X_dot)*PHYSICS_RADIUS_UNIT)*2;
   vector.y = (vector.y * proj_speed) + (obj_coord_from_fix(new_state.Y_dot)*PHYSICS_RADIUS_UNIT)*2;
   vector.z = (vector.z * proj_speed) + (obj_height_from_fix(new_state.Z_dot)*PHYSICS_RADIUS_UNIT)*2;

   obj_move_to_vel(proj_id, &loc, TRUE, vector.x, vector.y, vector.z);
   apply_gravity_to_one_object(proj_id, SLOW_PROJECTILE_GRAVITY);

   obj_ph = objs[proj_id].info.ph;

//   Spew(DSRC_PHYSICS_Collisions, ("We are ignoring collisions between ph: %d and ph: %d\n",objs[PLAYER_OBJ].info.ph, obj_ph));
   EDMS_ignore_collisions(objs[PLAYER_OBJ].info.ph, obj_ph);

   // make it wall proof
   EDMS_get_robot_parameters(obj_ph, &da_robot);
   da_robot.cyber_space = -1;
   EDMS_set_robot_parameters(obj_ph, &da_robot);

   mfd_force_update(); // due to overload
   chg_set_flg(INVENTORY_UPDATE);

   return TRUE;
}


// -----------------------------------------------------------------------------
// player_fire_energy_proj()
//

#define OVERLOAD_EXPLODE 5
#define PROJ_X_OFFSET 20
#define PROJ_Y_OFFSET 20

bool player_fire_energy_proj(LGPoint *pos, ubyte slot, int gun_triple)
{
   weapon_slot    *ws = &player_struct.weapons[slot];
   ubyte          charge = OVERLOAD_VALUE(ws->setting) ? (MAX_HEAT) : ws->setting;
   int            subclass= SCTRIP(gun_triple);

   if(!weapon_energy_drain(ws, charge, BeamprojGunProps[SCTRIP(gun_triple)].max_charge))
      return FALSE;

   // the TRIPLE of the projectile will determine the "power"
   player_fire_slow_projectile(BeamprojGunProps[subclass].proj_triple,
      MAKETRIP(charge,0,ws->subtype),
      (fix_make(BeamprojGunProps[subclass].attack_mass, 0)*20),
      fix_make(BeamprojGunProps[subclass].attack_speed, 0),
      BeamprojGunProps[SCTRIP(gun_triple)].speed,
      pos);

   // deal with feedback of energy usage
   mfd_force_update();
   chg_set_flg(INVENTORY_UPDATE);

   return TRUE;
}


// used to decide when handart should return to screen
ulong next_fire_time = 0;

// it decides how much to light up the world - this
// is so automatic weapons don't keep the same brightness
byte gun_fire_offset = 0;

// ---------------------------------
// fire_player_weapon()
//
// pos - point on viewscreen (not converted to -100/+100 map coords)
// pull - whether the gun was just pulled.

#define AUTOFIRE_SPEED 1500
#define SOFTWARE_SPEW_FIRE_RATE  60

ulong software_fire_remainder = 0;

char cspace_digi_fxs[] = { SFX_DRILL, SFX_DATASTORM, SFX_NONE, SFX_DISC, SFX_PULSER, SFX_NONE, SFX_NONE};
int cspace_slow_projs[] = { DRILLSLOW_TRIPLE, SPEWSLOW_TRIPLE, 0, DISCSLOW_TRIPLE, CYBERSLOW_TRIPLE, 0, 0};

bool fire_player_software(LGPoint *pos, LGRegion *, bool pull)
{
   int soft = player_struct.actives[ACTIVE_COMBAT_SOFT];
   bool retval;
   char shots = 1;

   if (!player_struct.softs.combat[soft])
      return FALSE;
   if (!pull)
      return FALSE;

   player_struct.last_fire = player_struct.game_time;

   // Note fullwise our clever (or not so clever) usage of weapon_triple as the
   // level of the ware firing the projectile.
   play_digi_fx(cspace_digi_fxs[soft],1);
   switch (soft)
   {
      case SOFTWARE_DRILL:
      case SOFTWARE_PULSER: 
      case SOFTWARE_DISC: 
      case SOFTWARE_SPEW:
         retval = player_fire_slow_projectile(cspace_slow_projs[soft], player_struct.softs.combat[soft], 0, FIX_UNIT, 3, pos);
         break;
   }
   return retval;
}

ulong weapon_fire_remainder = 0;
bool handart_flash = FALSE;

// ---------------------------------------------------------------------------
// fire_player_weapon()
//

char weapon_combat_fxs[] = { 
SFX_GUN_MINIPISTOL    , 
SFX_GUN_DARTPISTOL    , 
SFX_GUN_MAGNUM        ,
SFX_GUN_ASSAULT       ,
SFX_GUN_RIOT          ,
SFX_GUN_FLECHETTE     ,
SFX_GUN_SKORPION      ,
SFX_GUN_MAGPULSE      ,
SFX_GUN_RAILGUN       ,
SFX_GUN_PIPE_MISS     ,
SFX_GUN_LASEREPEE_MISS,
SFX_GUN_PHASER        ,
SFX_GUN_BLASTER       ,
SFX_GUN_IONBEAM       ,
SFX_GUN_STUNGUN       ,
SFX_GUN_PLASMA        ,
};

ObjID damage_sound_id;
char damage_sound_fx = -1;

#define CYBER_FIRE_WAIT 60
#define MIN_AUTO_SHOT 3
#define AUTO_FIRE_CLICK_WAIT 60
#define CURSOR_WAIT 60
#define MAX_AUTO_FIRE 8

bool fire_player_weapon(LGPoint *pos, LGRegion *r, bool pull)
{
   int      w = player_struct.actives[ACTIVE_WEAPON];
   int      shots = 1;
   int      i;
   int      gun_triple = current_weapon_trip();
   short    deltax, deltay;
   bool     handart_ok = TRUE;
   LGPoint    realpos = *pos;
   LGPoint    cp;
   LGRect     rc;
   ObjID    hit_obj = OBJ_NULL;
   extern bool game_paused;         // prevent firing when time ain't running
   extern bool time_passes;

   if (player_struct.dead || game_paused || !time_passes)
      return (FALSE);

   region_abs_rect(r,r->r,&rc);
   if (DoubleSize)
      rc.lr.y = SCONV_Y(rc.lr.y) >> 1;
   cp = realpos;
   if (!DoubleSize)
	   ss_mouse_convert(&(cp.x),&(cp.y),TRUE);
#ifdef STEREO_SUPPORT
   if (convert_use_mode == 5)
   {
      switch (i6d_device)
      {
         case I6D_VFX1:
            realpos.x = realpos.x << 1;
            break;
      }
   }
#endif
   if (!RECT_TEST_PT(&rc,cp))
   {
      realpos.x = r->abs_x + RectWidth(r->r)/2;
      realpos.y = r->abs_y + RectHeight(r->r)/2;

//���      ui_mouse_put_xy(realpos.x,realpos.y);
      // Heck we know this is the first time you're firing. 
      pull = TRUE;
   }

   if (global_fullmap->cyber)
   {
      if ((CYBER_FIRE_WAIT+player_struct.last_fire) > player_struct.game_time)
         return FALSE;
      player_struct.last_fire = player_struct.game_time;
      return(fire_player_software(&realpos,r,pull));
   }

   if((player_struct.weapons[w].type == GUN_SUBCLASS_AUTO) && !player_struct.weapons[w].ammo)
      weapon_mfd_for_reload();

   // if we didn't just pull the gun and we have an automatic weapon or
   //  if we are out of ammo for an automatic weapon - snap back to normal cursor
   if ((!pull && (player_struct.weapons[w].type != GUN_SUBCLASS_AUTO)) ||
      ((player_struct.weapons[w].type == GUN_SUBCLASS_AUTO) && !player_struct.weapons[w].ammo))
   {
      extern bool fire_slam;

      if (fire_slam && (player_struct.last_fire+CURSOR_WAIT < player_struct.game_time))
      {
         extern uiSlab fullscreen_slab;
         extern uiSlab main_slab;
		
		if (full_game_3d)
	         uiPopSlabCursor(&fullscreen_slab);
	     else
     	    uiPopSlabCursor(&main_slab);
         fire_slam = FALSE;
      }
      return FALSE;
   }

   // Don't fire if we're out of ammo, and we're notfiring a beam weapon or hand-to-hand
   if ((player_struct.weapons[w].type != GUN_SUBCLASS_BEAM) &&
       (player_struct.weapons[w].type != GUN_SUBCLASS_HANDTOHAND) &&
       (player_struct.weapons[w].type != GUN_SUBCLASS_BEAMPROJ) &&
       (player_struct.weapons[w].ammo == 0)) {
      weapon_mfd_for_reload();
      return FALSE;
   }

   // if the trigger is being held down, fire as many shots
   // as time has passed.  (But not more shots than we actually have
   // in our clip!)

   if (player_struct.weapons[w].type == GUN_SUBCLASS_AUTO)
   {
      if (!pull)
      {
         ulong    deltat = (player_struct.game_time-player_struct.last_fire) * player_struct.fire_rate + weapon_fire_remainder;

         if ((player_struct.auto_fire_click+AUTO_FIRE_CLICK_WAIT) >  player_struct.game_time)
            return FALSE;

         shots = deltat/AUTOFIRE_SPEED;

         shots = min(min(shots, player_struct.weapons[w].ammo), MAX_AUTO_FIRE);
         weapon_fire_remainder = deltat % AUTOFIRE_SPEED;
         if (shots)
            gun_fire_offset = RndRange(&effect_rnd, 0, 30) - 15;
      }
      else
      {
         // make sure we're not just clicking very fast
         if ((player_struct.auto_fire_click+AUTO_FIRE_CLICK_WAIT) > player_struct.game_time)
            return FALSE;
         // clear the weapon remainder - if we just pulled the trigger
         weapon_fire_remainder = 0;
         shots = MIN_AUTO_SHOT;
         player_struct.auto_fire_click = player_struct.game_time;
      }

      next_fire_time = player_struct.game_time;
   }
   else
   {
      if (((GunProps[CPTRIP(gun_triple)].fire_rate*10)+player_struct.last_fire) > player_struct.game_time)
         return FALSE;
      next_fire_time = (GunProps[CPTRIP(gun_triple)].fire_rate*10)+player_struct.game_time;
      gun_fire_offset = 0;
   }
   if (shots)
      player_struct.last_fire = player_struct.game_time;
   else
      return FALSE;

   deltax = mouse_attack_x = realpos.x;  deltay = mouse_attack_y = realpos.y;
   switch(player_struct.weapons[w].type)
   {
      case (GUN_SUBCLASS_PISTOL):
         handart_ok = player_fire_projectile(&realpos, r, w,gun_triple);
         break;
      case (GUN_SUBCLASS_AUTO):
         for (i=0;i<shots;i++)
         {
            if (!player_fire_projectile(&realpos, r, w,gun_triple))
               break;
         }
         break;
      case (GUN_SUBCLASS_SPECIAL):
         handart_ok = player_fire_slow_projectile_weapon(&realpos, w,gun_triple);
         // do slow projectile
         break;
      case (GUN_SUBCLASS_HANDTOHAND):
         handart_ok = player_fire_handtohand(&realpos,w,&hit_obj,gun_triple);
         break;
      case (GUN_SUBCLASS_BEAM):
         handart_ok = player_fire_energy(&realpos, w,gun_triple);
         break;
      case (GUN_SUBCLASS_BEAMPROJ):
         handart_ok = player_fire_energy_proj(&realpos, w,gun_triple);
         break;
   }
   if (handart_ok)
   {
      int trip;
      handart_show = 2;
      handart_fire = FALSE;
      handart_flash = TRUE;

      if (_current_loop <= FULLSCREEN_LOOP)
      {
         chg_set_flg(INVENTORY_UPDATE);
         mfd_notify_func(MFD_WEAPON_FUNC, MFD_WEAPON_SLOT, FALSE, MFD_ACTIVE, FALSE);
      }

      player_struct.rounds_fired += shots;

      // Play some sound effects!!
      trip = MAKETRIP(CLASS_GUN, player_struct.weapons[w].type, player_struct.weapons[w].subtype);
      switch (trip)
      {
         case BATON_TRIPLE:
            if (hit_obj != OBJ_NULL)
            {
               if ((objs[hit_obj].obclass == CLASS_CRITTER)&&
                   (objs[hit_obj].subclass == CRITTER_SUBCLASS_MUTANT))
                     play_digi_fx(SFX_GUN_PIPE_HIT_MEAT,1);
               else
                  play_digi_fx(SFX_GUN_PIPE_HIT_METAL,1);
            }
            break;
         case LASERAPIER_TRIPLE:
            if (hit_obj != OBJ_NULL)
               play_digi_fx(SFX_GUN_LASEREPEE_HIT,1);
            break;
         default:
            play_digi_fx(weapon_combat_fxs[CPTRIP(trip)],1);
            break;
      }
      if (damage_sound_fx != -1)
      {
         play_digi_fx_obj(damage_sound_fx,1,damage_sound_id);
      }
      damage_sound_fx = -1;
   }
   return(handart_ok);
}

#ifdef SELFRUN
// ----------------------------------------------
// demo_fire_weapon()
//

void demo_fire_weapon(short x, short y)
{
   LGPoint aimpos;

   aimpos.x = x;
   aimpos.y = y;
   ui_mouse_put_xy(x, y);
   fire_player_weapon(&aimpos,_current_view,TRUE);
}
#endif

// ---------------------------------------------------------------
// get_available_ammo_type()
//

void get_available_ammo_type(int guntype, int gun_subtype, int *num_ammo_types, ubyte *bitflag, int *ammo_subclass)
{
   int         i;
   int         subclass;
   int         type;
   int         triple;
   int         count = 0;

   if (guntype == GUN_SUBCLASS_BEAM || guntype == GUN_SUBCLASS_HANDTOHAND)
   {
      *num_ammo_types = 0;
      return;
   }

   triple = MAKETRIP(CLASS_GUN, guntype, gun_subtype);
   type = GunProps[CPTRIP(triple)].useable_ammo_type;
   subclass = *ammo_subclass = AMMOTYPE_SUBCLASS(type);

   type = AMMOTYPE_TYPE(type);
   for (i=0; i<3; i++, type >>= 1)
   {
      if (type & 0x01)
      {
         triple = MAKETRIP(CLASS_AMMO, subclass, i);
         if (player_struct.cartridges[CPTRIP(triple)] != 0
         || player_struct.partial_clip[CPTRIP(triple)] > 0)
            bitflag[count++] = i;
      }
   }
   *num_ammo_types = count;
}

// --------------------------------------------------------------------------------------
// change_ammo_type()
//

bool change_ammo_type(ubyte ammo_type)
{
   weapon_slot *ws;
   int         subclass;
   int         type;
   int         triple;
   bool changed = FALSE;

   ws = &player_struct.weapons[player_struct.actives[ACTIVE_WEAPON]];
   triple = MAKETRIP(CLASS_GUN, ws->type, ws->subtype);
   type = GunProps[CPTRIP(triple)].useable_ammo_type;
   subclass = AMMOTYPE_SUBCLASS(type);

   // This needs to be fixed for the new object regime
   triple = MAKETRIP(CLASS_AMMO, subclass, ammo_type);
   if (player_struct.cartridges[CPTRIP(triple)] != 0)
   {
      // Decrement the cartridge count
      player_struct.cartridges[CPTRIP(triple)]--;
      ws->ammo = AmmoProps[CPTRIP(triple)].cartridge_size;
      changed = TRUE;
   }
   else if (player_struct.partial_clip[CPTRIP(triple)] != 0)
   {
      ws->ammo = player_struct.partial_clip[CPTRIP(triple)];
      player_struct.partial_clip[CPTRIP(triple)] = 0;
      changed = TRUE;
   }
   if (changed)
   {
      ws->ammo_type = ammo_type;
      chg_set_flg(INVENTORY_UPDATE);
      switch(triple)
      {
         case PRAMMO_TRIPLE:
         case MRAMMO_TRIPLE:
         case HNAMMO_TRIPLE:
         case SPLAMMO_TRIPLE:
            play_digi_fx(SFX_RELOAD_2,1);
            break;
         default:
            play_digi_fx(SFX_RELOAD_1,1);
            break;
      }
      mfd_notify_func(MFD_WEAPON_FUNC,MFD_WEAPON_SLOT,FALSE,MFD_ACTIVE,TRUE);
      return TRUE;
   }
   else
      return FALSE;
}

//--------------------------------------------------------------------
// unload_current_weapon()
//
// removes all ammo and stuffs it in a partial clip. 

void unload_current_weapon(void)
{
   weapon_slot *ws;
   int         subclass;
   int         type;
   int         triple;
   int         clipsize; 

   ws = &player_struct.weapons[player_struct.actives[ACTIVE_WEAPON]];
   triple = MAKETRIP(CLASS_GUN, ws->type, ws->subtype);
   type = GunProps[CPTRIP(triple)].useable_ammo_type;
   subclass = AMMOTYPE_SUBCLASS(type);
   triple = MAKETRIP(CLASS_AMMO, subclass, ws->ammo_type);
   clipsize = AmmoProps[CPTRIP(triple)].cartridge_size;
   player_struct.partial_clip[CPTRIP(triple)] += ws->ammo;

   if(clipsize<=0) return;
   while(player_struct.partial_clip[CPTRIP(triple)] >= clipsize)
   {
      player_struct.cartridges[CPTRIP(triple)]++;
      player_struct.partial_clip[CPTRIP(triple)] -= clipsize;
   }
   chg_set_flg(INVENTORY_UPDATE);
   ws->ammo = 0;
}


// ----------------------------------------
// cool_off_beam_weapons()
//
// Cool off beam weapons, duh

#define WEAPON_COOL_OFF_TIME  25
#define HEAT_FUDGE            2

bool temp_critical = FALSE;

void check_temperature(weapon_slot *ws, bool clear)
{
   if (clear)
   {
      if (temp_critical)
         hud_unset(HUD_BEAMHOT);
      temp_critical = FALSE;
   }
   if ((ws->type != GUN_SUBCLASS_BEAM) &&
       (ws->type != GUN_SUBCLASS_BEAMPROJ))
      return;

   if (!OVERLOAD_VALUE(ws->setting) &&
      ((ws->heat+(ws->setting/2)) > (OVERHEAT_THRESHOLD+HEAT_FUDGE)))
   {
      if (!temp_critical)
         hud_set(HUD_BEAMHOT);
      temp_critical = TRUE;
   }
   else
   {
      if (temp_critical)
         hud_unset(HUD_BEAMHOT);
      temp_critical = FALSE;
   }
}

void cool_off_beam_weapons(void)
{
   static long running_dt;
   int i;
   weapon_slot  *ws;

   running_dt += player_struct.deltat;
   while (running_dt > WEAPON_COOL_OFF_TIME)
   {
      running_dt = max(0,running_dt - WEAPON_COOL_OFF_TIME); // must be non frame-rate dependant, even if goofily so.

      for (i = 0; i < NUM_WEAPON_SLOTS; i++)
      {
         ws = &player_struct.weapons[i];

         if ((ws->type == GUN_SUBCLASS_BEAM) ||
             (ws->type == GUN_SUBCLASS_BEAMPROJ))
         {
            if (ws->heat > 0)
            {
               ws->heat--;

               chg_set_flg(INVENTORY_UPDATE);
               if (player_struct.actives[ACTIVE_WEAPON] == i)
               {
                  mfd_notify_func(MFD_WEAPON_FUNC,MFD_WEAPON_SLOT,FALSE,MFD_ACTIVE,TRUE);
                  check_temperature(ws, FALSE);
               }
            }   
         }
      }
   }

   return;
}

// ---------------------------------------------------------------------------
// randomize_cursor_pos()
//
// Jerks the cursor around randomly, to a degree determined by
// a percentage which is from 0->100%

void randomize_cursor_pos(LGPoint *cpos, LGRegion *reg, ubyte p)
{
   int newx, newy;
   LGRect r;

   if (p < 2) return;

   newx = cpos->x + (rand() % (p+1)) - (p/2);
   newy = cpos->y + (rand() % (p+1)) - (p/2);

   r = *(reg->r);
#ifdef SVGA_SUPPORT
   ss_mouse_convert(&(r.ul.x),&(r.ul.y),FALSE);
   ss_mouse_convert(&(r.lr.x),&(r.lr.y),FALSE);
#endif   
   cpos->x = max(r.ul.x,(min(newx,r.lr.x)));
   cpos->y = max(r.ul.y,(min(newy,r.lr.y)));

#ifdef STEREO_SUPPORT
   if (convert_use_mode == 5)
   {
      switch (i6d_device)
      {
         case I6D_VFX1:
            mouse_put_xy(cpos->x >> 1,cpos->y);
            break;
         default:
            mouse_put_xy(cpos->x,cpos->y);
            break;
      }
   }
   else
#endif
	if (DoubleSize)
		mouse_put_xy(cpos->x*2, cpos->y*2);
	else
     	mouse_put_xy(cpos->x,cpos->y);
   uiSetCursor();
}

// ---------------------------------------------------------------------------
// drain_energy()
//
// takes a request for energy from the central reservoir.  Returns how
// much energy was actually drained and thus given to the requesting function

#define ENERGY_VAR_RATE 50

ubyte drain_energy(ubyte e)
{
   extern int bio_energy_var;
   ubyte ret;

   if (e > player_struct.energy) {
      ret = player_struct.energy;
      player_struct.energy = 0;
   }
   else {
      ret = e;
      player_struct.energy -= e;
   }
   chg_set_flg(VITALS_UPDATE);
   if (ret > 0)
      bio_energy_var += ret*ENERGY_VAR_RATE;
   
   return ret;
}

// returns the triple of the player's currently selected weapon.
// returns -1 if no weapon is currently selected.

int current_weapon_trip()
{
   weapon_slot *ws;
   int slot;

   slot = player_struct.actives[ACTIVE_WEAPON];
   if(slot==EMPTY_WEAPON_SLOT) return -1;

   ws = &player_struct.weapons[slot];

   return( MAKETRIP(CLASS_GUN,ws->type,ws->subtype) );
}

// CHANGING CURSOR STUFF
//

#define NUM_MOTION_CURSORS 15
short cursor_color_offset = RED_BASE+4;
extern grs_bitmap motion_cursor_bitmaps[NUM_MOTION_CURSORS];

ubyte weapon_colors[NUM_SC_GUN] = {RED_BASE+4,        // pistol
                                 GREEN_BASE,          // auto
                                 0x4A,                // special - yellow
                                 0x40,                // hand-to-hand - brown
                                 BLUE_BASE+4,         // beam
                                 TURQUOISE_BASE+3};    // beam proj

                              extern ubyte handart_count;
// -----------------------------------------------------------------------------
// change_selected_weapon()
// 
// deals with changing cursor color
//

void change_selected_weapon(int new_weapon)
{
   extern void reset_handart_count(int wpn);
   weapon_slot   *ws=&player_struct.weapons[new_weapon];
   grs_bitmap   *curs = motion_cursor_bitmaps;
   uchar    *bits;
   int      i,j,size;
   short    new_offset;

   if (global_fullmap->cyber)
      return;

   new_offset = weapon_colors[ws->type];

/* KLC - this color changing code doesn't work with new hi-res cursors.
   for (i=0;i<NUM_MOTION_CURSORS;i++)
   {
      bits = curs->bits;
      size = curs->w * curs->h;
      for (j=0;j<size;j++,bits++)
      {
         if ((*bits) && (!((*bits)%2)))
            *bits = *bits+(new_offset-cursor_color_offset);
      }
      curs++;
   }
   cursor_color_offset = new_offset;
*/
   check_temperature(ws, TRUE);
   if ((ws->type == GUN_SUBCLASS_BEAM) ||
       (ws->type == GUN_SUBCLASS_BEAMPROJ))
   {
      check_temperature(ws, FALSE);
   }
   player_struct.auto_fire_click = 0;

   // reset the handart count
   handart_show = 1;
   reset_handart_count(new_weapon);
}


bool gun_takes_ammo(int guntrip, int ammotrip)
{
   int type;

   type = GunProps[CPTRIP(guntrip)].useable_ammo_type;
   return(AMMOTYPE_TYPE(type)!=0 && TRIP2SC(ammotrip) == AMMOTYPE_SUBCLASS(type));
}

bool reload_current_weapon(void)
{
   weapon_slot* ws = &player_struct.weapons[player_struct.actives[ACTIVE_WEAPON]];
   int ammosc;
   ubyte ammo_types[3];
   int num_types;
   int i;

//   unload_current_weapon();
//   if (change_ammo_type(ws->ammo_type))
//      return TRUE;
   get_available_ammo_type(ws->type,ws->subtype,&num_types,ammo_types,&ammosc);
   for (i = num_types -1; i >= 0; i--)
      if (change_ammo_type(ammo_types[i]))
         return TRUE;
   return FALSE;
}


bool reload_weapon_hotkey(short, ulong, void* data)
{
   int differ=(int)data;
   weapon_slot* ws = &player_struct.weapons[player_struct.actives[ACTIVE_WEAPON]];
   int ammosc;
   ubyte ammo_types[3];
   int num_types;
   int i, cur;

   if(!differ) {
      // attempts to reload the current weapon with the same ammo type it
      // held before, but if that fails will reload with anything available.
      unload_current_weapon();
      mfd_notify_func(MFD_WEAPON_FUNC,MFD_WEAPON_SLOT,FALSE,MFD_ACTIVE,TRUE);
      if (change_ammo_type(ws->ammo_type)) {
         return TRUE;
      }
      return(reload_current_weapon());
   }
   else {
      unload_current_weapon();
      mfd_notify_func(MFD_WEAPON_FUNC,MFD_WEAPON_SLOT,FALSE,MFD_ACTIVE,TRUE);
      get_available_ammo_type(ws->type,ws->subtype,&num_types,ammo_types,&ammosc);
      // find index of current ammo type
      for(cur=0;cur<num_types-1;cur++)
         if(ammo_types[cur]==ws->ammo_type)
            break;
      // try available ammo types, current one last.  Note that if our
      // current type is not avaiable, then cur will be num_types, or
      // zero mode num_types, so this will try all numbers modulo num_types
      // in the order 1, ... ,num_types-1,0
      for(i=1;i<num_types;i++) {
         if(change_ammo_type(ammo_types[(cur+i)%num_types])) {
            return TRUE;
         }
      }
   }
   return FALSE;
}


bool ready_to_draw_handart(void)
{
   ubyte active = player_struct.actives[ACTIVE_WEAPON];
   bool val = FALSE;

   switch (player_struct.weapons[active].type)
   {
      case(GUN_SUBCLASS_HANDTOHAND):
         val = TRUE;
         break;
      case(GUN_SUBCLASS_BEAM):
      case(GUN_SUBCLASS_BEAMPROJ):
         val = (player_struct.weapons[active].heat < OVERHEAT_THRESHOLD);
         break;
      default:
         val = (player_struct.game_time >= next_fire_time);
         break;
   }
   return (val);
}
