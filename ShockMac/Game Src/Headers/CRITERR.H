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
#ifndef __CRITERR_H
#define __CRITERR_H

/*
 * $Source: n:/project/cit/src/inc/RCS/criterr.h $
 * $Revision: 1.3 $
 * $Author: xemu $
 * $Date: 1994/05/26 17:20:11 $
 *
 */

// Defines
#define CRITERR_CLASSES	16
#define CRITERR_CODES		0x10000

#define CRITERR_CFG     		0x1000  // config errors
#define CRITERR_RES     		0x2000  // Resource errors
#define CRITERR_MEM     	0x3000  // Out of memory 
#define CRITERR_FILE    		0x4000   // Misc file error
#define CRITERR_EXEC    	0x5000   // execution error
#define CRITERR_MISC    	0xF000  // miscellaneous/glitches
#define CRITERR_TEST    	0x0000  // Test code 

#define NO_CRITICAL_ERROR	0x0000  

// Prototypes
void criterr_init(void);
// Initializes critical error system.

void critical_error(short code);
// Exits the program with error status, printing 
// the error string for the specified code. 

#endif // __CRITERR_H
