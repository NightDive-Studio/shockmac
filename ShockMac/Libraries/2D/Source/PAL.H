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
 * $Source: r:/prj/lib/src/2d/RCS/pal.h $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/07/28 23:40:18 $
 *
 * Prototypes and macros for palette manipulation routines.
 *
 * This file is part of the 2d library.
 *
 * $Log: pal.h $
 * Revision 1.3  1994/07/28  23:40:18  kevin
 * c:\dev\mach64\install 33 gamma_set_pal prototype.
 * 
 * Revision 1.2  1994/06/18  04:05:47  lmfeeney
 * added set palette with gamma correct
 * ,
 * 
 * Revision 1.1  1993/02/04  17:43:02  kaboom
 * Initial revision
 * 
 */

#ifndef __PAL_H
#define __PAL_H

extern void gr_set_pal (int start, int n, uchar *pal_data);
extern void gr_set_gamma_pal (int start, int n, fix gamma);
extern void gr_get_pal (int start, int n, uchar *pal_data);

#endif /* !__PAL_H */
