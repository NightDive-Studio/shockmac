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

// Macro to determine whether or not a given object is ice'd.  Assumes that 
// A. We are in cyberspace and
// B. That there are no otherwise animating objects in cyberspace (well, besides things of CLASS_ANIMATING)
// If either of these assumptions change, this will probably have to change as well.

#define obj_ICE_ICE_BABY(cobj)    ((cobj)->info.current_frame && ((cobj)->obclass != CLASS_ANIMATING) && ((cobj)->obclass != CLASS_CRITTER))
#define obj_DEICE(cobj)           do {(cobj)->info.current_frame = 0;} while (0)
#define obj_ICE_AGIT(cobj)        ((cobj)->info.make_info)
#define obj_SET_ICE_AGIT(cobj,nv) ((cobj)->info.make_info=nv)
#define obj_ICE_LEVEL(cobj)       ((cobj)->info.inst_flags>>6)
// note level is overall level, color in 3d
// agit is how annoyed it is
// hp is strength/size, moded by level and agit, i guess

#define ICE_ICE_BABY(id)          obj_ICE_ICE_BABY(&objs[(id)])
#define DEICE(id)                 obj_DEICE(&objs[(id)]) 
#define ICE_AGIT(id)              obj_ICE_AGIT(&objs[(id)])
#define SET_ICE_AGIT(id,newval)   obj_SET_ICE_AGIT(&objs[(id)],newval)
#define ICE_LEVEL(id)             obj_ICE_LEVEL(&objs[(id)]) 

#define MAX_AGIT  255
