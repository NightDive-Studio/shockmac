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
 * $Source: n:/project/lib/src/2d/RCS/screen.h $
 * $Revision: 1.2 $
 * $Author: kaboom $
 * $Date: 1993/05/03 13:56:43 $
 *
 * Screen handling prototypes.
 *
 * This file is part of the 2d library.
 *
 * $Log: screen.h $
 * Revision 1.2  1993/05/03  13:56:43  kaboom
 * Added prototype for gr_set_screen().
 * 
 * Revision 1.1  1993/02/04  17:44:28  kaboom
 * Initial revision
 */

#ifndef __SCREEN_H
#define __SCREEN_H
extern grs_screen *gr_alloc_screen (short w, short h);
extern void gr_free_screen (grs_screen *s);
extern void gr_set_screen (grs_screen *s);
#endif /* !__SCREEN_H */
