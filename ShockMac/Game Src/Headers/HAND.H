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
#ifndef __HAND_H
#define __HAND_H

/*
 * $Source: n:/project/cit/src/inc/RCS/hand.h $
 * $Revision: 1.4 $
 * $Author: minman $
 * $Date: 1994/01/24 07:26:03 $
 *
 * $Log: hand.h $
 * Revision 1.4  1994/01/24  07:26:03  minman
 * cleaned up code
 * 
 * Revision 1.3  1993/12/21  03:04:01  minman
 * get_handart asks for a mouse_y now
 * 
 * Revision 1.2  1993/11/09  02:56:51  xemu
 * added mouse_x to handart
 * 
 * Revision 1.1  1993/09/08  18:44:46  minman
 * Initial revision
 * 
 *
 */

// Includes
#include "objwpn.h"

extern ubyte weapon_to_handart[NUM_GUN];

Ref get_handart(int *x_offset, int *y_offset, short mouse_x, short mouse_y);
void notify_draw_handart(void);

#endif // __HAND_H
