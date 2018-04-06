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
 * $Source: r:/prj/lib/src/2d/RCS/vtab.h $
 * $Revision: 1.1 $
 * $Author: kevin $
 * $Date: 1994/07/28 01:24:07 $
 *
 * Prototypes for 2d temporary vtab creation.
 *
 * This file is part of the 2d library.
 *
 * $Log: vtab.h $
 * Revision 1.1  1994/07/28  01:24:07  kevin
 * Initial revision
 * 
 * Revision 1.1  1993/02/22  14:33:56  kaboom
 * Initial revision
 * 
 */

#ifndef __VTAB_H
#define __VTAB_H

#if (defined(powerc) || defined(__powerc))	
extern long *gr_make_vtab (grs_bitmap *bm);
#else
extern asm long *gr_make_vtab (grs_bitmap *bm);
#endif

#endif
