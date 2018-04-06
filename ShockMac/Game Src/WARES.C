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
 * $Source: r:/prj/cit/src/RCS/wares.c $
 * $Revision: 1.109 $
 * $Author: dc $
 * $Date: 1994/11/22 15:59:26 $
 *
 */

#include <string.h>

#include "wares.h"
#include "hud.h"
#include "player.h"
#include "gamesys.h"
#include "sideicon.h"
#include "newmfd.h"
#include "cybstrng.h"
#include "gamestrn.h"
#include "textmaps.h"
#include "render.h"
#include "frparams.h"
#include "FrUtils.h"
#include "objsim.h"
#include "otrip.h"
#include "mainloop.h"
#include "gameloop.h"
#include "musicai.h"
#include "sfxlist.h"
#include "objbit.h"
#include "objprop.h"
#include "tools.h"
#include "faketime.h"
#include "weapons.h"
#include "fullscrn.h"
#include "mainloop.h"
#include "map.h"
#include "physics.h"
#include "softdef.h"
#include "cyber.h"
#include "damage.h"

//----------------
//  Internal Prototypes
//----------------
bool is_passive_hardware(int n);
bool is_oneshot_misc_software(int n);
int energy_cost(int warenum);
void hardware_closedown(bool visible);
void hardware_startup(bool visible);
void hardware_power_outage(void);
bool check_game(void);


// ------
// Globals
// -------

// forward decls for arrays at end of file
extern WARE HardWare[NUM_HARDWAREZ];
extern WARE Combat_SoftWare[NUM_COMBAT_SOFTS];
extern WARE Defense_SoftWare[NUM_DEFENSE_SOFTS];
extern WARE Misc_SoftWare[NUM_MISC_SOFTS];

#define MAX_VERSIONS 5
extern short energy_cost_vec[NUM_HARDWAREZ][MAX_VERSIONS];

long ware_base_triples[NUM_WARE_TYPES] =
{
   MAKETRIP(CLASS_HARDWARE,0,0),
   MAKETRIP(CLASS_SOFTWARE,SOFTWARE_SUBCLASS_OFFENSE,0),
   MAKETRIP(CLASS_SOFTWARE,SOFTWARE_SUBCLASS_DEFENSE,0),
   MAKETRIP(CLASS_SOFTWARE,SOFTWARE_SUBCLASS_ONESHOT,0),
};


// The existence of this array is a crime.  I should be shot. 
ubyte waretype2invtype[] =
{
   MFD_INV_HARDWARE,
   MFD_INV_SOFT_COMBAT,
   MFD_INV_SOFT_DEFENSE,
   MFD_INV_SOFT_MISC,
};

#define IDX_OF_TYPE(type,trip) (OPTRIP(trip) - OPTRIP(ware_base_triples[type]))

#define PASSIVE_WARE_FLAG 1


// EXTERNALS
// =========
// -------------------------------------------------------
// get_ware_triple() converts our stupid representation for 
// a ware triple into the standard one.  

int get_ware_triple(int waretype, int num)
{
   return nth_after_triple(ware_base_triples[waretype],num);
}

// ---------------------------------------------------------------------------
// get_ware_name()
// 
// Returns the name of a ware to the inventory system.

                                          
char* get_ware_name(int waretype, int num, char* buf, int sz)
{
   get_object_short_name(nth_after_triple(ware_base_triples[waretype],num), buf, sz);
   return buf;
}


bool is_passive_hardware(int n)
{
   ushort cflags = (ObjProps[OPTRIP(MAKETRIP(CLASS_HARDWARE,0,0))+n].flags & CLASS_FLAGS) >> CLASS_FLAGS_SHF;
   return (cflags & PASSIVE_WARE_FLAG);
}

bool is_oneshot_misc_software(int n)
{
   return ((n < NUM_ONESHOT_SOFTWARE));
}

// INTERNALS
// =========

int energy_cost(int warenum)
{
   uchar version = player_struct.hardwarez[warenum];
   if (version == 0)
      return 0;
   if (warenum == CPTRIP(LANTERN_HARD_TRIPLE))
      version = LAMP_SETTING(player_struct.hardwarez_status[warenum]) + 1;
   if (warenum == CPTRIP(SHIELD_HARD_TRIPLE))
      version = LAMP_SETTING(player_struct.hardwarez_status[warenum]) + 1;
   if (warenum == CPTRIP(MOTION_HARD_TRIPLE) && motionware_mode == MOTION_SKATES)
      version = MOTION_SKATES;
   if (warenum == CPTRIP(JET_HARD_TRIPLE))
      return 0;
   return energy_cost_vec[warenum][version-1];
}
   

// ---------------------------------------------------------------------------
// use_ware()
//
// Called from the UI/Inventory, this routine figures out what is being used
// and what function to call.  Turns things on or off as appropriate.

void use_ware(int waretype, int num)
{
   ubyte *player_wares, *player_status;
   WARE  *wares;
   int   n,ecost;
   int ware_sfx = SFX_NONE,hnd;
//   int   i;
//   ubyte invtype;
   if ((!global_fullmap->cyber != (waretype == 0))   // boolean equality, yum.
      && !(waretype == WARE_SOFT_MISC && num == 4) // special games ware hack
      && !(waretype == WARE_HARD && num == HARDWARE_FULLSCREEN))
         return;
   get_ware_pointers(waretype, &player_wares, &player_status, &wares, &n);
   if ((player_wares[num] == 0) && (!(WareActive(player_status[num]))))
   {
      return; // don't turn on a ware we don't have, only turn off one we're
              // discarding
   }

   // Hey, can we even use this kind of ware right now?
   if (wares[num].check != NULL)
      if (!wares[num].check())
         return;

   // check to see if we have enough power
   if (waretype == WARE_HARD && !WareActive(player_status[num]) && player_struct.energy < (energy_cost(num)+4)/5)
   {
      string_message_info(REF_STR_WareNoPower);
      return;
   }
   player_status[num] ^= WARE_ON;

   if (wares[num].sideicon != SI_NONE)
      side_icon_expose(wares[num].sideicon);
 
   if (!WareActive(player_status[num])) {    // we're turning a ware off

      // note that the energy_cost function may use state which is
      // dependent on the ware being on to figure out the correct
      // cost (e.g., motionware).
      if(waretype==WARE_HARD)
         ecost=energy_cost(num);

      if (wares[num].turnoff) wares[num].turnoff(TRUE,TRUE);
      switch (num)
      {
         case HARDWARE_360:
         case HARDWARE_SHIELD:
         case HARDWARE_EMAIL:
            break;
         case HARDWARE_GOGGLE_INFRARED:
            ware_sfx = SFX_VIDEO_DOWN;
            break;
         default:
            ware_sfx = SFX_HUDFROB;
            break;
      }
   }
   else {                                   // we're turning a ware on

      // Turn on the durned thing
      if (wares[num].turnon) wares[num].turnon(TRUE,TRUE);

      // note that the energy_cost function may use state which is
      // dependent on the ware being on to figure out the correct
      // cost (e.g., motionware).
      if(waretype==WARE_HARD)
         ecost=energy_cost(num);

      if ((waretype == WARE_HARD) &&
          (num >= FIRST_GOGGLE_WARE) &&
          (num <= LAST_GOGGLE_WARE))
      {

      // Play the goggle sound effect
         ware_sfx = SFX_GOGGLE;
      }
      else
      {
         switch (num)
         {
            case HARDWARE_SHIELD:
               break;
            default:
               ware_sfx = SFX_HUDFROB;
               break;
         }
      }



//      // keep the invtype around in case we want to let mfd know about it
//      if      (waretype == WARE_HARD)           invtype = MFD_INV_HARDWARE;
//      else if (waretype == WARE_SOFT_COMBAT)    invtype = MFD_INV_SOFT_COMBAT;
//      else if (waretype == WARE_SOFT_DEFENSE)   invtype = MFD_INV_SOFT_DEFENSE;
//      else                                      invtype = MFD_INV_SOFT_MISC;

   }
   if (ware_sfx!=SFX_NONE)
   {
      extern char secret_global_pan;
      int ci_idx=wares[num].sideicon;
      secret_global_pan=(ci_idx==SI_NONE)?SND_DEF_PAN:(ci_idx<5)?5:122;
      hnd = play_digi_fx(ware_sfx,1);
	 secret_global_pan=SND_DEF_PAN;
   }
   if (waretype == WARE_HARD) 
   {
      short newe = player_struct.energy_spend;
      if (WareActive(player_status[num]))
         newe = min(newe+ecost,MAX_ENERGY);
      else
         newe = max(newe-ecost,0);
      set_player_energy_spend((ubyte)newe);
   }
   if (_current_loop <= FULLSCREEN_LOOP) 
      chg_set_flg(INVENTORY_UPDATE);
   mfd_notify_func(NOTIFY_ANY_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,TRUE);
   return;
}


// Hey, we're closing down a game. return to normalcy. 
void hardware_closedown(bool visible)
{
   int i;
   for (i = 0; i < NUM_HARDWAREZ; i++)
   {
//      if (i != HARDWARE_FULLSCREEN)
         if (WareActive(player_struct.hardwarez_status[i]))
            if (HardWare[i].turnoff != NULL)
               HardWare[i].turnoff(visible,FALSE);
   }
}

void hardware_startup(bool visible)
{
   int i;
   for (i = 0; i < NUM_HARDWAREZ; i++)
   {
//      if (i != HARDWARE_FULLSCREEN)
         if (WareActive(player_struct.hardwarez_status[i]))
            if (HardWare[i].turnon != NULL)
               HardWare[i].turnon(visible,FALSE);
   }
}

void hardware_power_outage(void)
{
   int i;
   for (i = 0; i < NUM_HARDWAREZ; i++)
   {
      if (energy_cost(i) > 0 && WareActive(player_struct.hardwarez_status[i]))
         use_ware(WARE_HARD,i);          
   }
   if (WareActive(player_struct.hardwarez_status[CPTRIP(JET_HARD_TRIPLE)]))
      use_ware(WARE_HARD,CPTRIP(JET_HARD_TRIPLE));
}


// ---------------------------------------------------------------------------
// get_ware_pointers()
//
// Sets a number of pointers to point at the appropriate ware structures
// for a given type. 

void get_ware_pointers(int type, ubyte **player_wares, ubyte **player_status,
                       WARE **wares, int *n)
{
   if (type == WARE_HARD)
      {
         *n             = NUM_HARDWAREZ;
         *player_wares  = player_struct.hardwarez;
         *player_status = player_struct.hardwarez_status;
         *wares         = HardWare;
      }

   else if (type == WARE_SOFT_COMBAT)
      {
         *n             = NUM_COMBAT_SOFTS;
         *player_wares  = player_struct.softs.combat;
         *player_status = player_struct.softs_status.combat;
         *wares         = Combat_SoftWare;
      }
   else if (type == WARE_SOFT_DEFENSE)
      {
         *n             = NUM_DEFENSE_SOFTS;
         *player_wares  = player_struct.softs.defense;
         *player_status = player_struct.softs_status.defense;
         *wares         = Defense_SoftWare;
      }
   else
      {        
         *n             = NUM_MISC_SOFTS;
         *player_wares  = player_struct.softs.misc;
         *player_status = player_struct.softs_status.misc;
         *wares         = Misc_SoftWare;
      }

   return;
}


// --------------------------------------------------
//
// get_player_ware_version returns the version number 
// of a ware in the player's inventory.  zero means 
// the player doesn't have it. 

int get_player_ware_version(int type, int n)
{
   WARE *Pwares;
   ubyte *pver;
   ubyte *pstat;
   int   num;
   get_ware_pointers(type,&pver,&pstat,&Pwares,&num);
   return pver[n];
}


// ---------------------------------------------------------------------------
// wares_update()
//
// Called from the main loop, this routine cycles through all active wares
// and sees if any need attention.

void wares_update()
{
   ubyte  *player_status, *player_wares;
   WARE   *wares;
   int    i, j, n;

   if ((player_struct.game_time - player_struct.last_ware_update) >= WARE_UPDATE_FREQ) {

      player_struct.last_ware_update = player_struct.game_time;

      // Iterate through all types of wares...

      // NOTE: At some point, we may want to differentiate here between
      // wares that get updated in the cyberloop as opposed to the
      // real world.  For now, we leave it all mashed together.

      for (i = 0; i < NUM_WARE_TYPES; i++) {

         get_ware_pointers(i, &player_wares, &player_status, &wares, &n);

         // Now we know what type of ware we're looking at.
         // Look at all wares of this type.

         for (j = 0; j < n; j++) {

            // Is it active? If so...
            if (WareActive(player_status[j])) {

               // Does it have a continually active effect?
               if (wares[j].effect) wares[j].effect();

            }
         }

         // We're ready to look at the next type of ware.
      }

   }
   for (j = 0; j < NUM_HARDWAREZ; j++)
      if (player_struct.hardwarez_status[j] & WARE_FLASH)
         side_icon_expose(HardWare[j].sideicon);

   return;
}



// ---------------------------------------------------------------------------
// wares_init()
//
// Sets the static values for all wares.

void wares_init()
{
   return;
}

// CALLBACKS
// =========

// ---------------------------------------------------------------------------
// wares_dummy_func()
//
// Temporary dummy function for all wares callbacks.

//void wares_dummy_func()
//{
//   return;
//}



// ----------
// * BIO WARE
// ----------
void bioware_turnon(bool visible, bool real_start);
void bioware_turnoff(bool visible, bool real_stop);
void bioware_effect(void);

// ---------------------------------------------------------------------------
// bioware_turnon()
//
// Let the MFD system know that the bioware is active, and take over
// the appropriate info MFD

void bioware_turnon(bool visible, bool)
{
   int i;
   if (visible)
   {
      mfd_notify_func(MFD_BIOWARE_FUNC, MFD_INFO_SLOT, TRUE, MFD_FLASH, TRUE);
      i =  mfd_grab_func(MFD_BIOWARE_FUNC,MFD_INFO_SLOT);
      mfd_change_slot(i,MFD_INFO_SLOT);
   }

   return;
}

// ---------------------------------------------------------------------------
// bioware_turnoff()
//
// Let the MFD system know that the bioware is deactivated, and toss it off
// the info slot, replacing it with a blank.

void bioware_turnoff(bool, bool real_stop)
{
   if (real_stop && player_struct.mfd_all_slots[MFD_INFO_SLOT] == MFD_BIOWARE_FUNC)
      mfd_notify_func(MFD_EMPTY_FUNC, MFD_INFO_SLOT, TRUE, MFD_EMPTY, TRUE);

   return;
}

// ---------------------------------------------------
// bioware_effect()
//
// updates the mfd.

void bioware_effect(void)
{
   mfd_notify_func(MFD_BIOWARE_FUNC,MFD_INFO_SLOT,FALSE,MFD_ACTIVE,FALSE);
}


// ---------------
// * INFRARED WARE
// ---------------
void infrared_turnon(bool visible, bool real_start);
void infrared_turnoff(bool visible, bool real_start);

extern char curr_clut_table;
// ---------------------------------------------------------------------------
// infrared_turnon()
//
// Turns on the infrared ware
void infrared_turnon(bool visible, bool)
{
   if (visible)
   {
      gr_set_light_tab(bw_shading_table);
      curr_clut_table = 1;
      chg_set_flg(_current_3d_flag);
      hud_set(HUD_INFRARED);
   }
   return;
}

// ---------------------------------------------------------------------------
// infrared_turnoff()
//
// Turns off the infrared ware

void infrared_turnoff(bool visible, bool)
{
   if (visible)
   {
      gr_set_light_tab(shading_table);
      chg_set_flg(_current_3d_flag);
      curr_clut_table = 0;

      hud_unset(HUD_INFRARED);
   }
   return;
}



// --------------------
// * TARGETING WARE
// --------------------
void targeting_turnon(bool visible, bool real_start);
void targeting_turnoff(bool visible, bool real_start);

// ---------------------------------------------------------------------------
// targeting_turnon()
//
// Turn on the targeting ware

void targeting_turnon(bool visible, bool real_start)
{
   extern void select_closest_target();

   player_struct.hardwarez_status[CPTRIP(TARG_GOG_TRIPLE)] &= ~WARE_ON;
   if (visible && real_start)
   {
      if (player_struct.curr_target == OBJ_NULL)
         select_closest_target();
      mfd_change_slot(mfd_grab_func(MFD_TARGET_FUNC,MFD_TARGET_SLOT),MFD_TARGET_SLOT);
   }
   return;
}

// ---------------------------------------------------------------------------
// targeting_turnoff()
//
// Turn off the targeting ware

void targeting_turnoff(bool, bool)
{
   return;
}


// -------------------------------------------------------
//  LANTERN WARE
// ---------------
void lamp_set_vals(void);
void lamp_set_vals_with_offset(byte offset);
void lamp_turnon(bool visible, bool real_start);
void lamp_change_setting(byte offset);
void lamp_turnoff(bool visible, bool real_stop);
bool lantern_change_setting_hkey(short keycode, ulong context, void* data);

struct _lampspec
{
   int rad1;
   int base1;
   int rad2;
   int base2;
   fix slope;
   fix yint;
}
lamp_specs[] =
{
   {  0,10,5,0,-2*FIX_UNIT,12*FIX_UNIT},
   {  1,20,6,0,-4*FIX_UNIT,24*FIX_UNIT},
   {  1,18,7,0,-3*FIX_UNIT,21*FIX_UNIT},
   {  1,14,8,0,-2*FIX_UNIT,16*FIX_UNIT}
};

// other oldest lowest value
//   {  0,15,5,0,-3*FIX_UNIT,18*FIX_UNIT},   
// old lowest value (old 0)
//   {  0,8,4,0,-2*FIX_UNIT,8*FIX_UNIT},
// is level 3 above ever used?

extern bool muzzle_fire_light;

void lamp_set_vals(void)
{
   lamp_set_vals_with_offset(0);
}

#define OFF_SHF 3
void lamp_set_vals_with_offset(byte offset)
{
   int n = IDX_OF_TYPE(WARE_HARD,LANTERN_HARD_TRIPLE), s;
   struct _lampspec *lspec;

   s = (muzzle_fire_light) ? LAMP_SETTING(player_struct.light_value) : LAMP_SETTING(player_struct.hardwarez_status[n]);
   lspec = &lamp_specs[s];

   _frp.lighting.yint    = lspec->yint + (offset<<(16-OFF_SHF));
   _frp.lighting.slope   = lspec->slope;
   _frp.lighting.rad[0]  = (uchar)lspec->rad1;
   _frp.lighting.base[0] = (uchar) ((lspec->base1+(offset>>OFF_SHF) > 0) ? (lspec->base1+(offset>>OFF_SHF)) : 0);
   if (offset!=0)
   {
      fix slope_based_mod;
      slope_based_mod=fix_div((offset<<(16-OFF_SHF)),-lspec->slope);
	   _frp.lighting.rad[1] = (uchar)(lspec->rad2+(slope_based_mod>>16));
//      slope_based_mod=fix_mul((offset<<(16-OFF_SHF)),-lspec->slope);
      _frp.lighting.yint  += -lspec->slope; 
      _frp.lighting.rad[0]++;
   }
   else _frp.lighting.rad[1] = (uchar)lspec->rad2;
   _frp.lighting.base[1] = (uchar)lspec->base2;

   chg_set_flg(_current_3d_flag);
//   Warning(("New parms %x %x, %x %x, line %x %x from %x %x\n",
//      _frp.lighting.rad[0], _frp.lighting.base[0],
//      _frp.lighting.rad[1], _frp.lighting.base[1],
//      _frp.lighting.yint,   _frp.lighting.slope,offset,s));
}

void lamp_turnon(bool visible, bool)
{
   lamp_set_vals();
   if (visible)
   {
      _frp_light_bits_set(LIGHT_BITS_CAM);
      mfd_notify_func(MFD_LANTERN_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, FALSE);
   }
}

void lamp_change_setting(byte offset)
{
   lamp_set_vals_with_offset(offset);
   _frp_light_bits_set(LIGHT_BITS_CAM);
}

void lamp_turnoff(bool visible, bool real_stop)
{
   if (visible)
   {
      _frp_light_bits_clear(LIGHT_BITS_CAM);
      chg_set_flg(_current_3d_flag);
      if (real_stop)
         mfd_notify_func(MFD_LANTERN_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, FALSE);
   }
}

bool lantern_change_setting_hkey(short, ulong, void*)
{
   int n=CPTRIP(LANTERN_HARD_TRIPLE);
   int v=player_struct.hardwarez[n];
   int s=player_struct.hardwarez_status[n];
   bool on=s&WARE_ON;
   void mfd_lantern_setting(int setting);

   s=LAMP_SETTING(s);
   if(s==0 && on) {
      use_ware(WARE_HARD,n);
      mfd_notify_func(MFD_LANTERN_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, FALSE);
      return TRUE;
   }

   s=(s+v-1)%v; // decrement current setting
   mfd_lantern_setting(s);

   if(!on)
      use_ware(WARE_HARD,n);
   mfd_notify_func(MFD_LANTERN_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, FALSE);

   return TRUE;
}

//--------------------------
// SHIELD WARE
//-------------------------- 
void shield_set_absorb(void);
void shield_toggle(bool visible, bool real);
bool shield_change_setting_hkey(short keycode, ulong context, void* data);

#define SHIELD_IDX (CPTRIP(SHIELD_HARD_TRIPLE))

ubyte shield_absorb_rates[] = {20, 40, 75, 75}; 
ubyte shield_thresholds[] = {0, 10, 15, 30};

extern void set_shield_raisage(bool going_up);

void shield_set_absorb(void)
{
   ubyte s = player_struct.hardwarez_status[SHIELD_IDX];
   if (s & WARE_ON)
   {
      player_struct.shield_absorb_rate = shield_absorb_rates[LAMP_SETTING(s)];
      player_struct.shield_threshold = shield_thresholds[LAMP_SETTING(s)];
   }
   else
   {
      player_struct.shield_absorb_rate = 0;
      player_struct.shield_threshold = 0;
   }
}

void shield_toggle(bool, bool real)
{
   ubyte s = player_struct.hardwarez_status[SHIELD_IDX];
   if (real)
   {
      if (s & WARE_ON)
      {
         set_shield_raisage(TRUE);
         play_digi_fx(SFX_SHIELD_UP,1);
      }
      else
      {
         set_shield_raisage(FALSE);
         play_digi_fx(SFX_SHIELD_DOWN,1);
      }
      mfd_notify_func(MFD_SHIELD_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, FALSE);
   }
   shield_set_absorb();
}

bool shield_change_setting_hkey(short, ulong, void*)
{
   int n=CPTRIP(SHIELD_HARD_TRIPLE);
   int v=player_struct.hardwarez[n];
   int s=player_struct.hardwarez_status[n];
   bool on=s&WARE_ON;
   void mfd_shield_setting(int setting);

   // version 4 has only one setting.
   if(v==4) v=1;

   s=LAMP_SETTING(s);
   if(s==0 && on) {
      use_ware(WARE_HARD,n);
      mfd_notify_func(MFD_SHIELD_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, FALSE);
      return TRUE;
   }

   s=(s+v-1)%v; // decrement current setting
   mfd_shield_setting(s);

   if(!on)
      use_ware(WARE_HARD,n);
   mfd_notify_func(MFD_SHIELD_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, FALSE);

   return TRUE;
}


//----------------
// NAV WARE
//----------------
void nav_turnon(bool visible, bool real_start);
void nav_turnoff(bool visible, bool real_start);


void nav_turnon(bool visible, bool)
{
   if (visible)
      hud_set(HUD_COMPASS);
}

void nav_turnoff(bool visible, bool)
{
   if (visible)
   {
      hud_unset(HUD_COMPASS);
   }
}


//----------------------
// MOTION WARE
//---------------------- 
void motionware_update(bool visible,  bool real, bool on);
void motionware_turnon(bool visible, bool real);
void motionware_turnoff(bool visible, bool real);

ubyte motionware_mode = MOTION_INACTIVE;

#define MOTION_SETTING LAMP_SETTING

void motionware_update(bool visible,  bool, bool on)
{
   ubyte s = player_struct.hardwarez_status[CPTRIP(MOTION_HARD_TRIPLE)];
   if (on)
      motionware_mode = MOTION_SETTING(s)+1;
   else
      motionware_mode = MOTION_INACTIVE;
   if (visible)
   {
      Pelvis elvis;
      mfd_notify_func(MFD_MOTION_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, TRUE);
      EDMS_get_pelvis_parameters(PLAYER_PHYSICS,&elvis);
      switch(motionware_mode)
      {
         case MOTION_SKATES:
            if (!global_fullmap->cyber)
            {
               elvis.cyber_space = PELVIS_MODE_SKATES;
            }
            break;
         default:
            if (!global_fullmap->cyber)
            {
               elvis.cyber_space = PELVIS_MODE_NORMAL;
            }
            break;
      }
      EDMS_set_pelvis_parameters(PLAYER_PHYSICS,&elvis);
   }
   
}

void motionware_turnon(bool visible, bool real)
{
   motionware_update(visible, real, TRUE);
}

void motionware_turnoff(bool visible, bool real)
{
   motionware_update(visible, real, FALSE);
}


// ---------------------
//    JUMP JET WARE
// ---------------------
void activate_jumpjets(fix* xcntl, fix* ycntl, fix* zcntl);

static short jumpjet_controls[] = { -25, -50, -75};
static fix   jumpjet_thrust_scales[] = { FIX_UNIT/64, FIX_UNIT/32, FIX_UNIT/16};

bool jumpjets_active = FALSE;

// modifies z control based on jumpject ware. 
void activate_jumpjets(fix* xcntl, fix* ycntl, fix* zcntl)
{
   int ecost;
   short edrain;

   ubyte n = CPTRIP(JET_HARD_TRIPLE);
   ubyte v = player_struct.hardwarez[n];
   ubyte s = player_struct.hardwarez_status[n];
   
   jumpjets_active = FALSE;
   if ((s & WARE_ON) == 0 || player_struct.energy == 0)
      return;
   ecost = energy_cost_vec[n][v-1] * player_struct.deltat + player_struct.jumpjet_energy_fraction;
   player_struct.jumpjet_energy_fraction = ecost % APPROX_CIT_CYCLE_HZ;
   ecost /= APPROX_CIT_CYCLE_HZ;
   edrain = drain_energy(ecost);
   *zcntl = fix_make(jumpjet_controls[v-1],0);
   if (edrain < ecost)
      *zcntl = (*zcntl)*edrain/ecost;
   *ycntl = fix_mul(*ycntl,jumpjet_thrust_scales[v-1]);
   *xcntl = 0;
   jumpjets_active = TRUE;
}

//-----------------------
//   FULLSCREEN WARE
//-----------------------
void fullscreen_turnon(bool visible, bool real_start);
void fullscreen_turnoff(bool visible, bool real_start);
extern Boolean	DoubleSize;

void fullscreen_turnon(bool visible, bool)
{
	if (visible)
	{
		_new_mode = FULLSCREEN_LOOP;
		chg_set_flg(GL_CHG_LOOP);
	}
}

void fullscreen_turnoff(bool visible, bool)
{
	if (visible)
	{
		_new_mode = GAME_LOOP;
		chg_set_flg(GL_CHG_LOOP);
	}
}

//-----------------------
//   CYBERSPACE ONESHOTS
//-----------------------
void do_turbo_stuff(bool from_drug);
void turbo_turnon(bool visible, bool real_start);
void turbo_turnoff(bool visible, bool real_start);
void fakeid_turnon(bool visible, bool real_start);
void decoy_turnon(bool visible, bool real_start);
void decoy_turnoff(bool visible, bool real_stop);
void recall_turnon(bool visible, bool real_start);

void do_turbo_stuff(bool from_drug)
{
   if (cspace_effect_times[CS_TURBO_EFF] == 0)
   {
      if (from_drug)
      {
         hud_set(HUD_TURBO);
         chg_set_flg(INVENTORY_UPDATE);
      }
   }
}

void turbo_turnon(bool visible, bool real_start)
{
   ulong hammer_time = cspace_effect_durations[CS_TURBO_EFF];
   do_turbo_stuff(visible);
   if (real_start)
   {
      play_digi_fx(SFX_TURBO,1);
      player_struct.softs.misc[SOFTWARE_TURBO]--;
      cspace_effect_times[CS_TURBO_EFF] = player_struct.game_time + hammer_time;
   }
}   

void turbo_turnoff(bool visible, bool)
{
   if (visible)
   {
      hud_unset(HUD_TURBO);
   }
   cspace_effect_times[CS_TURBO_EFF] = 0;
}

void fakeid_turnon(bool visible, bool real_start)
{
   if (!(player_struct.hud_modes & HUD_FAKEID) && visible)
   {
      if (real_start)
      {
         player_struct.softs.misc[SOFTWARE_FAKEID]--;
         play_digi_fx(SFX_FAKEID,1);
      }
      hud_set(HUD_FAKEID);
      chg_set_flg(INVENTORY_UPDATE);
   }
}   

void decoy_turnon(bool, bool real_start)
{
   if (real_start)
   {
      if (cspace_decoy_obj != OBJ_NULL)
         decoy_turnoff(TRUE,TRUE);
      cspace_decoy_obj = obj_create_base(TARGET_TRIPLE);
      if (cspace_decoy_obj != OBJ_NULL)
      {
         obj_move_to(cspace_decoy_obj, &objs[PLAYER_OBJ].loc, FALSE);
         cspace_effect_times[CS_DECOY_EFF] = player_struct.game_time + cspace_effect_durations[CS_DECOY_EFF];
         if (real_start)
         {
            player_struct.softs.misc[SOFTWARE_DECOY]--;
            play_digi_fx(SFX_DECOY,1);
         }
         hud_set(HUD_DECOY);
         chg_set_flg(INVENTORY_UPDATE);
      }
   }
}   

void decoy_turnoff(bool visible, bool real_stop)
{
   if (visible)
   {
      hud_unset(HUD_DECOY);
   }
   cspace_effect_times[CS_DECOY_EFF] = 0;
   if (real_stop)
   {
      if (cspace_decoy_obj != OBJ_NULL)
         ADD_DESTROYED_OBJECT(cspace_decoy_obj);
   }
   cspace_decoy_obj = OBJ_NULL;
}

void recall_turnon(bool visible, bool real_start)
{
   if (visible && real_start)
   {
      player_struct.softs.misc[SOFTWARE_RECALL]--;
      obj_move_to(PLAYER_OBJ, &recall_objloc, TRUE);
      chg_set_flg(INVENTORY_UPDATE);
      play_digi_fx(SFX_RECALL,1);
   }
}   


// =================
// THE STATIC ARRAYS

extern void view360_turnon(bool visible, bool real_start),view360_turnoff(bool visible, bool real_start);
extern bool view360_check();
extern void email_turnon(bool visible, bool real_start);
extern void email_turnoff(bool visible, bool real_start);
extern void plotware_turnon(bool visible, bool real_start);

WARE HardWare[NUM_HARDWAREZ] =
{             
//"infrared"   
   { WARE_FLAGS_NONE, SI_SIXTH, infrared_turnon, NULL, infrared_turnoff, NULL},
//"target info"
   { WARE_FLAGS_NONE, SI_NONE, targeting_turnon, NULL, targeting_turnoff, NULL},
//"360 view"   
   { WARE_FLAGS_NONE, SI_THIRD, view360_turnon, NULL, view360_turnoff, view360_check },
//"aim"        
   { WARE_FLAGS_NONE, SI_NONE, NULL, NULL, NULL,NULL },
//"HUD"        
   { WARE_FLAGS_NONE, SI_NONE, NULL, NULL, NULL, NULL},
//"bioscan"    
   { WARE_FLAGS_NONE, SI_FIRST, bioware_turnon, bioware_effect, bioware_turnoff , NULL},
//"nav unit"   
   { WARE_FLAGS_NONE, SI_SEVENTH, nav_turnon, NULL, nav_turnoff , NULL},
//"shield"     
   { WARE_FLAGS_NONE, SI_FIFTH, shield_toggle, NULL, shield_toggle , NULL},
//"data reader"
   { WARE_FLAGS_NONE, SI_EIGHTH, email_turnon, NULL, email_turnoff , NULL},
//"lantern"    
   { WARE_FLAGS_NONE, SI_FOURTH, lamp_turnon, NULL, lamp_turnoff , NULL},
//"fullscreen"  
   { WARE_FLAGS_NONE, SI_SECOND, fullscreen_turnon, NULL, fullscreen_turnoff , NULL},
//"enviro-suit"
   { WARE_FLAGS_NONE, SI_NONE, NULL, NULL, NULL , NULL},
//"motion"    
   { WARE_FLAGS_NONE, SI_NINTH, motionware_turnon, NULL, motionware_turnoff , NULL},
//"skates"     
   { WARE_FLAGS_NONE, SI_TENTH, NULL, NULL, NULL , NULL},
//"status"     
   { WARE_FLAGS_NONE, SI_NONE, plotware_turnon, NULL, NULL , NULL},
};


// Except for jumpjets, these costs are in points per 
// minute of use.   

short energy_cost_vec[NUM_HARDWAREZ][MAX_VERSIONS] =
{
//"infrared"   
   { 50, 50, 50, }, 
//"target info"
   { 0, },
//"360 view"   
   { 9, 9, 9 },
//"aim"        
   { 0, },
//"HUD"        
   { 0, }, 
//"bioscan"    
   { 1,},
//"nav unit"   
   { 0, }, 
//"shield"     
   { 24, 60, 105, 30}, 
//"data reader"
   { 0, },
//"lantern"    
//   { 15, 25, 30, },
   { 10, 25, 30, },
//"robo comm"  
   { 0, },
//"enviro-suit"
   { 0, },
//"motion"    
   { 0, 40, },
//"jumpjets"     
   // these are in points per second of thrust
   { 25, 30, 35 },
//"status"     
   { 0 },
};


bool check_game(void)
{
   return (!global_fullmap->cyber);
}
extern void mfd_games_turnon(bool,bool), mfd_games_turnoff(bool,bool);


WARE Combat_SoftWare[NUM_COMBAT_SOFTS];
WARE Defense_SoftWare[NUM_DEFENSE_SOFTS];
WARE Misc_SoftWare[NUM_MISC_SOFTS] =
{
   { WARE_FLAGS_NONE, SI_NONE, turbo_turnon, NULL, NULL, NULL},
   { WARE_FLAGS_NONE, SI_NONE, fakeid_turnon, NULL, NULL, NULL},
   { WARE_FLAGS_NONE, SI_NONE, decoy_turnon, NULL, decoy_turnoff, NULL},
   { WARE_FLAGS_NONE, SI_NONE, recall_turnon, NULL, NULL, NULL},
   { WARE_FLAGS_NONE, SI_NONE, mfd_games_turnon, NULL, mfd_games_turnoff, check_game},
};
