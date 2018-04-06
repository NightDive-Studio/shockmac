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
#ifndef __STATICS_H
#define __STATICS_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/statics.h $
 * $Revision: 1.6 $
 * $Author: xemu $
 * $Date: 1994/11/01 09:19:38 $
 *
 * $Log: statics.h $
 * Revision 1.6  1994/11/01  09:19:38  xemu
 * share memory for cutscens and 128x128
 * 
 * Revision 1.5  1994/09/09  18:40:44  xemu
 * object pool stuff.
 * 
 * Revision 1.4  1994/09/06  21:57:29  xemu
 * increase frame buffer a teeny bit
 * 
 * Revision 1.3  1994/09/05  09:35:58  xemu
 * rearrange
 * 
 * Revision 1.2  1994/09/05  06:25:36  xemu
 * expanded buffering capability
 * 
 * Revision 1.1  1994/09/01  13:38:14  minman
 * Initial revision
 * 
 *
 */

// the idea is you can take just obj mem, tmap and obj, or both 
// + either big buffer or the frame buffer or both

// still need to have static.h for people to use this stuff...

// how do we get this aligned right.
// perhaps do this file in asm for real?
// if we did it in asm, and had these point, then we could use 
// labels for top and bottom, which would be good...
// sadly, this alphabetizes, since it is so cool

// put big buffer here? and have a define after it

#include "textmaps.h"
extern uchar       tmap_static_mem[NUM_STATIC_TMAPS*SIZE_STATIC_TMAP];
#ifdef SVGA_CUTSCENES
extern uchar       tmap_big_buffer[NUM_STATIC_TMAPS * SIZE_BIG_TMAP];
#endif

#include "objects.h"
#include "objapp.h"
extern Obj         objs[NUM_OBJECTS];
extern ObjRef      objRefs[NUM_REF_OBJECTS];
extern uchar       objsDealt[NUM_OBJECTS/8];

// put rest of obj system here, define after it

#define FRAME_BUFFER_SIZE  (320*200) + 28
extern uchar       frameBuffer[FRAME_BUFFER_SIZE];

#include "mfddims.h"
extern uchar       *mfd_canvas_bits;

#define ALTERNATE_BUFFER         frameBuffer
#define ALTERNATE_BUFFER_SIZE    ((MFD_VIEW_HGT * MFD_VIEW_WID) + FRAME_BUFFER_SIZE)

#include "map.h"
#define STATIC_MAP_SIZE    16 << (DEFAULT_XSHF + DEFAULT_YSHF)

extern uchar static_map[STATIC_MAP_SIZE];

#include "objprop.h"
#define OBJ_BITMAP_POOL_SIZE ((NUM_OBJECT * 2) + 230)
extern grs_bitmap obj_bitmap_pool[OBJ_BITMAP_POOL_SIZE];

#endif // __STATICS_H
