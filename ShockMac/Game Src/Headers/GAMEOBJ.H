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
#ifndef __FAUXOBJD_H
#define __FAUXOBJD_H
/*
 * $Source: n:/project/cit/src/inc/RCS/gameobj.h $
 * $Revision: 1.2 $
 * $Author: dc $
 * $Date: 1994/01/22 23:30:59 $
 */

#include "objects.h"

void show_obj(ObjRefID oRef);

// list of special obj numbers for renderer
#define FAUBJ_UNKNOWN   0
#define FAUBJ_TEXTPOLY  1
#define FAUBJ_BITMAP    2
#define FAUBJ_TPOLY     3
#define FAUBJ_CRIT      4
#define FAUBJ_ANIMPOLY  5
#define FAUBJ_VOX       6
#define FAUBJ_NOOBJ     7
#define FAUBJ_TEXBITMAP 8
#define FAUBJ_FLATPOLY  9
#define FAUBJ_MULTIVIEW 10
#define FAUBJ_SPECIAL   11
#define FAUBJ_TL_POLY   12

#define NUM_OBJ_RENDER_TYPES  (FAUBJ_TL_POLY+1)

#endif // __FAUXOBJD_H
