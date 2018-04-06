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
 * $Source: n:/project/lib/src/2d/RCS/mode.h $
 * $Revision: 1.3 $
 * $Author: kaboom $
 * $Date: 1993/10/08 01:16:10 $
 *
 * Constants for graphics modes.
 *
 * This file is part of the 2d library.
 *
 * $Log: mode.h $
 * Revision 1.3  1993/10/08  01:16:10  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.2  1993/05/16  00:36:57  kaboom
 * Added mirror constants with lower case x's.
 * 
 * Revision 1.1  1993/05/03  13:46:37  kaboom
 * Initial revision
 */

#ifndef __MODE_H
#define __MODE_H

#include "grs.h"
/* definitions for all supported device-independent graphics modes. */
enum {
   GRM_320x200x8,
   GRM_320x200x8X,
   GRM_320x400x8,
   GRM_320x240x8,
   GRM_320x480x8,
   GRM_640x400x8,
   GRM_640x480x8,
   GRM_800x600x8,
   GRM_1024x768x8,
   GRM_1280x1024x8,
   GRM_320x200x24,
   GRM_640x480x24,
   GRM_800x600x24,
   GRM_1024x768x24,
   GRM_1280x1024x24
};

enum {
   GRM_320X200X8,
   GRM_320X200X8X,
   GRM_320X400X8,
   GRM_320X240X8,
   GRM_320X480X8,
   GRM_640X400X8,
   GRM_640X480X8,
   GRM_800X600X8,
   GRM_1024X768X8,
   GRM_1280X1024X8,
   GRM_320X200X24,
   GRM_640X480X24,
   GRM_800X600X24,
   GRM_1024X768X24,
   GRM_1280X1024X24,
   GRD_MODES
};

extern grs_mode_info grd_mode_info[];
extern int gr_set_mode (int mode, int clear);
#endif /* !__MODE_H */
