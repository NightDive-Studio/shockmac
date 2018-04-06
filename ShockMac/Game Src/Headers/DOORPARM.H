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
// at which frame and above the door is considered "open" to physics....
#define DOOR_OPEN_FRAME    3

#define DOOR_CLOSED(id) (objs[(id)].info.current_frame < DOOR_OPEN_FRAME)
#define DOOR_REALLY_CLOSED(id) (objs[(id)].info.current_frame==0)

#define NEVER_AUTOCLOSE_COOKIE ((ubyte)-1)


extern bool check_object_dist(ObjID obj1, ObjID obj2, fix crit);
extern bool door_locked(ObjID);
extern bool door_moving(ObjID,bool);
