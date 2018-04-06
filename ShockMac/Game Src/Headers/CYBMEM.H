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
#ifndef __CYBMEM_H
#define __CYBMEM_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/cybmem.h $
 * $Revision: 1.23 $
 * $Author: xemu $
 * $Date: 1994/11/01 09:19:29 $
 *
 */

// Defines

// Typedefs

// Prototypes
errtype load_dynamic_memory(int mask);
errtype free_dynamic_memory(int mask);
int avail_memory(int debug_src);
void Memory_Check();
int slorkatron_memory_check();

// If LZW stuff ever gets lots more efficient, may need to raise this up some.
#define BIG_BUFFER_SIZE (LZW_BUFF_SIZE+3)

#define MINIMUM_GAME_THRESHOLD       540000   // this will be tweaked as appropriate...
#define BIG_CACHE_THRESHOLD         MINIMUM_GAME_THRESHOLD + 1900000

#define EXTRA_TMAP_THRESHOLD        BIG_CACHE_THRESHOLD

#define BLEND_THRESHOLD             EXTRA_TMAP_THRESHOLD + 64000
#define BIG_HACKCAM_THRESHOLD       BLEND_THRESHOLD + 32000

#define PRELOAD_ANIMATION_THRESHOLD EXTRA_TMAP_THRESHOLD  // for simplicity, can be set different if we are psyched

// Globals
#ifdef __CYBMEM_SRC
uchar big_buffer[BIG_BUFFER_SIZE+16384];
int start_mem;
#else
extern uchar big_buffer[BIG_BUFFER_SIZE+16384];
extern int start_mem;
#endif

#endif // __CYBMEM_H
