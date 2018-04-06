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
 * $Source: n:/project/lib/src/2d/RCS/vesa.h $
 * $Revision: 1.2 $
 * $Author: kaboom $
 * $Date: 1993/05/03 14:05:06 $
 *
 * VESA-related data structures.
 *
 * This file is part of the 2d library.
 *
 * $Log: vesa.h $
 * Revision 1.2  1993/05/03  14:05:06  kaboom
 * Took out vesa_mode structure and vesa_get_mode and vesa_set_bank declarations.
 * Added declarations for vesa_get_info() and vesa_get_gran().
 * 
 * Revision 1.1  1993/01/07  21:11:57  kaboom
 * Initial revision
 */

#ifndef __VESA_H
#define __VESA_H
#define VESA_GET_CARD_INFO 0x4f00
#define VESA_GET_MODE_INFO 0x4f01
#define VESA_SET_MODE 0x4f02
#define VESA_GET_MODE 0x4f03

/* VESA support routines prototypes. */
extern int vesa_get_info (grs_sys_info *info);
extern int vesa_get_gran (int mode);
#endif /* !__VESA_H */
