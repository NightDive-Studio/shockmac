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
#ifndef __GAMELOOP_H
#define __GAMELOOP_H

/*
 * $Source: n:/project/cit/src/inc/RCS/gameloop.h $
 * $Revision: 1.9 $
 * $Author: xemu $
 * $Date: 1993/09/18 00:16:39 $
 *
 * $Log: gameloop.h $
 * Revision 1.9  1993/09/18  00:16:39  xemu
 * FULLSCREEN_UPDATE
 * 
 * Revision 1.8  1993/09/02  23:07:40  xemu
 * angle me baby
 * 
 * Revision 1.7  1993/07/06  01:13:37  mahk
 * Added inventory stuff
 * 
 * Revision 1.6  1993/06/06  00:44:11  xemu
 * real rendering change flags
 * 
 * Revision 1.5  1993/06/03  17:49:54  minman
 * added anim update
 * 
 * Revision 1.4  1993/05/23  19:01:35  xemu
 * rmoved time flags
 * 
 * Revision 1.3  1993/05/18  15:19:00  xemu
 * new time constants
 * 
 * Revision 1.2  1993/05/14  15:48:40  xemu
 * change flags
 * 
 * Revision 1.1  1993/05/12  14:20:21  xemu
 * Initial revision
 * 
 *
 */

// Includes

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes


// Defines
#define VITALS_UPDATE      LL_CHG_BASE<<0
#define MFD_UPDATE         LL_CHG_BASE<<1
#define ANIM_UPDATE        LL_CHG_BASE<<2
#define DEMOVIEW_UPDATE    LL_CHG_BASE<<3
#define INVENTORY_UPDATE   LL_CHG_BASE<<4
#define FULLSCREEN_UPDATE  LL_CHG_BASE<<5

// Prototypes
void game_loop(void);

// Globals

#endif // __GAMELOOP_H




