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
//	Here is the EDMS Bipedally Locomated Object Model(tm), or BiLocoMo,
//	for short.  The trick here is to combine as many dynamical degrees
//	of freedom together as possible, so as to make possible real
//	time walking, running, and balancing under arbitrarily adverse
//	conditions...
//	=============


//	Jon Blackley, July 25, 1993
//	===========================


#include <iostream.h>
#include <conio.h>

#ifdef EDMS_SHIPPABLE
#include <mout.h>
#endif

#include <EDMS_int.h>
#include <EDMS_vt.h>
#include <physhand.h>


//	State...
//	========
extern EDMS_Argblock_Pointer	A;
extern Q	S[MAX_OBJ][6][4],
		I[MAX_OBJ][DOF_MAX];


//	Functions...
//	============
extern void	( *idof_functions[MAX_OBJ] )( int ),
		( *equation_of_motion[MAX_OBJ][6] )( int ),
		null_function( int dummy );

extern Q	*utility_pointer[MAX_OBJ];


//	Control Storage...
//	==================
Q	control_x_t = 0,
	control_y_t = 0,
	control_gamma_t = 0;


//	BiPed tools and dies (in order of appearance)...
//	================================================


//	Vector goodies...
//	=================
static Q	H_r[3] = {0,0,0},		H_l[3] = {0,0,0},
		N_r[3] = {0,0,0},		N_l[3] = {0,0,0},
		T_r[3] = {0,0,0},		T_l[3] = {0,0,0},
		R_w_r[3] = {0,0,0},		R_w_l[3] = {0,0,0},
		delta_r[3] = {0,0,0},		delta_l[3] = {0,0,0},
		dR_r[3] = {0,0,0},
		foot_r[3] = {0,0,0},		foot_l[3] = {0,0,0},
		dF_r[3] = {0,0,0},		dF_l[3] = {0,0,0},
		foot_hat_r[3] = {0,0,0},	foot_hat_l[3] = {0,0,0},
		F_right[3] = {0,0,0},		F_left[3] = {0,0,0},
		F_total[3] = {0,0,0},		Vl[3] = {0,0,0};
			 
			
       	
//	Scalar goodies...
//	=================
static Q	sin_alpha = 0,		cos_alpha = 0,
		sin_beta = 0,		cos_beta = 0,
		sin_gamma = 0,		cos_gamma = 0,
		mag_delta_r = 0,	mag_delta_l = 0,
		mag_dR = 0,		delta_chi = 0,
		radius_r = 0,		radius_l = 0,
		rconst_r = 0,		rconst_l = 0,
		sin_chi_r = 0,		sin_chi_l = 0,
		cos_chi_r = 0,		cos_chi_l = 0,
		fc_r = 0,		fc_l = 0,
		terrain_r_dx = 0,	terrain_l_dx = 0,
		terrain_r_dy = 0,	terrain_l_dy = 0,
		terrain_r = 0,		terrain_l = 0,
		mag_foot_r = 0,		mag_foot_l = 0,
		normal_r = 0,		normal_l = 0,
		damp_r = 0,		damp_l = 0,
		right_foot_mag = 0,	left_foot_mag = 0,
		T_right_beta = 0,	T_left_beta = 0,
		T_right_gamma = 0,	T_left_gamma = 0,
		xi_alpha = 0,		xi_beta = 0,
		T_alpha_t = 0,		T_beta_t = 0,
		T_alpha = 0,		T_beta = 0,
		T_alpha_temp = 0,	T_gamma_temp = 0,
		T_gamma = 0,		T_beta_temp = 0,
		tan_alpha = 0,		sec_alpha = 0,
		tau_r = 0,		tau_l = 0,
		T_control_alpha = 0,	T_control_beta = 0,
		alpha_dot = 0,		beta_dot = 0,
		gamma_dot = 0;
			

//	We gotta save the updated positions, but AFTER calclation...
//	============================================================
Q	save_r_w_r[MAX_OBJ][3],
	save_r_w_l[MAX_OBJ][3],
	save_chi[MAX_OBJ];

		
//	Constants...
//	============
static Q	pi = 3.1415,
		two_pi = 6.2831,
		point_five = .5,
		point_two_five = .25,
		point_nine = 0.9,
		two = 2.,
		four = 4.,
		point_oo_one = .001,
		point_oo_five = .005;
       	
//	Limiters...
//	===========
static Q	radius_max = .9;


//	Counters...
//	===========
int		not_yet1[MAX_OBJ],
		not_yet2[MAX_OBJ],
		not_yet3[MAX_OBJ];


//	For terrain differentiation...
//	==============================
Q	ground_info = 0;


//	Here goes...	
//	============

	
	
//	Have some internal degrees of freedom...
//	========================================
void biped_idof( int object ) {



//	This, like me, can be easily replaced...
//	----------------------------------------
terrain_ff      TFF;


//		Tools...
//		========
void		globalize( Q &X, Q &Y, Q &Z ),
		localize( Q &X, Q &Y, Q &Z ),
		place_right_knee( int ),
		place_left_knee( int );


//	Find feet...
//	============
	sincos( -A[object][3][0], &sin_alpha, &cos_alpha );
	sincos( -A[object][4][0], &sin_beta, &cos_beta );
	sincos( -A[object][5][0], &sin_gamma, &cos_gamma );

//	Get local...
//	============
	alpha_dot = -( A[object][3][1]*cos_beta + A[object][5][1]*sin_beta*cos_alpha );
	beta_dot = -( A[object][4][1] - A[object][5][1]*sin_alpha );
	gamma_dot = -( -A[object][3][1]*sin_beta + A[object][5][1]*cos_beta*cos_alpha );

//	mout << "Dots: " << alpha_dot << " : " << beta_dot << " : " << gamma_dot << "\n";

//	Get local velocity information...
//	---------------------------------
	Vl[0] = A[object][0][1];	
	Vl[1] = A[object][1][1];	
	Vl[2] = A[object][2][1];
	localize( Vl[0], Vl[1], Vl[2] );
	
//	Find geometry...
//	----------------
	H_r[2] = H_l[2] = 0;
	H_r[0] = N_r[0] = I[object][23];				//hips and legs...	
	H_l[0] = N_l[0] =-I[object][23];	

	N_r[2] = - I[object][0];// + I[object][10]*I[object][1];
	N_l[2] = - I[object][0];// + I[object][10]*I[object][2];


//	Move reference...
//	-----------------
	R_w_r[0] = N_r[0];	R_w_l[0] = N_l[0];			//In L coords...
	R_w_r[1] = N_r[1];	R_w_l[1] = N_l[1];
	R_w_r[2] = N_r[2];	R_w_l[2] = N_l[2];


//	Displacement space...
//	---------------------
	globalize( R_w_r[0], R_w_r[1], R_w_r[2] );
	globalize( R_w_l[0], R_w_l[1], R_w_l[2] );

	R_w_r[0] += A[object][0][0];	R_w_l[0] += A[object][0][0];	//STATE!
	R_w_r[1] += A[object][1][0];	R_w_l[1] += A[object][1][0];
	R_w_r[2] += A[object][2][0];	R_w_l[2] += A[object][2][0];


//	Here's the local displacement...
//	--------------------------------
	delta_r[0] = R_w_r[0] - I[object][3];	delta_l[0] = R_w_l[0] - I[object][6];
	delta_r[1] = R_w_r[1] - I[object][4];	delta_l[1] = R_w_l[1] - I[object][7];
	delta_r[2] = R_w_r[2] - I[object][5];	delta_l[2] = R_w_l[2] - I[object][8];

	localize( delta_r[0], delta_r[1], delta_r[2] );
	localize( delta_l[0], delta_l[1], delta_l[2] );


//	Update history, as it were...
//	-----------------------------
//	Ladies and gentlemen, this is as ugly as it gets...

		if ( not_yet1[object]++ == 0 ) {				//Multi-pass integration, ya know...
		save_r_w_r[object][0] = R_w_r[0];	save_r_w_l[object][0] = R_w_l[0];
		save_r_w_r[object][1] = R_w_r[1];	save_r_w_l[object][1] = R_w_l[1];
		save_r_w_r[object][2] = R_w_r[2];	save_r_w_l[object][2] = R_w_l[2];
		}

		else {
		not_yet1[object] = 0;
		I[object][3] = save_r_w_r[object][0];	I[object][6] = save_r_w_l[object][0];
		I[object][4] = save_r_w_r[object][1];	I[object][7] = save_r_w_l[object][1];
		I[object][5] = save_r_w_r[object][2];	I[object][8] = save_r_w_l[object][2];
		}

//	Now we need the magnitudes of the deltas...
//	-------------------------------------------
	mag_delta_r = sqrt( delta_r[0]*delta_r[0]
			  + delta_r[1]*delta_r[1] 			//Walking only!
			  + delta_r[2]*delta_r[2] );

	mag_delta_l = sqrt( delta_l[0]*delta_l[0]
			  + delta_l[1]*delta_l[1] 
			  + delta_l[2]*delta_l[2] );

//	mout << mag_delta_r << " : " << mag_delta_l << "\n";

			  
//	Now get derivatives (magnitude r ONLY here)...
//	----------------------------------------------
	dR_r[0] = A[object][0][1];					//STATE!!!
	dR_r[1] = A[object][1][1];
	dR_r[2] = A[object][2][1];
	localize( dR_r[0], dR_r[1], dR_r[2] );

	dR_r[0] += N_r[2]*alpha_dot;
	dR_r[1] +=-N_r[2]*beta_dot + N_r[0]*gamma_dot;
	dR_r[2] +=-N_r[0]*alpha_dot; 

	mag_dR = sqrt( dR_r[0]*dR_r[0]					//We only care about walking...
		     + dR_r[1]*dR_r[1] );
		     //+ dR_r[2]*dR_r[2] );


//	Now set things up...
//	--------------------
	radius_r = I[object][11]*mag_dR;			       		//Radius...
	radius_r = (radius_r>radius_max*I[object][0])*radius_max*I[object][0]	//Maximize...
		 + (radius_r<=radius_max*I[object][0])*radius_r;

//	delta_chi = four*asin( point_five*mag_delta_r / (radius_r + point_oo_one) );		//Angle...

	if ( radius_r == 0 ) delta_chi = 0;
	else delta_chi = 2*mag_delta_r / radius_r;						//Angle...

//	Need these for the actual placement...
//	--------------------------------------
	if ( abs(delta_chi) < point_oo_five ) radius_l = 0;
	else radius_l = 2*mag_delta_l / delta_chi;
//	radius_l = radius_r;

//	Now get the actual guesses...
//	-----------------------------
	I[object][9] += delta_chi;

//	Check and go!
//	-------------
	if ( I[object][9] > two_pi ) I[object][9] -= two_pi;
	sincos( I[object][9], &sin_chi_r, &cos_chi_r );
	sin_chi_l = -sin_chi_r;
	cos_chi_l = -cos_chi_r;

	rconst_r = -radius_r / ( mag_delta_r + .01 );			//One must love dividing...
	rconst_l = -radius_l / ( mag_delta_l + .01 );


//	Clean house: Thanks, integrator...
//	----------------------------------
	if ( not_yet2[object]++ == 0 ) {
		save_chi[object] = I[object][9];
		I[object][9] -= delta_chi;
		}

		else {
		not_yet2[object] = 0;
		I[object][9] = save_chi[object];
		}


//	Here they are...
//	----------------
	foot_r[0] = N_r[0] + rconst_r*delta_r[0]*cos_chi_r;
	foot_r[1] = N_r[1] + rconst_r*delta_r[1]*cos_chi_r;
 	foot_r[2] = N_r[2] + I[object][10]*radius_r*( 1.5 + sin_chi_r ); 		//Step height...

	foot_l[0] = N_l[0] + rconst_l*delta_l[0]*cos_chi_l;
	foot_l[1] = N_l[1] + rconst_l*delta_l[1]*cos_chi_l;
 	foot_l[2] = N_l[2] + I[object][10]*radius_l*( 1.5 + sin_chi_l ); 		//Step height...

//	mout << "N: " << N_r[2] << " : " << N_r[2] << "\n";
//	mout << "R: " << radius_r << " : " << radius_l << "\n";
//	mout << "f: " << foot_r[2] << " : " << foot_l[2] << "\n";

	globalize( foot_r[0], foot_r[1], foot_r[2] );		//Here goes...
	globalize( foot_l[0], foot_l[1], foot_l[2] );

//	mout << "f: " << foot_r[2] << " : " << foot_l[2] << "\n";

//	Finally, check the terrain...
//	=============================
	TFF.caller = on2ph[object];
	TFF.terrain_information = 0;
        ff_terrain( foot_r[0] + A[object][0][0], foot_r[1] + A[object][1][0], foot_r[2] + A[object][2][0], &TFF );
        terrain_r.fix_to( TFF.g_height );       terrain_r -= A[object][2][0];
        terrain_r_dx.fix_to( TFF.g_dx );
        terrain_r_dy.fix_to( TFF.g_dy );


//	Simple, right foot check for now...
//	===================================
	ground_info.fix_to( TFF.terrain_information );


        ff_terrain( foot_l[0] + A[object][0][0], foot_l[1] + A[object][1][0], foot_l[2] + A[object][2][0], &TFF );
        terrain_l.fix_to( TFF.g_height );       terrain_l -= A[object][2][0];
        terrain_l_dx.fix_to( TFF.g_dx );
        terrain_l_dy.fix_to( TFF.g_dy );

        fc_r = fc_l = 0;                						//How are your Dr. Scholls...

	if ( foot_r[2] <= terrain_r ) { foot_r[2] = terrain_r;	fc_r = 1; }
	if ( foot_l[2] <= terrain_l ) { foot_l[2] = terrain_l;	fc_l = 1; }


//	Pull a derivative before retransformation...
//	--------------------------------------------	
	dF_r[0] = -A[object][0][1] - XPA_0( foot_r, object );
	dF_r[1] = -A[object][1][1] - XPA_1( foot_r, object );
	dF_r[2] = -A[object][2][1] - XPA_2( foot_r, object );
	
	dF_l[0] = -A[object][0][1] - XPA_0( foot_l, object );
	dF_l[1] = -A[object][1][1] - XPA_1( foot_l, object );
	dF_l[2] = -A[object][2][1] - XPA_2( foot_l, object );
	

//	Need the new basis now...
//	-------------------------
	localize( foot_r[0], foot_r[1], foot_r[2] );
	localize( foot_l[0], foot_l[1], foot_l[2] );
	localize( dF_r[0], dF_r[1], dF_r[2] );
	localize( dF_l[0], dF_l[1], dF_l[2] );


//	Yes! Yes! There it is...
//	========================
	foot_r[0] = H_r[0] - foot_r[0];		foot_l[0] = H_l[0] - foot_l[0];
	foot_r[1] = H_r[1] - foot_r[1];		foot_l[1] = H_l[1] - foot_l[1];
	foot_r[2] = H_r[2] - foot_r[2];		foot_l[2] = H_l[2] - foot_l[2];
	
	mag_foot_r = sqrt( foot_r[0]*foot_r[0]
			 + foot_r[1]*foot_r[1] 
			 + foot_r[2]*foot_r[2] );

	mag_foot_l = sqrt( foot_l[0]*foot_l[0]
			 + foot_l[1]*foot_l[1] 
			 + foot_l[2]*foot_l[2] );


//	Let's not blow it all...
//	------------------------
	normal_r = 1. / ( mag_foot_r + point_oo_one );
	normal_l = 1. / ( mag_foot_l + point_oo_one );

	foot_hat_r[0] = normal_r*foot_r[0];	foot_hat_l[0] = normal_l*foot_l[0];
	foot_hat_r[1] = normal_r*foot_r[1];	foot_hat_l[1] = normal_l*foot_l[1];
	foot_hat_r[2] = normal_r*foot_r[2];	foot_hat_l[2] = normal_l*foot_l[2];


//	Find some damping...
//	--------------------
	damp_r = dF_r[0]*foot_hat_r[0]
	       + dF_r[1]*foot_hat_r[1]
	       + dF_r[2]*foot_hat_r[2];

	damp_l = dF_l[0]*foot_hat_l[0]
	       + dF_l[1]*foot_hat_l[1]
	       + dF_l[2]*foot_hat_l[2];

	if (damp_r > 0) damp_r *= .2;
	if (damp_l > 0) damp_l *= .2;


//	Copy for rendering...
//	=====================
	if( not_yet3[object]++ > 0 ) {

//	Elbows...
//	---------
	utility_pointer[object][36] = sin_chi_r;
	utility_pointer[object][37] = cos_chi_r;

	utility_pointer[object][39] = sin_chi_l;
	utility_pointer[object][40] = cos_chi_l;

//	Wrists...
//	---------
	utility_pointer[object][42] = Vl[1];


//	Heels...
//	-------
	utility_pointer[object][6]  = -foot_r[0] + H_r[0];
	utility_pointer[object][7]  = -foot_r[1] + H_r[1];
	utility_pointer[object][8]  = -foot_r[2] + H_r[2];

	utility_pointer[object][9]  = -foot_l[0] + H_l[0];
	utility_pointer[object][10] = -foot_l[1] + H_l[1];
	utility_pointer[object][11] = -foot_l[2] + H_l[2];


//	And knees...
//	------------
	place_right_knee( object );
	place_left_knee( object );


//	Begin, begin begin agin...
//	--------------------------	
	not_yet3[object] = 0;
	}



//	Now get the magnitudes of the moments...
//	----------------------------------------
	right_foot_mag = fc_r*( I[object][21]*( (I[object][0] - mag_foot_r) )		//Signs!
		       + I[object][22]*damp_r );

        left_foot_mag  = fc_l*( I[object][21]*( (I[object][0] - mag_foot_l) )
		       + I[object][22]*damp_l );

//	Push, pull, what's the difference...
//	------------------------------------
//	if(right_foot_mag<0) right_foot_mag = 0;
//	if(left_foot_mag<0) left_foot_mag = 0;


//	Help me to balance more efficiently...
//	--------------------------------------
//	right_foot_mag -= A[object][4][0]*(A[object][4][0]<0);
//	left_foot_mag  -= A[object][4][0]*(A[object][4][0]>0);


//	Have some moments...
//	====================
	T_right_beta  =-I[object][23]*right_foot_mag*foot_hat_r[2];
	T_right_gamma =0;//-I[object][23]*right_foot_mag*foot_hat_r[1];

	T_left_beta   = I[object][23]*left_foot_mag*foot_hat_l[2];
	T_left_gamma  =0;// I[object][23]*left_foot_mag*foot_hat_l[1];

//	You need to have some balance in your life...
//	---------------------------------------------
        xi_alpha = -I[object][28]*A[object][3][0];// + .02*alpha_dot;
	xi_beta  = -I[object][28]*A[object][4][0] + .05*Vl[1];// + .03*beta_dot;		//Include velocity anticipation!!!!


//	Copy for rendering...
//	---------------------
	utility_pointer[object][27] = xi_alpha;
	utility_pointer[object][28] = xi_beta;

//	T_alpha_t = -I[object][26]*I[object][27]*I[object][29]
//		  * sin( A[object][3][0] + xi_alpha );
//	T_beta_t  = -I[object][26]*I[object][27]*I[object][29]				//What small displacement limits??
//		  * sin( A[object][4][0] + xi_beta );

	T_alpha_t = -I[object][27]*(-4*A[object][3][0] - .5*A[object][3][1]);
	T_beta_t  = -I[object][27]*(-2*(A[object][4][0] - .08*Vl[1]) - .5*A[object][4][1]);


//	Throw them into world coordinates...
//	====================================
	F_right[0] = right_foot_mag*foot_hat_r[0];	F_left[0]  = left_foot_mag*foot_hat_l[0];
	F_right[1] = right_foot_mag*foot_hat_r[1];	F_left[1]  = left_foot_mag*foot_hat_l[1];
	F_right[2] = right_foot_mag*foot_hat_r[2];	F_left[2]  = left_foot_mag*foot_hat_l[2];


//	Stops on a dime...
//	------------------
	control_x_t     = (- I[object][18] - .2*Vl[0]*50.)*( 1 - ground_info );
	control_y_t     = (  I[object][19] - .2*Vl[1]*50.)*( 1 - ground_info );
	control_gamma_t = (  I[object][20] - .2*gamma_dot*5.)*( 1 - ground_info );

	if ( abs(I[object][18]) < 0.01 ) control_x_t     -= Vl[0]*50*( 1 - ground_info );
	if ( abs(I[object][19]) < 0.01 ) control_y_t     -= Vl[1]*50*( 1 - ground_info );
//	if ( abs(I[object][20]) < 0.01 ) control_gamma_t -= gamma_dot*5;

//	control_x_t *= .01*(right_foot_mag+left_foot_mag);
//	control_y_t *= .01*(right_foot_mag+left_foot_mag);
//	control_gamma_t *= .01*(right_foot_mag+left_foot_mag);


//	Add...
//	------
	F_total[0] = F_right[0] + F_left[0] + control_x_t;
	F_total[1] = F_right[1] + F_left[1] + control_y_t;
	F_total[2] = F_right[2] + F_left[2];


//	Here are the unfortunate results of friction...
//	-----------------------------------------------		  
	T_control_beta = .002*( right_foot_mag*mag_foot_r*control_x_t
		            + left_foot_mag*mag_foot_l*control_x_t );	  
		  
	T_control_alpha = 0;//-.005*( right_foot_mag*mag_foot_r*control_y
		           // + left_foot_mag*mag_foot_l*control_y_t );	  
		  
//	Here's the switch...
//	--------------------
	T_alpha_temp = I[object][13]*(T_alpha_t + T_control_alpha + T_right_beta + T_left_beta );
	T_beta_temp  = I[object][14]*(T_beta_t + T_control_beta );
	T_gamma_temp = I[object][15]*(T_right_gamma + T_left_gamma + control_gamma_t);


//	Convert...
//	----------
	globalize( F_total[0], F_total[1], F_total[2] );

	tan_alpha = sin_alpha / cos_alpha;	sec_alpha = 1. / cos_alpha;	//oh, so slow...

	T_alpha = T_alpha_temp*cos_beta - T_gamma_temp*sin_beta;
	T_beta = T_beta_temp + T_alpha_temp*sin_beta*tan_alpha + T_gamma_temp*cos_beta*tan_alpha;
	T_gamma = T_alpha_temp*sin_beta*sec_alpha + T_gamma_temp*cos_beta*sec_alpha;

	T_alpha *= -1;
	T_beta  *= -1;
	T_gamma *= -1;	   

	T_alpha *= 0;
	T_beta  *= 0;
//	T_gamma *= 0;	   



//	Actual bodily forces, collisions, and cetera...
//	===============================================
        ff_terrain( A[object][0][0], A[object][1][0], A[object][2][0], &TFF );

Q	object0;
Q	object1;

const Q	fix_one = 1;

	object0.fix_to( TFF.w_x );
	object1.fix_to( TFF.w_y );

Q	checker = sqrt( object0*object0
		      + object1*object1 );


	if (checker > 0) checker = fix_one / checker;		//To get primitive...
	else checker = 0;

Q	object4	= checker*object0;				//The primitive V_n...
Q	object5	= checker*object1;

Q	object7 = I[object][22]*( A[object][0][1]*object4	//Delta_magnitude...
				+ A[object][1][1]*object5 );

	object4 = 1.5*object7*object4;				//Delta...
	object5 = 1.5*object7*object5;


Q	object8 = 2*I[object][21];				//Omega_magnitude...

	F_total[0] *= (	object0 == 0 );
	F_total[1] *= (	object1 == 0 );

	F_total[0] += object8*object0 - object4;
	F_total[1] += object8*object1 - object5;



//	Done with internal DOF...
//	=========================
	if( S[object][3][0] > two_pi ) S[object][3][0] -= two_pi;
	if( S[object][3][0] <-two_pi ) S[object][3][0] += two_pi;



//	Equations of motion...
//	======================


//	Biped (world) X coordinate...
//	=============================
	S[object][0][2] = I[object][12]*( F_total[0] );


//	Biped (world) Y coordinate...
//	=============================
	S[object][1][2] = I[object][12]*( F_total[1] );


//	Biped (world) Z coordinate...
//	=============================
	S[object][2][2] = I[object][12]*F_total[2] - I[object][29];  


//	Biped (world) alpha coordinate...
//	=================================
	S[object][3][2] = T_alpha;


//	Biped (world) beta coordinate...
//	================================
	S[object][4][2] = T_beta;  


//	Biped (world) gamma coordinate...
//	=================================
	S[object][5][2] = T_gamma;  


//	Ho, ho ho ho ho ho.  Ho.
//	========================


}





//	Utilities...
//	============





//	Here we find some knee action...
//	================================
void	place_right_knee( int object ) {

Q	alpha,
	beta,
	h,
	r,
	m,
	rho,
	mu,
	k_long;

Q	knee[3],
	l_hat[3];

Q	foot_length = .45*I[object][25];


//	Let's get the correct foot_gamma...
//	-----------------------------------
	if ( fc_r == 0 ) I[object][16] = A[object][5][0]+.5*pi;
//	I[object][16] = .5*pi;	

//	Now we manufacture the foot (simple for now)...
//	-----------------------------------------------
	utility_pointer[object][0] = foot_length*cos( -I[object][16] );
	utility_pointer[object][1] = foot_length*sin( -I[object][16] );
	utility_pointer[object][2] = utility_pointer[object][0]*terrain_r_dx
				   + utility_pointer[object][1]*terrain_r_dy;
//	if ( fc_r == 0 ) utility_pointer[object][2] += .5*foot_length;

	localize( utility_pointer[object][0],
		  utility_pointer[object][1],
		  utility_pointer[object][2] );

//	Now get the knee displacement...
//	--------------------------------
	alpha = I[object][0];
	beta = I[object][24] - I[object][25];

//	Here's the knee perpendicular...
//	--------------------------------	
	h = -.5*sqrt( (alpha*alpha - mag_foot_r*mag_foot_r)
	       *(mag_foot_r*mag_foot_r - beta*beta ) )/(mag_foot_r+.01);

//	Now we need the unit vector in the leg basis in that direction...
//	-----------------------------------------------------------------
	m   =-foot_hat_r[0]*utility_pointer[object][0]
	    - foot_hat_r[1]*utility_pointer[object][1];
	    //- foot_hat_r[2]*utility_pointer[object][2];

//	Safety First...
//	---------------
Q	mu_condom = sqrt( abs(foot_length*foot_length - m*m) );
	if (mu_condom > .01)	mu  = 1. / mu_condom;
	else			mu = 0;
//	mu = 0;

	l_hat[0] = mu*( m*foot_hat_r[0] + utility_pointer[object][0] );
	l_hat[1] = mu*( m*foot_hat_r[1] + utility_pointer[object][1] );
	l_hat[2] = mu*( m*foot_hat_r[2] + utility_pointer[object][2] );

//	r = I[object][24]/I[object][25];
	k_long = .5*mag_foot_r;

//	Undo it, just undo it...
//	========================
	knee[0] = -foot_r[0]*k_long + h*l_hat[0];
	knee[1] = -foot_r[1]*k_long + h*l_hat[1];
	knee[2] = -foot_r[2]*k_long + h*l_hat[2];

//	Now, let's copy these guys into the biped array for rendering...
//	================================================================
	utility_pointer[object][12] = H_r[0] + knee[0];
	utility_pointer[object][13] = H_r[1] + knee[1];
	utility_pointer[object][14] = H_r[2] + knee[2];

	utility_pointer[object][0] = utility_pointer[object][6] - utility_pointer[object][0];
	utility_pointer[object][1] = utility_pointer[object][7] - utility_pointer[object][1]; 
	utility_pointer[object][2] = utility_pointer[object][8] - utility_pointer[object][2]; 

}




//	Now the left...
//	===============
void	place_left_knee( int object ) {

Q	alpha,
	beta,
	h,
	r,
	m,
	rho,
	mu,
	k_long;

Q	knee[3],
	l_hat[3];

Q	foot_length = .45*I[object][25];


//	Let's get the correct foot_gamma...
//	-----------------------------------
	if ( fc_l == 0 ) I[object][17] = A[object][5][0]+.5*pi;
//	I[object][17] = .5*pi;	

//	Now we manufacture the foot (simple for now)...
//	-----------------------------------------------
	utility_pointer[object][3] = foot_length*cos( -I[object][17] );
	utility_pointer[object][4] = foot_length*sin( -I[object][17] );
	utility_pointer[object][5] = utility_pointer[object][3]*terrain_l_dx
				   + utility_pointer[object][4]*terrain_l_dy;
//	if ( fc_l == 0 ) utility_pointer[object][5] += .5*foot_length;

	localize( utility_pointer[object][3],
		  utility_pointer[object][4],
		  utility_pointer[object][5] );

//	Now get the knee displacement...
//	--------------------------------
	alpha = I[object][0];
	beta = I[object][24] - I[object][25];

//	Here's the knee perpendicular...
//	--------------------------------	
	h = -.5*sqrt( (alpha*alpha - mag_foot_l*mag_foot_l)
	           *(mag_foot_l*mag_foot_l - beta*beta ) )/(mag_foot_l+point_oo_one);

//	Now we need the unit vector in the leg basis in that direction...
//	-----------------------------------------------------------------
	m   =-foot_hat_l[0]*utility_pointer[object][3]
	    - foot_hat_l[1]*utility_pointer[object][4];
	    //- foot_hat_l[2]*utility_pointer[object][5];

//	Safety First...
//	---------------
Q	mu_condom = sqrt( abs(foot_length*foot_length - m*m) );
	if (mu_condom > .01)	mu  = 1. / mu_condom;
	else			mu = 0;
//	mu = 0;

	l_hat[0] = mu*( m*foot_hat_l[0] + utility_pointer[object][3] );
	l_hat[1] = mu*( m*foot_hat_l[1] + utility_pointer[object][4] );
	l_hat[2] = mu*( m*foot_hat_l[2] + utility_pointer[object][5] );

//	r = I[object][24]/I[object][25];
	k_long = .5*mag_foot_l;

//	Undo it, just undo it...
//	========================
	knee[0] = -foot_l[0]*k_long + h*l_hat[0];
	knee[1] = -foot_l[1]*k_long + h*l_hat[1];
	knee[2] = -foot_l[2]*k_long + h*l_hat[2];

//	Now, let's copy these guys into the biped array for rendering...
//	================================================================
	utility_pointer[object][15] = H_l[0] + knee[0];
	utility_pointer[object][16] = H_l[1] + knee[1];
	utility_pointer[object][17] = H_l[2] + knee[2];

	utility_pointer[object][3] = utility_pointer[object][9]  - utility_pointer[object][3];
	utility_pointer[object][4] = utility_pointer[object][10] - utility_pointer[object][4]; 
	utility_pointer[object][5] = utility_pointer[object][11] - utility_pointer[object][5]; 

}








//	We need to transform from global coordinates to the local airplane
//	coordinaates, a simple rotation.  The order is left up to this routine
//	which MODIFIES ITS ARGUMENTS...
//	===============================
void globalize( Q &X, Q &Y, Q &Z ) {

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



//	We need to transform from local coordinates back to the global coordinates
//	for the actual EDMS model...
//	============================
void localize( Q &X, Q &Y, Q &Z ) {

Q	x = X,
	y = Y,
	z = Z;


//	Rotate on Euler angles...
//	=========================
		X    = 	x*( cos_alpha*cos_gamma )
		     +  y*( cos_alpha*sin_gamma )
		     +  z*( -sin_alpha );

		Y    =  x*( cos_gamma*sin_alpha*sin_beta - cos_beta*sin_gamma )
		     +  y*( cos_beta*cos_gamma + sin_alpha*sin_beta*sin_gamma )
		     +  z*( cos_alpha*sin_beta );		

		Z    = 	x*( cos_beta*cos_gamma*sin_alpha + sin_beta*sin_gamma )
		     +  y*( cos_beta*sin_alpha*sin_gamma - cos_gamma*sin_beta )
		     +  z*( cos_alpha*cos_beta );



}





