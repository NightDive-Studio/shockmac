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
 * $Source: r:/prj/cit/src/inc/RCS/bark.h $
 * $Revision: 1.3 $
 * $Author: tjs $
 * $Date: 1994/08/02 13:56:04 $
 *
 * $Log: bark.h $
 * Revision 1.3  1994/08/02  13:56:04  tjs
 * Rescaled bark timeouts.
 * 
 * Revision 1.2  1994/05/26  20:52:23  tjs
 * Added null bark timeout
 * 
 * Revision 1.1  1994/05/26  17:59:08  tjs
 * Initial revision
 * 
 * 
 *
 */

#include "mfdint.h" // for MFD struct

#define MFD_BARK_FUNC 17

#define NULL_BARK_TIMEOUT 3

#define mfd_bark_string (*(int*)(&player_struct.mfd_func_data[MFD_BARK_FUNC][0]))
#define mfd_bark_speaker (*(ObjID*)(&player_struct.mfd_func_data[MFD_BARK_FUNC][sizeof(int)]))
#define mfd_bark_color  (player_struct.mfd_func_data[MFD_BARK_FUNC][sizeof(int)+sizeof(ObjID)])
#define mfd_bark_mug (player_struct.mfd_func_data[MFD_BARK_FUNC][sizeof(int)+sizeof(ObjID)+sizeof(uchar)])

extern void mfd_bark_expose(MFD* mfd, ubyte control);
extern void long_bark(ObjID speaker_id, uchar mug_id, int string_id, ubyte color);

typedef struct {
   ushort timestamp;
   ushort type;
} BarkSchedEvent;
