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
#include <HEADER.h>

// Initializes the TNG 
errtype tng_GADGET_NAME_init(void *ui_data, TNG *ptng, TNGStyle *sty, int alignment, int min, int max, int value, int increm, LGPoint size)
{
}

// Deallocate all memory used by the TNG 
errtype tng_GADGET_NAME_destroy(TNG *ptng)
{
}

// Draw the specified parts (may be all) of the TNG at screen coordinates loc
// assumes all appropriate setup has already been done!
errtype tng_GADGET_NAME_2d_draw(TNG *ptng, ushort partmask, LGPoint loc)
{
}

// Fill in ppt with the size...
errtype tng_GADGET_NAME_size(TNG *ptng, LGPoint *ppt)
{
}

// Returns the current "value" of the TNG
int tng_GADGET_NAME_getvalue(TNG *ptng)
{
}

// React appropriately for receiving the specified cooked key
bool tng_GADGET_NAME_keycooked(TNG *ptng, ushort key)
{
}

// React appropriately for receiving the specified mouse button event
bool tng_GADGET_NAME_mousebutt(TNG *ptng, uchar type, LGPoint loc)
{
}

// Handle incoming signals
bool tng_GADGET_NAME_signal(TNG *ptng, ushort signal)
{
}

