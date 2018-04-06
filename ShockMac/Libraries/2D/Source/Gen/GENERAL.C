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
 * $Source: n:/project/lib/src/2d/RCS/general.c $
 * $Revision: 1.23 $
 * $Author: unknown $
 * $Date: 1993/04/08 16:25:15 $
 *
 * Generic primitive drawing routines.
 *
 * This file is part of the 2d library.
 *
 * $Log: general.c $
 * Revision 1.23  1993/04/08  16:25:15  unknown
 * Took out dummy functions for strings.
 * 
 * Revision 1.22  1993/03/29  18:30:17  kaboom
 * Removed dummy functions for concave polygons.
 * 
 * Revision 1.21  1993/03/03  17:54:53  kaboom
 * Removed stubs for gen_int_{u}disk().
 * 
 * Revision 1.20  1993/02/25  12:58:40  kaboom
 * Removed stubs for Gouraud shaders.
 * 
 * Revision 1.19  1993/02/16  15:41:38  kaboom
 * Moved the rest of the primitive routines to other files.
 * 
 * Revision 1.18  1993/02/04  17:17:24  kaboom
 * Moved many functions into other files.  Added functab entries for cpoly.
 * 
 * Revision 1.14  1993/01/11  17:58:25  matt
 * Added gr_int_spoly() (code in gour.c)
 * 
 * Revision 1.13  1993/01/07  21:06:50  kaboom
 * Updated references to dr_xxx to grd_xxx.  Updated faux function table
 * with init_driver entry.
 * 
 * Revision 1.10  1992/12/30  15:13:40  kaboom
 * Removed calc_vram() and calc_row(), reserved for drivers.  Removed
 * function table, reserved for drivers.
 * 
 * Revision 1.9  1992/12/14  18:13:37  kaboom
 * Changed NULL table entries to gr_null.
 * 
 * Revision 1.8  1992/12/11  20:30:42  kaboom
 * Added slots in function table for wait_display and sub_bm functions.
 * 
 * Revision 1.4  1992/11/12  13:27:20  kaboom
 * Removed gen_bitmap and gen_ubitmap. The dispatching is now done from
 * the actual gr_bitmap macro.
 * 
 * Revision 1.3  1992/10/21  15:54:04  kaboom
 * Added function blanks for additional driver functions, including fixed-
 * point and integral versions of many functions and general, convex and
 * concave polygon functions.  Updated 2d structure prefix from gr_ to be
 * grs_.
 * 
 * Revision 1.1  1992/10/10  12:00:00  kaboom
 * Initial revision.
 */

#include "grs.h"
#include "grnull.h"
#include "general.h"

void gen_fix_ucircle (void)
{ 
}

void gen_fix_circle (void)
{
}

void gen_fix_udisk (void)
{
}

void gen_fix_disk (void)
{
}

void gen_int_urod (void)
{
}

void gen_int_rod (void)
{
}

void gen_fix_urod (void)
{
}

void gen_fix_rod (void)
{
}

//int  gen_flat24_ubitmap (grs_bitmap *bm, short x, short y)
//void gen_flat24_ubitmap (grs_bitmap *bm, short x, short y)

#ifdef INCLUDE_GEN_FUNC_TABLES
void (*gen_func[grd_FUNCS])() = {
   gr_null,          /* set_upixel */
   gr_null,          /* set_pixel */
   gr_null,          /* get_upixel */
   gr_null,          /* get_pixel */

   gen_clear,
   gen_upoint,
   gen_point,
   gen_uhline,
   gen_hline,
   gen_uvline,
   gen_vline,
   gen_urect,
   gen_rect,
   gen_ubox,
   gen_box,

   gen_fix_uline,
   gen_fix_line,
   gen_fix_upoly,
   gen_fix_poly,
   gen_fix_uspoly,
   gen_fix_spoly,
   gen_fix_ucpoly,
   gen_fix_cpoly,
   gen_fix_utmap,
   gen_fix_tmap,

   gen_int_ucircle,
   gen_int_circle,
   gen_fix_ucircle,
   gen_fix_circle,
   gen_int_udisk,
   gen_int_disk,
   gen_fix_udisk,
   gen_fix_disk,
   gen_int_urod,
   gen_int_rod,
   gen_fix_urod,
   gen_fix_rod,

   /* bitmap drawing functions. */
   gr_null,             /* draw bitmap device->device */
   gr_null,
   gen_mono_ubitmap,
   gen_mono_bitmap,
   gen_flat8_ubitmap,
   gen_flat8_bitmap,
   gr_null,             /* draw 24 bit */
   gr_null,
   gen_rsd8_ubitmap,
   gen_rsd8_bitmap,

   /* bitmap get functions. */
   gr_null,             /* get bitmap device->device */
   gr_null,
   gr_null,             /* get mono bitmap */
   gr_null,
   gen_get_flat8_ubitmap,
   gen_get_flat8_bitmap,
   gr_null,             /* get 24 bit bitmap */
   gr_null,
   gr_null,             /* get rsd8 bitmap */
   gr_null,

   /* bitmap transform functions. */
   gen_scale_ubitmap,
   gen_scale_bitmap,
   gen_roll_ubitmap,
   gen_roll_bitmap,
                
   /* text/font functions. */
   gen_ustring,
   gen_string,

   /* span drawing functions. */
   gen_solid_lr,
   gen_opaque_lrpp,
   gr_null,             /* draw_lrii */
   gr_null,             /* draw_lrcc */

   /* device functions. */
   gr_null,             /* init_driver */
   gr_null,             /* init_screen */
   gr_null,             /* save_mode */
   gr_null,             /* rest_mode */
   gr_null,             /* calc_row */
   gr_null,             /* calc_vram */
   gr_null,             /* wait_vsync */
   gr_null,             /* wait_display */
   gr_null,             /* sub_bm */
   gr_null,             /* cut_screen */
   gr_null,             /* set_pal */
   gr_null              /* get_pal */
};
#endif /* INCLUDE_GEN_FUNC_TABLES */
