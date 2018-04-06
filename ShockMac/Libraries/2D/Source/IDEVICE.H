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
 * $Source: n:/project/lib/src/2d/RCS/idevice.h $
 * $Revision: 1.1 $
 * $Author: kaboom $
 * $Date: 1993/05/03 13:46:06 $
 *
 * Symbolic constants for function table references.
 *
 * This file is part of the 2d library.
 *
 * $Log: idevice.h $
 * Revision 1.1  1993/05/03  13:46:06  kaboom
 * Initial revision
 * 
 */

#ifndef __IDEVICE_H
#define __IDEVICE_H
enum {
   GRT_INIT_DEVICE,
   GRT_CLOSE_DEVICE,
   GRT_SET_MODE,
   GRT_GET_MODE,
   GRT_SET_STATE,
   GRT_GET_STATE,
   GRT_STAT_HTRACE,
   GRT_STAT_VTRACE,
   GRT_SET_PAL,
   GRT_GET_PAL,
   GRT_SET_WIDTH,
   GRT_GET_WIDTH,
   GRT_SET_FOCUS,
   GRT_GET_FOCUS,
   GRT_CANVAS_TABLE,
   GRT_SPAN_TABLE,
   GRD_DEVICE_FUNCS
};
#endif /* !__IDEVICE_H */
