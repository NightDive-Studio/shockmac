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
 * $Source: r:/prj/lib/src/2d/RCS/flat8.h $
 * $Revision: 1.50 $
 * $Author: kevin $
 * $Date: 1994/08/16 15:31:34 $
 * 
 * Prototypes for routines for drawing into flat8 bitmaps.
 *
 * This file is part of the 2d library.
 *
 * $Log: flat8.h $
 * Revision 1.50  1994/08/16  15:31:34  kevin
 * removed obsolete scaler function declarations.
 * 
 * Revision 1.49  1994/07/18  17:05:21  kevin
 * Moved temp_ functions to general.h.
 * 
 * Revision 1.48  1994/04/09  07:19:32  lmfeeney
 * added routines for scaled mono bitmaps
 * 
 * Revision 1.47  1994/03/29  17:31:21  kevin
 * Added doofy floor and wall mapper chaining primitives.
 * 
 * Revision 1.46  1994/03/15  13:07:34  kevin
 * Added clut_bitmap procedures.
 * 
 * Revision 1.45  1994/03/14  17:58:22  kevin
 * Added declarations for bitmap doubling routines.
 * 
 * Revision 1.44  1994/02/26  22:50:39  kevin
 * Fixed declarations for texture mapping functions.
 * 
 * Revision 1.43  1994/02/24  22:22:33  baf
 * Added tluc8 clut scaler.
 * 
 * Revision 1.42  1994/02/14  20:38:35  baf
 * Added dummy parameter to cpoly and spoly routines, for uniformity needed by 3D.
 * 
 * Revision 1.41  1994/01/17  22:13:16  baf
 * Redid tluc8 spolys (again).
 * 
 * Revision 1.40  1994/01/13  12:18:47  kevin
 * Added new scaling primitives.
 * 
 * Revision 1.39  1994/01/05  04:30:45  kevin
 * new lit linear mapper.
 * 
 * Revision 1.38  1994/01/03  23:32:03  kevin
 * Added prototypes for new temorary chaining primitives.
 * 
 * Revision 1.37  1993/12/30  11:04:28  baf
 * non/span solid filled polygons
 * 
 * Revision 1.36  1993/12/28  22:04:29  baf
 * Added solid RSD stuff
 * 
 * Revision 1.35  1993/12/04  17:29:57  kevin
 * Added clut_per_umap declaration.
 * 
 * Revision 1.34  1993/12/04  12:14:19  kevin
 * Added declarations for clut wall and floor mappers.
 * ..
 * 
 * Revision 1.33  1993/12/01  21:18:56  baf
 * Added some tluc8 stuff.
 * 
 * Revision 1.32  1993/11/24  01:49:05  kevin
 * Added declarations for wall and floor texture mapping primitives.
 * 
 * Revision 1.31  1993/11/18  23:31:25  kevin
 * Changed flat8 perspective mapper names in honor of installing
 * working versions of eric's spiffy algorithm.
 * 
 * Revision 1.30  1993/11/10  22:46:55  kevin
 * Added declaration for flat8 clut scaler.
 * 
 * Revision 1.29  1993/10/26  02:09:10  kevin
 * Added prototypes for rsd bitmap scaling and clut-scaling primitives.
 * 
 * Revision 1.28  1993/10/20  15:20:29  kaboom
 * Updated prototypes for spoly routines.
 * 
 * Revision 1.27  1993/10/19  09:56:04  kaboom
 * Updated names of polygon routines.
 * 
 * Revision 1.26  1993/10/08  00:37:46  kevin
 * Added declaration for clut linear mapper optimized for flat8 canvasses.
 * 
 * Revision 1.25  1993/10/06  13:28:30  kevin
 * Added clut version of hflip routine.
 * 
 * Revision 1.24  1993/10/02  01:58:13  kaboom
 * Put accidentally deleted prototype for flat8_lin_lit_utmap() back in.
 * 
 * Revision 1.23  1993/10/01  16:02:12  kaboom
 * Updated names for linear and perspective mappers.
 * 
 * Revision 1.22  1993/09/08  21:48:34  kaboom
 * Added prototype for flat8_lin_lit_utmap().
 * 
 * Revision 1.21  1993/09/07  17:45:18  kaboom
 * Renamed flat8_lin_umap to flat8_flat8_lin_umap().
 * 
 * Revision 1.20  1993/09/02  20:04:44  kaboom
 * Added prototypes for 24-bit pixel routines.
 * 
 * Revision 1.19  1993/08/10  19:06:30  kaboom
 * Added prototype for flat8_lit_utmap.
 * 
 * Revision 1.18  1993/08/05  20:07:09  jaemz
 * Added fl8ntrp2 and fl8fltr2
 * 
 * Revision 1.17  1993/07/01  22:11:05  spaz
 * Added prototypes for fl8clin, fl8slin
 * 
 * Revision 1.16  1993/06/14  14:09:38  kaboom
 * Added prototypes for new lin_{u}map and lin_lit_lin_{u}map routines.
 * 
 * Revision 1.15  1993/06/06  15:10:56  kaboom
 * Added prototype for flat8_hflip_flat8_ubitmap().
 * 
 * Revision 1.14  1993/05/03  13:50:16  kaboom
 * Moved declarations for span rendering routines to different file.
 * 
 * Revision 1.13  1993/03/29  18:29:54  kaboom
 * Removed convex_ from polygon scanner names.
 * 
 * Revision 1.12  1993/03/02  19:44:55  kaboom
 * Took out prototype for flat8_int_uline().
 * 
 * Revision 1.11  1993/02/26  17:51:53  kaboom
 * Added prototype for flat8_fix_convex_upoly.
 * 
 * Revision 1.10  1993/02/25  12:58:54  kaboom
 * Added prototypes for flat 8 Gouraud shaders.
 * 
 * Revision 1.9  1993/02/24  11:02:47  kaboom
 * Added prototypes for more span functions.
 * 
 * Revision 1.8  1993/02/22  20:31:28  kaboom
 * Added prototypes for flat 8 span routines.
 * 
 * Revision 1.7  1993/02/16  14:30:33  kaboom
 * Added prototype for flat8_urect().
 * 
 * Revision 1.6  1993/01/07  21:04:21  kaboom
 * Moved declaration for flat8_func here.
 * 
 * Revision 1.5  1992/12/30  15:11:51  kaboom
 * Added prototypes for flat8_calc_vram() and flat8_calc_row().
 * 
 * Revision 1.4  1992/12/14  18:12:45  kaboom
 * Added prototype for flat8_sub_bm().
 * 
 * Revision 1.3  1992/12/11  13:19:29  kaboom
 * Added prototype for flat8_get_flat8_ubitmap.
 * 
 * Revision 1.2  1992/11/19  09:15:16  kaboom
 * Corrected typo---flat8_upoint occurred twice.
 * 
 * Revision 1.1  1992/11/19  02:34:30  kaboom
 * Initial revision
 */

#ifndef __FLAT8_H
#define __FLAT8_H
#include "grs.h"
#include "plytyp.h"
#include "tmapint.h"

/* 8-bit pixel prototypes. */
extern void flat8_set_upixel (long color, short x, short y);
extern int flat8_set_pixel (long color, short x, short y);
extern long flat8_get_upixel (short x, short y);
extern long flat8_get_pixel (short x, short y);

/* 24-bit pixel prototypes. */
extern void flat8_set_upixel24 (long color, short x, short y);
extern int flat8_set_pixel24 (long color, short x, short y);
extern long flat8_get_upixel24 (short x, short y);
extern long flat8_get_pixel24 (short x, short y);

/* straight, rectangular-type primitives. */
#if (defined(powerc) || defined(__powerc))	
extern void flat8_clear (long color);
#else
extern asm void flat8_clear (long color);
#endif
extern void flat8_upoint (short x, short y);
extern int flat8_point (short x, short y);
extern void flat8_uhline (short x0, short y0, short x1);
extern void flat8_uvline (short x0, short y0, short y1);
extern void flat8_urect (short left, short top, short right, short bot);

/* fixed-point rendering-type primitives. */
extern void flat8_fix_uline (fix x0, fix y0, fix x1, fix y1);
extern void flat8_fix_usline (fix x0, fix y0, fix i0, fix x1, fix y1, fix i1);
extern void flat8_fix_ucline (fix x0, fix y0, grs_rgb c0, fix x1, fix y1, grs_rgb c1);
extern void flat8_upoly(long c,int n,grs_vertex **vpl);
extern void flat8_uspoly(long c, int n, grs_vertex **vpl);
extern void flat8_ucpoly(long c, int n, grs_vertex **vpl);
extern void flat8_interp2_ubitmap(grs_bitmap *bm);
extern void flat8_filter2_ubitmap(grs_bitmap *bm);
extern void flat8_tluc8_upoly(long c, int n,grs_vertex **vpl);
extern void flat8_tluc8_uspoly(long c, int n,grs_vertex **vpl);

extern void flat8_flat8_lin_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void flat8_lit_lin_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void flat8_clut_lin_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);
extern void flat8_flat8_solid_lin_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, int c);

extern void flat8_flat8_wall_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void flat8_flat8_lit_wall_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void flat8_flat8_clut_wall_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);

extern void flat8_flat8_floor_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void flat8_flat8_lit_floor_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern void flat8_flat8_clut_floor_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);

extern int flat8_flat8_per_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern int flat8_flat8_lit_per_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl);
extern int flat8_flat8_clut_per_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, uchar *cl);
extern int flat8_flat8_solid_per_umap
   (grs_bitmap *bm, int n, grs_vertex **vpl, int c);

extern void flat8_lin_lit_utmap (int n, fix *vlist, grs_bitmap *bm, fix *m, fix *l);

extern void flat8_scale_ubitmap (grs_bitmap *bm, short x, short y,
   short w, short h);
extern int flat8_scale_bitmap (grs_bitmap *bm, short x, short y,
   short w, short h);


extern void flat8_mono_scale_ubitmap (grs_bitmap *bm, short x, short y,
   short w, short h);
extern int flat8_mono_scale_bitmap (grs_bitmap *bm, short x, short y,
   short w, short h);

extern void flat8_rsd8_scale_ubitmap (grs_bitmap *bm, short x, short y,
   short w, short h);
extern int flat8_rsd8_scale_bitmap (grs_bitmap *bm, short x, short y,
   short w, short h);

extern void flat8_flat8_clut_scale_ubitmap
   (grs_bitmap *bm, short x, short y, short w, short h, uchar *cl);

extern void flat8_rsd8_clut_scale_ubitmap
   (grs_bitmap *bm, short x, short y, short w, short h, uchar *cl);
extern int flat8_rsd8_clut_scale_bitmap
   (grs_bitmap *bm, short x, short y, short w, short h, uchar *cl);

extern void flat8_rsd8_scale_solid_ubitmap
   (grs_bitmap *bm, short x, short y, short w, short h, int c);
extern int flat8_rsd8_scale_solid_bitmap
   (grs_bitmap *bm, short x, short y, short w, short h, int c);

extern void flat8_tluc8_scale_ubitmap (grs_bitmap *bm, short x, short y,
   short w, short h);
extern int flat8_tluc8_scale_bitmap (grs_bitmap *bm, short x, short y,
   short w, short h);

extern void flat8_tluc8_clut_scale_ubitmap (grs_bitmap *bm, short x, short y,
   short w, short h, uchar *cl);
extern int flat8_tluc8_clut_scale_bitmap (grs_bitmap *bm, short x, short y,
   short w, short h, uchar *cl);

// internal scaler/mapper prototypes
int gri_opaque_solid_scale_umap_init(grs_tmap_loop_info *info, grs_vertex **vert);
int gri_trans_scale_umap_init(grs_tmap_loop_info *, grs_vertex **);
int gri_trans_solid_scale_umap_init(grs_tmap_loop_info *info, grs_vertex **vert);
int gri_opaque_scale_umap_init(grs_tmap_loop_info *tli);
int gri_trans_clut_scale_umap_init(grs_tmap_loop_info *tli);
int gri_opaque_clut_scale_umap_init(grs_tmap_loop_info *tli);

/* bitmap drawing functions. */
extern void flat8_mono_ubitmap (grs_bitmap *bm, short x, short y);
#if (defined(powerc) || defined(__powerc))	
extern void flat8_flat8_ubitmap (grs_bitmap *bm, short x, short y);
#else
extern asm void flat8_flat8_ubitmap (grs_bitmap *bm, short x, short y);
#endif
extern void flat8_rsd8_ubitmap (grs_bitmap *bm, short x, short y);
extern void flat8_tluc8_ubitmap (grs_bitmap *bm, short x, short y);
extern int flat8_rsd8_bitmap (grs_bitmap *bm, short x, short y);

extern void flat8_flat8_clut_ubitmap (grs_bitmap *bm, short x, short y, uchar *cl);

extern void flat8_rsd8_solid_ubitmap (grs_bitmap *bm, short x, short y, int c);
extern int flat8_rsd8_solid_bitmap (grs_bitmap *bm, short x, short y, int c);

/* bitmap get routines. */
extern void flat8_get_flat8_ubitmap (grs_bitmap *bm, short x, short y);

/* bitmap horizontal flip routines. */
extern void flat8_hflip_flat8_ubitmap (grs_bitmap *bm, short x, short y);

/* bitmap color lookup table horizontal flip routines. */
extern void flat8_clut_hflip_flat8_ubitmap (grs_bitmap *bm, short x, short y, uchar *cl);

/* device-specific routines. */
extern short flat8_calc_row (short w);
extern grs_bitmap *flat8_sub_bitmap (grs_bitmap *bm, short x, short y,
   short w, short h);

/* bitmap doubling routines. */
extern void flat8_flat8_h_double_ubitmap (grs_bitmap *bm);
extern void flat8_flat8_smooth_h_double_ubitmap(grs_bitmap *srcb, grs_bitmap *dst);
extern void flat8_flat8_smooth_hv_double_ubitmap(grs_bitmap *src, grs_bitmap *dst);

extern void flat8_flat8_v_double_ubitmap (grs_bitmap *bm);
extern void flat8_flat8_hv_double_ubitmap (grs_bitmap *bm);
extern void flat8_flat8_smooth_v_double_ubitmap (grs_bitmap *bm);
#endif /* !__FLAT8_H */
