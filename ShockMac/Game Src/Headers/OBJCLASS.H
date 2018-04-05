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
#ifndef __OBJCLASS_H
#define __OBJCLASS_H

/*
 * $Source: n:/project/cit/src/inc/RCS/objclass.h $
 * $Revision: 1.6 $
 * $Author: dc $
 * $Date: 1994/04/03 05:59:14 $
 *
 * $Log: objclass.h $
 * Revision 1.6  1994/04/03  05:59:14  dc
 * OPNUM and the other ??NUM three, running a bit faster, still should probably change
 * to inline assembler, along with the other main macros...
 * perhaps just make ObjProps[OPNUM(id)] an inlined asm thing
 * 
 * Revision 1.5  1994/01/02  21:09:50  xemu
 * new containers
 * 
 * Revision 1.4  1993/10/01  23:55:13  xemu
 * new object regime
 * 
 * Revision 1.3  1993/09/02  23:08:12  xemu
 * angle me baby
 * 
 * Revision 1.2  1993/08/17  21:54:15  minman
 * added prototype for nth_after_triple and get_nth_from_triple
 * 
 * Revision 1.1  1993/08/05  14:05:20  minman
 * Initial revision
 * 
 *
 */

// Includes
#include "objects.h"
 
// Some macros...
#define MAKETRIP(obclass, subclass, type) ((obclass << 16) + (subclass << 8) + type)
#define ID2TRIP(id) MAKETRIP(objs[id].obclass, objs[id].subclass, objs[id].info.type)

#define TRIP2CL(trip) ((ObjClass)(trip >> 16))
#define TRIP2SC(trip) ((trip & 0xFF00) >> 8)
#define TRIP2TY(trip) (trip & 0xFF)
#define OBJBASE(triple) (((TRIP2CL(triple) & 0xF) << 4) + (TRIP2SC(triple) & 0xF))

#define OPTRIP(triple) (ObjBaseArray[OBJBASE(triple)] + TRIP2TY(triple))
#define CPTRIP(triple) (ClassBaseArray[TRIP2CL(triple)][TRIP2SC(triple)] + TRIP2TY(triple))
#define SCTRIP(triple) TRIP2TY(triple)

#define OPNUM(id) (ObjBaseArray[(objs[id].obclass<<4)+(objs[id].subclass)]+objs[id].info.type)
#define CPNUM(id) (ClassBaseArray[objs[id].obclass][objs[id].subclass]+objs[id].info.type)
#define SCNUM(id) (objs[id].info.type)

#ifdef SLOW
#define OPNUM(id) OPTRIP(ID2TRIP(id))
#define CPNUM(id) CPTRIP(ID2TRIP(id))
#define SCNUM(id) SCTRIP(ID2TRIP(id))
#endif

// Prototypes
short num_types(uchar obclass, uchar subclass);

// Told to find the nth object of class "class", will return appropriate
// triple or -1 if nth object didn't exist
int get_triple_from_class_nth_item(uchar obclass, uchar n);

// returns the triple which is n past the given base
int nth_after_triple(int base, uchar n);

// given a triple, returns n which represents the count
// past the first of its class
int get_nth_from_triple(int triple);

// Defines
/*
#define COMMON_OBJSPEC_FIELDS 	\   
   union {                       \
		ObjID id;						\	  
		ObjSpecID headused;        \
	};                            \
	union {                       \
		ObjSpecID next;				\		
		ObjSpecID headfree;        \
	};                            \
	ObjSpecID prev

#define COMMON_OBJSPEC_SIZE   (sizeof(ObjSpecID) * 3)
*/

#endif // __OBJCLASS_H


