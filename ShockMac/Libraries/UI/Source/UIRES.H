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
#ifndef __UIRES_H
#define __UIRES_H

/*
 * $Source: r:/prj/lib/src/ui/RCS/uires.h $
 * $Revision: 1.1 $
 * $Author: mahk $
 * $Date: 1994/11/18 12:18:09 $
 *
 */

#include "cursors.h"
#include "res.h"
#include "2dres.h"

// -------
// DEFINES
// -------

// -------
// GLOBALS
// -------


// This is a global buffer used as temporary memory by the cursor extraction
// routine.  The client can fill in an address and size of memory
// that will be safe to use by these routines.  If this data is not filled in,
// data loading routines will use Malloc to get temporary buffer space.  They
// will Free their buffers immediately.
extern struct _uirestempbuffer
{
   char* mem;
   uint size;
} uiResTempBuffer;


// ----------
// PROTOTYPES
// ----------

// Loads in a cursor from a ref into an image resource.
// The hotspot for the cursor is filled in from the image's anchor point data
// The arguments are as follows:
// Cursor* c: a pointer to a cursor struct to be filled in with cursor data
// grs_bitmap* bmp: a pointer to a bitmap to be filled in with the cursor's bitmap
// Ref rid: The ref of the bitmap to be loaded. 
// bool alloc: if alloc is true, the bmp->bits will be set to a Malloc'd buffer
//             for the bitmap's bits. 
//             if alloc is false, then bmp->bits must already point to a memory
//             buffer big enough to hold the bitmaps bits. 

errtype uiLoadRefBitmapCursor(LGCursor* c, grs_bitmap* bmp, Ref rid, bool alloc);

#endif // __UIRES_H

