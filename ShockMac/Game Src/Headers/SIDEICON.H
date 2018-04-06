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
#ifndef __SIDEICON_H
#define __SIDEICON_H

/*
 * $Source: n:/project/cit/src/inc/RCS/sideicon.h $
 * $Revision: 1.7 $
 * $Author: mahk $
 * $Date: 1994/03/04 07:05:11 $
 *
 * $Log: sideicon.h $
 * Revision 1.7  1994/03/04  07:05:11  mahk
 * Full screen mania.
 * 
 * Revision 1.6  1993/09/15  00:28:47  xemu
 * use different LGRegion creation macro
 * 
 * Revision 1.5  1993/09/02  23:08:36  xemu
 * angle me baby
 * 
 * Revision 1.4  1993/08/22  17:53:28  spaz
 * added all SI_ numbers
 * 
 * Revision 1.3  1993/08/18  20:44:22  spaz
 * SI_SIXTH, for infrared button
 * 
 * Revision 1.2  1993/07/26  16:58:53  spaz
 * Threw in some #define's for side icon identification
 * 
 * Revision 1.1  1993/07/21  18:02:41  spaz
 * Initial revision
 * 
 *
 */

// Includes

// Defines

#define SI_NONE    0xff

#define SI_FIRST   0
#define SI_SECOND  1
#define SI_THIRD   2
#define SI_FOURTH  3
#define SI_FIFTH   4
#define SI_SIXTH   5
#define SI_SEVENTH 6
#define SI_EIGHTH  7
#define SI_NINTH   8
#define SI_TENTH   9

// Typedefs

// Prototypes

extern void init_side_icon(ubyte side_icon, int type, int num);
extern void side_icon_expose_all();
extern void side_icon_expose(ubyte side_icon);
extern void init_all_side_icons();
void init_side_icon_popups(void);
extern void screen_init_side_icons(LGRegion* root); 
errtype side_icon_load_bitmaps();
errtype side_icon_free_bitmaps();

// Globals

#define macro_region_create_with_autodestroy(parent,child,LGRect) \
   region_create(parent,child,LGRect,0,0,REG_USER_CONTROLLED|AUTODESTROY_FLAG,NULL,NULL,NULL,NULL)

#endif // __SIDEICON_H
