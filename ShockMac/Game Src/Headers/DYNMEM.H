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
#define DYNMEM_ALL         0xFFFFFFFF
#define DYNMEM_NONE        0x00000000
#define DYNMEM_TEXTURES    0x00000001
#define DYNMEM_SIDEICONS   0x00000002
#define DYNMEM_FHANDLE_1   0x00000004
#define DYNMEM_FHANDLE_2   0x00000008
#define DYNMEM_FHANDLE_3   0x00000010
#define DYNMEM_FHANDLE_4   0x00000020
#define DYNMEM_FILEHANDLES DYNMEM_FHANDLE_1|DYNMEM_FHANDLE_2|DYNMEM_FHANDLE_3|DYNMEM_FHANDLE_4
#define DYNMEM_PARTIAL     DYNMEM_SIDEICONS|DYNMEM_FHANDLE_1|DYNMEM_FHANDLE_3|DYNMEM_FHANDLE_4
