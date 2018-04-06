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
#ifndef _HASH_H
#define _HASH_H
#include "lg.h" 
#include "error.h" 

/*
 * $Source: n:/project/lib/src/dstruct/RCS/hash.h $
 * $Revision: 1.5 $
 * $Author: mahk $
 * $Date: 1994/01/18 08:16:01 $
 *
 * $Log: hash.h $
 * Revision 1.5  1994/01/18  08:16:01  mahk
 * Added hash_copy
 * 
 * Revision 1.4  1993/08/06  13:46:46  mahk
 * changed libdbg.h to _dstruct.h
 * 
 * Revision 1.3  1993/06/14  21:11:50  xemu
 * step func
 * 
 * Revision 1.2  1993/03/25  19:55:25  mahk
 * Added rep exposure warning.
 * 
 * Revision 1.1  1993/03/25  19:20:09  mahk
 * Initial revision
 * 
 *
 */

// Equivalence function, returns values are as per strcmp
// i.e. 0 if data1 == data2, >0  if data1 > data2, <0 if data1 < data2. 
typedef int (*Equfunc)(void* data1,void* data2);

// hashing function: data1 == data2 ==> hash(data1) == hash(data2)
typedef int (*Hashfunc)(void* data);



typedef struct _hashtable
{
   int size;
   int sizelog2; 
   int elemsize;
   int fullness;
   Equfunc efunc;
   Hashfunc hfunc;
   char *statvec;
   char *vec;
} Hashtable;



errtype hash_init(Hashtable* h, int elemsize, int vecsize, Hashfunc hfunc, Equfunc efunc);
// initialize a hashtable with the specified hashfunc and equfunc, using elemsize as 
// the size of an element, and using vecsize as the initial table size.

errtype hash_set(Hashtable* h,void* elem);
// insert an element into a hashtable, overwriting any element 
// that is equal to it.  

errtype hash_insert(Hashtable* h,void* elem);
// REQUIRES there is no member of h equal to "elem". 
// inserts "elem" into hashtable h.  Faster than hash_set.



errtype hash_lookup(Hashtable* h, void* elem, void** result);
// Looks up elem in the hashtable, sets *result to point to the
// element in h which is equal to elem.  Or NULL if no such element
// exists. 
// WARNING WARNING DANGER WILL ROBINSON.  HEINOUS REP EXPOSURE.
// MODIFY **RESULT AT YOUR OWN PERIL 

errtype hash_delete(Hashtable* h, void* elem);
// Find and remove the element in h which is equal to elem,
// or do nothing if no such element exists.


typedef bool (*HashIterFunc)(void* elem, void* data);

errtype hash_iter(Hashtable* h, HashIterFunc ifunc, void* data);
// Applies ifunc(elem,data) to every element of h, one at a time, until 
// ifunc returns true.  

errtype hash_copy(Hashtable* t, Hashtable* s);
// Initializes t to be a copy of s

errtype hash_step(Hashtable *h, void **result, int *index);
// Will step through a hashtable, returning the elements one at a time.

errtype hash_destroy(Hashtable* h);
// Destroys hashtable h.  Does not free h itself, but frees
// subordinate data structures. 

#endif // _HASH_H
