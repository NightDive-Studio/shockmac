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
//
// $Source: n:/project/lib/src/2d/RCS/stastk.asm $
// $Revision: 1.1 $
// $Author: kaboom $
// $Date: 1993/05/16 00:43:46 $
//
// Graphics state push routine.
//
// This file is part of the 2d library.
//
// $Log: stastk.asm $
// Revision 1.3  1994/10/18  13:22:38  kevin
// renamed gr_{push,pop}_state to gr_{push,pop}_video_state.
//
// Revision 1.2  1994/10/13  10:34:38  ept
// Upped allocation in table to 2K from 1K so that can
// allocate up to two graphics states.  Also now checks
// if the get_state routine returns < 0 and if so just
// returns it.  So gr_push_state now returns an int.
//
// Revision 1.1  1993/05/16  00:43:46  kaboom
// Initial revision
// 

#include "grd.h"
#include "idevice.h"
#include "tabdat.h"
#include "state.h"

// globals
long	grd_state_stack[512];
char	*grd_state_stack_p = (char *) grd_state_stack;

int gr_push_video_state (int flags)
 {
 	long	bytes;
 	
	bytes = ((int (*)(void *buf,int flags)) grd_device_table[GRT_GET_STATE])((void *) grd_state_stack_p, flags);
	if (bytes<0) return(bytes);
	
	grd_state_stack_p += bytes;
	* (long *) grd_state_stack_p = bytes;
	grd_state_stack_p += 4L;
	
	return(bytes);
 }
 
void gr_pop_video_state (int clear)
 {
 	long	bytes;

	grd_state_stack_p -= 4L;
	bytes = * (long *) grd_state_stack_p;
	grd_state_stack_p -= bytes;
	
	((int (*)(void *buf,int clear))grd_device_table[GRT_SET_STATE])(grd_state_stack_p, clear);
 }
 
