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
 * $Source: n:/project/lib/src/2d/RCS/grnull.c $
 * $Revision: 1.2 $
 * $Author: kaboom $
 * $Date: 1993/08/20 15:05:22 $
 *
 * Null-function placeholder.
 *
 * This file is part of the 2d library.
 *
 * $Log: grnull.c $
 * Revision 1.2  1993/08/20  15:05:22  kaboom
 * Took out mprintf.
 * 
 * Revision 1.1  1993/02/04  17:36:07  kaboom
 * Initial revision
 */

#include "grnull.h"

void gr_null (void) {}
void gr_not_imp (void) {DebugStr("\pGraphics function not implemented");}
void gr_not_imp_test (void) {} 
