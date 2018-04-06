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
 * $Source: n:/project/cit/src/inc/RCS/amaploop.h $
 * $Revision: 1.1 $
 * $Author: dc $
 * $Date: 1994/04/06 07:27:10 $
 */

#define AMAP_FULLEXPOSE (LL_CHG_BASE<<1)
#define AMAP_MAP_EV     (LL_CHG_BASE<<2)
#define AMAP_BUTTON_EV  (LL_CHG_BASE<<3)
#define AMAP_MESSAGE_EV (LL_CHG_BASE<<4)

void automap_loop(void);
void fsmap_startup(void);
void fsmap_free(void);

