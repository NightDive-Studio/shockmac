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
//===============================================================
//
// kbMac.c - All the keyboard handling routines that are specific to the Macintosh.
//
//===============================================================

#include <string.h>
#include <Timer.h>
#include "lg.h"
#include "kb.h"
#include "kbglob.h"


//------------------
//  Globals
//------------------
int 			pKbdStatusFlags;
uchar		pKbdGetKeys[16];


//---------------------------------------------------------------
//  Startup and keyboard handlers and initialize globals.   Shutdown follows.
//---------------------------------------------------------------
int kb_startup(void *)
{
	pKbdStatusFlags = 0;
	
	LG_memset(pKbdGetKeys, 0, 16);								// Zero out the GetKeys buffer

	return (0);
}

int kb_shutdown(void)
{
	return (0);
}

//---------------------------------------------------------------
//  Get and set the global flags.
//---------------------------------------------------------------
int kb_get_flags()
{
	return (pKbdStatusFlags);
}

void kb_set_flags(int flags)
{
	pKbdStatusFlags = flags;
}

//---------------------------------------------------------------
//  Get the next available key from the event queue.
//---------------------------------------------------------------
kbs_event kb_next(void)
{
	int				flags = kb_get_flags();
	bool				gotKey = FALSE;
	EventRecord	theEvent;
	kbs_event		retEvent = { 0xFF, 0x00 };

	while(!gotKey)
	{
		gotKey = GetOSEvent(keyDownMask | autoKeyMask, &theEvent);		// Get a key
		if (gotKey)
		{
			retEvent.code = (uchar)(theEvent.message >> 8);
			retEvent.state = KBS_DOWN;
			retEvent.ascii = (uchar)(theEvent.message & charCodeMask);
			retEvent.modifiers = (uchar)(theEvent.modifiers >> 8);
		}
		else if ((flags & KBF_BLOCK) == 0)					// If there was no key and we're
			return (retEvent);										// not blocking, then return.
	}
	return (retEvent);
}

//---------------------------------------------------------------
//  See if there is a key waiting in the queue.
//---------------------------------------------------------------
kbs_event kb_look_next(void)
{
	int				flags = kb_get_flags();
	bool				gotKey = FALSE;
	EventRecord	theEvent;
	kbs_event		retEvent = { 0xFF, 0x00 };

	while(!gotKey)
	{
		gotKey = OSEventAvail(keyDownMask | autoKeyMask, &theEvent);		// Get a key
		if (gotKey)
		{
			retEvent.code = (uchar)(theEvent.message >> 8);
			retEvent.state = KBS_DOWN;
			retEvent.ascii = (uchar)(theEvent.message & charCodeMask);
			retEvent.modifiers = (uchar)(theEvent.modifiers >> 8);
		}
		else if (flags & KBF_BLOCK == 0)					// If there was no key and we're
			return (retEvent);										// not blocking, then return.
	}
	return (retEvent);
}


//---------------------------------------------------------------
//  Flush keyboard events from the event queue.
//---------------------------------------------------------------
void kb_flush(void)
{
	FlushEvents(keyDownMask | autoKeyMask, 0);
}


//---------------------------------------------------------------
//  Return the state of the indicated key (scan code).
//---------------------------------------------------------------
uchar kb_state(uchar code)
{
	GetKeys((UInt32 *) pKbdGetKeys);
	return ((pKbdGetKeys[code>>3] >> (code & 7)) & 1);
}
