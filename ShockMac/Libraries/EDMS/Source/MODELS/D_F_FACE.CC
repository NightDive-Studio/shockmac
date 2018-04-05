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
//	Here is the bridge routine for maintenance and upkeep of the dirac frame models...
//	==================================================================================


#include "fixpp.h"
#include "EDMS_int.h"



//      This matrix is for Douggie and his magic circus...
//      ==================================================
fix     EDMS_Dirac_basis[9];

//	Here we need include files for each and every model that we'll be using...
//	==========================================================================
#include "d_frame.h"

//	The physics handles definitions...
//	==================================
#include "physhand.h"

//	Data...
//	=======
#include "EDMS_mod.h"


//	Structs...
//	==========


//	Dirac Frame...
//	--------------
typedef struct {

fix	mass,
	hardness,
	roughness,
	gravity;
	
fix	corners[10][4];

} Dirac_frame;


//	Here we go...
//	=============
#define DIRAC_HARD_FAC 10

//      Hack hack hack...
//      =================
static Q        size = .3;


//      Thank God we have only 16 bits of fraction!
//      ===========================================
static Q       old_state[7],
               new_state[7];


//	We need to link to c...
//	=======================
//extern "C" {
// KLC no we don't




//	Here are the bridge routines to the models...
//	=============================================






//      Dirac Frame routines...
//	=======================
physics_handle EDMS_make_Dirac_frame( Dirac_frame *d, State *s ) {



//	Variables for tha actual conversion...
//	--------------------------------------
Q		params[10],
		init_state[6][3];

Q		mass,
		hardness,
		gravity,
		roughness;

int		on = 0;

physics_handle	ph = 0;



	init_state[0][0].fix_to( s->X );		init_state[0][1].fix_to( s->X_dot );
	init_state[1][0].fix_to( s->Y );		init_state[1][1].fix_to( s->Y_dot );    
	init_state[2][0].fix_to( s->Z );		init_state[2][1].fix_to( s->Z_dot );    
	init_state[3][0].fix_to( s-> alpha );		init_state[3][1].fix_to( s->alpha_dot );
	init_state[4][0].fix_to( s-> beta );		init_state[4][1].fix_to( s->beta_dot );
	init_state[5][0].fix_to( s-> gamma );		init_state[5][1].fix_to( s->gamma_dot );



        mass.fix_to( d -> mass );
	roughness.fix_to( d -> roughness );
	hardness.fix_to( d -> hardness );
	gravity.fix_to( d -> gravity );


//	hardness = hardness*(mass*4/size);
	hardness = hardness*(mass*DIRAC_HARD_FAC/size);         //Node Size goes here!!!

	params[0] = mass;
	params[1] = 1 / mass;
	params[2] = 1 / ( .4*mass*size*size );
	params[3] = hardness;
	params[4] = sqrt( params[3] ) * sqrt( mass );
//	params[4] = 20*sqrt( params[0] * mass );
	params[5] = roughness;
	params[6] = .2;
	params[7] = 0;//gravity;
	params[8] = 0;
	params[9] = 0;


//		Now actulally DO the dirty work...
//		----------------------------------
                on = make_Dirac_frame( init_state, params );
		ph = EDMS_bind_object_number( on );


		return ph;

}







//      At some point we need the viewpoint offered by the neck...
//      ==========================================================
void EDMS_get_Dirac_frame_viewpoint( physics_handle ph, State *s )
{


//      For getting the new basis...
//      ----------------------------
//void render_globalize( Q &X, Q &Y, Q &Z, int );
void render_localize( Q &X, Q &Y, Q &Z, int );



//	For Euler angle conversion...
//	-----------------------------
Q       dirac_temp[9];
Q	alpha,
	beta,
	gamma;


int     on = ph2on[ph];

Q       delta = 0;


	if ( I[on][30] == D_FRAME ) {


        new_state[0] = ( S[on][0][0] );
        new_state[1] = ( S[on][1][0] );
        new_state[2] = ( S[on][2][0] );

        new_state[3] = S[on][3][0];
        new_state[4] = S[on][4][0];
        new_state[5] = S[on][5][0];
        new_state[6] = S[on][6][0];


        delta =         (new_state[0] - old_state[0])*(new_state[0] - old_state[0])
                      + (new_state[1] - old_state[1])*(new_state[1] - old_state[1])
                      + (new_state[2] - old_state[2])*(new_state[2] - old_state[2])
                      + (new_state[3] - old_state[3])*(new_state[3] - old_state[3])
                      + (new_state[4] - old_state[4])*(new_state[4] - old_state[4])
                      + (new_state[5] - old_state[5])*(new_state[5] - old_state[5])
                      + (new_state[6] - old_state[6])*(new_state[6] - old_state[6]);


        if ( delta > .00003 ) {

                old_state[0] = new_state[0];
                old_state[1] = new_state[1];
                old_state[2] = new_state[2];

                old_state[3] = new_state[3];
                old_state[4] = new_state[4];
                old_state[5] = new_state[5];
                old_state[6] = new_state[6];

        }

        s->X = old_state[0].to_fix();    
        s->Y = old_state[1].to_fix();    
        s->Z = old_state[2].to_fix();    

	EDMS_get_Euler_angles( alpha, beta, gamma, on );

        s->alpha = -gamma.to_fix();
        s->beta  = -alpha.to_fix();
        s->gamma = -beta.to_fix();


//      Set up global vectors...
//      ------------------------
        dirac_temp[0] = dirac_temp[1] = dirac_temp[2] =
        dirac_temp[3] = dirac_temp[4] = dirac_temp[5] =
        dirac_temp[6] = dirac_temp[7] = dirac_temp[8] = 0;

        dirac_temp[0] = 1;
        dirac_temp[4] = 1;
        dirac_temp[8] = 1;

//      Transform to the new basis...
//      -----------------------------
        render_localize( dirac_temp[0], dirac_temp[1], dirac_temp[2], on );
        render_localize( dirac_temp[3], dirac_temp[4], dirac_temp[5], on );
        render_localize( dirac_temp[6], dirac_temp[7], dirac_temp[8], on );

//      Stuff into Matt's order...
//      --------------------------
/*
        EDMS_Dirac_basis[0] = dirac_temp[0].to_fix();
	EDMS_Dirac_basis[1] =-dirac_temp[6].to_fix();
	EDMS_Dirac_basis[2] = dirac_temp[3].to_fix();
	EDMS_Dirac_basis[3] =-dirac_temp[2].to_fix();
	EDMS_Dirac_basis[4] = dirac_temp[8].to_fix();
	EDMS_Dirac_basis[5] =-dirac_temp[5].to_fix();
	EDMS_Dirac_basis[6] = dirac_temp[1].to_fix();
	EDMS_Dirac_basis[7] =-dirac_temp[7].to_fix();
	EDMS_Dirac_basis[8] = dirac_temp[4].to_fix();
*/

//      Almost...
/*
        EDMS_Dirac_basis[0] = dirac_temp[3].to_fix();
	EDMS_Dirac_basis[1] =-dirac_temp[6].to_fix();
	EDMS_Dirac_basis[2] =-dirac_temp[0].to_fix();
	EDMS_Dirac_basis[3] =-dirac_temp[5].to_fix();
	EDMS_Dirac_basis[4] = dirac_temp[8].to_fix();
	EDMS_Dirac_basis[5] = dirac_temp[2].to_fix();
	EDMS_Dirac_basis[6] = dirac_temp[4].to_fix();
	EDMS_Dirac_basis[7] =-dirac_temp[7].to_fix();
	EDMS_Dirac_basis[8] =-dirac_temp[1].to_fix();
*/

        EDMS_Dirac_basis[0] =-dirac_temp[3].to_fix();
	EDMS_Dirac_basis[1] =-dirac_temp[6].to_fix();
	EDMS_Dirac_basis[2] = dirac_temp[0].to_fix();
	EDMS_Dirac_basis[3] = dirac_temp[5].to_fix();
	EDMS_Dirac_basis[4] = dirac_temp[8].to_fix();
	EDMS_Dirac_basis[5] =-dirac_temp[2].to_fix();
	EDMS_Dirac_basis[6] =-dirac_temp[4].to_fix();
	EDMS_Dirac_basis[7] =-dirac_temp[7].to_fix();
	EDMS_Dirac_basis[8] = dirac_temp[1].to_fix();


        }       //End of check for Dirac_frame or not...
           


}




//	Utilities for the weak spirited...
//	==================================
void EDMS_set_Dirac_frame_parameters( physics_handle ph, Dirac_frame *d ) {

Q	mass,
	hardness,
	roughness,
	gravity;

	mass.fix_to( d -> mass );
	hardness.fix_to( d -> hardness );
	gravity.fix_to( d -> gravity );
	roughness.fix_to( d -> roughness );

	int on = physics_handle_to_object_number( ph );

	hardness = hardness*(mass*DIRAC_HARD_FAC/size);
	I[on][20] = mass;
	I[on][21] = 1 / mass;
	I[on][22] = 1 / ( .4*mass*size*size );
	I[on][23] = hardness;
	I[on][24] = sqrt( I[on][23] ) * sqrt( mass );
//	I[on][24] = 20*sqrt( I[on][20] * mass );
	I[on][25] = roughness;
	I[on][26] = .2;
	I[on][27] = 0;//gravity;
	I[on][28] = 0;
	I[on][29] = 0;

//	Done!
//	-----

}



//	And the weak minded...
//	======================
void EDMS_get_Dirac_frame_parameters( physics_handle ph, Dirac_frame *d )
{
	int on = physics_handle_to_object_number( ph );

	d -> roughness = ( I[on][23] / I[on][26] ).to_fix();
	d -> hardness = ( I[on][26] / I[on][20]*DIRAC_HARD_FAC ).to_fix();
	d -> mass = I[on][20].to_fix();
	d -> gravity = I[on][27].to_fix();

}



void EDMS_control_Dirac_frame( physics_handle ph, fix forward, fix pitch, fix yaw, fix roll ) {

int on = ph2on[ph];

Q       F,
        P,
	Y,
        R;

        F.fix_to(forward);

//      System shock angle order, definition...
//      =======================================        
        R.fix_to(roll);
        P.fix_to(yaw);
	Y.fix_to(pitch);
        Y =- Y;

        control_dirac_frame( on, F, P, Y, R );

}



void render_localize( Q &X, Q &Y, Q &Z, int object ) {


Q	e0, e1, e2, e3;


Q	x = X,
	y = Y,
	z = Z;

	e0 =  S[object][3][0];	e1 =  S[object][4][0];
	e2 =  S[object][5][0];	e3 =  S[object][6][0];


//	Go for it, sonny...
//	-------------------	
	X = x*( e0*e0 + e1*e1 - e2*e2 - e3*e3 )
	  + y*( 2*( e1*e2 - e0*e3 ) )
	  + z*( 2*( e1*e3 + e0*e2 ) );

	Y = x*( 2*( e1*e2 + e0*e3 ) )
	  + y*( e0*e0 - e1*e1 + e2*e2 - e3*e3 )
	  + z*( 2*( e2*e3 - e0*e1 ) );
	  
	Z = x*( 2*(-e0*e2 + e1*e3 ) )
	  + y*( 2*( e2*e3 + e0*e1 ) )
	  + z*( e0*e0 - e1*e1 - e2*e2 + e3*e3 );


}



 


//}       //End of Extern "C"...
