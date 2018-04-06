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
#ifndef __HUD_H
#define __HUD_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/hud.h $
 * $Revision: 1.23 $
 * $Author: mahk $
 * $Date: 1994/08/11 18:31:34 $
 *
 *
 */

// Includes
#include "frtypesx.h"

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes


// Defines
#define HUD_RADIATION   0x00000001 // Are we in radiation
#define HUD_BIOHAZARD   0x00000002 // Are we in a bio area
#define HUD_RANGE       0x00000004 // Range to target
#define HUD_FATIGUE     0x00000008 // High fatigue levels
#define HUD_TARGRECT    0x00000010 // Rectangle around target
#define HUD_GRENADE     0x00000020 // Time to detonate grenade
#define HUD_INFRARED    0x00000040 // Infrared active
#define HUD_SHIELD      0x00000080 // Show shield absorption
#define HUD_SHODOMETER  0x00000100 // Changes in shodometer
#define HUD_DETECT_EXP  0x00000200 // Explosion detection
#define HUD_COMPASS     0x00000400 // Compass
#define HUD_ZEROGRAV    0x00000800 // abnormal gravity conditions
#define HUD_FAKEID      0x00001000 // fakeid software in use
#define HUD_DECOY       0x00002000 // decoy software in use
#define HUD_TURBO       0x00004000 // turbo software in use
#define HUD_CYBERTIME   0x00008000 // time remaining until SHODAN sends his avatar against you
#define HUD_CYBERDANGER 0x00010000 // imminent peril of being ejected from cspace
#define HUD_RADPOISON   0x00100000 // taking radiation damage
#define HUD_BIOPOISON   0x00200000 // taking bio damage
#define HUD_BEAMHOT     0x00400000 // weapon about to overheat
#define HUD_MSGLINE     0x00800000 // message line.
#define HUD_GAMETIME    0x01000000 // time remaining in game
#define HUD_ENERGYUSE   0x02000000 // energy usage notification
#define HUD_ENVIROUSE   0x04000000 // enviro suit drain/absorb
#define HUD_MESSAGE     0x80000000 // general hud bit for hud_message (NIY)

#define HUD_ALL         0xFFFFFFFF

#define HUD_COLOR_BANKS 3
#define HUD_COLORS_PER_BANK 5

// Prototypes

// Update the HUD.  If redraw_whole is TRUE, better draw the
// whole durned thing.
errtype hud_update(bool redraw_whole,frc* context);
errtype cyber_hud_update(bool redraw_whole);

// Set the data being displayed by the HUD.
errtype hud_set(ulong hud_modes);
errtype hud_unset(ulong hud_modes);
errtype hud_set_time(ulong hud_modes, ulong ticks);

// Globals
extern LGRect target_screen_rect;
extern ubyte hud_colors[HUD_COLOR_BANKS][HUD_COLORS_PER_BANK];
extern ubyte hud_color_bank;




#endif // __HUD_H

