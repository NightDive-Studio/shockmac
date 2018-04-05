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
#ifndef __GAMESTRN_H
#define __GAMESTRN_H

/*
 * $Source: n:/project/cit/src/inc/RCS/gamestrn.h $
 * $Revision: 1.6 $
 * $Author: dc $
 * $Date: 1993/10/18 03:35:48 $
 *
 * $Log: gamestrn.h $
 * Revision 1.6  1993/10/18  03:35:48  dc
 * adding dumb-o @ifndef __SPEW
 * 
 * Revision 1.5  1993/10/08  03:11:27  mahk
 * Added more getters for greater flexibility.
 * 
 * Revision 1.4  1993/09/02  23:07:40  xemu
 * angle me baby
 * 
 * Revision 1.3  1993/08/06  15:20:53  mahk
 * Added long/short name accessors.
 * 
 * Revision 1.2  1993/07/20  14:24:57  mahk
 * Added init_strings
 * 
 * Revision 1.1  1993/07/06  01:13:43  mahk
 * Initial revision
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

// Prototypes
void init_strings(void);
char* get_string(int num,char* buf,int bufsize);
char* get_object_short_name(int triple,char* buf,int bufsize); 
char* get_object_long_name(int triple,char* buf,int bufsize); 
char* get_alloc_string(int num);
char* get_temp_string(int num);
void shutdown_strings(void);
char* get_texture_name(int abs_texture,char* buf, int bufsiz);
char* get_texture_use_string(int abs_texture, char* buf, int bufsiz);

// Globals

#endif // __GAMESTRN_H







