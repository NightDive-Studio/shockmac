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
/*
 * $Header: r:/prj/lib/src/h/RCS/2dres.h 1.6 1994/07/06 18:38:48 jaemz Exp $
 *
 * Macros to use 2d calls with resources
 *
 * $Log: 2dres.h $
 * Revision 1.6  1994/07/06  18:38:48  jaemz
 * Added fields to cylbm frame
 * 
 * Revision 1.5  1994/06/15  11:51:49  jaemz
 * Added cylindrical bitmap object data types
 * 
 * Revision 1.4  1994/01/27  13:21:19  eric
 * Added gr_cpy_pal_image to copy image pallette from 
 * REFerence to a pallette in memory.
 * 
 * Revision 1.3  1993/09/28  01:12:21  kaboom
 * Converted #include "xxx" to #include <xxx> for watcom.
 * 
 * Revision 1.2  1993/08/24  20:04:52  rex
 * Turned framedesc's updateArea into union of updateArea, anchorArea, anchorPt
 * 
 * Revision 1.1  1993/04/27  12:06:52  rex
 * Initial revision
 */

#ifndef _2DRES_H
#define _2DRES_H

#include "2d.h"
#include "res.h"
#include "rect.h"

// A Ref in a resource gets you a Frame Descriptor:

typedef struct {
   grs_bitmap bm;       // embedded bitmap, bm.bits set to NULL
   union {
      LGRect updateArea;  // update area (for anims)
      LGRect anchorArea;  // area to anchor sub-bitmap
      LGPoint anchorPt;   // point to anchor from
      };
   long pallOff;        // offset to pallette
                        // bitmap's bits follow immediately
} FrameDesc;

// These are the ref-based 2d macros.  They have the goofy do/while(0)
// so that you may use them in an if statement without C whining.

// Draw a bitmap, given its ref (unclipped)

#define gr_ubitmap_ref(ref,x,y)  do {     \
   FrameDesc *pfd = RefGet(ref);          \
   pfd->bm.bits = (uchar *)(pfd + 1);     \
   gr_ubitmap(&pfd->bm, x, y);            \
   } while (0)

// Draw a bitmap, given its ref (clipped)

#define gr_bitmap_ref(ref,x,y) do {       \
   FrameDesc *pfd = RefGet(ref);          \
   pfd->bm.bits = (uchar *)(pfd + 1);     \
   gr_bitmap(&pfd->bm, x, y);             \
   } while(0)

// Draw a scaled bitmap, given its ref (unclipped)

#define gr_scale_ubitmap_ref(ref,x,y,w,h) do {  \
   FrameDesc *pfd = RefGet(ref);                \
   pfd->bm.bits = (uchar *)(pfd + 1);           \
   gr_scale_ubitmap(&pfd->bm, x, y, w, h);      \
   } while(0)

// Draw a scaled bitmap, given its ref (clipped)

#define gr_scale_bitmap_ref(ref,x,y,w,h) do {   \
   FrameDesc *pfd = RefGet(ref);                \
   pfd->bm.bits = (uchar *)(pfd + 1);           \
   gr_scale_bitmap(&pfd->bm, x, y, w, h);       \
   } while(0)

// Set an image's associated (partial) palette, if any

#define gr_set_pal_imgref(ref) do {             \
   FrameDesc *pfd = RefGet(ref);                \
   if (pfd->pallOff) {                          \
      short *p = (short *)((uchar *) ResGet(REFID(ref)) + pfd->pallOff);   \
      gr_set_pal(*p, *(p+1), (uchar *)(p+2));   \
      }                                         \
   } while(0)

// Copy an image's associated (partial) palette to memory 
//  (palp is a pointer to start of destination pallette)

#define gr_cpy_pal_imgref(ref, palp) do {       \
   FrameDesc *pfd = RefGet(ref);                \
   if (pfd->pallOff) {                          \
      short *p = (short *)((uchar *) ResGet(REFID(ref)) + pfd->pallOff);   \
      LG_memcpy((uchar *)(palp + (*p * 3)), (uchar *)(p+2), *(p+1) * 3 );         \
      }                                         \
   } while(0)

// Data types for a cylindrical bitmap object
// eventually we'll want one for a full 3d one
  
typedef struct {
   int nviews;    // number of views
   fix ppu;       // pixels per unit
   bool bisym;    // bilateral symmetry or not (means its a mirror)
   int off[1];    // offsets, in reality there should be off[nview] of them
} CylBMObj;       // cylindrical 3d bitmap object

typedef struct {
   grs_bitmap bm;
   byte  u1,v1;   // anchor point 1
   byte  u2,v2;   // anchor point 2
   fix   vper1;   //  v1 / (v2-v1)
   fix   vper2;   // (h-v2) / (v2-v1)
   fix   uper;    // (w - u) / u
} CylBMFrame;     // one frame of the cylindrical bm object.  Always put the bits after this.  S


#endif

