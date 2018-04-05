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
#ifndef __SLAB_H
#define __SLAB_H

/*
 * $Source: n:/project/lib/src/ui/RCS/slab.h $
 * $Revision: 1.3 $
 * $Author: dc $
 * $Date: 1993/10/11 20:27:32 $
 *
 * $Log: slab.h $
 * Revision 1.3  1993/10/11  20:27:32  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.2  1993/04/28  14:40:21  mahk
 * Preparing for second exodus
 * 
 * Revision 1.1  1993/04/05  23:43:26  mahk
 * Initial revision
 * 
 *
 */

// A slab is a collection of information about where to send input
// events, and where to look for mouse cursors.  Every slab comes complete with: 
//  1) a root cursor region, which will be traversed upon to 
//     find regional mouse cursors.  
//  2) A default cursor stack.  If the mouse is in a region
//     which doesn't specify a mouse cursor, the top of the slab's cursor stack
//     is used instead.  
//  3) A focus chain.  This determines which regions have input focus.  

// Only one slab is active at one time, and that slabbed is looked at
// by uiPoll.    



// Includes
#include "lg.h"  // every file should have this
#include "error.h"
#include "array.h"
#include "region.h"
#include "cursors.h"

// Defines

typedef struct _ui_slab
{
   LGRegion* creg;  // cursor region.
   struct _focus_chain
   {
      Array chain;
      int curfocus;
   } fchain;  // focus chain
   cursor_stack cstack;
} uiSlab;


// Prototypes

errtype uiMakeSlab(uiSlab* slab,LGRegion* cursor_reg, LGCursor* default_cursor);
// Initialize a region with the specified cursor region, default cursor. 
// the initial focus is usually the root region.  

errtype uiSetCurrentSlab(uiSlab* slab);
// Sets the current active slab.  

errtype uiGetCurrentSlab(uiSlab** slab);
// Gets the current active slab;

errtype uiDestroySlab(uiSlab* slab);
// shuts down a slab, freeing any satellite data. 


// Globals

#endif // __SLAB_H
