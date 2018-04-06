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
 * $Source: n:/project/lib/src/2d/RCS/fill.h $
 * $Revision: 1.8 $
 * $Author: unknown $
 * $Date: 1993/11/19 17:23:18 $
 *
 * Fill constants.
 *
 * This file is part of the 2d library.
 *
 * $Log: fill.h $
 * Revision 1.8  1993/11/19  17:23:18  unknown
 * Removed tluc fill type
 * 
 * Revision 1.7  1993/11/18  03:57:56  baf
 * Added tluc fill type
 * 
 * Revision 1.6  1993/10/08  19:54:24  baf
 * Added solid fill mode
 * 
 * Revision 1.5  1993/08/23  14:44:32  jaemz
 * Added blending primitives
 * 
 * Revision 1.4  1993/06/03  15:07:31  kaboom
 * Added constant for XOR fill mode.
 * 
 * Revision 1.3  1993/05/03  13:49:17  kaboom
 * Added GRD_FILL_TYPES to end of enum instead of as define.
 * 
 * Revision 1.2  1993/02/24  10:56:24  kaboom
 * Took out FILL_TRANS and added FILL_CLUT.
 * 
 * Revision 1.1  1993/02/04  17:12:12  kaboom
 * Initial revision
 */

#ifndef __FILL_H
#define __FILL_H
/* span fill types. */
enum {
   FILL_NORM,
   FILL_CLUT,
   FILL_XOR,
   FILL_BLEND,
   FILL_SOLID,
   GRD_FILL_TYPES
};
#endif /* !__FILL_H */
