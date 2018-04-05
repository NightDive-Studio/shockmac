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
 * $Source: r:/prj/lib/src/2d/RCS/scrdat.h $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/10/19 17:51:25 $
 *
 * Declarations for current screen and related globals.
 *
 * This file is part of the 2d library.
 *
 * $Log: scrdat.h $
 * Revision 1.2  1994/10/19  17:51:25  kevin
 * Make grd_bpal, grd_pal, grd_ipal into globals.
 * 
 * Revision 1.1  1993/10/19  10:24:38  kaboom
 * Initial revision
 * 
 */

#ifndef __SCRDAT_H
#define __SCRDAT_H
#include "grs.h"

extern grs_screen *grd_screen;
extern uchar grd_default_pal[];
extern uchar *grd_pal;
extern grs_rgb grd_default_bpal[];
extern grs_rgb *grd_bpal;
extern uchar *grd_ipal;

#endif /* !__SCRDAT_H */
