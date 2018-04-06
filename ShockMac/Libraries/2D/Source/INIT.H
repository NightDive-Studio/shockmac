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
 * $Source: r:/prj/lib/src/2d/RCS/init.h $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/12/05 21:13:53 $
 *
 * Declarations for public initialization functions.
 *
 * This file is part of the 2d library.
 *
 * $Log: init.h $
 * Revision 1.2  1994/12/05  21:13:53  kevin
 * reworked to aviod linking in unused function tables.
 * 
 * Revision 1.1  1993/05/03  13:46:14  kaboom
 * Initial revision
 * 
 */

#ifndef __INIT_H
#define __INIT_H
extern int gr_init (void);
extern int gr_vga_init (void);
extern int gr_flat8_init (void);
#define gr_svga_init gr_init
#define gr_vesa_init gr_init
#endif /* !__INIT_H */
