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
//	Here is the FF grenade/missile(maybe) model, DEATH.  What fun.  It is becoming
//	increasingly evident that the path to true glory is multiple parameter sets.
//	============================================================================

//	Jon Blackley, January 31, 1993
//	==============================


#include <iostream.h>
//#include <dos.h>
#include "EDMS_Int.h"					//This is the model type library. It is universal.
#include "idof.h"

//#ifdef EDMS_SHIPPABLE
//#include <mout.h>
//#endif

#include "physhand.h"

//	Functions...
//	------------
extern void	( *idof_functions[MAX_OBJ] )( int ),
		( *equation_of_motion[MAX_OBJ][7] )( int ),
		null_function( int dummy );

//	Much better...
//	--------------
Q	e0,  e1,  e2,  e3,				//For speed plus beauty!
 	ed0, ed1, ed2, ed3,
 	alpha_dot,
 	beta_dot,
 	gamma_dot;

Q	object12,
 	object13;

//		For collisions...
//		-----------------	
int     	C,
	        V,
        	other_object;
static fix	location[3];


//	For now...
//	----------
terrain_ff	TFF;							//3 guesses, first 2 don't count...


//	Utilitie...
//	-----------
Q		ground_round = 0,
		walls_of = 0,
		f_abs = 0;


#pragma require_prototypes off


//	Here are the internal degrees of freedom...
//	===========================================
void deadly_idof( int object )
{
   //	Now deal with the quaternion (2nd order) stuff...
   //	=================================================
   //	e0 =  A[object][3][0];	e1 =  A[object][4][0];	e2 =  A[object][5][0];	e3 =  A[object][6][0];
   //	ed0 = A[object][3][1];	ed1 = A[object][4][1];	ed2 = A[object][5][1];	ed3 = A[object][6][1];

   //	beta_dot  = 2*( e0*ed1 + e3*ed2 - e2*ed3 - e1*ed0 );
   //	alpha_dot = 2*(-e3*ed1 + e0*ed2 + e1*ed3 - e2*ed0 );
   //	gamma_dot = 2*( e2*ed1 - e1*ed2 + e0*ed3 - e3*ed0 );


	TFF.caller = on2ph[object];

	ff_terrain( A[object][0][0],
		    A[object][1][0],
		    A[object][2][0],
          FALSE,
		    &TFF );


	ground_round.fix_to( TFF.g_height );
	walls_of.fix_to( abs(TFF.w_x) + abs(TFF.w_y) );

	if (	( walls_of > 0 ) || ( ground_round > A[object][2][0] - I[object][27] ) )
   {
      // Autodestruct...
      //	---------------
		if ( I[object][38] > 0 ) I[object][38] = -1;

      //	Callback...
      //	-----------
	   location[0] = A[object][0][0].to_fix();
      location[1] = A[object][1][0].to_fix();
      location[2] = A[object][2][0].to_fix();
	   EDMS_object_collision( on2ph[object],
					       on2ph[object],
					       -9999,
					       ( TFF.DATA1 )*(ground_round > A[object][2][0] - I[object][27]),
					       ( TFF.DATA2 )*(walls_of > 0),
					       location );
		EDMS_autodestruct( on2ph[object] );
	}

   ulong mask = are_you_there (object);
   ulong bit = 0;

   while (mask != 0)
   {
      if (mask & 1)
      {
         for (other_object = bit; other_object < MAX_OBJ && S[other_object][0][0] > END; other_object += NUM_OBJECT_BITS)
         {
            if (other_object != object && I[object][IDOF_COLLIDE].to_int() != other_object
                                       && I[object][IDOF_MODEL] != DEATH)           // make deaths not hit each other!
            {
               Q Ao00 = A[other_object][0][0];
               Q Ao10 = A[other_object][1][0];
               Q Ao20 = A[other_object][2][0];
               Q A00 = A[object][0][0];
               Q A10 = A[object][1][0];
               Q A20 = A[object][2][0];

               Q dx = A00 - Ao00;
               Q dy = A10 - Ao10;
               Q dz = A20 - Ao20;

               if (abs(dx) < object13 && abs(dy) < object13 && abs(dz) < object13)
               {
            		object12 = sqrt (dx*dx + dy*dy + dz*dz); // little R

            		object13 = ( I[object][31] + I[other_object][31] );			//Big R...

            		if ( ( object12 < object13 )&&( object12 > .1 ) )
                  {
                     //	Autodestruct...
                     //	---------------
                     if ( I[object][38] > 0 ) I[object][38] = -1;


                     //	Callback...
                     //	-----------
		               C = on2ph[object];	V = on2ph[other_object];
		               location[0] = A[object][0][0].to_fix();
                     location[1] = A[object][1][0].to_fix();
                     location[2] = A[object][2][0].to_fix();
		               EDMS_object_collision( C, V, -9999, 0, 0, location );
		               EDMS_autodestruct( C );

                     // Wakeup...
                     // =========
//	   	            if (no_no_not_me[other_object] == 0 ) collision_wakeup( other_object );
                  }                
         		}
            }
   		}
		}

      mask >>= 1;
      bit++;
	}


//	Equations of motion...
//	======================

	S[object][0][2] = I[object][21]*( - I[object][25]*A[object][0][1] );
	S[object][1][2] = I[object][21]*( - I[object][25]*A[object][1][1] );
	S[object][2][2] = I[object][21]*( - I[object][25]*A[object][2][1] ) - I[object][26];



//	For the missile, we'll need some thrust and such...
//	--------------------------------------------------
//	S[object][3][2] = -.5*( e1*T_beta_temp + e2*T_alpha_temp + e3*T_gamma_temp + ed1*beta_dot + ed2*alpha_dot + ed3*gamma_dot );
//			+ lagrange_multiplier*( ed0*lagrange + e0*lagrange_dot );

//	S[object][4][2] =  .5*( e0*T_beta_temp + e2*T_gamma_temp - e3*T_alpha_temp + ed0*beta_dot + ed2*gamma_dot - ed3*alpha_dot );
//			+ lagrange_multiplier*( ed1*lagrange + e1*lagrange_dot );

//	S[object][5][2] =  .5*( e0*T_alpha_temp + e3*T_beta_temp - e1*T_gamma_temp + ed0*alpha_dot + ed3*beta_dot - ed1*gamma_dot );
//			+ lagrange_multiplier*( ed2*lagrange + e2*lagrange_dot );

//	S[object][6][2] =  .5*( e1*T_alpha_temp - e2*T_beta_temp + e0*T_gamma_temp + ed0*gamma_dot + ed1*alpha_dot - ed2*beta_dot );
//			+ lagrange_multiplier*( ed3*lagrange + e3*lagrange_dot );



//	That's all, folks. Give 'em some air.  Move along...
//	====================================================



}







//	Sets up everything needed to manufacture hot death with initial state vector
//	init_state[][] and EDMS motion parameters params[] into soliton. Returns the 
//	object number, or else a negative error code (see Soliton.CPP for error handling and codes).
//	============================================================================================
int make_death( Q init_state[6][3], Q params[10] ) {


//	Have some variables...
//	======================
int	object_number = -1,						//Three guesses...
	error_code = -1;						//Guilty until...

//	We need ignorable coordinates...
//	================================
extern void null_function( int );

int     coord = 0,
        deriv = 0;

Q       sin_alpha = 0,
        cos_alpha = 0,
        sin_beta  = 0,
        cos_beta  = 0,
        sin_gamma = 0,
        cos_gamma = 0;


//	First find out which object we're going to be...
//	================================================
	while ( S[++object_number][0][0] > END );			//Jon's first C trickie...


//	Is it an allowed object number?  Are we full?  Why are we here?  Is there a God?
//	================================================================================
	if ( object_number < MAX_OBJ ) {




//		Now we can create the airplane:  first dump the initial state vector...
//		=====================================================================
		for ( coord = 0; coord < 3; coord++ ) {
		for ( deriv = 0; deriv < 2; deriv++ ) {
		S[object_number][coord][deriv] = 
		A[object_number][coord][deriv] = init_state[coord][deriv];	//For collisions...
		}}


//		Now convert the input Euler angles and derivatives into quaternion
//		initial conditions...
//		=====================

//		Zeros...
//		--------
                sincos( .5*init_state[3][0], &sin_alpha, &cos_alpha );                
                sincos( .5*init_state[4][0], &sin_beta,  &cos_beta  );                
                sincos( .5*init_state[5][0], &sin_gamma, &cos_gamma );                
                
//		S[object_number][3][0] = A[object_number][3][0] =
//		                cos_gamma*cos_alpha*cos_beta + sin_gamma*sin_alpha*sin_beta;
                        
//		S[object_number][4][0] = A[object_number][4][0] =
//		                cos_gamma*cos_alpha*sin_beta - sin_gamma*sin_alpha*cos_beta;
                
//		S[object_number][5][0] = A[object_number][5][0] =
//		                cos_gamma*sin_alpha*cos_beta + sin_gamma*cos_alpha*sin_beta;

//		S[object_number][6][0] = A[object_number][6][0] =
//		               -cos_gamma*sin_alpha*sin_beta + sin_gamma*cos_alpha*cos_beta;

//		Firsts...
//		---------
//		S[object_number][3][1] = -.5*( S[object_number][4][0]*init_state[4][1]
//					     + S[object_number][5][0]*init_state[3][1]
//					     + S[object_number][6][0]*init_state[5][1] );

//		S[object_number][4][1] =  .5*( S[object_number][3][0]*init_state[4][1]
//					     + S[object_number][5][0]*init_state[5][1]
//					     - S[object_number][6][0]*init_state[3][1] );

//		S[object_number][5][1] =  .5*( S[object_number][3][0]*init_state[3][1]
//					     + S[object_number][6][0]*init_state[4][1]
//					     - S[object_number][4][0]*init_state[5][1] );

//		S[object_number][6][1] =  .5*( S[object_number][3][0]*init_state[5][1]
//					     + S[object_number][4][0]*init_state[3][1]
//					     - S[object_number][5][0]*init_state[4][1]  );
		

//		To be sure...
//		=============
		S[object_number][3][0] =
		S[object_number][4][0] =
		S[object_number][5][0] = END;


//		Put in the appropriate deadly parameters...
//		===========================================
		for ( int copy = 0; copy < 10; copy++ ) {
		I[object_number][copy + 20] = params[copy];
		}
		I[object_number][30] = DEATH;	    			//Hey, you are what you eat.

//		Now tell Soliton where to look for the equations of motion...
//		=============================================================
		idof_functions[object_number] = deadly_idof;

		equation_of_motion[object_number][0] = 
		equation_of_motion[object_number][1] = 
		equation_of_motion[object_number][2] = 
		equation_of_motion[object_number][3] =          		//Nice symmetries, huh.
		equation_of_motion[object_number][4] = 
		equation_of_motion[object_number][5] = 
		equation_of_motion[object_number][6] = null_function;


//		Put in the collision information...
//		===================================
		I[object_number][31] = I[object_number][27];
		I[object_number][32] =
		I[object_number][33] =
		I[object_number][34] =
		I[object_number][35] = 0;
		I[object_number][36] = I[object_number][20];			//Shrugoff "mass"...
		I[object_number][37] = -1;
		I[object_number][38] = 0;					//Not killed yet...

//		Wake me up...
//		=============
		no_no_not_me[object_number] = 1;
               

//		Things seem okay...
//		===================
		error_code = object_number;
	

	}


//	Inform the caller...
//	====================
	return error_code;

}


#pragma require_prototypes on



//	ATTENZIONE:  Los parametros del model son:
//	==========================================


//	Non compiling parameters...
//	---------------------------

//	Number   |   Comment
//	--------------------
//	0        |   Mass
//	1        |   One over Mass
//	2        |   1/Ialpha
//	3        |   1/Ibeta
//	4        |   1/Igamma
//	5        |   Fluid drag
//	6        |   gravity
//	7	 |   size
//	8        |   Not yet
//	9	 |   Not yet
//	==========================================
//	So there.

