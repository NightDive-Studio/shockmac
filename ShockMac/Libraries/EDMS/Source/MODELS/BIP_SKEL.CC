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
//	Takes data from the Biped models and fills out a pretransformed skeleton, which
//	requires only camera rotation (NOT LOCAL) and translation before rendering...
//	=============================================================================



#include <iostream.h>

#ifdef EDMS_SHIPPABLE
//#include <mout.h>
#endif

#include "EDMS_int.h"
#include "EDMS_vt.h"



//	State...
//	========
extern Q	S[MAX_OBJ][7][4],					//State info...
		I[MAX_OBJ][DOF_MAX];					//Private info...
extern int	no_no_not_me[MAX_OBJ];

//	Here is where the skeleton lives for the biped...
//	=================================================
extern Q	*utility_pointer[MAX_OBJ];


//	Here are some static globals for speed...
//	=========================================
static Q	sin_xi_alpha = 0,
		sin_xi_beta = 0,
		cos_xi_alpha = 0,
		cos_xi_beta = 0,
		vert_x = 0,
		vert_y = 0,
		vert_z = 0,
		sin_right = 0,
		cos_right = 0,
		sin_left = 0,
		cos_left = 0,
		elbow_speed = 0,
		et_y = 0,
		et_z = 0;


//	Here are the variables for the transformations...
//	=================================================
static Q 	cos_alpha, 
		cos_beta,
		cos_gamma, 
		sin_alpha, 
		sin_beta,
		sin_gamma; 




//	Now just do the work...
//	=======================
void make_biped_skeleton( int object ) {


//	This globalizes on the state rather that argument vector...
//	-----------------------------------------------------------
void	globalize_S( Q &X, Q &Y, Q &Z );



//	Get the Euler rotations...
//	--------------------------
	sincos( -S[object][3][0], &sin_alpha, &cos_alpha );
	sincos( -S[object][4][0], &sin_beta, &cos_beta );
	sincos( -S[object][5][0], &sin_gamma, &cos_gamma );



//	Here we can save some time...
//	-----------------------------
Q	xi_alpha = utility_pointer[object][27],
	xi_beta  = utility_pointer[object][28];


//	Are you who you say you are?
//	----------------------------
	if ( I[object][30] != BIPED ) return;

//	Got anything new for me?
//	------------------------	
//	if ( no_no_not_me[object] == 0 ) return;


//	Toes...
//	-------
	globalize_S(				//Right then left...
		     utility_pointer[object][0],
		     utility_pointer[object][1],
		     utility_pointer[object][2] );

	globalize_S(
		     utility_pointer[object][3],
		     utility_pointer[object][4],
		     utility_pointer[object][5] );

//	Heels...
//	--------
	globalize_S(
		     utility_pointer[object][6],
		     utility_pointer[object][7],
		     utility_pointer[object][8] );

	globalize_S(
		     utility_pointer[object][9],
		     utility_pointer[object][10],
		     utility_pointer[object][11] );

//	Knees...
//	--------
	globalize_S(
		     utility_pointer[object][12],
		     utility_pointer[object][13],
		     utility_pointer[object][14] );

	globalize_S(
		     utility_pointer[object][15],
		     utility_pointer[object][16],
		     utility_pointer[object][17] );


//	Pelvic joints...
//	----------------
	utility_pointer[object][18] = I[object][23];	//Right then left...
	utility_pointer[object][21] =-I[object][23];
	utility_pointer[object][19] = 
	utility_pointer[object][20] = 
	utility_pointer[object][22] = 
	utility_pointer[object][23] = 0;

	globalize_S(
		     utility_pointer[object][18],
		     utility_pointer[object][19],
		     utility_pointer[object][20] );

	globalize_S(
		     utility_pointer[object][21],
		     utility_pointer[object][22],
		     utility_pointer[object][23] );



//	Center o' mass...
//	-----------------
	utility_pointer[object][24] = 
	utility_pointer[object][25] = 
	utility_pointer[object][26] = 0;



//	Neck...
//	-------
	sincos( -xi_alpha, &sin_xi_alpha, &cos_xi_alpha );	
	sincos( -xi_beta,  &sin_xi_beta,  &cos_xi_beta  );	

	utility_pointer[object][27] = I[object][26]*sin_xi_alpha;
	utility_pointer[object][28] = I[object][26]*sin_xi_beta;
	utility_pointer[object][29] = I[object][26]*cos_xi_alpha*cos_xi_beta;

	globalize_S(
		     utility_pointer[object][27],
		     utility_pointer[object][28],
		     utility_pointer[object][29] );


//	Shoulders...
//	------------
	vert_x = .75*I[object][26]*sin_xi_alpha;
	vert_y = .75*I[object][26]*sin_xi_beta;
	vert_z = .75*I[object][26]*cos_xi_alpha*cos_xi_beta;

	utility_pointer[object][30] = vert_x + I[object][23];
	utility_pointer[object][33] = vert_x - I[object][23];

	utility_pointer[object][31] =
	utility_pointer[object][34] = vert_y;

	utility_pointer[object][32] =
	utility_pointer[object][35] = vert_z;

	globalize_S(
		     utility_pointer[object][30],
		     utility_pointer[object][31],
		     utility_pointer[object][32] );

	globalize_S(
		     utility_pointer[object][33],
		     utility_pointer[object][34],
		     utility_pointer[object][35] );


//	Elbows...
//	---------

	elbow_speed = utility_pointer[object][42];

	sincos( .2*elbow_speed*utility_pointer[object][40] - 2*S[object][4][0] - .5, &sin_left, &cos_left );
//	sin_left = -utility_pointer[object][36];		//Data passed...
//	cos_left = utility_pointer[object][37];

	sincos( .1*elbow_speed*utility_pointer[object][37] - 2*S[object][4][0] - .5, &sin_right, &cos_right );
//	sin_right = -utility_pointer[object][39];
//	cos_right = utility_pointer[object][40];

	utility_pointer[object][36] = vert_x + I[object][23];
	utility_pointer[object][39] = vert_x - I[object][23];

	et_y = .8*I[object][23];
	et_z = .4*I[object][26];


	utility_pointer[object][37] = vert_y + R1( sin_right, cos_right, et_y, et_z );
	utility_pointer[object][40] = vert_y + R1( sin_left, cos_left, et_y, et_z );

	utility_pointer[object][38] = vert_z - R2( sin_right, cos_right, et_y, et_z );
	utility_pointer[object][41] = vert_z - R2( sin_left, cos_left, et_y, et_z );


	globalize_S(
		     utility_pointer[object][36],
		     utility_pointer[object][37],
		     utility_pointer[object][38] );

	globalize_S(
		     utility_pointer[object][39],
		     utility_pointer[object][40],
		     utility_pointer[object][41] );


//	Wrists...
//	---------
	utility_pointer[object][42] = vert_x + I[object][23];
	utility_pointer[object][45] = vert_x - I[object][23];

	et_z = 2*I[object][23];
	et_y = .6*I[object][26];

	utility_pointer[object][43] = vert_y + R1( sin_right, cos_right, et_y, et_z );
	utility_pointer[object][46] = vert_y + R1( sin_left, cos_left, et_y, et_z );

	utility_pointer[object][44] = vert_z - R2( sin_right, cos_right, et_y, et_z );
	utility_pointer[object][47] = vert_z - R2( sin_left, cos_left, et_y, et_z );

	globalize_S(
		     utility_pointer[object][42],
		     utility_pointer[object][43],
		     utility_pointer[object][44] );

	globalize_S(
		     utility_pointer[object][45],
		     utility_pointer[object][46],
		     utility_pointer[object][47] );


//	Wow.
//	----


}







//	All hail the transformer...
//	===========================
void globalize_S( Q &X, Q &Y, Q &Z ) {

Q	x = X,
	y = Y,
	z = Z;


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

