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
#ifndef __TARGET_H
#define __TARGET_H

/*
 * $Source: q:/inc/RCS/target.h $
 * $Revision: 1.2 $
 * $Author: xemu $
 * $Date: 1993/09/02 23:08:43 $
 *
 * $Log: target.h $
 * Revision 1.2  1993/09/02  23:08:43  xemu
 * angle me baby
 * 
 * Revision 1.1  1993/08/24  12:23:02  spaz
 * Initial revision
 * 
 *
 */

// Includes
#include "mfdint.h"
#include "objects.h"

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes


// Defines

// Prototypes
void select_current_target(ObjID id, bool force_mfd);
void mfd_target_expose(MFD *m, ubyte control);
bool mfd_target_handler(MFD *m, uiEvent *e);
void toggle_current_target();

// Globals

#endif // __TARGET_H
