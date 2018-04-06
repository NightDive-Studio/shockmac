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
 * $Source: n:/project/lib/src/2d/RCS/valloc.h $
 * $Revision: 1.2 $
 * $Author: kaboom $
 * $Date: 1993/05/03 14:01:59 $
 *
 * Declarations for video memory manager.
 *
 * This file is part of the 2d library.
 *
 * $Log: valloc.h $
 * Revision 1.2  1993/05/03  14:01:59  kaboom
 * Added declaration for grd_valloc_mode.
 * 
 * Revision 1.1  1993/02/22  20:35:27  kaboom
 * Initial revision
 */

#ifndef __VALLOC_H
#define __VALLOC_H
extern uchar grd_valloc_mode;
extern uchar *valloc (short w, short h);
extern void vfree (uchar *p);
#endif /* !__VALLOC_H */
