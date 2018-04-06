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
 * $Source: r:/prj/lib/src/2d/RCS/general.h $
 * $Revision: 1.51 $
 * $Author: kevin $
 * $Date: 1994/11/12 02:19:47 $
 * 
 * Prototypes for general purpose 2d functions.
 *
 * This file is part of the 2d library.
 *
 * $Log: general.h $
 * Revision 1.51  1994/11/12  02:19:47  kevin
 * Added interrupt crunchy pixel setting declarations.
 * 
 * Revision 1.50  1994/10/25  15:11:36  kevin
 * Added rsd blitters.
 * 
 * Revision 1.49  1994/08/16  15:30:55  kevin
 * Added more temp functions.
 * 
 * Revision 1.48  1994/07/29  12:02:15  kevin
 * added per_map function.
 * 
 * Revision 1.47  1994/07/18  17:05:49  kevin
 * Moved temp_ functions from flat8.h to general.h.
 * 
 * Revision 1.46  1994/07/04  01:35:59  kevin
 * Added per_umap declaration.
 * 
 * 
 * Revision 1.45  1994/06/20  22:19:31  kevin
 * Added temp_wall_umap.
 * 
 * Revision 1.44  1994/05/24  23:53:22  kevin
 * gen_tluc8_clut_lin_umap yay!
 * 
 * Revision 1.43  1994/04/09  07:22:30  lmfeeney
 * added routines for scaled strings
 * 
 * Revision 1.42  1994/03/15  06:08:10  kevin
 * Added clut_bitmap procedures.
 * 
 * Revision 1.41  1994/02/14  20:38:37  baf
 * Added dummy parameter to cpoly and spoly routines, for uniformity needed by 3D.
 * 
 * Revision 1.40  1994/01/17  22:13:18  baf
 * Redid tluc8 spolys (again).
 * 
 * Revision 1.39  1993/12/30  11:04:29  baf
 * non/span solid filled polygons
 * 
 * Revision 1.38  1993/12/28  22:04:37  baf
 * Added solid RSD stuff
 * 
 * Revision 1.37  1993/12/28  19:33:26  kevin
 * Added unpack and chain versions of rsd bitmap functions.
 * 
 * Revision 1.36  1993/12/06  13:08:48  kevin
 * Added declarations for rsd8 versions of texture mappers.
 * 
 * Revision 1.35  1993/12/04  17:31:23  kevin
 * Added gen_clut_per_[u]map declarations;
 * Fixed gen_lit_per_map declaration;
 * 
 * Revision 1.34  1993/12/02  14:35:26  baf
 * Added generic tluc8 scaled bitmaps
 * 
 * Revision 1.33  1993/12/01  21:18:36  baf
 * Added some tluc8 stuff.
 * 
 * Revision 1.32  1993/11/24  01:50:13  kevin
 * Added declarations for wall and floor texture mapping primitives.
 * 
 * Revision 1.31  1993/10/26  02:10:15  kevin
 * Added prototypes for rsd bitmap scaling and clut-scaling primitives.
 * 
 * Revision 1.30  1993/10/20  15:20:43  kaboom
 * Updated prototypes for spoly routines.
 * 
 * Revision 1.29  1993/10/19  09:56:05  kaboom
 * Updated names of polygon routines.
 * 
 * Revision 1.28  1993/10/06  13:30:28  kevin
 * added clut versions of horizontal flip and linear mapping routines.
 * 
 * Revision 1.27  1993/10/02  01:18:21  kaboom
 * Updated names and arguments of linear and perspective mappers.
 * 
 * Revision 1.26  1993/09/02  20:05:23  kaboom
 * Added prototypes for flat 24 pixel, bitmap and linmap routines.
 * 
 * Revision 1.25  1993/08/19  21:51:09  jaemz
 * Added bitmap scale clut and voxel functions
 * 
 * Revision 1.24  1993/08/10  19:07:11  kaboom
 * Added prototypes for gen_lit_{u}tmap().
 * 
 * Revision 1.23  1993/07/20  16:24:52  jaemz
 * Added interp2 and filterw2
 * 
 * Revision 1.22  1993/06/22  19:58:41  spaz
 * Added prototypes for goroud-shaded lines:
 * gen_fix_usline, sline, ucline, and cline.
 * 
 * Revision 1.21  1993/06/14  14:09:55  kaboom
 * Added prototypes for new lin_{u}map and lin_lit_lin_{u}map routines.
 * 
 * Revision 1.20  1993/06/06  15:12:21  kaboom
 * Added prototypes for gen_hflip_flat8_{u}bitmap.
 * 
 * Revision 1.19  1993/06/03  15:12:20  kaboom
 * Moved prototypes for span functions to another file.
 * 
 * Revision 1.18  1993/06/01  13:49:32  kaboom
 * Added prototypes for lighting perspective tmappers.
 * 
 * Revision 1.17  1993/05/03  13:50:48  kaboom
 * Removed the declarations for obsolete span rendering routines.
 * 
 * Revision 1.16  1993/04/08  18:56:04  kaboom
 * Added prototypes for gen_uchar() and gen_char().
 * 
 * Revision 1.15  1993/04/01  21:51:42  kaboom
 * Added full arguments to polygon shader & tmap prototypes.
 * 
 * Revision 1.14  1993/03/29  18:30:59  kaboom
 * Removed prototypes for concave polygons.
 * 
 * Revision 1.13  1993/02/24  11:04:09  kaboom
 * Added prototypes for new span routines.
 * 
 * Revision 1.12  1993/02/04  17:30:23  kaboom
 * Added prototypes for gen_xxx_cpoly() functions.
 * 
 * Revision 1.11  1993/01/22  19:51:17  kaboom
 * Filled in args to gen_int_tmap() macro.
 * 
 * Revision 1.10  1993/01/15  21:44:43  kaboom
 * Added prototypes for stencilled lr drawing routines.
 * 
 * Revision 1.9  1992/12/30  15:14:50  kaboom
 * Added parameters for polygon and span rendering functions.
 * 
 * Revision 1.8  1992/12/14  18:15:58  kaboom
 * Added prototype for gen_clear().
 * 
 * Revision 1.7  1992/12/11  14:13:47  kaboom
 * Added prototypes for gen_get_flat8_[u]bitmap.
 * 
 * Revision 1.6  1992/12/10  11:21:22  kaboom
 * Added prototypes for unclipped versions of span functions.
 * 
 * Revision 1.5  1992/11/19  02:37:06  kaboom
 * Added prototypes for unclipped versions of gen_scale_bitmap as well as
 * gen_roll_bitmap.
 * 
 * Revision 1.4  1992/11/12  13:50:11  kaboom
 * Inserted arguments into prototypes for completed functions.  Added prototypes
 * for monochrome and rsd8 bitmap routines.
 * 
 * Revision 1.3  1992/10/21  16:04:32  kaboom
 * Changed protoypes to match new naming for integer & fixed point functions.
 * 
 * Revision 1.2  1992/10/13  12:18:20  kaboom
 * Added prototypes for fixed-point and integer versions of most 2d
 * functions.
 *
 * Revision 1.1  1992/10/10  12:00:00  kaboom
 * Initial revision.
 */

#ifndef __GENERAL_H
#define __GENERAL_H
#include "plytyp.h" /* must fix */

/* the general-purpose driver has no entries for the following standard
   functions: set_pixel, set_upixel, get_pixel, and get_upixel.  These
   functions must be defined; they are the minimal components of a driver. */

extern int gen_set_pixel(long color, short x, short y);
extern int gen_set_pixel_interrupt(long color, short x, short y);
extern void gen_fill_upixel(long color, short x, short y);
extern int gen_fill_pixel(long color, short x, short y);

extern void gen_clear (long color);

extern void gen_upoint (short x, short y);
extern int gen_point (short x, short y);
extern void gen_uhline (short x0, short y0, short x1);
extern int gen_hline (short x0, short y0, short x1);
extern void gen_uvline (short x0, short y0, short y1);
extern int gen_vline (short x0, short y0, short y1);
extern void gen_urect (short left, short top, short right, short bot);
extern int gen_rect (short left, short top, short right, short bot);
extern void gen_ubox (short left, short top, short right, short bot);
extern int gen_box (short left, short top, short right, short bot);

extern void gen_fix_uline (fix x0, fix y0, fix x1, fix y1);
extern int gen_fix_line (fix x0, fix y0, fix x1, fix y1);
extern void gen_fix_usline(fix x0, fix y0, fix i0, fix x1, fix y1, fix i1);
extern int gen_fix_sline(fix x0, fix y0, fix i0, fix x1, fix y1, fix i1);
extern void gen_fix_ucline(fix x0, fix y0, grs_rgb c0, fix x1, fix y1, grs_rgb c1);
extern int gen_fix_cline(fix x0, fix y0, grs_rgb c0, fix x1, fix y1, grs_rgb c1);
extern void gen_upoly (long c, int n, grs_vertex **vpl);
extern int gen_poly (long c, int n, grs_vertex **vpl);
extern void gen_uspoly(long c, int n, grs_vertex **vpl);
extern int gen_spoly(long c, int n, grs_vertex **vpl);
extern void gen_tluc8_upoly (long c, int n, grs_vertex **vpl);
extern int gen_tluc8_poly (long c, int n, grs_vertex **vpl);
extern void gen_tluc8_uspoly (long c, int n, grs_vertex **vpl);
extern int gen_tluc8_spoly (long c, int n, grs_vertex **vpl);

extern void gen_ucpoly(long c, int n, grs_vertex **vpl);
extern int gen_cpoly(long c, int n, grs_vertex **vpl);

extern int gen_fix_cpoly (int n, fix *vlist, grs_rgb *c);
extern void gen_vox_rect(fix x[4],fix y[4],fix dz[3],int near_ver,grs_bitmap *col,grs_bitmap *ht,int dotw,int doth);
extern void gen_vox_poly(fix x[4],fix y[4],fix dz[3],int near_ver,grs_bitmap *col,grs_bitmap *ht);
extern void gen_vox_cpoly(fix x[4],fix y[4],fix dz[3],int near_ver,grs_bitmap *col,grs_bitmap *ht);
extern void gen_interp2_ubitmap(grs_bitmap *bm);
extern void gen_filter2_ubitmap(grs_bitmap *bm);
extern void gen_scale_ubitmap
   (grs_bitmap *bm, short x, short y, short w, short h);
extern int gen_scale_bitmap
   (grs_bitmap *bm, short x, short y, short w, short h);
extern void gen_rsd8_scale_ubitmap
   (grs_bitmap *bm, short x, short y, short w, short h);
extern int gen_rsd8_scale_bitmap
   (grs_bitmap *bm, short x, short y, short w, short h);
extern void unpack_rsd8_scale_ubitmap
   (grs_bitmap *bm, short x, short y, short w, short h);
extern int unpack_rsd8_scale_bitmap
   (grs_bitmap *bm, short x, short y, short w, short h);
extern void gen_tluc8_scale_ubitmap
   (grs_bitmap *bm, short x, short y, short w, short h);
extern gen_tluc8_scale_bitmap
   (grs_bitmap *bm, short x, short y, short w, short h);

extern void gen_rsd8_clut_scale_ubitmap
   (grs_bitmap *bm, short x, short y, short w, short h, uchar *cl);
extern int gen_rsd8_clut_scale_bitmap
   (grs_bitmap *bm, short x, short y, short w, short h, uchar *cl);
extern void gen_rsd8_scale_solid_ubitmap
   (grs_bitmap *bm, short x, short y, short w, short h, int c);
extern int gen_rsd8_scale_solid_bitmap
   (grs_bitmap *bm, short x, short y, short w, short h, int c);
extern void unpack_rsd8_clut_scale_ubitmap
   (grs_bitmap *bm, short x, short y, short w, short h, uchar *cl);
extern int unpack_rsd8_clut_scale_bitmap
   (grs_bitmap *bm, short x, short y, short w, short h, uchar *cl);
extern void gen_clut_scale_ubitmap
   (grs_bitmap *bm, short x, short y, short w, short h, uchar *cl);
extern void gen_clut_scale_bitmap
   (grs_bitmap *bm, short x, short y, short w, short h, uchar *cl);

extern void gen_roll_ubitmap
   (grs_bitmap *bm, fix angle, short x, short y);
extern gen_roll_bitmap ();

extern void gen_flat8_wall_umap
   (grs_bitmap *bm, int n, fix **vpl);
extern void gen_flat8_lit_wall_umap
   (grs_bitmap *bm, int n, fix **vpl);

extern void gen_flat8_floor_umap
   (grs_bitmap *bm, int n, fix **vpl);
extern void gen_flat8_lit_floor_umap
   (grs_bitmap *bm, int n, fix **vpl);

extern void temp_point(short x, short y);
extern void temp_upoint(short x, short y);

extern void temp_flat8_ubitmap (grs_bitmap *bm, int x, int y);
extern void temp_flat8_bitmap (grs_bitmap *bm, int x, int y);

extern void temp_flat8_mask_bitmap (grs_bitmap *bm, int x, int y, grs_stencil *sten);
extern void temp_flat8_clut_ubitmap (grs_bitmap *bm, int x, int y, uchar *cl);
extern void temp_rsd8_bitmap (grs_bitmap *bm, int x, int y);
extern void temp_rsd8_ubitmap (grs_bitmap *bm, int x, int y);

extern void temp_tluc8_ubitmap (grs_bitmap *bm, int x, int y);

extern int temp_poly (long c, int n, grs_vertex **vpl);
extern void temp_upoly (long c, int n, grs_vertex **vpl);
extern int temp_spoly (long c, int n, grs_vertex **vpl);
extern void temp_uspoly (long c, int n, grs_vertex **vpl);
extern int temp_cpoly (long c, int n, grs_vertex **vpl);
extern void temp_ucpoly (long c, int n, grs_vertex **vpl);
extern int temp_tpoly (long c, int n, grs_vertex **vpl);
extern void temp_utpoly (long c, int n, grs_vertex **vpl);
extern int temp_stpoly (long c, int n, grs_vertex **vpl);
extern void temp_ustpoly (long c, int n, grs_vertex **vpl);


extern void temp_lin_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void temp_lin_map
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void temp_lit_lin_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void temp_lit_lin_map
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void temp_clut_lin_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);
extern void temp_clut_lin_map
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);

extern void temp_wall_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void temp_lit_wall_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void temp_clut_wall_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);

extern void temp_floor_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void temp_lit_floor_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void temp_clut_floor_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);

extern void temp_per_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void temp_per_map
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void temp_lit_per_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void temp_clut_per_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);
extern void temp_clut_per_map
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);

extern void temp_scale_umap
   (grs_bitmap *bm, short x, short y, short w, short h);
extern int temp_scale_map
   (grs_bitmap *bm, short x, short y, short w, short h);
extern void temp_clut_scale_umap
   (grs_bitmap *bm, short x, short y, short w, short h, uchar *cl);
extern int temp_clut_scale_map
   (grs_bitmap *bm, short x, short y, short w, short h, uchar *cl);

extern void gen_per_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern int gen_per_map
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void gen_lit_per_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern int gen_lit_per_map
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void gen_clut_per_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);
extern int gen_clut_per_map
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);
extern void gen_solid_per_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, int c);
extern int gen_solid_per_map
   (grs_bitmap *bm, int n, grs_vertex **vpl, int c);

extern void gen_rsd8_per_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern int gen_rsd8_per_map
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void gen_rsd8_lit_per_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern int gen_rsd8_lit_per_map
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void gen_rsd8_clut_per_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);
extern int gen_rsd8_clut_per_map
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);

extern void gen_lin_lit_utmap
   (int n, fix *vlist, grs_bitmap *bm, fix *m, fix *l);
extern int gen_lin_lit_tmap
   (int n, fix *vlist, grs_bitmap *bm, fix *m, fix *l);
extern void gen_bilin_lit_utmap
   (int n, fix *vlist, grs_bitmap *bm, fix *m, fix *l);
extern int gen_bilin_lit_tmap
   (int n, fix *vlist, grs_bitmap *bm, fix *m, fix *l);

extern void gen_flat8_lin_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern int gen_flat8_lin_map
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void gen_flat24_lin_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern int gen_flat24_lin_map
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void gen_rsd8_lin_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern int gen_rsd8_lin_map
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void gen_tluc8_lin_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern int gen_tluc8_lin_map
   (grs_bitmap *bm, int n, grs_vertex **vpl);

extern void gen_lit_lin_umap
   (int n, grs_bitmap *bm, grs_vertex **vpl);
extern int gen_lit_lin_map
   (int n, grs_bitmap *bm, grs_vertex **vpl);

extern void gen_clut_lin_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);
extern int gen_clut_lin_map
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);
extern void gen_tluc8_clut_lin_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);
extern void gen_flat8_solid_lin_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, int c);
extern int gen_flat8_solid_lin_map
   (grs_bitmap *bm, int n, grs_vertex **vpl, int c);

extern void gen_rsd8_lit_lin_umap
   (int n, grs_bitmap *bm, grs_vertex **vpl);
extern int gen_rsd8_lit_lin_map
   (int n, grs_bitmap *bm, grs_vertex **vpl);

extern void gen_rsd8_clut_lin_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);
extern int gen_rsd8_clut_lin_map
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);

extern void gen_int_ucircle (short x, short y, short r);
extern int gen_int_circle (short x0, short y0, short r);
extern void gen_fix_ucircle (void);
extern void gen_fix_circle (void);
extern void gen_int_udisk (short x0, short y0, short r);
extern void gen_int_disk (short x0, short y0, short r);
extern void gen_fix_udisk (void);
extern void gen_fix_disk (void);
extern void gen_int_urod (void);
extern void gen_int_rod (void);
extern void gen_fix_urod (void);
extern void gen_fix_rod (void);

extern void gen_rsd8_solid_ubitmap (grs_bitmap *bm, short x, short y, int c);
extern int gen_rsd8_solid_bitmap (grs_bitmap *bm, short x, short y, int c);

/* bitmap drawing functions. */
extern void gen_mono_ubitmap (grs_bitmap *bm, short x, short y);
extern int gen_mono_bitmap (grs_bitmap *bm, short x, short y);
extern void gen_flat8_ubitmap (grs_bitmap *bm, short x, short y);
extern int gen_flat8_bitmap (grs_bitmap *bm, short x, short y);
extern void gen_flat24_ubitmap (grs_bitmap *bm, short x0, short y0);
extern int gen_flat24_bitmap (grs_bitmap *bm, short x0, short y0);
extern void gri_gen_rsd8_ubitmap (grs_bitmap *bm, short x, short y);
extern int gri_gen_rsd8_bitmap (grs_bitmap *bm, short x, short y);
extern void unpack_rsd8_ubitmap (grs_bitmap *bm, short x, short y);
extern int unpack_rsd8_bitmap (grs_bitmap *bm, short x, short y);
extern void gen_tluc8_ubitmap (grs_bitmap *bm, short x, short y);
extern int gen_tluc8_bitmap (grs_bitmap *bm, short x, short y);

/* clut bitmap drawing functions. */
extern void gen_flat8_clut_ubitmap (grs_bitmap *bm, short x, short y, uchar *clut);
extern int gen_flat8_clut_bitmap (grs_bitmap *bm, short x, short y, uchar *clut);
extern void unpack_rsd8_clut_ubitmap (grs_bitmap *bm, short x, short y, uchar *clut);
extern int unpack_rsd8_clut_bitmap (grs_bitmap *bm, short x, short y, uchar *clut);

/* bitmap get functions. */
extern void gen_get_flat8_ubitmap (grs_bitmap *bm, short x, short y);
extern int gen_get_flat8_bitmap (grs_bitmap *bm, short x, short y);

/* bitmap horizontal flip routines. */
extern void gen_hflip_flat8_ubitmap (grs_bitmap *bm, short x, short y);
extern int gen_hflip_flat8_bitmap (grs_bitmap *bm, short x, short y);

/* bitmap color lookup table horizontal flip routines. */
extern void gen_clut_hflip_flat8_ubitmap (grs_bitmap *bm, short x, short y, uchar *cl);
extern int gen_clut_hflip_flat8_bitmap (grs_bitmap *bm, short x, short y, uchar *cl);

extern void gen_font_ustring (grs_font *f, char *s, short x, short y);
extern int gen_font_string (grs_font *f, char *s, short x, short y);

extern void gen_font_scale_ustring (grs_font *f, char *s, short x, short y, short w, short h);
extern int gen_font_scale_string (grs_font *f, char *s, short x, short y, short w, short h);

extern void gen_font_uchar (grs_font *f, char c, short x, short y);
extern int gen_font_char (grs_font *f,  char c, short x, short y);

//extern void gen_opaque_ubitmap (grs_bitmap *bm, short x, short y);

#endif /* !__GENERAL_H */
