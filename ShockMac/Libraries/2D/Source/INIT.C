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
 * $Source: r:/prj/lib/src/2d/RCS/init.c $
 * $Revision: 1.12 $
 * $Author: kevin $
 * $Date: 1994/12/05 21:07:38 $
 *
 * Routine to initialize the 2d system for use.
 *
 * This file is part of the 2d library.
 *
 */

#include "grd.h"
#include "Init.h"
#include "detect.h"
#include "grdev.h"
#include "state.h"
#include "invtab.h"
#include "memall.h"
#include "tmpalloc.h"
#include "InitInt.h"

/* flag for whether 2d system has been fired up. */
int grd_active = 0;

/* start up 2d system.  try to detect what kind of video hardware is
   present, call device-dependent initialization, and save state.
   returns same as gr_detect() 0 if all is well, or error code. */
int gri_init(void)
{
   int err;
   MemStack *tmp;

   if (grd_active != 0)
      return 0;
   tmp=temp_mem_get_stack();
   if (tmp==NULL)
      if (err=temp_mem_init(NULL))
         return err;
   if ((err=gr_detect (&grd_info)) != 0)
		return err;
   gr_push_video_state (1);
   grd_active = 1;
   init_inverse_table();

   return 0;
}
