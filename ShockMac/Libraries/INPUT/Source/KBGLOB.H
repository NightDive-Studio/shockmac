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
 * $Source: n:/project/lib/src/input/RCS/kbglob.h $
 * $Revision: 1.1 $
 * $Author: kaboom $
 * $Date: 1994/02/12 18:22:07 $
 *
 * Declarations for low buffer globals.
 *
 * This file is part of the input library.
 */

extern long *kbd_global;

enum {
   KBI_QUEUE_HEAD,
   KBI_LAST_CODES,
   KBI_OLD_REAL_HANDLER,
   KBI_STATUS_FLAGS
};

