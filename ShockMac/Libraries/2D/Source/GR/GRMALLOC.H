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
 * $Source: n:/project/lib/src/2d/RCS/grmalloc.h $
 * $Revision: 1.1 $
 * $Author: kaboom $
 * $Date: 1993/02/04 17:35:18 $
 *
 * Declarations for 2d internal memory allocation routines.
 *
 * This file is part of the 2d library.
 */

#ifndef __GRMALLOC_H
#define __GRMALLOC_H

extern void gr_set_malloc (void *(*malloc_func)(int bytes));
extern void gr_set_free (void (*free_func)(void *mem));
extern void *(*gr_malloc)(int n);
extern void (*gr_free)(void *p);

#endif /* !__GRMALLOC_H */
