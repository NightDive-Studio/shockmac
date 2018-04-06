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
//	Here is an include file for EDMS internal functions.  It contains all that
//	Stuff that a growing model might need.  Also, it's a good idea, dammit!
//	=======================================================================




//	A girl's gotta have some standards.
//	===================================
#define EDMS_DIV_ZERO_TOLERANCE .0005



//	State and args...
//	=================
extern EDMS_Argblock_Pointer	A;
extern Q								S[MAX_OBJ][7][4],
										I[MAX_OBJ][DOF_MAX];


//	Functions...
//	============
extern void	( *idof_functions[MAX_OBJ] )( int ),
				( *equation_of_motion[MAX_OBJ][7] )( int );



//	Callbacks...
//	------------
extern void	( *EDMS_object_collision )( physics_handle caller, physics_handle victim, int badness, long DATA1, long DATA2, fix location[3] ),
				( *EDMS_wall_contact )( physics_handle caller );


//	Collision systems...
//	--------------------
extern int	are_you_there( int );			//May not be needed by most models,
//extern int	check_for_hit( int );			//due to use of Intrsect.cc



// Sleepy Snoozy...
// ----------------
extern int  no_no_not_me[MAX_OBJ];
