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
#ifndef __WEAPONS_H
#define __WEAPONS_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/weapons.h $
 * $Revision: 1.40 $
 * $Author: minman $
 * $Date: 1994/09/06 02:46:08 $
 *
 */

// Includes
#include "objwpn.h"    // for GUN_SUBCLASS_BEAM
#include "player.h"

// Defines

#define MAX_HEAT             100
#define OVERHEAT_THRESHOLD    80
#define MINIMUM_OVERLOAD      30
#define WARM_THRESHOLD        45

#define MIN_ENERGY_USE        10

#define OVERLOAD_VALUE(X)     (X & 0x80)
#define OVERLOAD_SET(X)       (X |= 0x80)
#define OVERLOAD_RESET(X)     (X &= ~0x80)
#define BEAM_SETTING_VAL(X)   (X & 0x7F)
#define BEAM_SETTING(X, Y)    (OVERLOAD_VALUE(X) | (BEAM_SETTING_VAL(Y)))

// moved WEAPON_COOL_OFF_TIME to weapons.c for easier tweaking
#define MAX_WEAPON_TYPE       6
#define MAX_WEAPON_SUBTYPE    5

// slow projectile flags
#define PROJ_LIGHT_FLAG             0x01
#define PROJ_PRESERVE_WALL          0x02
#define PROJ_PRESERVE_HIT           0x04
#define PROJ_PRESERVE_PROJ_HIT      0x08

// extern char ammo_type_letters[];
#define AMMO_TYPE_LETTER(l) (get_temp_string(REF_STR_AmmoTypeLetters)[l])
#define is_energy_weapon(wtype) (wtype == GUN_SUBCLASS_BEAM)
#define is_handtohand_weapon(wtype) (wtype == GUN_SUBCLASS_HANDTOHAND)

#define AMMO_SUBCLASSES num_subclasses[CLASS_AMMO]

#define AMMOTYPE_SUBCLASS(X)   ((X) >= (AMMO_SUBCLASSES << 4) ? AMMO_SUBCLASSES-1 : (X) >> 4)
#define AMMOTYPE_TYPE(X)       ((X) & 0xF)

#define set_shield_rate(X) (player_struct.shield_absorb_rate = (X))
#define get_shield_rate(X) (player_struct.shield_absorb_rate)

#ifdef __WEAPONS_SRC
ubyte handart_show = 0;
ubyte handart_remainder = 0;
bool handart_fire = FALSE;
#else
extern ubyte handart_show;
extern ubyte handart_remainder;
extern bool handart_fire;
//extern handart_frame_info handart_info[NUM_HANDART_ANIM][HANDART_FRAMES];
extern ubyte weapon_to_handart[NUM_GUN];
#endif

// Prototypes
// Get the name of a weapon, given its type and subtype
char* get_weapon_name(int type, int subtype, char* buf);
char* get_weapon_long_name(int type, int subtype, char *buf);

// Get the fire rate of a weapon, given its type and subtype
#define weapon_fire_rate(WTYPE, SUBTYPE) (GunProps[CPTRIP(MAKETRIP(CLASS_GUN, WTYPE, SUBTYPE))].fire_rate)

// Fire the player's current weapon.  Pos is the cursor position,
// the routine will translate by itself into the x & y angles of 
// direction to fire, 
// in units of -100-+100 spanning the view cone. 
// pull is true if the  trigger was just pulled.
// returns - TRUE if player fired weapon
bool fire_player_weapon(LGPoint *pos,LGRegion *r,bool pull);

// Set the maximum charge on a beam weapon.  index is into player_struct.weapons,
// max_charge can't be higher than 100
void set_beam_weapon_max_charge(ubyte index, ubyte max_charge);

void get_available_ammo_type(int type, int subtype, int *num_ammo_types, ubyte *bitflag, int *ammo_subclass);
bool change_ammo_type(ubyte ammo_type);

// change_weapon() - changes the current selected weapon
void change_selected_weapon(int new_weapon);

// Called at a constant factor to blow off heat on energy weapons
void cool_off_beam_weapons();

// This routine is used to jerk the cursor around, as a result of
// poor accuracy, or ammo recoil
void randomize_cursor_pos(LGPoint *cpos, LGRegion *r, ubyte percentage);

// drain energy from central reservior, returns how much was actually drained
ubyte drain_energy(ubyte desired_energy);

// return triple of currently-selected weapon, or -1 for none such.
int current_weapon_trip(void);

bool gun_takes_ammo(int guntrip, int ammotrip);

// Globals

#endif // __WEAPONS_H

