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
/*
 * $Header: r:/prj/lib/src/new_edms/models/RCS/marble.cc 1.7 1994/12/12 13:49:49 dfan Exp $
 */

//	Marble.cpp contains the equations of motion (in regular guy 3+1 space) and the 
//	internal degrees of freedom (in arbitrary configuration or phase spaces) for the elastic
//	marble on the T[x][y][grad] surface. In addition, it contains the creation utility for
//	the marbles, as required by soliton and its minions. This is the prototype for future
//	vehicle and object model modules.
//	=================================


//	Jon Blackley, Oct. 26, 1992
//	===========================


#include <iostream.h>
#include "EDMS_Int.h"				//This is the object type library. It is universal.
#include "physhand.h"
#include "idof.h"

#ifdef EDMS_SHIPPABLE
//#include <mout.h>
#endif

extern void	( *idof_functions[MAX_OBJ] )( int ),
		( *equation_of_motion[MAX_OBJ][7] )( int ),
		null_function( int dummy );

static Q		fix_one = 1.,
			point_five = .5;

//	Collision callback stuff...
//	---------------------------
static int		badness;
static physics_handle	C,
			V;
static fix		location[3];

//	Callbacks themselves...
//	-----------------------
extern void		( *EDMS_object_collision )( physics_handle caller, physics_handle victim, int badness, long DATA1, long DATA2, fix location[3] ),
			( *EDMS_wall_contact )( physics_handle caller );




//	Just a thought...
//	=================
static Q		object0, object1, object2, object3, object4,		//Howzat??
			object5, object6, object7, object8, object9,
			object10, object11, object12, object13, object14,
			object15, object16, object17, object18, object19;


//	First, here are the equations of motion...
//	==========================================


//	Marble (world) z coordinate...
//	==============================
void	marble_Z( int object ) {

	S[object][2][2] = I[object][24]*( I[object][3]*I[object][8] )		//Elasticity...
					- I[object][25];


}



//	Marble (world) x coordinate...
//	==============================
void	marble_X( int object ) {

	S[object][0][2] = I[object][24]*( I[object][4]*I[object][8] 		//Elasticity...
					- I[object][23]*A[object][0][1]		//Drag... 
					+ I[object][10]				//Bump...
					+ I[object][18] );			//Control...

}



//	Marble (world) y coordinate...
//	==============================
void	marble_Y( int object ) {

	S[object][1][2] = I[object][24]*( I[object][5]*I[object][8]		//Elasticity... 
					- I[object][23]*A[object][1][1]		//Drag...
					+ I[object][11]				//Bump...
					+ I[object][19] );			//Control...

}



#pragma require_prototypes off


//	Here are the internal degrees of freedom:
//	=========================================

void	marble_idof( int object )
{
   object0 = terrain( A[object][0][0], A[object][1][0], 0 );		//Surface height...

	object1 = terrain( A[object][0][0], A[object][1][0], 1 );		//dzdx...
	object2 = terrain( A[object][0][0], A[object][1][0], 2 );		//dzdy...

	object3 = fix_one/( sqrt( fix_one + object1*object1 ));			//Cos thetaZ	
	object4 = -object1*object3;						//Sin thetaX
	object5 = -object2/( sqrt( fix_one + object2*object2 ));		//Sin thetaY

	object6 = A[object][2][0] - object0;					//Zeta...
	object7 = (object6 < I[object][22]);
	object8 = object7*( -I[object][20]*		  			//Z moment...
			     ( object6 - I[object][22] )
			     - I[object][21]*A[object][2][1] );	


	object10 = object11 = 0;						//B/C...

   //	Collision B/C...
   //	================

   ulong mask = are_you_there (object);
   ulong bit = 0;
   int other_object;

   while (mask != 0)
   {
      if (mask & 1)
      {
         for (other_object = bit; other_object < MAX_OBJ && S[other_object][0][0] > END; other_object += NUM_OBJECT_BITS)
         {
            if (other_object != object && I[object][IDOF_COLLIDE].to_int() != other_object )
            {
               Q dx = A[object][0][0] - A[other_object][0][0];
               Q dy = A[object][1][0] - A[other_object][1][0];
               Q dz = A[object][2][0] - A[other_object][2][0];

		         object13 = ( I[object][31] + I[other_object][31]);		//Big R...

               if (abs(dx) < object13 && abs(dy) < object13 && abs(dz) < object13) // let's not overflow
               {
 		            object12 = sqrt (dx*dx + dy*dy + dz*dz); // little R

		            if ( object12 < object13 )
                  {
                     //	Autodestruct...
                     //	---------------
              		   if ( I[object][38] > 0 ) I[object][38] = -1;

                     //	Callback...
                     //	-----------
		               C = on2ph[object];	V = on2ph[other_object];
		               badness = ( 20*(1. - object12/object13) ).to_int();
		               location[0] = A[object][0][0].to_fix();
                     location[1] = A[object][1][0].to_fix();
                     location[2] = A[object][2][0].to_fix();
		               EDMS_object_collision( C, V, badness, 0, 0, location );

		               object14 = ( A[object][0][0] - A[other_object][0][0] );			//Dx...
		               object15 = ( A[object][1][0] - A[other_object][1][0] );			//Dy...

		               object16 = I[object][20]*( object13 - object12 );		   	//Eta...

		               if (object12 > .01)
                     {
   		               object10 += object16*object14/object12;					//Actual conmpressions,
   		               object11 += object16*object15/object12;					//wathc out for 0s...
                     }

                     // Wakeup...
                     //	=========

         		      if (no_no_not_me[other_object] == 0 )
                        collision_wakeup( other_object );
                  }
               }   
      		}
   		}
		}

      mask >>= 1;
      bit++;
	}

   //	Control drag for sticky marbles...
   //	==================================
	object19 = I[object][23]*point_five;
	object17 = object7*( object19 + (abs(I[object][18]) < 0.01)*I[object][23] );
	object18 = object7*( object19 + (abs(I[object][19]) < 0.01)*I[object][23] );

   //	Try the equations of motion here for grins...
   //	=============================================
	S[object][2][2] = I[object][24]*( object3*object8			//Elasticity...
					+ I[object][17] )			//Control...
					- I[object][25];



	S[object][0][2] = I[object][24]*( object4*object8	 		//Elasticity...
					- object17*A[object][0][1]		//Drag... 
					+ object10				//Bump...
					+ object7*I[object][18] );		//Control...


	S[object][1][2] = I[object][24]*( object5*object8			//Elasticity... 
					- object18*A[object][1][1]		//Drag...
					+ object11				//Bump...
					+ object7*I[object][19] );		//Control...


   //	That's all, folks...
   //	====================
}




//	We might for now want to set some external forces on the marble...
//	==================================================================
void	marble_set_control( int marble, Q X, Q Y, Q Z ) {

//	These accelerations are normalized, of course...
//	------------------------------------------------
	I[marble][18] = I[marble][26]*X;
	I[marble][19] = I[marble][26]*Y;
	I[marble][17] = I[marble][26]*abs( Z );				//Jumping only!

	no_no_not_me[marble] = ( no_no_not_me[marble] || 
			         ( abs(I[marble][18]) + abs(I[marble][19]) + abs(I[marble][17]) > 0 ) );

}



int make_marble( Q init_state[6][3], Q params[10] )
{
//	Sets up everything needed to manufacture a marble with initial state vector
//	init_state[][] and EDMS motion parameters params[] into soliton. Returns the 
//	object number, or else a negative error code (see Soliton.CPP for error handling and codes).
//	============================================================================================



//	Have some variables...
//	======================
   int	object_number = -1,						//Three guesses...
      	error_code = -1;						//Guilty until...

//	We need ignorable coordinates...
//	================================
   extern void	null_function( int );



//	First find out which object we're going to be...
//	================================================
	while ( S[++object_number][0][0] > END );			//Jon's first C trickie...


//	Is it an allowed object number?  Are we full? Why are we here? Is there a God?
//	==============================================================================
	if ( object_number < MAX_OBJ )
   {

//		Now we can create the marble:  first dump the initial state vector...
//		=====================================================================
		for ( int coord = 0; coord < 6; coord++ )
      {
   		for ( int deriv = 0; deriv < 3;	deriv++ )
         {
		      S[object_number][coord][deriv] = 
		      A[object_number][coord][deriv] = init_state[coord][deriv];	//For collisions...
   		}
      }

//		Put in the appropriate marble parameters...
//		===========================================
		for ( int copy = 0; copy < 10; copy++ )
      {
   		I[object_number][copy + 20] = params[copy];
		}
		I[object_number][30] = MARBLE;					//Hey, you are what you eat.


//		Put in the collision information...
//		===================================
		I[object_number][31] = I[object_number][22];
		I[object_number][32] =
		I[object_number][33] =
		I[object_number][34] =
		I[object_number][35] =	0;
		I[object_number][36] = I[object_number][24];
		I[object_number][37] = -1;
		I[object_number][38] =  0;			//No kill I...


//		Zero the control initially...
//		=============================
		I[object_number][18] =
		I[object_number][19] =
		I[object_number][17] = 0;


//		Now tell Soliton where to look for the equations of motion...
//		=============================================================
		idof_functions[object_number] = marble_idof;

		equation_of_motion[object_number][0] = marble_X;
		equation_of_motion[object_number][1] = marble_Y;
		equation_of_motion[object_number][2] = marble_Z;
		equation_of_motion[object_number][3] = 				//Nice symmetries, huh.
		equation_of_motion[object_number][4] = 
		equation_of_motion[object_number][5] = null_function;

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


#pragma require_prototypes on



//	ATTENZIONE:  Los parametros del model son:
//	==========================================

//	Number   |   Comment
//	--------------------
//	0        |   K
//	1        |   d
//	2        |   Radius
//	3        |   Rolling Drag
//	4        |   1/Mass
//	5        |   gravity
//	6        |   mass
//	7	 |   Color
//	8,9      |   Not Applicable.

//	==========================================
//	So there.
