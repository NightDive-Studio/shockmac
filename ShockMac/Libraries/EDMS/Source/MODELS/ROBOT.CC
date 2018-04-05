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
//	Robot.cc is a test object for the Citadel physics system.  It uses the Citadel database
//	for B/C, and should be fairly simple and robust.  Use the vector integrator!
//	============================================================================

//	Seamus, June 29, 1993...
//	========================


#include <iostream.h>
//#include <conio.h>
#include "EDMS_Int.h"				//This is the object type library. It is universal.
#include "EDMS_vt.h"

//#ifdef EDMS_SHIPPABLE
//#include <mout.h>
//#endif


#include "EDMS_chk.h"

//extern "C" {
//#include <i86.h>   
//#include <dpmi.h>   
#include "ss_flet.h"
//}

//	State information and utilities...
//	==================================
extern EDMS_Argblock_Pointer	A;
extern Q	S[MAX_OBJ][7][4],
		I[MAX_OBJ][DOF_MAX];
extern int	no_no_not_me[MAX_OBJ];

#define SOLITION_FRAME_CNT

//	Functions...
//	============
extern void	( *idof_functions[MAX_OBJ] )( int ),
		( *equation_of_motion[MAX_OBJ][7] )( int );


//	Callbacks themselves...
//	-----------------------
extern void		( *EDMS_object_collision )( physics_handle caller, physics_handle victim, int badness, long DATA1, long data2, fix loc[3] ),
			( *EDMS_wall_contact )( physics_handle caller );


//extern int	are_you_there( int );			//Collisions...
//extern int	check_for_hit( int );


static Q		fix_one = 1.,
			point_five = .5,
			two_pi = 6.283185;

//	Just a thought...
//	=================
static Q		object0, object1, object2, object3, object4,		//Howzat??
			object5, object6, object7, object8, object9,
			object10, object11, object12, object13, object14,
			object15, object16, object17, object18, object19;


//	First, here are the equations of motion (outdated!)...
//	======================================================
int     EDMS_robot_global_badness_indicator = 0;

//	Variables that are NOT on the stack...
//	======================================
static Q	A00,	A10,
		A20,	A30,
		A01,	A11,
		A21,	A31;

static Q	checker,
		check0,
		check1,
                check2,
		V_wall0,
		V_wall1;

static Q	drug,
		butt; 

const Q		wt_pos = 0.001,
		wt_neg = -wt_pos;



#pragma require_prototypes off


//	Here are the internal degrees of freedom:
//	=========================================
void	robot_idof( int object ) {


//	Call me instead of having special code everywhere...
//	====================================================
extern void	shall_we_dance( int object, Q& result0, Q& result1, Q& result2 );


	A00 = A[object][0][0];						//Dereference NOW!
	A10 = A[object][1][0];
	A20 = A[object][2][0];
	A30 = A[object][3][0];
	A01 = A[object][0][1];
	A11 = A[object][1][1];
	A21 = A[object][2][1];
	A31 = A[object][3][1];

        I[object][17] = 0;                                              //Make sure we REALLY want to climb...


        EDMS_robot_global_badness_indicator = 0;

	indoor_terrain( A00,						//Get the info...
			A10,
			A20,
			I[object][22],
			on2ph[object] );
        
//        if (EDMS_robot_global_badness_indicator != 0 ) { 
//                                        mout << "!Robot.cc: thinks that X: " << A00 << ", Y: " << A10 << ", Z: " << A20 << "\n";
//                                        mout << "!Robot.cc: has object number " << object << ", ph: " << on2ph[object] << "\n";
//                                       }

//              Boy, will this be faster in the new order...
//              ============================================
Q               w0, w1, w2, w3, w4;
		w0.fix_to(terrain_info.wx);
		w1.fix_to(terrain_info.wy);
		w2.fix_to(terrain_info.wz);

//		w3 = sqrt( w0*w0 + w1*w1 + w2*w2 );
//		w4 = 2*( w3 - .5*I[object][22] );
//		if ( w4 < 0 ) w4 = 0;   //Ouch!
//		w4 = w4 / w3;
//		w0 *= w4;       w1 *= w4;       w2 *= w4;

		if( w0 == 0 ) check0 = 1;
		else check0 = 0;

		if( w1 == 0 ) check1 = 1;
		else check1 = 0;

		if( (terrain_info.fz == 0) && (terrain_info.cz == 0) ) check2 = 1;
		else check2 = 0;

		object0.fix_to( terrain_info.fx + terrain_info.cx );
		object1.fix_to( terrain_info.fy + terrain_info.cy );
		object2.fix_to( terrain_info.fz + terrain_info.cz );


                object0 += w0;  object1 += w1;  object2 += w2;


		checker = sqrt( object0*object0
			      + object1*object1
			      + object2*object2 );


		if (checker > .0005) { object3 = fix_one / checker; 	//To get primitive...
//                                               mout << "NZero!!  " << checker << "\n";
                                             }
		else checker = object3 = 0;

		object4	= object3*object0;				//The primitive V_n...
		object5	= object3*object1;
		object6	= object3*object2;

		object7 = .75*I[object][21]*( A01*object4			//Delta_magnitude...
					+ A11*object5
					+ A21*object6 );

		object8 = I[object][20];			       //Omega_magnitude...
//		if( I[object][10]<0 ) object7 *= 4;
//		if( I[object][10]<0 ) object8 *= 2;

//              mout << "o7: " << object7 << "\n";
		object4 = object7*object4;				//Delta...
		object5 = object7*object5;
		object6 = object7*object6;

		object9 = ( ( checker > .001 ) || ( I[object][10] > 0 ) );       //Are we in the rub???


//		Let's not power through the walls anymore...
//		--------------------------------------------
		I[object][18] *= check0;
		I[object][19] *= check1;


//      Here are collisions with other objects...
//      =========================================
        object10 = object11 = object12 = 0;

        if ( I[object][5] == 0 ) {
        shall_we_dance( object, object10, object11, object12 );
  	object10 *= I[object][20]*check0;                                              //More general than it was...
  	object11 *= I[object][20]*check1;
//  	object12 *= I[object][20]*check2;
        }



//      Climbing overriden with repulsors...
//      ====================================
        if ( ss_edms_bcd_flags & SS_BCD_REPUL_ON ) {

//              Get the speed... 
Q               repulsor_speed = 21;                                                                                        
                if ( (ss_edms_bcd_flags & SS_BCD_REPUL_SPD) == SS_BCD_REPUL_NORM ) repulsor_speed = 7;

//              Assume we're going up, unless...
                if ( (ss_edms_bcd_flags & SS_BCD_REPUL_TYPE) == SS_BCD_REPUL_DOWN) repulsor_speed *= -.5;

//              The parameter should be the desired height....
Q               repul_height;
                repul_height.fix_to(ss_edms_bcd_param);


Q               nearness_or_something = repul_height - A[object][2][0];
                if ( abs(nearness_or_something) <= .333 ) {
                        repulsor_speed *= 3 * nearness_or_something;
                        }

Q               io17 = repulsor_speed;

        	I[object][17] = I[object][26]*( ( io17 - A[object][2][1] ) + I[object][25] );

                object9 = 1;

                
         }



//      AutoClimbing(tm) is for wussies (is superseeded by climbing)...
//      ===============================================================
        if ( (ss_edms_bcd_flags & SS_BCD_MISC_STAIR) ) {

Q       o1 = 0, o0 = 0;

		if ( ( checker > 0 ) && (abs(I[object][18]) + abs(I[object][19]) > .01) ) {

Q                       ratio = (I[object][18]+A[object][0][1])*object0 + (I[object][19]+A[object][1][1])*object1;
                        
Q                       io17 = .5;

                        if ( ratio <= 0 ) { o1 = object1; o0 = object0; }
                        else  o1 = o0 = io17 = 0;
                                                     
	        	I[object][18] = -.3*I[object][22]*o0*object8/checker + .1*I[object][18];
		        I[object][19] = -.3*I[object][22]*o1*object8/checker + .1*I[object][19];
//	        	io18 = -.3*I[object][22]*o0*object8/checker + .1*I[object][18];
//		        io19 = -.3*I[object][22]*o1*object8/checker + .1*I[object][19];

//                    Set the mojo...
//                    ===============
                      I[object][17] = 800*( io17 - A[object][2][1] );

                       
                }
        }


//	Angular play (citadel) ...
//	==========================
	if( S[object][3][0] > two_pi ) S[object][3][0] -= two_pi;
	if( S[object][3][0] <-two_pi ) S[object][3][0] += two_pi;


//	Don't be stupid...
//	------------------
	drug = -object9*I[object][23];
//      mout << drug << "\n";
        butt = I[object][24];



//	Try the equations of motion here for grins...
//	=============================================
	S[object][2][2] = butt*(          object8*object2			//Elasticity...
					- object6				//Drag...
					+ I[object][17]				//Control...
					+ drug*A21 
                                        + object12 )

					- I[object][25];			//Grav'ty...


	S[object][0][2] = butt*(          object8*object0	 		//Elasticity...
					- object4				//Drag... 
					+ object9*I[object][18]			//Control...
					+ drug*A01				//Drag...
					+ object10 );				//Collide...


	S[object][1][2] = butt*(	  object8*object1			//Elasticity... 
 					- object5				//Drag...
					+ object9*I[object][19]			//Control...
					+ drug*A11				//Drag...
					+ object11 );				//Collide...


	S[object][3][2] = I[object][27]*( I[object][16]				//Control...
					- I[object][28]*A31 );			//Drag...

//        mout << "Butt: " << butt << "\n";
//        mout << "1X: " << object0 << " 1Y: " << object1 << " 1Z: " << object2 << "\n";
//        mout << "VX: " << A01 << " VY: " << A11 << " VZ: " << A21 << "\n";
//        mout << "2X: " << object4 << " 2Y: " << object5 << " 2Z: " << object6 << "\n";
//        mout << "3X: " << object8*object0 << " 3Y: " << object8*object1 << " 3Z: " << object8*object2 << "\n";
//        mout << "FX: " << object8*object0 - object4 << " FY: " << object8*object1 - object5 << " FZ: " << object8*object2 - object6 << "\n";
//        mout << "xx: " << drug*A01 << " yy: " << drug*A11 << " zz: " << drug*A21 << "\n";
//          mout << " ZZ: " << S[object][2][2] << " : " << butt*(object8*object2 - object6) - I[object][25] << 
//          " : " << drug*A21 << " : " << object12 << " : " << I[object][17] << "\n";
//        mout << I[object][17] << " : " << object << "\n";

//      Damnage...
//      ==========
Q	dam0 = object8*object0 - object4;
Q	dam1 = object8*object1 - object5;
Q	dam2 = object8*object2 - object6;


		I[object][14] = I[object][24]*( abs(dam0) + abs(dam1) + abs(dam2) );      		//Damage??


//	Is there a projectile hit?
//	==========================
	if ( I[object][35] > 0 ) {

//		Let's not power through the walls anymore...
//		--------------------------------------------
//      mout << "knock " << I[object][32] << " " << I[object][33] << " " << I[object][34] << ": I[24] " << I[object][24] << "\n";

//      if (I[object][24] > 1.0)
//      {
//         if ( abs(I[object][32]) > 1000 ) I[object][32] = 1000*(1-2*(I[object][32]<0));
//         if ( abs(I[object][33]) > 1000 ) I[object][33] = 1000*(1-2*(I[object][33]<0));
//         if ( abs(I[object][34]) > 1000 ) I[object][34] = 1000*(1-2*(I[object][34]<0));
//      }

//      mout << "clamp " << I[object][32] << " " << I[object][33] << " " << I[object][34] << "\n";

 		S[object][0][2] += /* I[object][24]* */ I[object][32]*check0;
		S[object][1][2] += /* I[object][24]* */ I[object][33]*check1;
		S[object][2][2] += /* I[object][24]* */ I[object][34]*check2;

//      mout << "  add " << /* I[object][24]* */ I[object][32]*check0 << " " << 
//                          /* I[object][24]* */ I[object][33]*check0 << " " <<
//                          /* I[object][24]* */ I[object][34]*check0 << "\n";

//                mout << "R: " << object << " K: " << I[object][24]*I[object][32]*check0 << " : " << I[object][24]*I[object][33]*check1 << " : " << I[object][24]*I[object][34] << "\n";


		I[object][35] = 0;
		I[object][32] = 0;
		I[object][33] = 0;
		I[object][34] = 0;

		}
					

//	That's all, folks...
//	====================

}




//	We might for now want to set some external forces on the robot...
//	==================================================================
void	robot_set_control( int robot, Q thrust_lever, Q attitude_jet, Q jump ) {

	sincos( S[robot][3][0], &object0, &object1 );


#ifdef EDMS_SHIPPABLE
        if (I[robot][30] != ROBOT) mout << "You are an idiot: I'm not a ROBOT!\n";
#endif


//	Here's the thrust of the situation...
//	-------------------------------------
	I[robot][18] = thrust_lever*object1*I[robot][26];
	I[robot][19] = thrust_lever*object0*I[robot][26];
	I[robot][17] = I[robot][26]*jump;

//	And the turn of the...
//	----------------------
	I[robot][16] = attitude_jet*I[robot][29];


//	Wakee wakee...
//	--------------
	no_no_not_me[robot] = ( abs( I[robot][18] ) + abs( I[robot][19] ) + abs( I[robot][16] ) + abs( I[robot][17] ) > 0 );


}



//	Here is a separate control routine for robots under AI domination...
//	====================================================================
void	robot_set_ai_control( int robot, Q desired_heading, Q desired_speed, Q sidestep, Q urgency, Q &there_yet, Q distance ) {

const Q	one_by_pi     = 0.31830,
	pi	      = 3.14159,
	two_pi	      = 6.28318;



#ifdef EDMS_SHIPPABLE
        if (I[robot][30] != ROBOT) mout << "Hey, don't call control_robot on non-robots!\n";
#endif



	if ( desired_heading > two_pi ) desired_heading -= two_pi;
	if ( desired_heading < 0 )      desired_heading += two_pi;


//	Nota bene:  Here the desired heading is specified is in the range 
//		    0 <= desired_heading < 2pi.	Urgency is a number in the range
//		    0 <= urgency <= 20.  A zero urgency will produce no control input.
//		    ==================================================================

//	Setup...
//	--------
Q	speed = sqrt( S[robot][0][1]*S[robot][0][1] + S[robot][1][1]*S[robot][1][1] ),
	direction = desired_heading - S[robot][3][0];

	sincos( S[robot][3][0], &object0, &object1 );

//	Heading...
//	----------
	if ( direction >  pi ) direction = -( direction - pi );
	if ( direction <=-pi ) direction = -( direction + pi );

//	Inform the caller if we're on course yet...
//	-------------------------------------------
	there_yet = one_by_pi*direction*( 1 - 2*(direction<0) );

//	Set the control...
//	------------------
	I[robot][16] = .1*urgency*direction*I[robot][29];
	
//	Speed...
//	--------
	I[robot][17] = urgency*(1 / (10*there_yet+5) )*(desired_speed - speed);		//temporary...
        if ( I[robot][17] < 0 ) I[robot][17] = 0;

        if ( distance < 1 ) I[robot][17] *= distance;

	I[robot][18] = object1*I[robot][17] + object0*sidestep;
	I[robot][19] = object0*I[robot][17] - object1*sidestep;
	I[robot][17] = 0;								//No jumping for AIs


//	Wakee wakee...
//	--------------
        if ( no_no_not_me[robot] == 0 ) {
        no_no_not_me[robot] = ( abs( I[robot][18] ) + abs( I[robot][19] )
                              + abs( I[robot][16] ) > 0 );
//        if ( no_no_not_me[robot] != 0 ) mout << "R: " << robot << ", ph= " << on2ph[robot] << " awoken!  " << no_no_not_me[robot] << "\n";
//        mout << ( abs( I[robot][18] ) + abs( I[robot][19] ) + 50*abs( I[robot][16] ) + abs( I[robot][17] ) ) << "\n";
//        mout << no_no_not_me[robot] << "\n";
        }

//	mout << "Robot #" << robot << " with: " << ( abs( I[robot][18] ) + abs( I[robot][19] ) + 50*abs( I[robot][16] ) + abs( I[robot][17] ) ) << ".\n";;


}







int make_robot( Q init_state[6][3], Q params[10] ) {

//	Sets up everything needed to manufacture a robot with initial state vector
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
	if ( object_number < MAX_OBJ ) {


//		Now we can create the robot:  first dump the initial state vector...
//		=====================================================================
		for ( int coord = 0; coord < 6; coord++ ) {
		for ( int deriv = 0; deriv < 3;	deriv++ ) {			//Has alpha now...
		S[object_number][coord][deriv] = 
		A[object_number][coord][deriv] = init_state[coord][deriv];	//For collisions...
		}}

//		Put in the appropriate robot parameters...
//		===========================================
		for ( int copy = 0; copy < 10; copy++ ) {
		I[object_number][copy + 20] = params[copy];
		}
		I[object_number][30] = ROBOT;					//Hey, you are what you eat.


//		Put in the collision information...
//		===================================
		I[object_number][31] = I[object_number][22];
		I[object_number][32] =
		I[object_number][33] =
		I[object_number][34] =
		I[object_number][35] =	0;
		I[object_number][36] = I[object_number][24];			//Shrugoff "mass"...
		I[object_number][37] = -1;
		I[object_number][38] = 0;					//No kill I...


//              Turn ON collisions for this robot...
//              ------------------------------------
                I[object_number][5] = 0;                                        //negative values are off...



//		Zero the control initially...
//		=============================
		I[object_number][16] =
		I[object_number][18] =
		I[object_number][19] =
		I[object_number][17] = 0;


//		Now tell Soliton where to look for the equations of motion...
//		=============================================================
		idof_functions[object_number] = robot_idof;

		equation_of_motion[object_number][0] = 
		equation_of_motion[object_number][1] = 
		equation_of_motion[object_number][2] = 
		equation_of_motion[object_number][3] = 
		equation_of_motion[object_number][4] = 		  		//Nice symmetries, huh.
		equation_of_motion[object_number][5] = null_function;


//               for (int tt = 0; tt < 10; tt++ ) mout << params[tt] << " : ";
//               mout << "\n";

//		Wakee wakee...
//		--------------
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

//	Number   |   Comment
//	--------------------
//	0        |   K
//	1        |   d
//	2        |   Radius
//	3        |   Rolling Drag
//	4        |   1/Mass
//	5        |   gravity
//	6        |   mass
//	7	 |   1/moi
//	8        |   rotational drag
//	9	 |   moi
//	==========================================
//	So there.





