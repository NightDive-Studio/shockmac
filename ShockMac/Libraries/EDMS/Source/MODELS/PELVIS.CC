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
//#include <conio.h>
#include "EDMS_Int.h"				//This is the object type library. It is universal.
#include "EDMS_vt.h"


//	Super secret Church-Blackley Boundary Condition Descriptor (BCD)...
//	===================================================================
//extern "C" {

#include "ss_flet.h"

Q       EDMS_CYBER_FLOW1X = 100;
Q       EDMS_CYBER_FLOW2X = 200;
Q       EDMS_CYBER_FLOW3X = 270;

int     EDMS_BCD = 0;
int     EDMS_pelvis_is_climbing = 0;
int     edms_ss_head_bcd_flags;


fix     hacked_head_bob_1 = fix_make(1,0),
        hacked_head_bob_2 = fix_make(1,0);


//}


#define EDMS_DIV_ZERO_TOLERANCE .0005

//      For lean-o-meter...
//      -------------------
static Q               	V_ceiling[3],
                	V_floor[3],
                	V_wall[3];


//	State information and utilities...
//	==================================
extern EDMS_Argblock_Pointer	A;
extern Q	S[MAX_OBJ][7][4],
		I[MAX_OBJ][DOF_MAX];
extern int	no_no_not_me[MAX_OBJ];


//	Functions...
//	============
extern void	( *idof_functions[MAX_OBJ] )( int ),
		( *equation_of_motion[MAX_OBJ][7] )( int );



//	Callbacks themselves...
//	-----------------------
extern void	 ( *EDMS_wall_contact )( physics_handle caller );

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


static Q                io17 = 0,
                        io18 = 0,
                        io19 = 0;

static Q                checker = 0,
                        wall_check = 0;


//      Global for head information...
//      ==============================
Q       head_delta[3],
        head_kappa[3],
        body_delta[3],
        body_kappa[3];


//	Storage for running feel...
//	===========================
Q	bob_arg = 0;


#pragma require_prototypes off


//	Here are the internal degrees of freedom:
//	=========================================
void	pelvis_idof( int object ) {

// attemp to speed up something
Q		*i_object = I[object];
Q		temp_Q;

//      To do the head motion, collisions, and climbing...
//      --------------------------------------------------
void    get_head_of_death( int ),
        get_body_of_death( int ),
        do_climbing( int object );

//	Call me instead of having special code everywhere...
//	====================================================
extern void	shall_we_dance( int object, Q& result0, Q& result1, Q& result2 );

                EDMS_pelvis_is_climbing = 0;

		indoor_terrain( A[object][0][0],			//Get the info...
										A[object][1][0],
										A[object][2][0],
										i_object[22],
										on2ph[object] );


    V_ceiling[0].fix_to( terrain_info.cx );			//Put it in...
		V_ceiling[1].fix_to( terrain_info.cy );
		V_ceiling[2].fix_to( terrain_info.cz );

		V_floor[0].fix_to( terrain_info.fx );
		V_floor[1].fix_to( terrain_info.fy );
		V_floor[2].fix_to( terrain_info.fz );

Q		mag = i_object[18]*i_object[18] + i_object[19]*i_object[19];
    if ( mag < .1 && abs( V_floor[0] ) < .05*i_object[22] && abs( V_floor[1] ) < .05*i_object[22] ) 
     {
      V_floor[1].val = 0;                                 //Turns on SlopeStand(tm)...
	    V_floor[0].val = 0;
     }


    V_wall[0].fix_to( terrain_info.wx );
		V_wall[1].fix_to( terrain_info.wy );
		V_wall[2].fix_to( terrain_info.wz );

		object0.val = V_wall[0].val + V_floor[0].val + V_ceiling[0].val;	//V_raw...
		object1.val = V_wall[1].val + V_floor[1].val + V_ceiling[1].val;
		object2.val = V_wall[2].val + V_floor[2].val + V_ceiling[2].val;

//		checker = sqrt(object0*object0 + object1*object1 + object2*object2);
		checker.val = fix_sqrt( fix_mul(object0.val,object0.val) +
														fix_mul(object1.val,object1.val) +
														fix_mul(object2.val,object2.val) );

		if (checker > EDMS_DIV_ZERO_TOLERANCE )
		 {      		//To get primitive...
      object3.val = fix_div(fix_one.val,checker.val);
      object9.val = fix_make(1,0);			//Are we in the rub???
		 }
		else 
			checker.val = object9.val = 0;

    if ( i_object[10] == 2 ) object9.val = fix_make(1,0);                  //Cyberspace...

		object4.val	= fix_mul(object3.val,object0.val);     				//The primitive V_n...
		object5.val	= fix_mul(object3.val,object1.val);
		object6.val	= fix_mul(object3.val,object2.val);
				
		object7.val = fix_mul(i_object[21].val,
										( fix_mul(A[object][0][1].val,object4.val)	//Delta_magnitude...
										+ fix_mul(A[object][1][1].val,object5.val)
										+ fix_mul(A[object][2][1].val,object6.val)));

		object8.val = i_object[20].val;

		if( i_object[10] > 0 ) 
		  {
		  	object7.val = fix_mul(object7.val,fix_make(2,0));
				object8.val = fix_mul(object8.val,fix_make(2,0));
			}
			
		object4.val = fix_mul(object7.val,object4.val);				//Delta...
		object5.val = fix_mul(object7.val,object5.val);
		object6.val = fix_mul(object7.val,object6.val);

//		CONTROL...
//		==========


//		Head motion for fucking hacking...
//		----------------------------------
Q		x_ease = A[object][0][0] - S[object][0][0];
Q		y_ease = A[object][1][0] - S[object][1][0];
Q		bob_delta = sqrt( x_ease*x_ease + y_ease*y_ease );
Q		bob_speed = sqrt( A[object][0][1]*A[object][0][1] + A[object][1][1]*A[object][1][1] );

		bob_arg.val += fix_div(fix_mul(fix_make(5,0),bob_delta.val),( bob_speed.val + fix_make(1,0)));

    if (bob_arg > two_pi) bob_arg.val = bob_arg.val - two_pi.val;

Q		bob_fac = bob_speed*abs(sin(bob_arg));

#define EDMS_HEAD_BOB_HEIGHT 2 

		if (bob_fac > EDMS_HEAD_BOB_HEIGHT) bob_fac = EDMS_HEAD_BOB_HEIGHT;

		bob_fac.val = fix_make(EDMS_HEAD_BOB_HEIGHT,0) - fix_mul(0x09999,bob_fac.val);	// 0x9999 = .6
    
    if (i_object[10] > 0) bob_fac = 1;
   
    io18.val = fix_mul(i_object[18].val,bob_fac.val);
    io19.val = fix_mul(i_object[19].val,bob_fac.val);
    io17.val = i_object[17].val;


//		Let's not power through the walls anymore...
//		--------------------------------------------
		io18 *= ( V_wall[0].val == 0 );
		io19 *= ( V_wall[1].val == 0 );
		io17 *= ( V_ceiling[2].val == 0 );
    if ( (V_floor[2].val == 0) && (io17.val > 0)) io17.val = 0;

//		Cyberama...
//		-----------		                          
		if ( (object9.val == 0) && (io17.val >= 0) && (i_object[25].val > 0x08000))	// 0x08000 = .5
   		io18.val = io19.val = io17.val = 0;


//      Here are collisions with other objects...
//      =========================================
		shall_we_dance( object, object10, object11, object12 );

  	object10.val = fix_mul(object10.val,i_object[20].val);                                              //More general than it was...
  	object11.val = fix_mul(object11.val,i_object[20].val);
  	object12.val = fix_mul(object12.val,i_object[20].val);

//	Let's not power through the walls anymore...
//	--------------------------------------------
//  	object10 *= ( (V_wall[0].val< 0x028f) && (V_wall[0].val>-0x028f));	// 0x028f = 0.01
		if (! ((V_wall[0].val< 0x028f) && (V_wall[0].val>-0x028f)) )
			object10.val = 0;
//  	object11 *= ( (V_wall[1].val<0x028f) && (V_wall[1].val>-0x028f));
		if ( !((V_wall[1].val<0x028f) && (V_wall[1].val>-0x028f)) )
			object11.val = 0;

//	Back to business...
//	===================
        sincos( A[object][3][0], &sin_alpha, &cos_alpha );                      //Positive for local...
        sincos( A[object][4][0], &sin_beta,  &cos_beta  );
        sincos( A[object][5][0], &sin_gamma, &cos_gamma );


//      The head...
//      ===========
int     edms_ss_bcd_flags_save = ss_edms_bcd_flags;                                                     //Save off info for after head call...
int     edms_ss_param_save = ss_edms_bcd_param;

Q       head_check_x = 0;
Q       head_check_y = 0;

        get_head_of_death( object );

        if ( terrain_info.wx == 0 ) head_check_x = 1;
        if ( terrain_info.wy == 0 ) head_check_y = 1;

//     	io18 *= head_check_x;
//      io19 *= head_check_y;

        edms_ss_head_bcd_flags = ss_edms_bcd_flags;
        if (terrain_info.cz != 0 || head_kappa[2] != 0) i_object[17] = io17 = 0;


//      The Body...
//      ===========
        get_body_of_death( object );        
//        io18 *= ( body_kappa[0] == 0 );
//        io19 *= ( body_kappa[1] == 0 );
				if (body_kappa[0].val != 0)
					io18.val = 0;
				if (body_kappa[1].val != 0)
					io19.val = 0;

        ss_edms_bcd_flags = edms_ss_bcd_flags_save;
        ss_edms_bcd_param = edms_ss_param_save;

//      Do climbing...
//      ==============
        do_climbing( object );

//      Fateful attempt(Jump)...
//      ========================
//        object18 = 800*(io17>0)*(object9>0)*( io17 - A[object][2][1] );					//Jump...
				object18.val = 0;
				if (io17.val > 0 && object9.val > 0)
					object18.val = fix_mul(fix_make(800,0), (io17.val - A[object][2][1].val));
				
//            Jump jets...
//            ------------
        if ( (io17 < 0) && (terrain_info.cz == 0) ) object18 = 800*( - io17 - A[object][2][1] ); 			//Jump jets...
         

//      Climbing overriden with repulsors...
//      ====================================
        if ( ss_edms_bcd_flags & SS_BCD_REPUL_ON ) 
         {

//              Get the speed... 
Q         repulsor_speed = 21;                                                                                        
          if ( (ss_edms_bcd_flags & SS_BCD_REPUL_SPD) == SS_BCD_REPUL_NORM ) repulsor_speed = 7;

//              Assume we're going up, unless...
          if ( (ss_edms_bcd_flags & SS_BCD_REPUL_TYPE) == SS_BCD_REPUL_DOWN) repulsor_speed *= -.5;

//              The parameter should be the desired height....
Q         repul_height;
          repul_height.fix_to(ss_edms_bcd_param);


Q         nearness_or_something = repul_height - A[object][2][0];
          if ( abs(nearness_or_something) <= .333 ) 
            repulsor_speed *= 3 * nearness_or_something;

          io17 = repulsor_speed;
          if ( ( abs(A[object][2][1] - i_object[17]) > .6*i_object[17]) && (terrain_info.cz == 0) && ( repulsor_speed >= 0)) 
          	io17+=50*i_object[17];

        	object18 = i_object[26]*( ( io17 - A[object][2][1] ) + i_object[25] );

          if ( abs(io18) < .01 ) io18 = i_object[18]*( V_wall[0] == 0 );
          if ( abs(io19) < .01 ) io19 = i_object[19]*( V_wall[1] == 0 );

          object9 = 1;
         }
                         
//      Do climbing...
//      ==============
        do_climbing( object );
  
//	Crouch torso bend thang and boogie boogie boogie...
//	===================================================
	if ((i_object[7] > 0.0) || (i_object[0] < i_object[6])) 
		i_object[0] = i_object[6]*(1 - .636*abs(S[object][4][0]));          //Crouch...                                        
  else 
  	i_object[0] = i_object[6];

 
//	Cyberspace for real...
//	======================
Q	drug_addict0 = i_object[23]*A[object][0][1];
Q	drug_addict1 = i_object[23]*A[object][1][1];
        if ( abs(io18) == 0 && i_object[10] > 0 ) drug_addict0 *= .2;                          //Skateware drag reduction...
        if ( abs(io19) == 0 && i_object[10] > 0 ) drug_addict1 *= .2;


//      Pelvis specifics...
//      ===================
        object20 = object8*object0 - object4 + head_kappa[0] - head_delta[0] + io18 + body_kappa[0] - body_delta[0]		//F_mxyz...
	                 + object9*( - drug_addict0 )
	                 + object10;

        object21 = object8*object1 - object5 + head_kappa[1] - head_delta[1] + io19 + body_kappa[1] - body_delta[1]
	                 + object9*( - drug_addict1 )
	                 + object11;

        object22 = object8*object2 - (object18==0)*object6 + head_kappa[2] - head_delta[2] + object18 + body_kappa[2] - body_delta[2]
									 + object9*(-i_object[23]*A[object][2][1])
	                 + object12;


//	Damage control...
//	=================
Q	dam0 = object8*object0 - object4 + head_kappa[0] - head_delta[0];
Q	dam1 = object8*object1 - object5 + head_kappa[1] - head_delta[1];
Q	dam2 = object8*object2 - (object18==0)*object6 + head_kappa[2] - head_delta[2];


		i_object[14] = abs(dam0) + abs(dam1) + abs(dam2) - 2*i_object[26]*i_object[25];		//Damage??
		if ( i_object[14] > 0 ) 
			i_object[14] *= i_object[24]*( io17 < .5 );
    else 
    	i_object[14] = 0;        


//	Is there a projectile hit?
//	==========================
	if ( i_object[35] > 0 ) 
	 {

//		Let's not power through the walls anymore...
//		--------------------------------------------
		i_object[32] *= ( (V_wall[0]<0.01) && (V_wall[0]>-0.01) );
		i_object[33] *= ( (V_wall[1]<0.01) && (V_wall[1]>-0.01) );
		i_object[34] *= ( (V_ceiling[2]<0.01) && (V_ceiling[2]>-0.01) );

		object20 += i_object[32];
		object21 += i_object[33];
    object22 += i_object[34];

		i_object[35] = 0;
		i_object[32] = 0;
		i_object[33] = 0;
		i_object[34] = 0;
	 }


    Fmxm = object20*cos_alpha + object21*sin_alpha;                         //Locals...
    Fmym =-object20*sin_alpha + object21*cos_alpha;

    lp_z =-.1*i_object[0]*cos_beta*cos_gamma;

Q   Head_tau_beta =  -.1*i_object[0]*sin_beta*(  head_kappa[2] - head_delta[2] ),
    Head_tau_gamma = -.1*i_object[0]*sin_gamma*( head_kappa[2] - head_delta[2] );

    if (((V_wall[1] != 0) && (head_check_y == 0)) || ((V_wall[0] != 0) && (head_check_x == 0))) 
    	i_object[15] = 0;

		T_beta  = -( lp_z*Fmxm )  + i_object[7]       + Head_tau_beta;               			//Actual torques...
    T_gamma = -( - Fmym*lp_z) + .04*i_object[16]* + Head_tau_gamma;


//	Kickbacks...
//	============
	if( abs(i_object[8]) > 0 ) 
	 {
		T_beta  -= cos_alpha*i_object[8] + sin_alpha*i_object[9];
		T_gamma  =-sin_alpha*i_object[8] + cos_alpha*i_object[9];
		
		object20 -= i_object[8];					//For zero g...
		object21 -= i_object[9];
		
		i_object[8] = i_object[9] = 0;
	 }

	object17 = i_object[28]*( 1 + 1.2*(i_object[16] == 0) );		// 3 is 2

//	Angular play (citadel) ...
//	==========================
	if( S[object][3][0] > two_pi ) S[object][3][0] -= two_pi;
	if( S[object][3][0] <-two_pi ) S[object][3][0] += two_pi;


//	Try the equations of motion here for grins...
//	=============================================
  S[object][0][2] = i_object[24]*( object20 );
  S[object][1][2] = i_object[24]*( object21 );
  S[object][2][2] = i_object[24]*( object22 ) - i_object[25];
  S[object][3][2] = i_object[27]*( i_object[16]	- object17*A[object][3][1]);
  S[object][4][2] = i_object[27]*( T_beta
                                  - 1.5*i_object[1]*A[object][4][0]/**(1-.5*(i_object[10]==1))*/
                                  - .8*i_object[2]*A[object][4][1] /**(1-.5*(i_object[10]==1))*/   );

  S[object][5][2] = i_object[27]*( T_gamma
                                  - i_object[1]*A[object][5][0]	  /**(1-.5*(i_object[10]==1))*/
                                  - .8*i_object[2]*A[object][5][1] /**(1-.5*(i_object[10]==1))*/	  
                                  + i_object[15] );


//	That's all, folks...
//	====================
}




//      Here we'll get the head information we all want so badly...
//      ===========================================================
void    get_head_of_death( int object ) 
 {
Q		*i_object = I[object];
Q       vec0,
        vec1,
        vec2,
        test,
        mul,
        vv0, vv1, vv2,
        dmag,
        kmag;


Q       offset_x = i_object[0]*sin( A[object][4][0] ),
        offset_y =-1.5*i_object[0]*sin( A[object][5][0] ),
				offset_z = i_object[0]*cos( A[object][4][0] )*cos( A[object][5][0] );

Q       sin_alpha = 0,
        cos_alpha = 0;

Q       final_x = 0,
        final_y = 0;        
        
        
        sincos( -A[object][3][0], &sin_alpha, &cos_alpha );
        final_x = cos_alpha*offset_x + sin_alpha*offset_y;
        final_y =-sin_alpha*offset_x + cos_alpha*offset_y;

				indoor_terrain( A[object][0][0] + final_x,
                        A[object][1][0] + final_y,
                        A[object][2][0] + offset_z, 
												.75*i_object[22],
												-1 /*on2ph[object]*/ );


Q		mag = i_object[18]*i_object[18] + i_object[19]*i_object[19];
                if ( mag < .1 && abs( V_floor[0] ) < .05*i_object[22] && abs( V_floor[1] ) < .05*i_object[22] ) {
                        terrain_info.fx = terrain_info.fy = 0;
                        }


 		vec0.fix_to( terrain_info.fx + terrain_info.cx + terrain_info.wx );
		vec1.fix_to( terrain_info.fy + terrain_info.cy + terrain_info.wy );
		vec2.fix_to( terrain_info.fz + terrain_info.cz + terrain_info.wz );

		test = sqrt(vec0*vec0
					      + vec1*vec1
					      + vec2*vec2 );


		if (test > EDMS_DIV_ZERO_TOLERANCE ) 
			mul = fix_one / test;		//To get primitive...
		else
			test = mul = 0;

		vv0	= mul*vec0;				//The primitive V_n...
		vv1	= mul*vec1;
		vv2	= mul*vec2;

		dmag = i_object[21]*(A[object][0][1]*vv0  			//Delta_magnitude...
						        		+ A[object][1][1]*vv1
							        	+ A[object][2][1]*vv2);

		head_delta[0] = dmag*vv0;                				//Delta...
		head_delta[1] = dmag*vv1;
		head_delta[2] = dmag*vv2;


//		if (test < .5*i_object[22]) kmag = i_object[20];			//Omega_magnitude...
//              else kmag = i_object[20]/test;
    kmag = i_object[20];

    head_kappa[0] =  kmag*vec0;
    head_kappa[1] =  kmag*vec1;
    head_kappa[2] =  kmag*vec2;
}


void get_body_of_death( int object )
{
Q		*i_object = I[object];

Q       vec0,
        vec1,
        vec2,
        test,
        mul,
        vv0, vv1, vv2,
        dmag,
        kmag;

Q       half_height = .5*i_object[0]; 

Q       offset_x = half_height*sin( A[object][4][0] ),
        offset_y =-1.5*half_height*sin( A[object][5][0] ),
				offset_z = half_height*cos( A[object][4][0] )*cos( A[object][5][0] );

Q       sin_alpha = 0,
        cos_alpha = 0;

Q       final_x = 0,
        final_y = 0;        
        
        
        sincos( -A[object][3][0], &sin_alpha, &cos_alpha );
        final_x = cos_alpha*offset_x + sin_alpha*offset_y;
        final_y =-sin_alpha*offset_x + cos_alpha*offset_y;

				indoor_terrain( A[object][0][0] + final_x,
                        A[object][1][0] + final_y,
                        A[object][2][0] + offset_z, 
												.33*i_object[0],
												-1 /*on2ph[object]*/ );

//      Zero result!
//      ============
        body_kappa[0] = body_kappa[1] = body_kappa[2] = 0;
        body_delta[0] = body_delta[1] = body_delta[2] = 0;

//      Do ANYTHING?
//      ------------
		Q abtotal =  abs(terrain_info.fx) +
		             abs(terrain_info.fy) +
		             abs(terrain_info.fz);
		abtotal +=   abs(terrain_info.wx) +
		             abs(terrain_info.wy) +
		             abs(terrain_info.wz);
		abtotal +=   abs(terrain_info.cx) +
		             abs(terrain_info.cy) +
		             abs(terrain_info.cz);
		if (abtotal != 0 )
		 {
Q			mag = i_object[18]*i_object[18] + i_object[19]*i_object[19];
			if ( mag < .1 && abs( V_floor[0] ) < .05*i_object[22] && abs( V_floor[1] ) < .05*i_object[22] )
				terrain_info.fx = terrain_info.fy = 0;

			vec0.fix_to( terrain_info.fx + terrain_info.cx + terrain_info.wx );
			vec1.fix_to( terrain_info.fy + terrain_info.cy + terrain_info.wy );
			vec2.fix_to( terrain_info.fz + terrain_info.cz + terrain_info.wz );

			test = sqrt(vec0*vec0
						+ vec1*vec1
			      		+ vec2*vec2 );

			if (test > EDMS_DIV_ZERO_TOLERANCE )
				mul = fix_one / test;		//To get primitive...
			else
				test = mul = 0;
	
			vv0	= mul*vec0;				//The primitive V_n...
			vv1	= mul*vec1;
			vv2	= mul*vec2;

			vec2 = vv2 = 0;

			dmag = i_object[21]*(A[object][0][1]*vv0  			//Delta_magnitude...
					        				+ A[object][1][1]*vv1
						        			+ A[object][2][1]*vv2 );

			body_delta[0] = dmag*vv0;                				//Delta...
			body_delta[1] = dmag*vv1;
			body_delta[2] = dmag*vv2;

			kmag = i_object[20];

            body_kappa[0] =  kmag*vec0;
            body_kappa[1] =  kmag*vec1;
            body_kappa[2] =  kmag*vec2;

		}       //Do NOTHING...
 }                                                                 


//      Climbing stuff also removed for speed of compilations...
//      ========================================================
void    do_climbing( int object )  
 {
Q		*i_object = I[object];

//      Hellishness...                                                    
//      ==============
  if ( (i_object[17] > 0) && ((ss_edms_bcd_flags & SS_BCD_MISC_CLIMB) || (edms_ss_head_bcd_flags & SS_BCD_MISC_CLIMB)) ) 
   {
Q   ass = sqrt( (.05*i_object[18])*i_object[18] + (.05*i_object[19])*i_object[19] );
Q   ratio = i_object[18]*object0 + i_object[19]*object1;
    
    if ( ratio > 0 ) ass = 0;

    EDMS_pelvis_is_climbing = 1;

		if (checker>0) 
		 {
      io17 = .02*ass;// + 100*( .2*i_object[22] - V_[floor][2] );
      if ( (terrain_info.cz != 0) ) 
      	io17 = 0;
    	io18 = -.4*i_object[22]*object0*object8/checker + .5*i_object[18];
      io19 = -.4*i_object[22]*object1*object8/checker + .5*i_object[19];
    	i_object[16] *= .5;

//                    Set the mojo...
//                    ===============
	    object18 = 800*(io17>0)*( io17 - A[object][2][1] );
		}
   }



                                                                                                   
//      AutoClimbing(tm) is for wussies (is superseeded by climbing)...
//      ===============================================================
  else if ((ss_edms_bcd_flags & SS_BCD_MISC_STAIR) /*&& (i_object[17] == 0) (io17==0) */)
   {
		if ( ( checker > 0 ) && (abs(i_object[18]) + abs(i_object[19]) > .01 ) ) 
		 {
Q     ratio = (i_object[18]+A[object][0][1])*object0 + (i_object[19]+A[object][1][1])*object1;
                        
      if ( ratio <= 0 ) 
       {
        io17 = .5;
			                                   
				io18 = -.3*i_object[22]*object0*object8/checker + .2*i_object[18];
			  io19 = -.3*i_object[22]*object1*object8/checker + .2*i_object[19];


//                              Set the mojo...
//                              ===============
        object18 = 800*(io17>0)*( io17 - A[object][2][1] );
			}
		else 
			{	
				io18 = i_object[18];
		    io19 = i_object[19]; 
		  }
		 }
  }
}       //End of climbing nonsense...




                         
//	We might for now want to set some external forces on the pelvis...
//	==================================================================
void	pelvis_set_control( int pelvis, Q forward, Q turn, Q sidestep, Q lean, Q jump, int crouch ) 
 {
const Q         pi_by_two = 1.5707;                                             //Yea, flixpoint...

	sincos( S[pelvis][3][0], &object0, &object1 );



//	Get rid of it all...
//	--------------------
	I[pelvis][15] = I[pelvis][16] = I[pelvis][17] = I[pelvis][18] = I[pelvis][19] = I[pelvis][7] = 0;


//		Here's the thrust of the situation...
//		-------------------------------------
		I[pelvis][18] = forward*object1*I[pelvis][26];
		I[pelvis][19] = forward*object0*I[pelvis][26];


//		And the sidestep is off by pi/two...
//      	------------------------------------
	  sincos( (S[pelvis][3][0] - pi_by_two), &object0, &object1 );
		I[pelvis][18] += sidestep*object1*I[pelvis][26];
    I[pelvis][19] += sidestep*object0*I[pelvis][26];

//		And the turn of the...
//		----------------------
		I[pelvis][16] = turn*I[pelvis][29];

//		Jump jets of joy...
//		-------------------
		if ( jump > 0 ) I[pelvis][17] = .003*I[pelvis][26]*jump;
		if ( jump < 0 ) I[pelvis][17] = .0006*I[pelvis][26]*jump;

//		And finally leaning about...
//		----------------------------
	  I[pelvis][15] = .04*lean*I[pelvis][1];				//Exactly the angle!
                                                             
//		Crouching (overpowers jumping )...
//		----------------------------------
		if ( crouch > 0 ) I[pelvis][7] = .20*crouch*I[pelvis][1];

//	Wake up...
//	==========
	no_no_not_me[pelvis] = ( abs(I[pelvis][15]) + abs(I[pelvis][16]) + abs(I[pelvis][17])
			       + abs(I[pelvis][18]) + abs(I[pelvis][19]) > 0 ) || (no_no_not_me[pelvis]==1);

}


//	Sets up everything needed to manufacture a pelvis with initial state vector
//	init_state[][] and EDMS motion parameters params[] into soliton. Returns the 
//	object number, or else a negative error code (see Soliton.CPP for error handling and codes).
//	============================================================================================
int make_pelvis( Q init_state[6][3], Q params[10] ) 
 {
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
		I[object_number][35] = 0;
		I[object_number][36] = I[object_number][24];			//Shrugoff "mass"...
		I[object_number][37] = -1;
		I[object_number][38] = 0;					//No kill I...


//		Zero the control initially...
//		=============================
                I[object_number][7]  =
		I[object_number][8]  =
		I[object_number][9]  =
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
//	7	 	 |   1/moi
//	8        |   rotational drag
//	9	 	 |   moi
//	==========================================
//	So there.



//      For mark...
//      -----------
//extern "C" {

void    EDMS_lean_o_meter( physics_handle ph, fix& lean, fix& crouch ) {

        
        lean = crouch = 0;

//      Are you for real?
//      -----------------
        if (ph > -1) {

int             on = ph2on[ph];

//              Are you a pelvis...
//              -------------------
                if ( I[on][30] == PELVIS ) {

                        lean = S[on][5][0].to_fix();
                        crouch = I[on][0].to_fix() - 3*V_floor[2].to_fix();

                }//Pelvis check...

        }//For real...

}

#pragma require_prototypes on


//}
