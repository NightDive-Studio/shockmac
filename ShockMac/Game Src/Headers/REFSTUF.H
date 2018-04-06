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
#include "objects.h"

#define CitrefsClearDealt()       do {LG_memset (refdealt, 0, NUM_REF_OBJECTS/8);}           while (0)
#define CitrefSetDealt(orefid)    do {refdealt[(orefid)>>3] |= 1 << ((orefid)&0x7);}      while (0)
#define CitrefClearDealt(orefid)  do {refdealt[(orefid)>>3] &= ~(1 << ((orefid)&0x7));}   while (0)
#define CitrefCheckDealt(orefid)  (refdealt[(orefid)>>3] & (1 << ((orefid)&0x7)))

#define CitrefsClearHomeSq()      do {LG_memset (homesquare, 0, NUM_REF_OBJECTS/8);}         while (0)
#define CitrefSetHomeSq(orefid)   do {homesquare[(orefid)>>3] |= 1 << ((orefid)&0x7);}    while (0)
#define CitrefClearHomeSq(orefid) do {homesquare[(orefid)>>3] &= ~(1 << ((orefid)&0x7));} while (0)
#define CitrefCheckHomeSq(orefid) (homesquare[(orefid)>>3] & (1 << ((orefid) & 0x7)))

#ifdef __OBJSIM_SRC
uchar homesquare[NUM_REF_OBJECTS/8];
uchar refdealt[NUM_REF_OBJECTS/8];
#else
extern uchar homesquare[NUM_REF_OBJECTS/8];
extern uchar refdealt[NUM_REF_OBJECTS/8];
#endif
