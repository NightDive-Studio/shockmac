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
#include "obj3d.h"

#define RES_MODEL_BASE     2300
#define OBJ3D_BASE         RES_object3d_0

#define get_model_data(model_num) RefLock(MKREF(RES_MODEL_BASE + (model_num),0))
#define release_model_data(model_num) RefUnlock(MKREF(RES_MODEL_BASE + (model_num),0))
#define model_valid(model_num)  ResInUse(RES_MODEL_BASE + (model_num))
