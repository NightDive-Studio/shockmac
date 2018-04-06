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
#ifndef __OBJLOAD_H
#define __OBJLOAD_H

/*
 * $Source: u:/inc/RCS/objload.h $
 * $Revision: 1.2 $
 * $Author: minman $
 * $Date: 1994/08/27 01:25:54 $
 *
 */

// Includes
#include "objprop.h"
#include "objapp.h"

#define NUM_OBJECT_BIT_LEN    ((NUM_OBJECT+7)>>3)

#define ObjLoadMeSetAll()       do {LG_memset (loadme, 0xFF, NUM_OBJECT_BIT_LEN);}           while (0)
#define ObjLoadMeClearAll()       do {LG_memset (loadme, 0, NUM_OBJECT_BIT_LEN);}           while (0)
#define ObjLoadMeSet(opnum)    do {loadme[(opnum)>>3] |= 1 << ((opnum)&0x7);}      while (0)
#define ObjLoadMeClear(opnum)  do {loadme[(opnum)>>3] &= ~(1 << ((opnum)&0x7));}   while (0)
#define ObjLoadMeCheck(opnum)  (loadme[(opnum)>>3] & (1 << ((opnum)&0x7)))

#define EXTRA_FRAMES 500

extern errtype obj_load_art(bool flush_all);

#ifdef __OBJSIM_SRC
LGPoint anchors_3d[NUM_OBJECT + EXTRA_FRAMES];
grs_bitmap *bitmaps_2d[NUM_OBJECT];
grs_bitmap *bitmaps_3d[NUM_OBJECT + EXTRA_FRAMES];
#else
extern LGPoint anchors_3d[NUM_OBJECT + EXTRA_FRAMES];
extern grs_bitmap *bitmaps_2d[NUM_OBJECT];
extern grs_bitmap *bitmaps_3d[NUM_OBJECT];
#endif

#endif // __OBJLOAD_H
