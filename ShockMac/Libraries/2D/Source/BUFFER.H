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
 * $Source: r:/prj/lib/src/2d/RCS/buffer.h $
 * $Revision: 1.2 $
 * $Author: kaboom $
 * $Date: 1994/08/01 22:02:42 $
 *
 * Prototypes for 2d temporary storage management.
 *
 * This file is part of the 2d library.
 */

#define GR_TEMP_USE_MEMSTACK
#ifdef GR_TEMP_USE_MEMSTACK
#include "memall.h"
#include "tmpalloc.h"
#define gr_alloc_temp temp_malloc
#define gr_free_temp temp_free
#else /* GR_TEMP_USE_MEMSTACK */
extern void *gr_alloc_temp (int n);
extern void gr_free_temp (void *p);
#endif /* GR_TEMP_USE_MEMSTACK */
