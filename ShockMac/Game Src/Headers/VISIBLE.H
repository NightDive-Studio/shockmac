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
#ifdef VIS_FROM_NEWAI
#define VISIBLE_MASS		0
#define VISIBLE_SIZE		fix_make(0,0x0a00)
#define VISIBLE_SPEED	0
#define VISIBLE_RANGE	fix_make(100,0)	
#else
#define VISIBLE_MASS		0
#define VISIBLE_SIZE		fix_make(0,0x1000)
#define VISIBLE_SPEED	fix_make(10,0)
#define VISIBLE_RANGE	fix_make(20,0)	
#endif

