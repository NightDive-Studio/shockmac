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
//	Many games require objects which travel faster than the renderer can possibly draw.  The
//	stuff in this file handles these things in various ways.  For instance, a laser weapon
//	can be raycast instantaneously, dependant only upon the terrain and object models.
//	==================================================================================


#include <EDMS_int.h>
#include <mout.h>
#include <EDMS_vt.h>
#include <physhand.h>


//	Here is the collision information...
//	====================================
extern unsigned	int	data[100][128];
extern Q		I[MAX_OBJ][DOF_MAX],
			S[MAX_OBJ][6][4];

extern int		no_no_not_me[MAX_OBJ];

extern Q		hash_scale;


//	Here is some stuff that the line finder needs that is stupid to pass around...
//	==============================================================================
static Q		initial_X[3] = {0,0,0},
			final_X[3]   = {0,0,0};


//	Here is the high velocity weapon primitive...
//	=============================================
physics_handle EDMS_cast_projectile( Q *X, Q D[3], Q speed, Q mass, Q size, Q range, int exclude ) {

physics_handle	object_check( unsigned int data_word, Q size, int exclude );	//Checks for hits...

int		stepper = 0,
		object_pointer = 1,			//For object checks...
		max_step = 30*( range.to_int() ),	//30 samples per meter...
		victim_on = 0;
unsigned int	must_check_objects[MAX_OBJ];
unsigned int	test_data;

physics_handle	victim = -1,				//It is what is says it is...
		return_victim = -1;			//The number actually returned...

Q		iota_c = 0;				//Some variable or other...


//	Reset the object collisions...
//	==============================
	must_check_objects[0] = 0;

//	Looks at terrain...
//	===================
fix	checker = 0;


//	Save the initial vectors for the line finder...
//	===============================================
	initial_X[0] = X[0];
	initial_X[1] = X[1];
	initial_X[2] = X[2];

//	Rescale direction to 1/3 decimeters...
//	======================================
	D[0] *= .0333;
	D[1] *= .0333;
	D[2] *= .0333;

//	PRINT3D( X );

//	Find impact point...
//	====================
	for ( stepper = 0; stepper < max_step && fix_abs( checker ) < fix_make(0,0x1000); stepper++ ) {
	
	checker = 0;

	indoor_terrain( X[0],						//Get the info...
			X[1],
			X[2],
			size,
			-1 );

//		Chexk the terrain...
//		====================
		checker = terrain_info.cx				//Let's see...
			+ terrain_info.cy
			+ terrain_info.cz

			+ terrain_info.fx
			+ terrain_info.fy
			+ terrain_info.fz

			+ terrain_info.wx
			+ terrain_info.wy
			+ terrain_info.wz;

//		Check for object collisions...
//		==============================
		test_data = data[ floor( hash_scale*X[0] ) ][ floor( hash_scale*X[1] ) ];

//		Is there someone NEW there?
//		===========================
		if ( ( test_data != 0 ) && ( test_data != must_check_objects[object_pointer-1] ) ) {

			must_check_objects[object_pointer] = test_data;
			object_pointer += 1;

		}


//	Move the check point...
//	=======================
	X[0] += D[0];
	X[1] += D[1];
	X[2] += D[2];

	}


//	Save the final point of the line segment...
//	===========================================
	final_X[0] = X[0];
	final_X[1] = X[1];
	final_X[2] = X[2];


//	Zero the last object check point...
//	===================================
	must_check_objects[object_pointer] = 0;


//	Now throw the results out to the masses, first checking for object hits, if need be...
//	======================================================================================
	if ( must_check_objects[1] != 0 ) {

		object_pointer = 1;

		while (must_check_objects[object_pointer++] != 0 ) {

                victim = object_check( must_check_objects[object_pointer], size, exclude ); 


//		Hurt me, oh, oh, baby...
//		------------------------		
		if ( victim > -1 ) {

			return_victim = victim;		//return the right guy!
			
			victim_on = ph2on[victim];
			iota_c = .5*I[victim_on][36]*mass*speed;

			I[victim_on][32] = D[0]*iota_c;	//Absolute blows off walls, remember explosions too...
			I[victim_on][33] = D[1]*iota_c;
			I[victim_on][34] = D[2]*iota_c;

			I[victim_on][35] = 1;		//Deweet!

			no_no_not_me[victim_on] = 1;	//Make sure we're up...

			}

	}}


//	If we did, in face, hit a wall, the 3D system precision may be insufficient to sort the hit
//	art in front of the wall.  Therefore...
//	=======================================
	X[0] -= .5*D[0];
	X[1] -= .5*D[1];
	X[2] -= .5*D[2];


//	Did we hit a wall, or did we hit range out?
//	===========================================
	if ( stepper == max_step ) {

		X[0] = 
		X[1] = 
		X[2] = END;

		}


//	Hit nothing for now...
//	======================
	return return_victim;

}





//	Here, since we know the line segment we're interested in, we check to make sure that we
//	didn't hit any objects, and return the one we did...
//	====================================================
physics_handle	object_check( unsigned int data_word, Q size, int exclude ) {


//		General purpose...
//		==================
int		object;
physics_handle	victim = -1;


//	For the lines...
//	================
Q	a = initial_X[0] - final_X[0],
	b = initial_X[1] - final_X[1],
	c = initial_X[2] - final_X[2],
	top_1 = 0,
	top_2 = 0,
	top_3 = 0,
	bottom = 0,
	kill_zone = 0,
        kzdist = 0,
        kzdisto = 10000;


//	Go through the word looking for who it was...
//	=============================================
	for ( object = 0; object < MAX_OBJ && S[object][0][0] > END; object++ ) {
		if ( (object_bit( object ) & data_word) && ( object != exclude ) ) {

		top_1 = c*(S[object][1][0] - initial_X[1]) - b*(S[object][2][0] - initial_X[2]);
		top_1 *= top_1;

		top_2 = a*(S[object][2][0] - initial_X[2]) - c*(S[object][0][0] - initial_X[0]);
		top_2 *= top_2;

		top_3 = b*(S[object][0][0] - initial_X[0]) - a*(S[object][1][0] - initial_X[1]);
		top_3 *= top_3;

		bottom = a*a + b*b + c*c;

		kill_zone = sqrt( (top_1 + top_2 + top_3) / bottom );		

		if ( kill_zone < (I[object][31] + size) ) {

                        kzdist = sqrt( (initial_X[0] - S[object][0][0])*(initial_X[0] - S[object][0][0])
                                     + (initial_X[1] - S[object][1][0])*(initial_X[1] - S[object][1][0])
                                     + (initial_X[2] - S[object][2][0])*(initial_X[2] - S[object][2][0]) );

			if (kzdist < kzdisto ) victim = on2ph[object];

                        }


		}	//End of object test...
	}


	return victim;

}
