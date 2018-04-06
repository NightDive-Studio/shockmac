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
 * $Source: r:/prj/lib/src/2d/RCS/devtab.h $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/12/05 21:13:51 $
 *
 * Declaration for device table list.
 *
 * This file is part of the 2d library.
 *
 * $Log: devtab.h $
 * Revision 1.2  1994/12/05  21:13:51  kevin
 * reworked to aviod linking in unused function tables.
 * 
 * Revision 1.1  1993/05/03  13:45:02  kaboom
 * Initial revision
 * 
 */

#ifndef __DEVTAB_H
#define __DEVTAB_H
extern void (*flat8_device_table[])();
extern void (*vga_device_table[])();
extern void (*vesa_device_table[])();
#endif /* !__DEVTAB_H */
