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
 * $Source: n:/project/lib/src/ui/RCS/curdat.h $
 * $Revision: 1.2 $
 * $Author: mahk $
 * $Date: 1994/02/07 16:14:57 $
 *
 * Declarations for cursor globals.
 *
 * $Log: curdat.h $
 * Revision 1.2  1994/02/07  16:14:57  mahk
 * Changed canvas variables.
 * 
 * Revision 1.1  1993/12/16  07:46:35  kaboom
 * Initial revision
 * 
 */

#include "curtyp.h"

extern int MouseLock;
extern LGRegion* CursorRegion;
extern LGCursor* CurrentCursor;
extern LGCursor* LastCursor;
extern LGRegion* LastCursorRegion;
extern LGPoint LastCursorPos;
extern LGRect* HideRect;
extern int curhiderect;
extern grs_canvas* CursorCanvas;
extern grs_canvas DefaultCursorCanvas;
extern struct _cursor_saveunder SaveUnder;

