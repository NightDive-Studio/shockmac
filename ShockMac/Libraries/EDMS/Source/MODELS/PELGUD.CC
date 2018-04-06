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
//	Pelvis.cc is a destroyed biped.  R.I.P.  Life is pain, and in addition this should make it
//	less fun to move in the indoor games.  You should probably use the Biped instead unless
//	you are a Twinkie and thus more concerned with inventory systems than dynamic fun...

//      Comment rescinded.  Pelvis now fun.  End of line.
//	=================================================

//	Seamus, Nov 2, 1993...
//	========================


#include <iostream.h>
#include <conio.h>
#include <EDMS_Int.h>				//This is the object type library. It is universal.
#include <EDMS_vt.h>
#include <mout.h>


//	Super secret Church-Blackley Boundary Condition Descriptor (BCD)...
//	===================================================================
extern "C" {
int	EDMS_BCD = 0; }

//	Functions...
//	============
extern void	( *idof_functions[MAX_OBJ] )( int ),
		( *equation_of_motion[MAX_OBJ][6] )( int );


//	Collision callback stuff...
//	---------------------------
static int		badness;
static physics_handle	C,
			V;

//	Callbacks themselves...
//	-----------------------
extern void		( *EDMS_object_collision )( physics_handle caller, physics_handle victim, int badness, long DATA1, long DATA2 ),
			( *EDMS_wall_contact )( physics_handle caller );

static Q		fix_one = 1.,
			point_five = .5,
			two_pi = 6.283185;

//	Just a thought...
//	=================
static Q		object0, object1, object2, object3, object4,		//Howzat??
			object5, object6, object7, object8, object9,
			object10, object11, object12, object13, object14,
			object15, object16, object17, object18, object19,
			object20, object21, object22;

static Q		sin_alpha = 0,	cos_alpha = 0,
			sin_beta  = 0,	cos_beta  = 0,
			sin_gamma = 0,	cos_gamma = 0,
			lp_x = 0,
			lp_y = 0,
			lp_z = 0,
			Fmxm = 0,	Fmym = 0,	Fmzm = 0,
			T_beta = 0,	T_gamma = 0;


//      Global for head information...
//      ==============================
Q       head_delta[3],
        head_kappa[3];



//	Here are the internal degrees of freedom:
//	=========================================
void	pelvis_idof( int object ) {


//      To do the head motion...
//      ------------------------
void    get_head_of_death( int );


Q	V_ceiling[3],
	V_floor[3],
	V_wall[3];

Q	checker,
	wall_check;



	indoor_terrain( A[object][0][0],				//Get the info...
			A[object][1][0],
			A[object][2][0],
			I[object][22],
			on2ph[object] );


		V_ceiling[0].fix_to( terrain_info.cx );			//Put it in...
		V_ceiling[1].fix_to( terrain_info.cy );
		V_ceiling[2].fix_to( terrain_info.cz );

		V_floor[0].fix_to( terrain_info.fx );
		V_floor[1].fix_to( terrain_info.fy );
		V_floor[2].fix_to( terrain_info.fz );

Q		mag = I[object][18]*I[object][18] + I[object][19]*I[object][19];
		mag = ( mag > .01 );
//		V_floor[1] *= mag;
//		V_floor[0] *= mag;

		V_wall[0].fix_to( terrain_info.wx );
		V_wall[1].fix_to( terrain_info.wy );
		V_wall[2].fix_to( terrain_info.wz );

		object0 = V_wall[0] + V_floor[0] + V_ceiling[0];	//V_raw...
		object1 = V_wall[1] + V_floor[1] + V_ceiling[1];
		object2 = V_wall[2] + V_floor[2] + V_ceiling[2];


                checker = sqrt(	object0*object0
			      + object1*object1
			      + object2*object2 );


		if (checker > 0.) {                     		//To get primitive...
                                        object3 = fix_one / checker;
                                        object9 = 1;			//Are we in the rub???
                                  }

		else checker = object9 = 0;

                if ( I[object][10] > 0 ) object9 = 1;                   //Cyberspace...

		object4	= object3*object0;				//The primitive V_n...
		object5	= object3*object1;
		object6	= object3*object2;

//              object8 = I[object][20]( 4*checker/(I[object][22]*I[object][22]) + 1 );
		object8 = I[object][20];
				
		object7 = I[object][21]*( A[object][0][1]*object4	//Delta_magnitude...
					+ A[object][1][1]*object5
					+ A[object][2][1]*object6 );

		object4 = object7*object4;				//Delta...
		object5 = object7*object5;
		object6 = object7*object6;

		I[object][14] = .01*sqrt(object4*object4 + object5*object5 + object6*object6);	//Damage??
//		if (I[object][14]>0.01) mout << "damage to Pelvis: " << I[object][14] << "!\n";

//		Let's not power through the walls anymore...
//		--------------------------------------------
		I[object][18] *= ( V_wall[0] == 0 );
		I[object][19] *= ( V_wall[1] == 0 );
		I[object][17] *= ( V_ceiling[2] == 0 );

		
//		Cyberama...
//		-----------		
		if ( object9 == 0 && I[object][10] == 0 ) {
		I[object][18] = 
		I[object][19] = 0;
		I[object][17] = 0;
		}


//              if (I[object][17]>0) mout << "17 after 9check: " << I[object][17] << "\n";


//		Collision B/C...
//		----------------
		object10 = object11 = 0;						//B/C...

		if ( are_you_there( object ) ) {

                for ( int other_object = 0; other_object < MAX_OBJ && S[other_object][0][0] > END; other_object++ ) {
		if ( check_for_hit_mac( other_object ) && object != other_object) {

		object12 = sqrt ( 
			( A[object][0][0] - A[other_object][0][0] )*
			( A[object][0][0] - A[other_object][0][0] )
		      + ( A[object][1][0] - A[other_object][1][0] )*
			( A[object][1][0] - A[other_object][1][0] )			
		      + ( A[object][2][0] - A[other_object][2][0] )*
			( A[object][2][0] - A[other_object][2][0] ) );			//Little ...

		object13 = ( I[object][31] + I[other_object][31] );			//Big ...

		if ( ( object12 < object13 )&&( object12 > 0.01 ) ) {


//		Callback...
//		-----------
		C = on2ph[object];
		V = on2ph[other_object];
		badness = ( 20*(1. - object12/object13) ).to_int();
		EDMS_object_collision( C, V, badness, 0, 0 );

		object14 = ( A[object][0][0] - A[other_object][0][0] );			//Dx...
		object15 = ( A[object][1][0] - A[other_object][1][0] );			//Dy...
		
		object16 = I[object][20]*( object13 - object12 );		   	//Eta...

		object10 += object16*object14/object12;
		object11 += object16*object15/object12;


//		Let's not power through the walls anymore...
//		--------------------------------------------
		object10 *= ( (V_wall[0]<0.01) && (V_wall[0]>-0.01) );
		object11 *= ( (V_wall[1]<0.01) && (V_wall[1]>-0.01) );

//		Wakeup...
//		=========
		if (no_no_not_me[other_object] == 0 ) {
//		mout << "Other guy was asleep!!\n";
		collision_wakeup( other_object );
		}


		}
		}
		}
		}


//	Back to business...
//	===================
        sincos( A[object][3][0], &sin_alpha, &cos_alpha );                      //Positive for local...
        sincos( A[object][4][0], &sin_beta,  &cos_beta  );
        sincos( A[object][5][0], &sin_gamma, &cos_gamma );


//      Hellishness...
//      ==============
        if( (I[object][17] > 0 && V_floor[2] == 0)  /*|| EDMS_BCD*/ ) {
Q       	ass = sqrt( I[object][18]*I[object][18]
			  + I[object][19]*I[object][19] );
		if (checker>0) {

                mout << "Ass = " << ass << ", object0 = " << object0 << ", object1 = " << object1 << "\n";
                
                I[object][17] = .02*ass;
		I[object][18] = -.2*I[object][22]*object0*object8/checker;
		I[object][19] = -.2*I[object][22]*object1*object8/checker;
        	I[object][16] *= .01;
		}
        }


//      Fateful attempt(Jump)...
//      ========================
        object18 = 800*(I[object][17]>0)*( I[object][17] - A[object][2][1] );	                     //Jump...


//	Crouch torso bend thang and boogie boogie boogie...
//	===================================================
	if ( (I[object][7] > 0.0) || (I[object][0] < I[object][6]) ) I[object][0] = I[object][6]*(1 - .636*abs(S[object][4][0]));          //Crouch...                                        
        else I[object][0] = I[object][6];

//      The head...
//      ===========
        get_head_of_death( object );


//      Pelvis specifics...
//      ===================
        object20 = object8*object0 - object4 + head_kappa[0] - head_delta[0] + I[object][18] 		//F_mxyz...
                 + object9*( - (1-.9*I[object][10])*I[object][23]*A[object][0][1] )
                 + object10;

        object21 = object8*object1 - object5 + head_kappa[1] - head_delta[1] + I[object][19]
                 + object9*( - (1-.9*I[object][10])*I[object][23]*A[object][1][1] )
                 + object11;

        object22 = object8*object2 - (object18==0)*object6 + head_kappa[2] - head_delta[2] + object18
		 + object9*( - I[object][23]*A[object][2][1] );


//	Is there a projectile hit?
//	==========================
	if ( I[object][35] > 0 ) {

//		Let's not power through the walls anymore...
//		--------------------------------------------
		I[object][32] *= ( (V_wall[0]<0.01) && (V_wall[0]>-0.01) );
		I[object][33] *= ( (V_wall[1]<0.01) && (V_wall[1]>-0.01) );
		I[object][34] *= ( (V_ceiling[2]<0.01) && (V_ceiling[2]>-0.01) );

		object20 += I[object][32];
		object21 += I[object][33];
                object22 += I[object][34];

		I[object][35] = 0;
		I[object][32] = 0;
		I[object][33] = 0;
		I[object][34] = 0;

		}


        Fmxm = object20*cos_alpha + object21*sin_alpha;                         //Locals...
        Fmym =-object20*sin_alpha + object21*cos_alpha;

        lp_z =-.1*I[object][0]*cos_beta*cos_gamma;

Q       Head_tau_beta =  -.1*I[object][0]*sin_beta*(  head_kappa[2] - head_delta[2] ),
        Head_tau_gamma = -.1*I[object][0]*sin_gamma*( head_kappa[2] - head_delta[2] );
                          

	T_beta  = -( lp_z*Fmxm ) + I[object][7] + Head_tau_beta;               			//Actual torques...
        T_gamma = -( - Fmym*lp_z) + .04*I[object][16] + Head_tau_gamma;


//	Kickbacks...
//	============
	if( abs(I[object][8]) > 0 ) {
		T_beta  -= cos_alpha*I[object][8] + sin_alpha*I[object][9];
		T_gamma  =-sin_alpha*I[object][8] + cos_alpha*I[object][9];
		I[object][8] = I[object][9] = 0;
	}

	object17 = I[object][28]*( 1 + 1.2*(I[object][16] == 0) );		// 3 is 2

//	Angular play (citadel) ...
//	==========================
	if( S[object][3][0] > two_pi ) S[object][3][0] -= two_pi;
	if( S[object][3][0] <-two_pi ) S[object][3][0] += two_pi;



//	Try the equations of motion here for grins...
//	=============================================
        S[object][0][2] = I[object][24]*( object20 );

        S[object][1][2] = I[object][24]*( object21 );

        S[object][2][2] = I[object][24]*( object22 ) - (1-.1*I[object][10])*I[object][25];

        S[object][3][2] = I[object][27]*( I[object][16]				
					- object17*A[object][3][1] );	

//        mout << S[object][3][2] << "\n";


        S[object][4][2] = I[object][27]*( T_beta
                                        - 1.5*I[object][1]*A[object][4][0]*(1-.3*I[object][10])
                                        - I[object][2]*A[object][4][1]    *(1-.7*I[object][10]) );

        S[object][5][2] = I[object][27]*( T_gamma
                                        - I[object][1]*A[object][5][0]	  *(1-.3*I[object][10])
                                        - I[object][2]*A[object][5][1]	  *(1-.7*I[object][10])
                                        + I[object][15] );


//	That's all, folks...
//	====================

}




//      Here we'll get the head information we all want so badly...
//      ===========================================================
void    get_head_of_death( int object ) {

Q       vec0,
        vec1,
        vec2,
        test,
        mul,
        vv0, vv1, vv2,
        dmag,
        kmag;


Q       offset_x = I[object][0]*sin( A[object][4][0] ),
        offset_y =-I[object][0]*sin( A[object][5][0] ),
        offset_z = I[object][0]*cos( (.2 + .8*(I[object][10]>0) )*A[object][4][0] )*cos( (.2 + .8*(I[object][10]>0) )*A[object][5][0] );
//      offset_z = I[object][0]*cos( -A[object][4][0] )*cos(-A[object][5][0] );

	indoor_terrain( A[object][0][0] + offset_x,
                        A[object][1][0] + offset_y,
                        A[object][2][0] + offset_z, 
			.75*I[object][22],
			on2ph[object] );

		vec0.fix_to( terrain_info.fx + terrain_info.cx + terrain_info.wx );
		vec1.fix_to( terrain_info.fy + terrain_info.cy + terrain_info.wy );
		vec2.fix_to( terrain_info.fz + terrain_info.cz + terrain_info.wz );

		test = sqrt(    vec0*vec0
			      + vec1*vec1
			      + vec2*vec2 );


		if (test > 0) mul = fix_one / test;		//To get primitive...
		else test = mul = 0;

		vv0	= mul*vec0;				//The primitive V_n...
		vv1	= mul*vec1;
		vv2	= mul*vec2;

		dmag =     I[object][21]*(        A[object][0][1]*vv0  			//Delta_magnitude...
			        		+ A[object][1][1]*vv1
				        	+ A[object][2][1]*vv2 );

		head_delta[0] = dmag*vv0;                				//Delta...
		head_delta[1] = dmag*vv1;
		head_delta[2] = dmag*vv2;


//		if (test < .5*I[object][22]) kmag = I[object][20];			//Omega_magnitude...
//              else kmag = I[object][20]/test;
                kmag = I[object][20];

                head_kappa[0] =  kmag*vec0;
                head_kappa[1] =  kmag*vec1;
                head_kappa[2] =  kmag*vec2;



}












//	We might for now want to set some external forces on the pelvis...
//	==================================================================
void	pelvis_set_control( int pelvis, Q forward, Q turn, Q sidestep, Q lean, Q jump, int crouch ) {

const Q         pi_by_two = 1.5707;                                             //Yea, flixpoint...

	sincos( A[pelvis][3][0], &object0, &object1 );



//	Get rid of it all...
//	--------------------
	I[pelvis][15] = I[pelvis][16] = I[pelvis][17] = I[pelvis][18] = I[pelvis][19] = I[pelvis][7] = 0;


//	Here's the thrust of the situation...
//	-------------------------------------
	I[pelvis][18] = forward*object1*I[pelvis][26];
	I[pelvis][19] = forward*object0*I[pelvis][26];


//      And the sidestep is off by pi/two...
//      ------------------------------------
        sincos( (A[pelvis][3][0] - pi_by_two), &object0, &object1 );
        I[pelvis][18] += sidestep*object1*I[pelvis][26];
        I[pelvis][19] += sidestep*object0*I[pelvis][26];

//	And the turn of the...
//	----------------------
	I[pelvis][16] = turn*I[pelvis][29];

//	Jump jets of joy...
//	-------------------
	if ( jump > 0 ) I[pelvis][17] = .002*I[pelvis][26]*jump;
	if ( jump < 0 ) I[pelvis][17] = -I[pelvis][26]*jump;

//      And finally leaning about...
//      ----------------------------
        I[pelvis][15] = I[pelvis][29]*lean*I[pelvis][1];				//Exactly the angle!
                                                             
//	Crouching (overpowers jumping )...
//	----------------------------------
	if ( crouch > 0 ) I[pelvis][7] = .15*crouch*I[pelvis][1];

//	Wake up...
//	==========
	no_no_not_me[pelvis] = ( abs(I[pelvis][15]) + abs(I[pelvis][16]) + abs(I[pelvis][17])
			       + abs(I[pelvis][18]) + abs(I[pelvis][19]) > 0 ) || (no_no_not_me[pelvis]==1);

}





int make_pelvis( Q init_state[6][3], Q params[10] ) {

//	Sets up everything needed to manufacture a pelvis with initial state vector
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

// hey make these our cool new inlined memsets, they are so hip

//		Now we can create the pelvis:  first dump the initial state vector...
//		=====================================================================
		for ( int coord = 0; coord < 6; coord++ ) {
		for ( int deriv = 0; deriv < 3;	deriv++ ) {			//Has alpha now...
		S[object_number][coord][deriv] = 
		A[object_number][coord][deriv] = init_state[coord][deriv];	//For collisions...
		}}

//		Put in the appropriate pelvis parameters...
//		===========================================
		for ( int copy = 0; copy < 10; copy++ ) {
		I[object_number][copy + 20] = params[copy];
		}
		I[object_number][30] = PELVIS;					//Hey, you are what you eat.


//              We need some information that won't fit in the usual areas...
//              =============================================================
//              I[object_number][0] =                                           //For reference...
                I[object_number][6] = .5*I[object_number][22];
                I[object_number][1] = 20*I[object_number][29];
                I[object_number][2] = 4*sqrt( I[object_number][29]*I[object_number][1] );

                                       
//		Put in the collision information...
//		===================================
		I[object_number][31] = I[object_number][22];
		I[object_number][32] =
		I[object_number][33] =
		I[object_number][34] =
		I[object_number][35] =	0;
		I[object_number][36] = I[object_number][26];			//Shrugoff "mass"...


//		Zero the control initially...
//		=============================
                I[object_number][7]  =
		I[object_number][15] =
		I[object_number][16] =
		I[object_number][18] =
		I[object_number][19] =
		I[object_number][17] = 0;


//		Now tell Soliton where to look for the equations of motion...
//		=============================================================
		idof_functions[object_number] = pelvis_idof;

		equation_of_motion[object_number][0] =				//Nice symmetries, huh. 
		equation_of_motion[object_number][1] = 
		equation_of_motion[object_number][2] = 
		equation_of_motion[object_number][3] = 
		equation_of_motion[object_number][4] = 		  		
		equation_of_motion[object_number][5] = null_function;


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
