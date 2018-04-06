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
// lgsprntf.h -- no-floats baby sprintf

// Tim Stellmach
// LookingGlass Technologies 1993

/*
* $Header: n:/project/lib/src/lg/rcs/lgsprntf.h 1.4 1994/02/27 02:25:50 tjs Exp $
* $Log: lgsprntf.h $
 * Revision 1.4  1994/02/27  02:25:50  tjs
 * Changed format to const
 * 
 * Revision 1.3  1993/11/04  09:44:35  tjs
 * Added this very header.
 * 
 * 
 *
 *
 *
*/

#include "lg_types.h"
#include <stdarg.h>

int lg_sprintf(char *buf, const char *format, ...);
int lg_vsprintf(char *buf, const char *format, va_list arglist);
void lg_sprintf_install_stringfunc(char *(*func)(ulong strnum));
