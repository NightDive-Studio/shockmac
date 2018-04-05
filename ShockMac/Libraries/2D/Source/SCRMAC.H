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
 * $Source: n:/project/lib/src/2d/RCS/scrmac.h $
 * $Revision: 1.1 $
 * $Author: kaboom $
 * $Date: 1993/10/19 10:24:50 $
 *
 *
 *
 * This file is part of the 2d library.
 *
 * $Log: scrmac.h $
 * Revision 1.1  1993/10/19  10:24:50  kaboom
 * Initial revision
 * 
 */

#ifndef __SCRMAC_H
#define __SCRMAC_H
#include "scrdat.h"

/* macros for getting & setting elements of the current screen. */
#define gr_get_light_tab() (grd_screen->ltab)
#define gr_set_light_tab(p) (grd_screen->ltab=(p))

#define gr_get_clut() (grd_screen->clut)
#define gr_set_clut(cl) (grd_screen->clut=(cl))
#endif /* !__SCRMAC_H */

