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
 * $Source: r:/prj/lib/src/2d/RCS/grd.c $
 * $Revision: 1.10 $
 * $Author: kevin $
 * $Date: 1994/12/05 21:07:24 $
 *
 * Global stuff.
 *
 * This file is part of the 2d library.
 */

#include "grs.h"

/* pointer to the currently set screen. */
grs_screen *grd_screen=NULL;

/* pointer to palette */
uchar grd_default_pal[768];
uchar *grd_pal=grd_default_pal;

/* pointer to blend palette */
grs_rgb grd_default_bpal[1024];
grs_rgb *grd_bpal=grd_default_bpal;

/* pointer to inverse palette */
uchar *grd_ipal=NULL;

/* pointer to a canvas for the current virtual screen. */
grs_canvas *grd_screen_canvas;

/* pointer to a canvas for the visible sub-region of the virtual screen. */
grs_canvas *grd_visible_canvas;

/* pointer to currently set canvas. */
grs_canvas *grd_canvas;

/* info for current graphics setup. */
grs_sys_info grd_info;

grs_drvcap grd_mode_cap;

/* capability info for currently set driver. */
grs_drvcap *grd_cap = &grd_mode_cap;

/* pointer to start of current device driver's function table. */
void (**grd_device_table)();

void (**grd_pixel_table)();

/* pointer to start of current bitmap driver's function table for clipped
   primitives. */
void (**grd_canvas_table)();

/* currently active graphics mode. -1 means unrecognized mode */
int grd_mode=-1;

/* flag for whether we are executing in an interrupt. */
bool grd_interrupt=0;

/* Function chaining globals.  Set during gr_set_canvas; that's why I moved them here. */
short grd_pixel_index, grd_canvas_index;
uchar chn_flags;

/* Graphics capability detection function pointer. */
int (*grd_detect_func)(grs_sys_info *info);
