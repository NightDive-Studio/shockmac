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
//	Sets up everything needed to manufacture a biped with initial state vector
//	init_state[][] and EDMS motion parameters params[] into soliton. Returns the 
//	object number, or else a negative error code (see Soliton.CPP for error handling and codes).
//	============================================================================================


//	Jon Blackley, July 25, 1993
//	===========================


#include <iostream.h>

#ifdef EDMS_SHIPPABLE
//#include <mout.h>
#endif

#include "EDMS_int.h"


//	Prototypes
//	==========
void		globalize( int model_number, Q &X, Q &Y, Q &Z );
extern void	biped_idof( int object );

//	State...
//	========
extern EDMS_Argblock_Pointer	A;
extern Q	S[MAX_OBJ][7][4],
		I[MAX_OBJ][DOF_MAX];
extern int	no_no_not_me[MAX_OBJ];

//	For rendering...
//	================


//	Functions...
//	============
extern void	( *idof_functions[MAX_OBJ] )( int ),
		( *equation_of_motion[MAX_OBJ][7] )( int ),
		null_function( int dummy );

extern Q	*utility_pointer[MAX_OBJ];




//	Here goes...
//	============
int make_biped( Q init_state[6][3], Q params[10], Q skeleton_pointer[] ) {



//	We also have to set up the histories...
//	=======================================
extern Q	save_r_w_r[MAX_OBJ][3],
		save_r_w_l[MAX_OBJ][3],
		save_chi[MAX_OBJ];


//	Counters...
//	===========
extern int	not_yet1[MAX_OBJ],
		not_yet2[MAX_OBJ],
		not_yet3[MAX_OBJ];


		
		
//	Have some variables...
//	======================
int	object_number = -1,						//Three guesses...
	error_code = -1;						//Guilty until...
Q	temp_r[3] = {0,0,0},
	temp_l[3] = {0,0,0};

//	We need ignorable coordinates...
//	================================
extern void null_function( int );


//	First find out which object we're going to be...
//	================================================
	while ( S[++object_number][0][0] > END );


//	Is it an allowed object number?  Are we full?  Why are we here?  Is there a God?
//	================================================================================
	if ( object_number < MAX_OBJ ) {


//		Now we can create the biped:  first dump the initial state vector...
//		=====================================================================
		for ( int coord = 0; coord < 6; coord++ ) {
		for ( int deriv = 0; deriv < 3;	deriv++ ) {
		S[object_number][coord][deriv] = 
		A[object_number][coord][deriv] = init_state[coord][deriv];	//For collisions...
		}}

//		Put in the appropriate bipedal parameters...
//		===========================================
		for ( int copy = 0; copy < DOF_MAX; copy++ ) I[object_number][copy] = 0;
		for ( int copy = 0; copy < 10; copy++ ) {
		I[object_number][copy + 20] = params[copy];
		}
		I[object_number][30] = BIPED;		    			//Hey, you are what you eat.

//		EDMS needs some precomputed stuff for this model...
//		===================================================
		I[object_number][0]  = I[object_number][24]			//Rho_extend...
				     + I[object_number][25];
		I[object_number][1]  = I[object_number][2] = 0;			//Radii..
		I[object_number][9]  = 0;					//Xi...
		I[object_number][10] = .3;					//Step_height...
		I[object_number][11] = .4*I[object_number][0];			//Raduis constant...

		I[object_number][12] = 1. / I[object_number][20];
		I[object_number][13] =
		I[object_number][14] = I[object_number][20]*(
				       I[object_number][23]*I[object_number][23]/4.
				     + (I[object_number][0] + .5*I[object_number][26])
				      *(I[object_number][0] + .5*I[object_number][26])/12. );
		I[object_number][15] = .5*I[object_number][20]*I[object_number][23]*I[object_number][23];

		I[object_number][13] = 1. / I[object_number][13];
		I[object_number][14] = 1. / I[object_number][14];
		I[object_number][15] = 1. / I[object_number][15];


//		Put in the collision information...
//		===================================
		I[object_number][31] = 1.25*I[object_number][23];		//Just the hips...
		I[object_number][32] =
		I[object_number][33] =
		I[object_number][34] =
		I[object_number][35] = 0;
		I[object_number][36] = I[object_number][20];			//Shrugoff "mass"...
		I[object_number][37] = -1;


//		mout << "I's: " << I[object_number][13] << " : "
//				<< I[object_number][14] << " : "
//				<< I[object_number][15] << "\n";


//		Get the initial displacement vector...
//		--------------------------------------
		temp_r[0] = I[object_number][24];
		temp_l[0] =-I[object_number][24];
		temp_r[1] = temp_l[1] = 0;
		temp_r[2] = temp_l[2] = -I[object_number][0];
 
		globalize( object_number, temp_r[0], temp_r[1], temp_r[2] );
		globalize( object_number, temp_l[0], temp_r[1], temp_r[2] );

		I[object_number][3] = temp_r[0] + S[object_number][0][0];
		I[object_number][4] = temp_r[1] + S[object_number][1][0];
		I[object_number][5] = temp_r[2] + S[object_number][2][0];

		I[object_number][6] = temp_l[0] + S[object_number][0][0];
		I[object_number][7] = temp_l[1] + S[object_number][1][0];
		I[object_number][8] = temp_l[2] + S[object_number][2][0];


//		Put reasonable initial values into the skeletal array...
//		--------------------------------------------------------
//		for ( coord = 0; coord < MAX_OBJ; coord++ ) skeleton_pointer[coord] = 0;

		skeleton_pointer[0] = skeleton_pointer[6] = I[object_number][3];
		skeleton_pointer[1] = skeleton_pointer[7] = I[object_number][4];
		skeleton_pointer[2] = skeleton_pointer[8] = I[object_number][5];

		skeleton_pointer[3] = skeleton_pointer[9]  = I[object_number][6];
		skeleton_pointer[4] = skeleton_pointer[10] = I[object_number][7];
		skeleton_pointer[5] = skeleton_pointer[11] = I[object_number][8];


//		Put reasonable values into the histories...
//		-------------------------------------------
		save_r_w_r[object_number][0] = I[object_number][3];
		save_r_w_r[object_number][1] = I[object_number][4];
		save_r_w_r[object_number][2] = I[object_number][5];
		
		save_r_w_l[object_number][0] = I[object_number][6];
		save_r_w_l[object_number][1] = I[object_number][7];
		save_r_w_l[object_number][2] = I[object_number][8];

		save_chi[object_number] = 0;


//		Set up the hacked updates...
//		============================
		for ( int coord = 0; coord < MAX_OBJ; coord++ ) {
		not_yet1[coord] = 0;
		not_yet2[coord] = 0;
		not_yet3[coord] = 1;
		}


//		Now tell Soliton where to look for the equations of motion...
//		=============================================================
		idof_functions[object_number] = biped_idof;

		equation_of_motion[object_number][0] = 
		equation_of_motion[object_number][1] = 
		equation_of_motion[object_number][2] = 
		equation_of_motion[object_number][3] = 
		equation_of_motion[object_number][4] = 
		equation_of_motion[object_number][5] = null_function;


//		Awake...
//		========
		no_no_not_me[object_number] = 1;


//		Things seem okay...
//		===================
		error_code = object_number;
	
      // Hey, let's make it exist.
      write_object (object_number);
	}

//	Inform the caller...
//	====================
	return error_code;
}





//	Rotation stuff for initialization...
//	====================================
void globalize( int object, Q &X, Q &Y, Q &Z ) {

Q	x = X,
	y = Y,
	z = Z;

static Q 	cos_alpha, 
		cos_beta,
		cos_gamma, 
		sin_alpha, 
		sin_beta,
		sin_gamma; 

	sincos( -A[object][3][0], &sin_alpha, &cos_alpha );
	sincos( -A[object][4][0], &sin_beta, &cos_beta );
	sincos( -A[object][5][0], &sin_gamma, &cos_gamma );



//	Rotate on Euler angles...
//	=========================
		X    = 	x*( cos_alpha*cos_gamma )
		     +  y*( cos_gamma*sin_alpha*sin_beta - cos_beta*sin_gamma )
		     +  z*( cos_beta*cos_gamma*sin_alpha + sin_beta*sin_gamma );

		Y    =     x*( cos_alpha*sin_gamma )
		     +  y*( cos_beta*cos_gamma + sin_alpha*sin_beta*sin_gamma )
		     +  z*( cos_beta*sin_alpha*sin_gamma - cos_gamma*sin_beta );		

		Z    = 	x*( -sin_alpha )
		     +  y*( cos_alpha*sin_beta )
		     +  z*( cos_alpha*cos_beta );


}























//	Attenzione! Biped parameters:
//	=============================
//
//
//	parameter	|	Description
//	----------------+------------------
//			|
//	20		|	Mass
//	21		|	Kappa_leg
//	22		|	Delta_leg
//	23		|	l_hip
//	24		|	l_thigh
//	25		|	l_shin
//	26		|	l_torso
//	27		|	m_bal
//	28		|	balance skill
//	29		|	gravity
//
//	Und das ist Alles...
//	====================
