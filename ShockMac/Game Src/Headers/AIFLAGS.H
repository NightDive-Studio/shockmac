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
#define AI_FLAG_CHASING    0x0001
#define AI_FLAG_NOLOOT     0x0002
#define AI_FLAG_CONFUSED   0x0004
#define AI_FLAG_TRANQ      0x0010

#define ai_critter_sleeping(osid) ((objCritters[(osid)].orders == AI_ORDERS_SLEEP) ||  (objCritters[(osid)].flags & AI_FLAG_TRANQ))
