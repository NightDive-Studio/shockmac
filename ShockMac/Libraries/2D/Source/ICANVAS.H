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
 * $Source: r:/prj/lib/src/2d/RCS/icanvas.h $
 * $Revision: 1.27 $
 * $Author: kevin $
 * $Date: 1994/11/12 02:23:37 $
 *
 * Symbolic constants for function table references.
 *
 * This file is part of the 2d library.
 *
 * $Log: icanvas.h $
 * Revision 1.27  1994/11/12  02:23:37  kevin
 * replaced obsolete (u)hline constants with interrupt
 * crunchy set pixel constants.
 * 
 * Revision 1.26  1994/10/19  16:27:41  kevin
 * Replaced obsolete line drawing indices with state push/pop entries.
 * 
 * Revision 1.25  1994/04/09  07:27:15  lmfeeney
 * added enum for new scaled string primiives
 * ,
 * 
 * Revision 1.24  1994/03/14  11:01:59  kevin
 * Added bitmap doubling placeholders and clut UNscaled bitmap placeholders.
 * 
 * Revision 1.23  1994/02/14  22:15:34  baf
 * Pruned vestigial 16/bit translucency
 * 
 * Revision 1.22  1993/12/30  11:04:31  baf
 * non/span solid filled polygons
 * 
 * Revision 1.21  1993/12/28  22:04:39  baf
 * Added solid RSD stuff
 * 
 * Revision 1.20  1993/12/04  11:51:47  kevin
 * Added placeholders for all kinds of texture mappers.
 * 
 * Revision 1.19  1993/11/29  20:31:37  baf
 * General revisions and tidying/up of translucency
 * and shading routines
 * 
 * Revision 1.18  1993/11/24  01:47:01  kevin
 * Added wall and floor texture mapping primitive placeholders.
 * 
 * Revision 1.17  1993/11/23  17:52:48  baf
 * Added tluc/8 interpolation
 * 
 * Revision 1.16  1993/11/19  17:29:20  baf
 * Added 8/bit translucent bitmaps
 * 
 * Revision 1.15  1993/11/15  03:32:11  baf
 * Added primitiveless chains.
 * 
 * Revision 1.14  1993/10/26  02:05:19  kevin
 * Reorganized scaling and clut-scaling entries to allow versions for
 * all bitmap types.
 * 
 * Revision 1.13  1993/10/21  01:14:40  baf
 * Added scaled translucent bitmaps and
 * translucent linear texture maps.
 * 
 * Revision 1.11  1993/10/15  12:06:39  baf
 * Added translucent bitmap functions
 * 
 * Revision 1.10  1993/10/06  13:41:56  kevin
 * added constants for CLUT_LIN_{U}MAP and CLUT_HFLIP_*.
 * 
 * Revision 1.9  1993/09/02  20:16:08  kaboom
 * Added table constants for 24-bit pixels and lin_{u}map dispatching.
 * 
 * Revision 1.8  1993/08/19  21:51:24  jaemz
 * Added bitmap scale clut and voxel functions
 * 
 * Revision 1.7  1993/08/10  19:16:49  kaboom
 * Added constants for LIT_PER_{U}MAP.
 * 
 * Revision 1.6  1993/07/20  16:24:55  jaemz
 * Added interp2 and filterw2
 * 
 * Revision 1.5  1993/06/22  19:59:14  spaz
 * Added slots for goroud-shaded lines:
 * FIX_USLINE, SLINE, UCLINE, and CLINE.
 * 
 * Revision 1.4  1993/06/14  14:12:23  kaboom
 * New constants for lin_{u}map and lin_lit_lin_{u}map routines.  Also
 * changed names of old tmappers to per_map, etc.
 * 
 * Revision 1.3  1993/06/06  15:13:42  kaboom
 * Added constants for horizontally flipped bitmap entries.
 * 
 * Revision 1.2  1993/06/01  13:50:35  kaboom
 * Added constants for lit perspective tmappers.
 * 
 * Revision 1.1  1993/05/03  13:45:46  kaboom
 * Initial revision
 */

#ifndef __ICANVAS_H
#define __ICANVAS_H
/* here are the indices for all the indirected driver functions. */
enum {
   /* first are the analytic primitive functions. */
   SET_UPIXEL8,            /* pixel 8-bit set/get */
   SET_PIXEL8,
   GET_UPIXEL8,
   GET_PIXEL8,

   SET_UPIXEL24,           /* pixel 24-bit set/get */
   SET_PIXEL24,
   GET_UPIXEL24,
   GET_PIXEL24,

   DRAW_CLEAR,             /* integral, straight primitives */
   DRAW_UPOINT,
   DRAW_POINT,
   SET_UPIXEL8_INTERRUPT,
   SET_PIXEL8_INTERRUPT,
   DRAW_UVLINE,
   DRAW_VLINE,
   DRAW_URECT,
   DRAW_RECT,
   DRAW_UBOX,
   DRAW_BOX,

   PUSH_STATE,              
   POP_STATE,

   FIX_USLINE,             /* fixed-point rendering primitives */
   FIX_SLINE,
   FIX_UCLINE,
   FIX_CLINE,
   FIX_UPOLY,
   FIX_POLY,
   FIX_USPOLY,
   FIX_SPOLY,
   FIX_UCPOLY,
   FIX_CPOLY,
   FIX_TLUC8_UPOLY,
   FIX_TLUC8_POLY,
   FIX_TLUC8_USPOLY,
   FIX_TLUC8_SPOLY,

   VOX_RECT,
   VOX_POLY,
   VOX_CPOLY,
   INTERP2_UBITMAP,
   FILTER2_UBITMAP,
   ROLL_UBITMAP,
   ROLL_BITMAP,

   FLAT8_WALL_UMAP,
   FLAT8_WALL_MAP,
   FLAT8_LIT_WALL_UMAP,
   FLAT8_LIT_WALL_MAP,
   FLAT8_CLUT_WALL_UMAP,
   FLAT8_CLUT_WALL_MAP,

   FLAT8_FLOOR_UMAP,
   FLAT8_FLOOR_MAP,
   FLAT8_LIT_FLOOR_UMAP,
   FLAT8_LIT_FLOOR_MAP,
   FLAT8_CLUT_FLOOR_UMAP,
   FLAT8_CLUT_FLOOR_MAP,

   DEVICE_ULMAP,           /* linear mapper */
   DEVICE_LMAP,
   MONO_ULMAP,
   MONO_LMAP,
   FLAT8_ULMAP,
   FLAT8_LMAP,
   FLAT24_ULMAP,
   FLAT24_LMAP,
   RSD_ULMAP,
   RSD_LMAP,
   TLUC8_ULMAP,
   TLUC8_LMAP,

   DEVICE_LIT_LIN_UMAP,           /* lit linear mapper */
   DEVICE_LIT_LIN_MAP,
   MONO_LIT_LIN_UMAP,
   MONO_LIT_LIN_MAP,
   FLAT8_LIT_LIN_UMAP,
   FLAT8_LIT_LIN_MAP,
   FLAT24_LIT_LIN_UMAP,
   FLAT24_LIT_LIN_MAP,
   RSD_LIT_LIN_UMAP,
   RSD_LIT_LIN_MAP,
   TLUC8_LIT_LIN_UMAP,
   TLUC8_LIT_LIN_MAP,

   DEVICE_CLUT_LIN_UMAP,           /* clut linear mapper */
   DEVICE_CLUT_LIN_MAP,
   MONO_CLUT_LIN_UMAP,
   MONO_CLUT_LIN_MAP,
   FLAT8_CLUT_LIN_UMAP,
   FLAT8_CLUT_LIN_MAP,
   FLAT24_CLUT_LIN_UMAP,
   FLAT24_CLUT_LIN_MAP,
   RSD_CLUT_LIN_UMAP,
   RSD_CLUT_LIN_MAP,
   TLUC8_CLUT_LIN_UMAP,
   TLUC8_CLUT_LIN_MAP,

   FLAT8_SOLID_LIN_UMAP,
   FLAT8_SOLID_LIN_MAP,            /* solid linear mapper */

   DEVICE_PER_UMAP,                /* perspective mapper */
   DEVICE_PER_MAP,
   MONO_PER_UMAP,
   MONO_PER_MAP,
   FLAT8_PER_UMAP,
   FLAT8_PER_MAP,
   FLAT24_PER_UMAP,
   FLAT24_PER_MAP,
   RSD_PER_UMAP,
   RSD_PER_MAP,
   TLUC8_PER_UMAP,
   TLUC8_PER_MAP,

   DEVICE_LIT_PER_UMAP,           /* lit perspective mapper */
   DEVICE_LIT_PER_MAP,
   MONO_LIT_PER_UMAP,
   MONO_LIT_PER_MAP,
   FLAT8_LIT_PER_UMAP,
   FLAT8_LIT_PER_MAP,
   FLAT24_LIT_PER_UMAP,
   FLAT24_LIT_PER_MAP,
   RSD_LIT_PER_UMAP,
   RSD_LIT_PER_MAP,
   TLUC8_LIT_PER_UMAP,
   TLUC8_LIT_PER_MAP,

   DEVICE_CLUT_PER_UMAP,           /* clut perspective mapper */
   DEVICE_CLUT_PER_MAP,
   MONO_CLUT_PER_UMAP,
   MONO_CLUT_PER_MAP,
   FLAT8_CLUT_PER_UMAP,
   FLAT8_CLUT_PER_MAP,
   FLAT24_CLUT_PER_UMAP,
   FLAT24_CLUT_PER_MAP,
   RSD_CLUT_PER_UMAP,
   RSD_CLUT_PER_MAP,
   TLUC8_CLUT_PER_UMAP,
   TLUC8_CLUT_PER_MAP,

   FLAT8_SOLID_PER_UMAP,           /* solid perspective mapper */
   FLAT8_SOLID_PER_MAP,

   INT_UCIRCLE,            /* curves, should change to fixed-point */
   INT_CIRCLE,
   FIX_UCIRCLE,
   FIX_CIRCLE,
   INT_UDISK,
   INT_DISK,
   FIX_UDISK,
   FIX_DISK,
   INT_UROD,
   INT_ROD,
   FIX_UROD,
   FIX_ROD,

   DRAW_DEVICE_UBITMAP,    /* bitmap drawing functions */
   DRAW_DEVICE_BITMAP,
   DRAW_MONO_UBITMAP,
   DRAW_MONO_BITMAP,
   DRAW_FLAT8_UBITMAP,
   DRAW_FLAT8_BITMAP,
   DRAW_FLAT24_UBITMAP,
   DRAW_FLAT24_BITMAP,
   DRAW_RSD8_UBITMAP,
   DRAW_RSD8_BITMAP,
   DRAW_TLUC8_UBITMAP,
   DRAW_TLUC8_BITMAP,

   CLUT_DRAW_DEVICE_UBITMAP,    /* bitmap drawing functions through a clut */
   CLUT_DRAW_DEVICE_BITMAP,
   CLUT_DRAW_MONO_UBITMAP,
   CLUT_DRAW_MONO_BITMAP,
   CLUT_DRAW_FLAT8_UBITMAP,
   CLUT_DRAW_FLAT8_BITMAP,
   CLUT_DRAW_FLAT24_UBITMAP,
   CLUT_DRAW_FLAT24_BITMAP,
   CLUT_DRAW_RSD8_UBITMAP,
   CLUT_DRAW_RSD8_BITMAP,
   CLUT_DRAW_TLUC8_UBITMAP,
   CLUT_DRAW_TLUC8_BITMAP,

   SOLID_RSD8_UBITMAP,      /* solid bitmap drawing functions */
   SOLID_RSD8_BITMAP,

   SCALE_DEVICE_UBITMAP,    /* scaled bitmap drawing functions */
   SCALE_DEVICE_BITMAP,
   SCALE_MONO_UBITMAP,
   SCALE_MONO_BITMAP,
   SCALE_FLAT8_UBITMAP,
   SCALE_FLAT8_BITMAP,
   SCALE_FLAT24_UBITMAP,
   SCALE_FLAT24_BITMAP,
   SCALE_RSD8_UBITMAP,
   SCALE_RSD8_BITMAP,
   SCALE_TLUC8_UBITMAP,
   SCALE_TLUC8_BITMAP,

   SOLID_SCALE_RSD8_UBITMAP,      /* solid bitmap drawing functions */
   SOLID_SCALE_RSD8_BITMAP,

   CLUT_SCALE_DEVICE_UBITMAP,    /* bitmap scaling functions through a clut */
   CLUT_SCALE_DEVICE_BITMAP,
   CLUT_SCALE_MONO_UBITMAP,
   CLUT_SCALE_MONO_BITMAP,
   CLUT_SCALE_FLAT8_UBITMAP,
   CLUT_SCALE_FLAT8_BITMAP,
   CLUT_SCALE_FLAT24_UBITMAP,
   CLUT_SCALE_FLAT24_BITMAP,
   CLUT_SCALE_RSD8_UBITMAP,
   CLUT_SCALE_RSD8_BITMAP,
   CLUT_SCALE_TLUC8_UBITMAP,
   CLUT_SCALE_TLUC8_BITMAP,

   MASK_DEVICE_UBITMAP,    /* bitmap mask draw functions */
   MASK_DEVICE_BITMAP,
   MASK_MONO_UBITMAP,
   MASK_MONO_BITMAP,
   MASK_FLAT8_UBITMAP,
   MASK_FLAT8_BITMAP,
   MASK_FLAT24_UBITMAP,
   MASK_FLAT24_BITMAP,
   MASK_RSD8_UBITMAP,
   MASK_RSD8_BITMAP,
   MASK_TLUC8_UBITMAP,
   MASK_TLUC8_BITMAP,

   GET_DEVICE_UBITMAP,     /* bitmap get functions */
   GET_DEVICE_BITMAP,
   GET_MONO_UBITMAP,
   GET_MONO_BITMAP,
   GET_FLAT8_UBITMAP,
   GET_FLAT8_BITMAP,
   GET_FLAT24_UBITMAP,
   GET_FLAT24_BITMAP,
   GET_RSD8_UBITMAP,
   GET_RSD8_BITMAP,
   GET_TLUC8_UBITMAP,
   GET_TLUC8_BITMAP,

   HFLIP_DEVICE_UBITMAP,   /* bitmap horizontal flip functions */
   HFLIP_DEVICE_BITMAP,
   HFLIP_MONO_UBITMAP,
   HFLIP_MONO_BITMAP,
   HFLIP_FLAT8_UBITMAP,
   HFLIP_FLAT8_BITMAP,
   HFLIP_FLAT24_UBITMAP,
   HFLIP_FLAT24_BITMAP,
   HFLIP_RSD8_UBITMAP,
   HFLIP_RSD8_BITMAP,
   HFLIP_TLUC8_UBITMAP,
   HFLIP_TLUC8_BITMAP,

   CLUT_HFLIP_DEVICE_UBITMAP,  /* bitmap color lookup table hozo flip fcts*/
   CLUT_HFLIP_DEVICE_BITMAP,
   CLUT_HFLIP_MONO_UBITMAP,
   CLUT_HFLIP_MONO_BITMAP,
   CLUT_HFLIP_FLAT8_UBITMAP,
   CLUT_HFLIP_FLAT8_BITMAP,
   CLUT_HFLIP_FLAT24_UBITMAP,
   CLUT_HFLIP_FLAT24_BITMAP,
   CLUT_HFLIP_RSD8_UBITMAP,
   CLUT_HFLIP_RSD8_BITMAP,
   CLUT_HFLIP_TLUC8_UBITMAP,
   CLUT_HFLIP_TLUC8_BITMAP,

   DOUBLE_H_DEVICE_UBITMAP,
   DOUBLE_H_DEVICE_BITMAP,
   DOUBLE_H_MONO_UBITMAP,
   DOUBLE_H_MONO_BITMAP,
   DOUBLE_H_FLAT8_UBITMAP,
   DOUBLE_H_FLAT8_BITMAP,
   DOUBLE_H_FLAT24_UBITMAP,
   DOUBLE_H_FLAT24_BITMAP,
   DOUBLE_H_RSD8_UBITMAP,
   DOUBLE_H_RSD8_BITMAP,
   DOUBLE_H_TLUC8_UBITMAP,
   DOUBLE_H_TLUC8_BITMAP,

   DOUBLE_V_DEVICE_UBITMAP,
   DOUBLE_V_DEVICE_BITMAP,
   DOUBLE_V_MONO_UBITMAP,
   DOUBLE_V_MONO_BITMAP,
   DOUBLE_V_FLAT8_UBITMAP,
   DOUBLE_V_FLAT8_BITMAP,
   DOUBLE_V_FLAT24_UBITMAP,
   DOUBLE_V_FLAT24_BITMAP,
   DOUBLE_V_RSD8_UBITMAP,
   DOUBLE_V_RSD8_BITMAP,
   DOUBLE_V_TLUC8_UBITMAP,
   DOUBLE_V_TLUC8_BITMAP,

   DOUBLE_HV_DEVICE_UBITMAP,
   DOUBLE_HV_DEVICE_BITMAP,
   DOUBLE_HV_MONO_UBITMAP,
   DOUBLE_HV_MONO_BITMAP,
   DOUBLE_HV_FLAT8_UBITMAP,
   DOUBLE_HV_FLAT8_BITMAP,
   DOUBLE_HV_FLAT24_UBITMAP,
   DOUBLE_HV_FLAT24_BITMAP,
   DOUBLE_HV_RSD8_UBITMAP,
   DOUBLE_HV_RSD8_BITMAP,
   DOUBLE_HV_TLUC8_UBITMAP,
   DOUBLE_HV_TLUC8_BITMAP,

   SMOOTH_DOUBLE_H_DEVICE_UBITMAP,
   SMOOTH_DOUBLE_H_DEVICE_BITMAP,
   SMOOTH_DOUBLE_H_MONO_UBITMAP,
   SMOOTH_DOUBLE_H_MONO_BITMAP,
   SMOOTH_DOUBLE_H_FLAT8_UBITMAP,
   SMOOTH_DOUBLE_H_FLAT8_BITMAP,
   SMOOTH_DOUBLE_H_FLAT24_UBITMAP,
   SMOOTH_DOUBLE_H_FLAT24_BITMAP,
   SMOOTH_DOUBLE_H_RSD8_UBITMAP,
   SMOOTH_DOUBLE_H_RSD8_BITMAP,
   SMOOTH_DOUBLE_H_TLUC8_UBITMAP,
   SMOOTH_DOUBLE_H_TLUC8_BITMAP,

   SMOOTH_DOUBLE_V_DEVICE_UBITMAP,
   SMOOTH_DOUBLE_V_DEVICE_BITMAP,
   SMOOTH_DOUBLE_V_MONO_UBITMAP,
   SMOOTH_DOUBLE_V_MONO_BITMAP,
   SMOOTH_DOUBLE_V_FLAT8_UBITMAP,
   SMOOTH_DOUBLE_V_FLAT8_BITMAP,
   SMOOTH_DOUBLE_V_FLAT24_UBITMAP,
   SMOOTH_DOUBLE_V_FLAT24_BITMAP,
   SMOOTH_DOUBLE_V_RSD8_UBITMAP,
   SMOOTH_DOUBLE_V_RSD8_BITMAP,
   SMOOTH_DOUBLE_V_TLUC8_UBITMAP,
   SMOOTH_DOUBLE_V_TLUC8_BITMAP,

   SMOOTH_DOUBLE_HV_DEVICE_UBITMAP,
   SMOOTH_DOUBLE_HV_DEVICE_BITMAP,
   SMOOTH_DOUBLE_HV_MONO_UBITMAP,
   SMOOTH_DOUBLE_HV_MONO_BITMAP,
   SMOOTH_DOUBLE_HV_FLAT8_UBITMAP,
   SMOOTH_DOUBLE_HV_FLAT8_BITMAP,
   SMOOTH_DOUBLE_HV_FLAT24_UBITMAP,
   SMOOTH_DOUBLE_HV_FLAT24_BITMAP,
   SMOOTH_DOUBLE_HV_RSD8_UBITMAP,
   SMOOTH_DOUBLE_HV_RSD8_BITMAP,
   SMOOTH_DOUBLE_HV_TLUC8_UBITMAP,
   SMOOTH_DOUBLE_HV_TLUC8_BITMAP,

   DRAW_USTRING,           /* text/font functions */
   DRAW_STRING,
   DRAW_SCALE_USTRING,
   DRAW_SCALE_STRING,
   DRAW_UCHAR,
   DRAW_CHAR,

   CALC_ROW,               /* bitmap type specific functions */
   SUB_BITMAP,

   START_FRAME,            /* no primitives, just chains */
   END_FRAME,

   GRD_CANVAS_FUNCS
};
#endif /* !__ICANVAS_H */
