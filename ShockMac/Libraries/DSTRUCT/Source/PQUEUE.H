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
#ifndef __PQUEUE_H
#define __PQUEUE_H

/*
 * $Source: n:/project/lib/src/dstruct/RCS/pqueue.h $
 * $Revision: 1.1 $
 * $Author: mahk $
 * $Date: 1993/08/09 20:31:11 $
 *
 * $Log: pqueue.h $
 * Revision 1.1  1993/08/09  20:31:11  mahk
 * Initial revision
 * 
 *
 */

// -----------------------------------
// Priority Queue Abstraction
// -----------------------------------
/* Herein lies a binary heap implementation of a priority queue 
   The queue can have elements of any size, as the client specifies
   the element size and comparison function.  */




// Includes
#include "lg.h"  // every file should have this
#include "error.h"

// Defines
// Comparson function, works like strcmp
typedef int (*QueueCompare)(void* elem1, void* elem2);

typedef struct _pqueue
{
   int size;
   int fullness;
   int elemsize;
   bool grow;
   char* vec;
   QueueCompare comp;
} PQueue;

// Prototypes
errtype pqueue_init(PQueue* q, int size, int elemsize, QueueCompare comp,  bool grow);
// Initializes a Priority queue to a particular size, with a 
// particular element size and comparison function.

errtype pqueue_insert(PQueue* q, void* elem);
// Insert an element into the queue (log time)

errtype pqueue_extract(PQueue* q, void* elem);
// Copies the least element in the queue into *elem,
// and removes that element. (log time)

errtype pqueue_least(PQueue* q, void* elem);
// Copies the least element into *elem, but does not 
// remove it.  (constant time)

errtype pqueue_write(PQueue* q,int fd,void (*writefunc)(int fd,void* elem));
// Writes out a queue to file number fd, calling writefunc to write out each element.
// If writefunc is NULL, simply writes the literal data in each element.  

errtype pqueue_read(PQueue* q, int fd, void (*readfunc)(int fd, void* elem));
// Reads in a queue from file number fd, calling readfunc to read each element.
// If readfunc is NULL, reads each element literally.  

errtype pqueue_destroy(PQueue* q);
// Destroys a priority queue.





// Globals

#endif // __PQUEUE_H

