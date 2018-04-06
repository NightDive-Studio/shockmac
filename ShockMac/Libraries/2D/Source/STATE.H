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
 * $Source: r:/prj/lib/src/2d/RCS/state.h $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/10/18 13:22:39 $
 *
 * Declarations for video state push/pop.
 *
 * This file is part of the 2d library.
 *
 * $Log: state.h $
 * Revision 1.3  1994/10/18  13:22:39  kevin
 * renamed gr_{push,pop}_state to gr_{push,pop}_video_state.
 * 
 * Revision 1.2  1994/10/13  10:36:10  ept
 * Added several constants to match the inc file and also
 * made gr_push_state return an int for error checking
 * purposes.
 * 
 * Revision 1.1  1993/05/16  00:44:01  kaboom
 * Initial revision
 * 
 */

#ifndef STATE_H
#define STATE_H

#define GRD_STATE_GRAPHICS_OURS  0        // in house graphics mode
#define GRD_STATE_GRAPHICS_VGA   1        // VGA or VESA graphics moed
#define GRD_STATE_BIOS_TEXT      2        // VGA text mode
#define GRD_STATE_VESA_TEXT      3        // VESA text mode

#define GRD_STATE_DEF 0
#define GRD_STATE_PAL 1

int gr_push_video_state (int flags);
void gr_pop_video_state (int clear);

#endif

