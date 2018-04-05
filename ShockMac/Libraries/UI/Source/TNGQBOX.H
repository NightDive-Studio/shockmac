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
#ifndef __TNGQBOX_H
#define __TNGQBOX_H

/*
 * $Source: n:/project/lib/src/ui/RCS/tngqbox.h $
 * $Revision: 1.13 $
 * $Author: xemu $
 * $Date: 1994/03/10 04:31:20 $
 *
 * $Log: tngqbox.h $
 * Revision 1.13  1994/03/10  04:31:20  xemu
 * uint slots
 * 
 * Revision 1.12  1993/12/31  18:01:38  xemu
 * rename slots
 * 
 * Revision 1.11  1993/10/11  20:27:41  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.10  1993/09/16  14:58:06  mahk
 * Added QB_FIX_SLOT
 * 
 * Revision 1.9  1993/08/11  11:11:06  xemu
 * stringset display option
 * 
 * Revision 1.8  1993/06/24  10:26:47  xemu
 * QB_ADDCLOSE
 * 
 * Revision 1.7  1993/06/23  18:56:11  xemu
 * byte, hex, octal, binary
 * 
 * Revision 1.6  1993/06/16  02:22:38  xemu
 * new options
 * 
 * Revision 1.5  1993/06/10  13:34:49  xemu
 * added QB_GRABFOCUS
 * 
 * Revision 1.4  1993/04/30  11:40:00  xemu
 * slider_size to aux_size
 * 
 * Revision 1.3  1993/04/29  19:02:51  xemu
 * support options
 * 
 * Revision 1.2  1993/04/28  14:40:26  mahk
 * Preparing for second exodus
 * 
 * Revision 1.1  1993/04/27  16:37:18  xemu
 * Initial revision
 * 
 * Revision 1.1  1993/04/22  15:09:55  xemu
 * Initial revision
 * 
 *
 */

// Includes
#include "lg.h"  // every file should have this
#include "tng.h"

// Typedefs
typedef struct _QuickboxSlot {
   char *label;                // Text Label
   ulong options;              // Options mask
   int   vartype;              // Type of the slot
   void *var;                  // Pointer to actual variable
   void *p1, *p2;              // parameters
   struct _QuickboxSlot *next; // next slot in the box
   TNG *aux_tng;               // An auxilliary gadget, if relevant
   LGPoint aux_size;
   TNG *tng;
} QuickboxSlot;

typedef struct {
   TNG *tng_data;
   LGPoint size, slot_size, spacing, border;
   Ref left_id, right_id;
   int aux_size, internal_margin;
   ushort options;
   QuickboxSlot *slots;
   QuickboxSlot *current_slot;
} TNG_quickbox;

// SlotTypes
#define QB_INT_SLOT        0   // var is an int *
#define QB_SHORT_SLOT      1   // var is a short *
#define QB_BYTE_SLOT       2   // var is a ubyte *
#define QB_TEXT_SLOT       3   // var is a char *
#define QB_BOOL_SLOT       4   // var is a bool, and should be displayed as TRUE/FALSE
#define QB_PUSHBUTTON_SLOT 5   // var is a GadgetCallback to be called when the button using the label as text is pressed
                               // requires parameters to be set.  P1 is the user_data for the callback.  If P2 is null,
                               // then the label will be the text of the button.  If P2 is not null, it is taken as a
                               // pointer to a resource to display in the button, and the label will appear off to the
                               // right, like for other slot types
#define QB_FIX_SLOT        6   // fixpoint number
#define QB_UINT_SLOT        7   // var is an uint *

// Overall Options
// Align up all the data fields nicely
#define QB_ALIGNMENT    0x01

// Have the quickbox set the slot width to the minimum setting
// large enough to encompass the data for  every slot.
#define QB_AUTOSIZE     0x02

// Has the quickbox grab keyboard focus upon creation,
// releasing it upon exit.
#define QB_GRABFOCUS    0x04

// Add a "close" button at the bottom of the quickbox which
// will close it down.  If you want your own code to get run
// when quickbox closed, don't use this option!
#define QB_ADDCLOSE     0x08
  
// ********************
// *** SLOT OPTIONS ***
// ********************

// no options for this slot, plain vanilla
#define QB_NO_OPTION    0x0000

// the var can be toggled up and down with clickable arrows. 
// SUPPORTED TYPES:  INT_SLOT, SHORT_SLOT, BYTE_SLOT, BOOL_SLOT
#define QB_ARROWS       0x0001UL   

// the var has an associated slider which can be used to set it's value.  Requires parameters 
// which are the max and min values of the variable.  If variables not provided, default to 0 and 100
// for minimum and maximum.
// SUPPORTED TYPES:  INT_SLOT, SHORT_SLOT, BYTE_SLOT
#define QB_SLIDER       0x0002UL

// the var is for output purposes only and should not be allowed to be changed.
// SUPPORTED TYPES:  ALL
#define QB_RD_ONLY      0x0004UL                               

// limit the upper and lower limit of the start to p1 min and p2 max.
// SUPPORTED TYPES:  INT_SLOT, SHORT_SLOT, BYTE_SLOT
#define QB_BOUNDED      0x0008UL

// like QB_BOUNDED, but when max exceeded, cycles around to min
#define QB_CYCLE        0x0010UL

// These all constrain the slot to that type of number
// SUPPORTED TYPES:  INT_SLOT, SHORT_SLOT, BYTE_SLOT, only if standard display options chosen
#define QB_HEX          0x0020UL
#define QB_OCTAL        0x0040UL
#define QB_BINARY       0x0080UL

// Display the value of the slot as one a set of string values
// SUPPORTED TYPES:  INT_SLOT, SHORT_SLOT, BYTE_SLOT
// automatically inherits QB_CYCLE from 0 to the number of provided values
#define QB_STRINGSET    0x0100UL

// *****************
// ** KEY DEFINES **
// *****************

#define QB_RETURN_KEY          0xd
#define QB_UP_KEY              0x48
#define QB_DOWN_KEY            0x50
#define QB_LEFT_KEY            0x4b
#define QB_RIGHT_KEY           0x4d

#define TNG_QB_SELECTION_SPACING 0
#define TNG_QB_DEFAULT_SLIDER_SIZE  60

// Prototypes

// Initializes the TNG 
errtype tng_quickbox_init(void *ui_data, TNG *ptng, TNGStyle *sty, ushort options, LGPoint slot_size, LGPoint spacing, LGPoint border,
   Ref left_id, Ref right_id);

// Add a line to a quickbox.  slot_type describes the type of slot, var is a pointer to the variable to be
// displaying, and slot_options describes any additional modifiers to the qbox.  Note that some bizarre-o 
// combinations of options and types might not be implemented.
errtype tng_quickbox_add(char *label, int slot_type, void *var, ulong slot_options);

// Just like gad_qbox_add but allows two parameters to be set for the slot.  Certain slot options require
// this form of accessing.
errtype tng_quickbox_add_parm(char *label, int slot_type, void *var, ulong slot_options, void *parm1, void *parm2);

// Deallocate all memory used by the TNG 
errtype tng_quickbox_destroy(TNG *ptng);

// This represents that the quickbox is done being created and is ready for display, input, etc.
errtype tng_quickbox_end();

// Draw the specified parts (may be all) of the TNG at screen coordinates loc
// assumes all appropriate setup has already been done!
errtype tng_quickbox_2d_draw(TNG *ptng, ushort partmask, LGPoint loc);

// Fill in ppt with the size of the TNG 
errtype tng_quickbox_size(TNG *ptng, LGPoint *ppt);

// Returns the current "value" of the TNG
int tng_quickbox_getvalue(TNG *ptng);

// React appropriately for receiving the specified cooked key
bool tng_quickbox_keycooked(TNG *ptng, ushort key);

// React appropriately for receiving the specified mouse button event
bool tng_quickbox_mousebutt(TNG *ptng, uchar type, LGPoint loc);

// Handle incoming signals
bool tng_quickbox_signal(TNG *ptng, ushort signal);

// Rename a slot
errtype tng_quickbox_rename_slot(TNG *qb, int slot_num, char *new_name);

// Macros
#define TNG_QB(ptng) ((TNG_quickbox *)(ptng->type_data))
#define QB_CURRENT(ptng) TNG_QB(ptng)->current_slot
#define QB_SLOTS(ptng) TNG_QB(ptng)->slots

#endif // __TNGQBOX_H

