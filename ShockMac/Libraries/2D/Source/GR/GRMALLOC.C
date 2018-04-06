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
 * $Source: n:/project/lib/src/2d/RCS/grmalloc.c $
 * $Revision: 1.1 $
 * $Author: kaboom $
 * $Date: 1993/02/04 17:35:34 $
 *
 * 2d internal memory allocation routines.
 *
 * This file is part of the 2d library.
 */

#include <stdlib.h>
#include "grmalloc.h"

/* dynamic memory allocation/deallocation is done through the indirected
   functions gr_malloc() and gr_free(). they default to malloc() and
   free(). library clients can change the default with gr_set_malloc()
   and gr_set_free(). */
typedef void *(*ptr_type)(int);
typedef void (*free_type)(void *);

void *(*gr_malloc)(int n) = (ptr_type) malloc;
void (*gr_free)(void *m) = (free_type)free;

/* set 2d's internal function pointer to a malloc routine. */
void gr_set_malloc (void *(*malloc_func)(int bytes))
{
   gr_malloc = malloc_func;
}

/* set 2d's internal function pointer to a free routine. */
void gr_set_free (void (*free_func)(void *mem))
{
   gr_free = free_func;
}
