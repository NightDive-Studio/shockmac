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
 * $Source: n:/project/lib/src/input/test/RCS/mousetst.c $
 * $Revision: 1.6 $
 * $Author: kaboom $
 * $Date: 1993/12/01 06:56:21 $
 */
// Hey, this is a goofy mouse test program

#include "lg.h"
#include "mouse.h"
#include <stdio.h>
#include <stdlib.h>

extern short mouseInstantX, mouseInstantY, mouseInstantButts;
int eventcount = 0;

void goofy_callback(mouse_event* e,void* data)
{
//   if (e->type == 1) return;
  eventcount++; 
}

void main()
{
	bool done = FALSE;
	int	 callbackid = -1;
	
	printf ("Click the mouse...\n\n");
	mouse_init(16,16);
	
	mouse_set_callback(goofy_callback,(void*)0x600F,&callbackid);
	while(!done)
	{
		short x,y;
		mouse_event e;
		errtype err = mouse_next(&e);
		
		if(err != OK && err != ERR_DUNDERFLOW)      
			printf ("Error %d from mouse_next",err);
		
		if (err == OK)
		{
			printf("Event: [%d](%d,%d)@%d %d\n",e.type,e.x,e.y,e.timestamp,e.buttons);
			eventcount++;
		}
		
		mouse_get_xy(&x,&y);
//		mouse_check_btn(0,&done);
if (eventcount > 3)
	done = TRUE;		
	}
	mouse_shutdown();
	printf("Event count = %d\n",eventcount);
}
