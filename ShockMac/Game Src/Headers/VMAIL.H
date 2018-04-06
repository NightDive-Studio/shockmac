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
#ifndef __VMAIL_H
#define __VMAIL_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/vmail.h $
 * $Revision: 1.2 $
 * $Author: minman $
 * $Date: 1994/09/05 06:43:57 $
 *
 * $Log: vmail.h $
 * Revision 1.2  1994/09/05  06:43:57  minman
 * got rid of clear_vmail!
 * 
 * Revision 1.1  1994/01/20  03:00:03  minman
 * Initial revision
 * 
 *
 */

// Includes

#define SHIELD_VMAIL    0
#define GROVE_VMAIL     1
#define BRIDGE_VMAIL    2

#define VINTRO_X  (SCREEN_VIEW_X+32)
#define VINTRO_Y  (SCREEN_VIEW_Y+6)

#define VINTRO_W  200
#define VINTRO_H  100

errtype play_vmail(byte vmail_no);

#endif // __VMAIL_H
