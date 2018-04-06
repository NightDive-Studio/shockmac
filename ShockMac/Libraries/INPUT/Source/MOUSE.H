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
//		Mouse.H		Mouse library header file
//		MAHK Leblanc 2/19/93
/*
 * $Source: n:/project/lib/src/input/RCS/mouse.h $
 * $Revision: 1.11 $
 * $Author: unknown $
 * $Date: 1993/09/01 00:19:18 $
 *
 * $Log: mouse.h $
 * Revision 1.11  1993/09/01  00:19:18  unknown
 * Changed left-handedness api
 * 
 * Revision 1.10  1993/08/29  03:12:35  mahk
 * Added mousemask and lefty support.
 * 
 * Revision 1.9  1993/08/27  14:05:06  mahk
 * Added shift factors
 * 
 * Revision 1.8  1993/07/28  18:15:58  jak
 * Added mouse_extremes() function
 * 
 * Revision 1.7  1993/06/28  02:04:59  mahk
 * Bug fixes for the new regime
 * 
 * Revision 1.6  1993/06/27  22:17:32  mahk
 * Added timestamps and button state to the mouse event structure.
 * 
 * Revision 1.5  1993/05/04  14:34:27  mahk
 * mouse_init no longer takes a screen mode argument.
 * 
 * Revision 1.4  1993/04/14  12:08:46  mahk
 * Hey, I got my mouse ups and downs backwards.
 * 
 * Revision 1.3  1993/03/19  18:46:57  mahk
 * Added RCS header
 * 
 *
 */

#ifndef MOUSE_H           
#define MOUSE_H

#include "lg.h"
#include "error.h"

typedef struct _mouse_event
{
   short x;    				// position
   short y;
   uchar type; 				// Event mask, bits defined below
   ulong timestamp;
   uchar buttons;
   uchar modifiers;			// Added for Mac version
   char pad[5];  			// pad to sixteen bytes
} mouse_event;

#define MOUSE_MOTION    1	// Event mask bits
#define MOUSE_LDOWN     2
#define MOUSE_LUP       4
#define MOUSE_RDOWN     8
#define MOUSE_RUP      16
#define MOUSE_CDOWN    32
#define MOUSE_CUP      64


// Mask of events that are allowed into the queue. 
extern ubyte mouseMask;

// type of mouse interrupt callback func
typedef void (*mouse_callfunc)(mouse_event* e,void* data);


#define NUM_MOUSE_BTNS 3
#define MOUSE_LBUTTON 0
#define MOUSE_RBUTTON 1
#define MOUSE_CBUTTON 2

#define MOUSE_BTN2DOWN(num) (1 << (1+ 2*(num)))
#define MOUSE_BTN2UP(num) (1 << (2+ 2*(num)))

// Initialize the mouse, specifying screen size.   
errtype mouse_init(short xsize, short ysize);

// shutdown mouse system
errtype mouse_shutdown(void);

// Tell the mouse library where to get timestamps from.
errtype mouse_set_timestamp_register(ulong* tstamp);

// Get the current mouse timestamp
ulong mouse_get_time(void);

// Get the mouse position
errtype mouse_get_xy(short* x, short* y);

// Set the mouse position
errtype mouse_put_xy(short x, short y);

// Check the state of a mouse button
errtype mouse_check_btn(short button, bool* result);

// look at the next mouse event.
errtype mouse_look_next(mouse_event* result);

// get & pop the next mouse event
errtype mouse_next(mouse_event* result);

// Flush the mouse queue
errtype mouse_flush(void);

// Add an event to the back of the mouse queue
errtype mouse_generate(mouse_event e);

// Set up an interrupt callback
errtype mouse_set_callback(mouse_callfunc f, void* data, int* id);

// Remove an interrupt callback 
errtype mouse_unset_callback(int id);

// Constrain the mouse coordinates
errtype mouse_constrain_xy(short xl, short yl, short xh, short yh);

// Set the mouse rate and accelleration threshhold
errtype mouse_set_rate(short xr, short yr, short thold);

// Get the mouse rate and accelleration threshhold
errtype mouse_get_rate(short* xr, short* yr, short* thold);

// Sets the mouse coordinate bounds to (0,0) - (x-1,y-1), 
// and scales the current values of the mouse sensitivity accordingly.  
errtype mouse_set_screensize(short x, short y);

// Find the min and max "virtual" coordinates of the mouse position
errtype mouse_extremes( short *xmin, short *ymin, short *xmax, short *ymax );

// Sets mouse handedness (true for left-handed)
errtype mouse_set_lefty(bool lefty);


#endif // _MOUSE_H


