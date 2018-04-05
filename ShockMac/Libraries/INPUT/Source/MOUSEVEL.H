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
#ifndef _MOUSEVEL_H
#define _MOUSEVEL_H
/*
 * $Source: n:/project/lib/src/input/RCS/mousevel.h $
 * $Revision: 1.1 $
 * $Author: mahk $
 * $Date: 1994/06/21 05:29:41 $
 *
 */

// --------------------------------------------------------------
//                MOUSE-LIBRARY SUPPORT FOR MOUSE 
//              EMULATION BY VELOCITY-BASED DEVICES 
// --------------------------------------------------------------




/* ------------------------------------------------------
   These routines allow the client to set a _velocity_ for the mouse
   pointer.   The mouse pointer will move at the specified velicity 
   whenever the mouse's current position is queried.   (Queries include
   mouse_get_xy (always) and queue checks (when the queue is empty).)
   Mouse velocity is specified in units of pixels per MOUSE_VEL_UNIT ticks,
   where ticks are the unit of the mouse library timestamp register.
   These routines are intended for use in emulating the mouse with other
   non-positional devices.  (keyboard, joystick, cyberbat, etc.) 
   ------------------------------------------------------ */


#define MOUSE_VEL_UNIT_SHF 16
#define MOUSE_VEL_UNIT (1 << MOUSE_VEL_UNIT_SHF)


errtype mouse_set_velocity_range(int xl, int yl, int xh, int yh);
// Specifies the range of the mouse pointer velocity.  
// (xl,yl) is the low end of the range, whereas (xh,yh) 
// is the high end.  For most applications xl and xh will have the same 
// absolute value, as with yl and yh. 

errtype mouse_set_velocity(int x, int y);
// Sets the velocity of the mouse pointer, 
// in units of pixels per MOUSE_VEL_UNIT ticks. 

errtype mouse_add_velocity(int x, int y);
// Adds x and y to the mouse pointer velocity, constraining it 
// to remain withing the range specified by mouse_set_velocity_range() 

errtype mouse_get_velocity(int *x, int *y);
// Gets the current value of the mouse pointer velocity, as 
// set by mouse_set_velocity and mouse_add_velocity.


#endif // _MOUSEVEL_H
