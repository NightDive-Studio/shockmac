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
#include "lg.h"
#include "slab.h" 

/*
 * $Source: r:/prj/lib/src/ui/RCS/slab.c $
 * $Revision: 1.4 $
 * $Author: mahk $
 * $Date: 1994/08/24 08:55:51 $
 *
 * $Log: slab.c $
 * Revision 1.4  1994/08/24  08:55:51  mahk
 * Cursor stacks and invisible regions.
 * 
 * Revision 1.3  1993/10/11  20:26:47  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.2  1993/04/28  14:40:01  mahk
 * Preparing for second exodus
 * 
 * Revision 1.1  1993/04/05  23:40:58  mahk
 * Initial revision
 * 
 *
 */



// ------------------- 
// Defines and Globals
// -------------------
errtype ui_init_slabs(void);

uiSlab* uiCurrentSlab = NULL;

// ---------
// INTERNALS
// ---------

errtype ui_init_slabs(void)
{
   uiCurrentSlab = NULL;
   return OK;
}


// -------------
// API FUNCTIONS
// -------------

errtype uiMakeSlab(uiSlab* slab, LGRegion* cursor_reg, LGCursor* default_cursor)
{
   errtype err;
   extern errtype ui_init_focus_chain(uiSlab* slab);
   extern errtype ui_init_cursor_stack(uiSlab* slab, LGCursor* default_cursor);

   slab->creg = cursor_reg;
   err = ui_init_focus_chain(slab);
   if (err != OK) return err;
   err = ui_init_cursor_stack(slab,default_cursor);
   if (err != OK) return err;
   return OK;
}

errtype uiDestroySlab(uiSlab* slab)
{
   slab->creg = NULL;
   uiDestroyCursorStack(&slab->cstack);
   array_destroy(&slab->fchain.chain);
   return OK;
}

errtype uiSetCurrentSlab(uiSlab* slab)
{
   uiCurrentSlab = slab;
   return OK;
}

errtype uiGetCurrentSlab(uiSlab** slab)
{
   *slab = uiCurrentSlab;
   return OK;
}


