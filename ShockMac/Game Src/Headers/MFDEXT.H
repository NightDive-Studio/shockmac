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
#ifndef __MFDEXT_H
#define __MFDEXT_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/mfdext.h $
 * $Revision: 1.21 $
 * $Author: mahk $
 * $Date: 1994/08/28 04:41:22 $
 *
 *
 */

// Includes
#include "player.h"

// ---------
// Constants
// ---------

// These are for the inventory system (invent.c)
#define MFD_INV_NULL           0
#define MFD_INV_DRUG           1
#define MFD_INV_HARDWARE       2
#define MFD_INV_GRENADE        3
#define MFD_INV_AMMO           4
#define MFD_INV_WEAPON         5
#define MFD_INV_GENINV         6
#define MFD_INV_SOFT_COMBAT    7
#define MFD_INV_SOFT_DEFENSE   8
#define MFD_INV_SOFT_MISC      9
#define MFD_INV_CATEGORIES     10

#define MFD_INV_NOTYPE        0xFF

// Which MFD?
#define MFD_LEFT         0 
#define MFD_RIGHT        1

// Flags for MFD Functions
#define MFD_CHANGEBIT      0x01         // Needs constant update of some sort
#define MFD_INCREMENTAL    0x02         // Uses standard MFD background
#define MFD_CHANGEBIT_FULL 0x04
#define MFD_NOSAVEREST     0x08        // don't save/restore me.

// Flags for Expose Control
#define MFD_EXPOSE       0x01
#define MFD_EXPOSE_FULL  0x02

// Type of each MFD slot
#define MFD_WEAPON_SLOT  0
#define MFD_ITEM_SLOT    1
#define MFD_MAP_SLOT     2
#define MFD_INFO_SLOT    4
#define MFD_TARGET_SLOT  3
#define MFD_SPECIAL_SLOT 5

// MFD Functions
#define NOTIFY_ANY_FUNC    0xFF  // for mfd_notify_func
#define MFD_EMPTY_FUNC     0
#define MFD_ITEM_FUNC      1
#define MFD_MAP_FUNC       2
#define MFD_TARGET_FUNC    3
#define MFD_ANIM_FUNC      4
#define MFD_WEAPON_FUNC    5 
#define MFD_BIOWARE_FUNC   6
#define MFD_LANTERN_FUNC   7
#define MFD_3DVIEW_FUNC    8
#define MFD_ELEV_FUNC      9 
#define MFD_GRENADE_FUNC   10
#define MFD_HUD_FUNC       11
#define MFD_FIXTURE_FUNC   12
#define MFD_KEYPAD_FUNC    13
#define MFD_EMAILMUG_FUNC  14
#define MFD_EMAILWARE_FUNC 15
#define MFD_PLOTWARE_FUNC  16
#define MFD_BARK_FUNC      17
#define MFD_ACCESSPANEL_FUNC      18
#define MFD_SHIELD_FUNC    19
#define MFD_MOTION_FUNC    20
#define MFD_SEVERED_HEAD_FUNC 21
#define MFD_TARGETWARE_FUNC 22
#define MFD_GUMP_FUNC      23
#define MFD_CARD_FUNC      24
#define MFD_BIOHELP_FUNC   25
#define MFD_GRIDPANEL_FUNC 26
#define MFD_GAMES_FUNC     27
#define MFD_CSPACE_FUNC    28
#define MFD_VIEWHELP_FUNC  29
#define MFD_GEAR_FUNC      30
      
// -------
// Externs
// -------

extern void set_inventory_mfd(ubyte l_class, ubyte type,bool grab);
extern void init_newmfd();
extern void screen_init_mfd(bool fullscrn);
extern void screen_init_mfd_draw();
extern void keyboard_init_mfd();
extern void mfd_update();
extern void mfd_notify_func(ubyte func, ubyte slot, bool grab, MFD_Status stat, bool FullRedraw);
extern void mfd_force_update_single(int which_mfd);
extern void mfd_force_update();
extern int mfd_grab(void);
extern int mfd_grab_func(int my_func, int my_slot);
extern bool mfd_yield_func(int func, int* mfd_id);
extern void mfd_change_slot(ubyte mfd_id, ubyte l_new);
extern void save_mfd_slot(int mfd_id);
extern void restore_mfd_slot(int mfd_id);

#endif // __MFDEXT_H

