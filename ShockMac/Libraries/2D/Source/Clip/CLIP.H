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
 * $Source: n:/project/lib/src/2d/rcs/clip.h $
 * $Revision: 1.11 $
 * $Author: kaboom $
 * $Date: 1993/09/02 19:41:01 $
 *
 * Prototypes for 2d clippers, constants for clipping codes.
 *
 * $Log: clip.h $
 * Revision 1.11  1993/09/02  19:41:01  kaboom
 * Added prototype for 24-bit bitmap clipper.
 * 
 * Revision 1.10  1993/08/10  18:48:49  kaboom
 * Added prototype for gr_clip_spoly.
 * 
 * Revision 1.9  1993/02/22  14:32:46  kaboom
 * Changed name of gr_clip_int_rect() to gr_clip_rect().  Removed the
 * prototypes for gr_clip_fix_rect() and gr_clip_int_poly().
 * 
 * Revision 1.8  1993/02/19  13:12:04  jaemz
 * Added definition for cpoly clipper
 * 
 * Revision 1.7  1993/02/04  17:09:02  kaboom
 * Fixed bug in prototype for gr_clip_fix_rect.  Moved clip code defines
 * to here.
 * 
 * Revision 1.6  1993/01/14  18:30:46  kaboom
 * Added prototype for gr_clip_fix_poly().  Changed clip_xxx to gr_clip_xxx.
 * 
 * Revision 1.5  1993/01/12  00:03:07  kaboom
 * Added prototype for clip_int_poly().
 * 
 * Revision 1.4  1992/11/12  13:46:58  kaboom
 * Changed flat8 bitmap clipper to not allocate a new bitmap; now it changes the
 * bitmap passed in and returns the clip code.  Added prototype for monochrome
 * bitmap clipper.
 * 
 * Revision 1.3  1992/10/21  16:02:32  kaboom
 * Updates references to gr_xxx structures to grs_xxx.
 * 
 * Revision 1.2  1992/10/13  17:40:12  kaboom
 * Added prototypes for clip_fix_line, clip_int_rect, clip_fix_rect, and also
 * for clip_int_bitmap.
 * 
 * Revision 1.1  1992/10/09  16:51:22  kaboom
 * Initial revision
 */

#ifndef __CLIP_H
#define __CLIP_H
/* prototypes for analytic clippers. */
extern int gr_clip_int_line (short *x0, short *y0, short *x1, short *y1);
extern int gr_clip_fix_line (long *x0, long *y0, long *x1, long *y1);
extern int gr_clip_fix_poly (int n, fix *vlist, fix *clist);
extern int gr_clip_spoly (int n, fix *vlist, fix *clist, fix *ilist, fix *cilist);
extern int gr_clip_fix_cpoly (int n, fix *vlist, grs_rgb *blist, fix *clist, grs_rgb *cblist);
extern int gr_clip_rect (short *left, short *top, short *right, short *bot);
extern int gr_clip_mono_bitmap (grs_bitmap *bm, short *x, short *y);
extern int gr_clip_flat8_bitmap (grs_bitmap *bm, short *x, short *y);
extern int gr_clip_flat24_bitmap (grs_bitmap *bm, short *x, short *y);

/* clip codes. */
#define CLIP_NONE    0
#define CLIP_LEFT    1
#define CLIP_TOP     2
#define CLIP_RIGHT   4
#define CLIP_BOT     8
#define CLIP_ALL     16
#endif /* !__CLIP_H */
