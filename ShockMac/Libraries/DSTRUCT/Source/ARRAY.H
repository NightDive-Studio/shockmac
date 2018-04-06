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
#ifndef __ARRAY_H
#define __ARRAY_H

/*
 * $Source: n:/project/lib/src/dstruct/RCS/array.h $
 * $Revision: 1.1 $
 * $Author: mahk $
 * $Date: 1993/04/16 22:09:58 $
 *
 * $Log: array.h $
 * Revision 1.1  1993/04/16  22:09:58  mahk
 * Initial revision
 * 
 * Revision 1.2  1993/03/22  15:23:41  mahk
 * Added prototype for array_destroy.
 * 
 * Revision 1.1  1993/03/22  15:21:34  mahk
 * Initial revision
 * 
 *
 */

// Includes
#include "lg.h"

// C Library Includes

// System Library Includes
#include "error.h" 

// Master Game Includes

// Game Library Includes

// Game Object Includes

// ======================
//  ARRAY TYPE 
// ======================
// Here is an implementation of a dynamically-growing array.  
// It is intended for used in places where superfluous calls to 
// Malloc are not desirable.  

// Defines

typedef struct _array
{
   int elemsize;  // How big is each array element
   int vecsize;   // How many elements in the vector
   int fullness;  // How many elements are used.
   int freehead;  // index to head of the free list.  
   int *freevec;  // free list
   char *vec;     // the actual vector;
} Array;


// Prototypes


// Initialize an array.  Fill in the structure, allocate the vector and free list.  
errtype array_init(Array* toinit, int elemsize, int vecsize);

// Find a place for a new element of the array, extending the array if necessary. 
// returns the new index in *index
errtype array_newelem(Array* a, int* index);

// Mark an element as unused and eligible for recycling by a subsequent
// array_newelem call. 
errtype array_dropelem(Array* a, int index);

// Destroy an array, deallocating its vec and freevec
errtype array_destroy(Array* a);


// Globals

#endif //__ARRAY_H
