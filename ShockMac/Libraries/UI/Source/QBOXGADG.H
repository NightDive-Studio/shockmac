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
#ifndef __QBOXGADG_H
#define __QBOXGADG_H

/*
 * $Source: n:/project/lib/src/ui/RCS/qboxgadg.h $
 * $Revision: 1.11 $
 * $Author: xemu $
 * $Date: 1993/12/31 18:01:37 $
 *
 * $Log: qboxgadg.h $
 * Revision 1.11  1993/12/31  18:01:37  xemu
 * rename slots
 * 
 * Revision 1.10  1993/10/11  20:27:26  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.9  1993/06/24  18:21:00  xemu
 * end_full
 * 
 * Revision 1.8  1993/04/29  19:02:44  xemu
 * support options
 * 
 * Revision 1.7  1993/04/28  14:40:18  mahk
 * Preparing for second exodus
 * 
 * Revision 1.6  1993/04/27  16:38:50  xemu
 * general improvement
 * 
 * Revision 1.5  1993/04/12  15:17:54  xemu
 * use better default font
 * ..
 * 
 * 
 * Revision 1.4  1993/04/08  23:57:15  xemu
 * 2d fonts
 * 
 * Revision 1.3  1993/04/02  14:41:21  xemu
 * Style Defines
 * 
 * Revision 1.2  1993/04/01  16:17:32  xemu
 * keyboard constants
 * 
 * Revision 1.1  1993/03/30  15:08:02  xemu
 * Initial revision
 * 
 *
 */

// So the basic concept behind quickboxes is as follows:  A simple method of quickly throwing together dialog boxes
// that allow displaying and accessing program data, ala the Underworld editor.  Each quickbox is composed of slots, each
// of which accesses a particular piece of data.  The basic data type for each slot needs to be set, as well as a
// text label for the slot and the variable itself.  In addition, a set of option flags can be OR-ed together for
// modified the format and attributes of the slot, such as being read-only, having sliders, etc.
// 
// Here is a quick example of what it takes to throw together a quickbox:
// Say we want to be able to edit some information on a critter.  We want to throw up a box which tells the name
// of the critter, and then has some stats that can be edited.
// 
// Gadget *qb, *parent_gadget;
// Critter crit;
// LGPoint origin;
// int min, max;
// 
// qb = gad_qbox_start(parent_gadget, origin, NULL, "sample_qbox");
// gad_qbox_add("Critter Name", QB_TEXT_SLOT, crit.name, QB_RD_ONLY);
// gad_qbox_add("Hit Points", QB_INT_SLOT, &crit.hp, QB_NO_OPTION);
// gad_qbox_add("Armor Class", QB_INT_SLOT, &crit.ac, QB_ARROWS);
// gad_qbox_add_parm("Movement Class", QB_INT_SLOT, &crit.moveclass, QB_SLIDER, min, max);
// gad_qbox_add_parm("Goofy?",QB_BOOL_SLOT, &crit.goofy, QB_RD_ONLY);
// gad_qbox_end();
// 
// That's it.  Simple enough?  Suggestions for more useful variable types and option types welcome...

// Includes
#include "lg.h"  // every file should have this
#include "gadgets.h"
//#include <slider.h>
#include "tng.h"
#include "tngqbox.h"

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes


// Defines

// Prototypes

// Begin a quick box.  Until the qbox is ended, all subsequent qbox calls will use the "current" qbox.
Gadget *gad_qbox_start(Gadget *parent, LGPoint coord, int z, TNGStyle *sty, ushort options, char *name, LGPoint ss);

Gadget *gad_qbox_start_full(Gadget *parent, LGPoint coord, int z, TNGStyle *sty, ushort options, char *name, LGPoint ss, LGPoint spacing,
   LGPoint border, Ref left_id, Ref right_id);

// Add a line to a quickbox.  slot_type describes the type of slot, var is a pointer to the variable to be
// displaying, and slot_options describes any additional modifiers to the qbox.  Note that some bizarre-o 
// combinations of options and types might not be implemented.
errtype gad_qbox_add(char *label, int slot_type, void *var, ulong slot_options);

// Just like gad_qbox_add but allows two parameters to be set for the slot.  Certain slot options require
// this form of accessing.
errtype gad_qbox_add_parm(char *label, int slot_type, void *var, ulong slot_options, void *parm1, void *parm2);

// This represents that the quickbox is done being created and is ready for display, input, etc.
// if end_full is used, then the passed pointer is used as the application's pointer to the quickbox
errtype gad_qbox_end();
errtype gad_qbox_end_full(Gadget **ptr);

// Rename a slot
errtype gad_qbox_rename_slot(Gadget *g, int slot_num, char *new_name);

// Globals

#endif // __QBOXGADG_H

