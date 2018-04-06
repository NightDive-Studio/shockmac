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
 * $Source: n:/project/lib/src/2d/RCS/grd.h $
 * $Revision: 1.7 $
 * $Author: kaboom $
 * $Date: 1993/10/19 10:20:07 $
 *
 * Declarations for globals.
 *
 * This file is part of the 2d library.
 *
 * $Log: grd.h $
 * Revision 1.7  1993/10/19  10:20:07  kaboom
 * Moved declarations for canvas-related globals to other files.
 * 
 * Revision 1.6  1993/10/08  01:15:58  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.5  1993/10/06  16:13:18  baf
 * Added span_texture global
 * 
 * Revision 1.4  1993/09/07  02:21:57  kaboom
 * New declaration for grd_mode.
 * 
 * Revision 1.3  1993/05/03  16:47:18  kaboom
 * Added declaration for grd_pixel_table.
 * 
 * Revision 1.2  1993/04/29  18:35:34  kaboom
 * Moved some of the globals to other files.
 * 
 * Revision 1.1  1993/02/04  17:34:19  kaboom
 * Initial revision
 */

#ifndef __GRD_H
#define __GRD_H
#include "grs.h"

extern grs_sys_info grd_info;
extern grs_drvcap *grd_cap;
extern grs_drvcap grd_mode_cap;
extern void (**grd_driver_list[])();
extern int grd_mode;

/* support old-syle dr_ naming for now. */
#define grd_scr_canv grd_screen_canvas
#define grd_vis_canv grd_visible_canvas
#define dr_screen grd_screen
#define dr_canvas grd_canvas
#define dr_scr_canv grd_screen_canvas
#define dr_vis_canv grd_visible_canvas

#define dr_bm       grd_bm
#define dr_gc       grd_gc
#define dr_ytab     grd_ytab
#define dr_int_clip grd_int_clip
#define dr_fix_clip grd_fix_clip
#define dr_clip     grd_clip
#define driver_func grd_driver_func

#endif /* !__GRD_H */
