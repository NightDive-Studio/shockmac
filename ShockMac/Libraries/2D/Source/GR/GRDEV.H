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
 * $Source: n:/project/lib/src/2d/RCS/grdev.h $
 * $Revision: 1.5 $
 * $Author: kaboom $
 * $Date: 1993/10/19 10:15:44 $
 *
 * Macros for table driven device driver functions.
 *
 * This file is part of the 2d library.
 *
 * $Log: grdev.h $
 * Revision 1.5  1993/10/19  10:15:44  kaboom
 * Now includes tabdat.h.  Also put null-check for init and close device
 * in here instead of requiring caller to do it.
 * 
 * Revision 1.4  1993/10/08  01:16:00  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.3  1993/05/16  00:35:28  kaboom
 * Added arguments to macros for gr_set_state() and gr_get_state().
 * 
 * Revision 1.2  1993/05/04  15:41:30  kaboom
 * Changed names of hblank and vblank to htrace and vtrace.
 * 
 * Revision 1.1  1993/04/29  18:36:04  kaboom
 * Initial revision
 */

#ifndef __GRDEV_H
#define __GRDEV_H
#include "grs.h"
#include "idevice.h"
#include "tabdat.h"

/* here are the definitions for all the table driven function. */
#define gr_init_device(info) \
   (grd_device_table[GRT_INIT_DEVICE] ?\
      ((int (*)(grs_sys_info *_info))grd_device_table[GRT_INIT_DEVICE])(info) :\
      0)
#define gr_close_device(info) \
   (grd_device_table[GRT_CLOSE_DEVICE] ?\
      ((int (*)(grs_sys_info *_info))grd_device_table[GRT_CLOSE_DEVICE])(info) :\
      0)
#define gr_set_screen_mode \
   ((int (*)(int mode,int clear))grd_device_table[GRT_SET_MODE])
#define gr_get_screen_mode \
   ((int (*)(void))grd_device_table[GRT_GET_MODE])
#define gr_set_state \
   ((int (*)(void *buf,int clear))grd_device_table[GRT_SET_STATE])
#define gr_get_state \
   ((int (*)(void *buf,int flags))grd_device_table[GRT_GET_STATE])
#define gr_stat_htrace \
   ((int (*)(void))grd_device_table[GRT_STAT_HTRACE])
#define gr_stat_vtrace \
   ((int (*)(void))grd_device_table[GRT_STAT_VTRACE])
#define gr_set_screen_pal \
   ((void (*)(int start,int n,uchar *pal_data))grd_device_table[GRT_SET_PAL])
#define gr_get_screen_pal \
   ((void (*)(int start,int n,uchar *pal_data))grd_device_table[GRT_GET_PAL])
#define gr_set_width \
   ((void (*)(short w))grd_device_table[GRT_SET_WIDTH])
#define gr_get_width \
   ((short (*)(void))grd_device_table[GRT_GET_WIDTH])
#define gr_set_focus \
   ((void (*)(short x,short y))grd_device_table[GRT_SET_FOCUS])
#define gr_get_focus \
   ((void (*)())grd_device_table[GRT_GET_FOCUS])
#endif /* !__GRDEV_H */
