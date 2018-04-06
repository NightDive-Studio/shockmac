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
#ifndef __SAVELOAD_H
#define __SAVELOAD_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/saveload.h $
 * $Revision: 1.9 $
 * $Author: tjs $
 * $Date: 1994/09/20 13:19:55 $
 *
 *
 */

// Includes

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes


// Defines
#define CFG_LEVEL_VAR "LEVEL"
#define OLD_LEVEL_ID_NUM   540
#define LEVEL_ID_NUM       100

// Prototypes
errtype save_current_map(FSSpec* fSpec, Id id_num, bool flush_mem, bool pack);
errtype load_current_map(Id id_num, FSSpec* dpath);
bool go_to_different_level(int targlevel);

// Globals

#endif // __SAVELOAD_H

