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
// Wacky macros and special texture map manipulation functions

// note: you must first include textmaps.h

// Note: places where this won't quite work right...
// cybmem.c, where we reclaim the memory
// textpal.c, where we reclaim the memory (since we loaded it wacky for the selector)
// of course, textmaps.c where we load them in the old way, and in texture_crunch_go there where we unload them

// ok, those should be fixed

#define BUILD_TEXTURE_BITMAPS
extern grs_bitmap *get_texture_map(int idx, int sz);
#define SAFE_TEXTURE (get_texture_map(0,0))

