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
 * $Source: r:/prj/lib/src/2d/RCS/ifcn.h $
 * $Revision: 1.1 $
 * $Author: kevin $
 * $Date: 1994/08/16 15:35:57 $
 *
 * Function type indices for table lookups.
 *
 * This file is part of the 2d library.
 *
 * $Log: ifcn.h $
 * Revision 1.1  1994/08/16  15:35:57  kevin
 * Initial revision
 * 
 * 
*/

#ifndef __IFCN_H
#define __IFCN_H

enum {
   /* lines and pixel primitives */
   GRC_PIXEL,
#define GRC_LINE GRC_PIXEL /* lines & pixels are multiplexed by bm type */
   GRC_WIRE_POLY_LINE,
   GRC_DEGEN_LINE, /* hlines and vlines */

   GRC_BITMAP,
   GRC_STENCIL_BITMAP,
   GRC_CLUT_BITMAP,

   GRC_HFLIP_BITMAP,
   GRC_CLUT_HFLIP_BITMAP,

   GRC_MASK_BITMAP,

   GRC_HDOUBLE_BITMAP,
   GRC_VDOUBLE_BITMAP,
   GRC_HVDOUBLE_BITMAP,

   GRC_HDOUBLE_BLEND_BITMAP,
   GRC_VDOUBLE_BLEND_BITMAP,
   GRC_HVDOUBLE_BLEND_BITMAP,

   GRC_SCALE,
   GRC_TRANS_SCALE,
   GRC_LIT_SCALE,
   GRC_TRANS_LIT_SCALE,
   GRC_CLUT_SCALE,
   GRC_TRANS_CLUT_SCALE,

   GRC_POLY,      /* Actually all poly types; uses bitmap types to multiplex */
   GRC_MORE_POLY, /* reserved for future use */

   GRC_LIN,
   GRC_TRANS_LIN,
   GRC_LIT_LIN,
   GRC_TRANS_LIT_LIN,
   GRC_CLUT_LIN,
   GRC_TRANS_CLUT_LIN,

   GRC_BILIN,
   GRC_TRANS_BILIN,
   GRC_LIT_BILIN,
   GRC_TRANS_LIT_BILIN,
   GRC_CLUT_BILIN,
   GRC_TRANS_CLUT_BILIN,

   GRC_FLOOR,
   GRC_TRANS_FLOOR,
   GRC_LIT_FLOOR,
   GRC_TRANS_LIT_FLOOR,
   GRC_CLUT_FLOOR,
   GRC_TRANS_CLUT_FLOOR,

   GRC_WALL2D,
   GRC_TRANS_WALL2D,
   GRC_LIT_WALL2D,
   GRC_TRANS_LIT_WALL2D,
   GRC_CLUT_WALL2D,
   GRC_TRANS_CLUT_WALL2D,

   GRC_WALL1D,
   GRC_TRANS_WALL1D,
   GRC_LIT_WALL1D,
   GRC_TRANS_LIT_WALL1D,
   GRC_CLUT_WALL1D,
   GRC_TRANS_CLUT_WALL1D,

   GRC_PER,
   GRC_TRANS_PER,
   GRC_LIT_PER,
   GRC_TRANS_LIT_PER,
   GRC_CLUT_PER,
   GRC_TRANS_CLUT_PER,

   GRC_PER_VSCAN,
   GRC_TRANS_PER_VSCAN,
   GRC_LIT_PER_VSCAN,
   GRC_TRANS_LIT_PER_VSCAN,
   GRC_CLUT_PER_VSCAN,
   GRC_TRANS_CLUT_PER_VSCAN,

   GRD_FUNCS
};
#endif /* !__IFCN_H */


