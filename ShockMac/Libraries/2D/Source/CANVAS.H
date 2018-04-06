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
 * $Source: n:/project/lib/src/2d/RCS/canvas.h $
 * $Revision: 1.2 $
 * $Author: kaboom $
 * $Date: 1993/06/02 16:31:39 $
 *
 * Prototypes for canvas routines.
 *
 * This file is part of the 2d library.
 *
 * $Log: canvas.h $
 * Revision 1.2  1993/06/02  16:31:39  kaboom
 * Added prototype for gr_make_canvas().
 * 
 * Revision 1.1  1993/02/04  17:00:25  kaboom
 * Initial revision
 * 
 */

#ifndef __CANVAS_H
#define __CANVAS_H

extern void gr_set_canvas (grs_canvas *c);
extern int gr_push_canvas (grs_canvas *c);
extern grs_canvas *gr_pop_canvas (void);
extern void gr_make_canvas (grs_bitmap *bm, grs_canvas *c);
extern void gr_init_canvas (grs_canvas *c, uchar *p, int type, short w, short h);
extern void gr_init_sub_canvas (grs_canvas *sc, grs_canvas *dc,
                                short x, short y, short w, short h);
extern grs_canvas *gr_alloc_canvas (int type, short w, short h);
extern void gr_free_canvas (grs_canvas *c);
extern grs_canvas *gr_alloc_sub_canvas (grs_canvas *c, short x, short y,
                                        short w, short h);
extern void gr_free_sub_canvas (grs_canvas *c);

#endif /* !__CANVAS_H */
