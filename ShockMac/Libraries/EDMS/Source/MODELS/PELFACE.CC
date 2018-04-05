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
//	Here is the bridge routine for maintenance and upkeep of the pelvis models...
//	=============================================================================


//#include <conio.h>
#include "fixpp.h"
#include "EDMS_int.h"



//#ifdef EDMS_SHIPPABLE
//#include <mout.h>
//#endif



//	Here we need include files for each and every model that we'll be using...
//	==========================================================================
#include "pelvis.h"


//	The physics handles definitions...
//	==================================
#include "physhand.h"


//	Pointers to skeletons (for bipeds, as it were and will be)...
//	=============================================================
extern Q	*utility_pointer[MAX_OBJ];


//	Pelvis...
//	---------
typedef struct {

fix	mass,
	size,
	hardness,
	pep,
	gravity,
	height;

int     cyber_space;

} Pelvis;


//	Here we go...
//	=============
#define HARD_FAC 6


//      Thatnk God we have only 16 bits of fraction!
//      ============================================
Q       old_state[6],
        new_state[6];


//	We need to link to c...
//	=======================
//extern "C" {





//	Here are the bridge routines to the models...
//	=============================================






//      Pelvis routines...
//	==================
physics_handle EDMS_make_pelvis( Pelvis *p, State *s ) {


Q		params[10],
		init_state[6][3];

Q		mass,
		pep,
		hardness,
		size,
		gravity,
		height;

int		on = 0,
		cyber_space = 0;

physics_handle	ph = 0;

	init_state[0][0].fix_to( s->X );		init_state[0][1].fix_to( s->X_dot );
	init_state[1][0].fix_to( s->Y );		init_state[1][1].fix_to( s->Y_dot );    
	init_state[2][0].fix_to( s->Z );		init_state[2][1].fix_to( s->Z_dot );    
	init_state[3][0].fix_to( s-> alpha );		init_state[3][1].fix_to( s->alpha_dot );
	init_state[4][0].fix_to( s-> beta );		init_state[4][1].fix_to( s->beta_dot );
	init_state[5][0].fix_to( s-> gamma );		init_state[5][1].fix_to( s->gamma_dot );



        mass.fix_to( p -> mass );
	size.fix_to( p -> size );
//	if ( size > .45/hash_scale ) size = .45/hash_scale;
	hardness.fix_to( p -> hardness );
	pep.fix_to( p -> pep );
	gravity.fix_to( p -> gravity );
	height.fix_to( p -> height );
        if ( height > 3*size ) height = 3*size;

//	mout << "Pelvis: \n";
//	mout << "	mass: " << mass << "\n";
//	mout << "	size: " << size << "\n";
//	mout << "	hard: " << hardness << "\n";
//	mout << "	pepp: " << pep << "\n";
//	mout << "	grav: " << gravity << "\n";
//	mout << "	hght: " << height << "\n";

//	hardness = hardness*(mass*4/size);
	hardness = hardness*(mass*HARD_FAC/size);
	params[0] = hardness;
	params[1] = 3*sqrt( params[0] * mass );
	params[2] = size;
	params[3] = pep * mass;
	params[4] = 1. / mass;
	params[5] = gravity;
	params[6] = mass;
	params[7] = 1. / ( .4*mass*size*size );
	params[8] = 5.*(1. / params[7]);
	params[9] = .4*mass*size*size;
//      mout << "I29 from make! " << params[9] << "\n";


                on = make_pelvis( init_state, params );

//              Turn on Cyber Space...
//              ----------------------
		cyber_space = p -> cyber_space;
		if ( cyber_space < 0 || cyber_space > 2 ) cyber_space = 0;	//Hey, why you do that?

		I[on][10] = cyber_space;
		I[on][0] = I[on][6] = height - size;  				//Ok...

                ph = EDMS_bind_object_number( on );

		return ph;

}




//      This works just like the robot model...
//      ---------------------------------------
void EDMS_control_pelvis( physics_handle ph, fix F, fix T, fix S, fix L, fix J, int C ) {

Q	FF,			//Silly, no?
	TT,
	SS,
        LL,
        JJ;


#ifdef EDMS_SHIPPABLE
	if ( ph < 0 ) mout << "Hey, you are and idiot...";
#endif


	FF.fix_to( F );
	TT.fix_to( T );
	SS.fix_to( S );
	LL.fix_to( L );
	JJ.fix_to( J );

        int on = physics_handle_to_object_number( ph );

        if( I[on][30] == PELVIS ) pelvis_set_control( on, FF, TT, SS, LL, JJ, C );

}




//      At some point we need the viewpoint offered by the neck...
//      ----------------------------------------------------------
void EDMS_get_pelvic_viewpoint( physics_handle ph, State *s ) {

int     on = ph2on[ph];

Q       delta = 0;

        if ( I[on][30] == PELVIS ) {

Q	new_neck = I[on][0];			//Rendered height...

Q	offset_x = new_neck*sin( S[on][4][0] ),
	offset_y =-1.5*new_neck*sin( S[on][5][0] ),
	offset_z = new_neck*cos( (1)*S[on][4][0] )*cos( (1)*S[on][5][0] );
//	offset_z = new_neck*cos( (.2 + .8*(I[on][10]>0) )*S[on][4][0] )*cos( (.2 + .8*(I[on][10]>0) )*S[on][5][0] );

Q       sin_alpha = 0,
        cos_alpha = 0;


Q       final_x = 0,
        final_y = 0;        
        
        
        sincos( -S[on][3][0], &sin_alpha, &cos_alpha );
        final_x = cos_alpha*offset_x + sin_alpha*offset_y;
        final_y =-sin_alpha*offset_x + cos_alpha*offset_y;

        new_state[0] = ( S[on][0][0] + final_x  );
        new_state[1] = ( S[on][1][0] + final_y  );
        new_state[2] = ( S[on][2][0] + offset_z );

        new_state[3] = S[on][3][0];
        new_state[4] = (.1  /*- .3*(I[on][10]>0)*/ )*S[on][4][0];
        new_state[5] = (.03 /*+ 1.6*(I[on][10]>0)*/ )*S[on][5][0];

//        new_state[3] = S[on][3][0];
//        new_state[4] = (.1  - .1*(I[on][10]>0) )*S[on][4][0];
//        new_state[5] = (.03 + 1.6*(I[on][10]>0) )*S[on][5][0];

	if ( I[on][10] == 2 ) {
	new_state[4] = S[on][4][0];
        new_state[5] = S[on][5][0];
	}



        delta =         (new_state[0] - old_state[0])*(new_state[0] - old_state[0])
                      + (new_state[1] - old_state[1])*(new_state[1] - old_state[1])
                      + (new_state[2] - old_state[2])*(new_state[2] - old_state[2])
                      + (new_state[3] - old_state[3])*(new_state[3] - old_state[3])
                      + (new_state[4] - old_state[4])*(new_state[4] - old_state[4])
                      + (new_state[5] - old_state[5])*(new_state[5] - old_state[5]);


        if ( delta > .00003 ) {

                old_state[0] = new_state[0];
                old_state[1] = new_state[1];
                old_state[2] = new_state[2];

                old_state[3] = new_state[3];
                old_state[4] = new_state[4];
                old_state[5] = new_state[5];

        }

        s->X = old_state[0].to_fix();    
        s->Y = old_state[1].to_fix();    
        s->Z = old_state[2].to_fix();    

        s->alpha = old_state[3].to_fix();
        s->beta  = old_state[4].to_fix();
        s->gamma = old_state[5].to_fix();


//        if ( delta > 40 ) {
//                mout << "Holy cow, batman, delta = " << delta << "\n";
//                getch();
//                }


        }       //End of check for pelvis or not...
           

//#ifdef EDMS_SHIPPABLE
//       else {  mout << "Pelvic Viewpoint: physics handle " << ph << ", object #" << on << " isn't a Pelvis model!\n";
//		mout << "Is is really a " << I[on][30] << " located at (" << S[on][0][0] << "," << S[on][1][0] << ")!\n"; }
//#endif




}   




//	Utilities for the weak spirited...
//	==================================
void EDMS_set_pelvis_parameters( physics_handle ph, Pelvis *p )
{
Q	mass,
	hardness,
	size,
	pep,
	height,
	gravity;

int	cyber_space = 0;

	mass.fix_to( p -> mass );
	size.fix_to( p -> size );
	hardness.fix_to( p -> hardness );
	pep.fix_to( p -> pep );
	gravity.fix_to( p -> gravity );
	height.fix_to( p -> height );
        if ( height > 3*size ) height = 3*size;

	int on = physics_handle_to_object_number( ph );

//	hardness = hardness*(mass*4/size);
	hardness = hardness*(mass*HARD_FAC/size);
	I[on][20] = hardness;
	I[on][21] = 3*sqrt( I[on][20] * mass );
	I[on][22] = size;
	I[on][23] = pep * mass;
	I[on][24] = 1. / mass;
	I[on][25] = gravity;
	I[on][26] = mass;
	I[on][27] = 1. / ( .4*mass*size*size );
	I[on][28] = 5.*(1. / I[on][27]);
	I[on][29] = .4*mass*size*size;
//      mout << "I29 from set! " << I[on][29] << "\n";
//        if ( I[on][30] != PELVIS ) mout << "!EDMS: You just screwed up the pelvis...\n";

	cyber_space = p -> cyber_space;

//	Turn on Cyber Space...
//	----------------------
	cyber_space = p -> cyber_space;
        I[on][7] = I[on][15] = 0;
	if ( cyber_space < 0 || cyber_space > 2 ) cyber_space = 0;	//Hey, why you do that?

	I[on][10] = cyber_space;
//      Won't need to be reset!
//	I[on][0] = I[on][6] = height - size;

//      Turn lean control off for skates!
//      ---------------------------------
        if (I[on][10] > 0) I[on][15] = I[on][7] = 0;

}



//	And the weak minded...
//	======================
void EDMS_get_pelvis_parameters( physics_handle ph, Pelvis *p )
{
	int on = physics_handle_to_object_number( ph );

	p -> pep = (I[on][23] / I[on][26]).to_fix();
	p -> size = I[on][22].to_fix();
//	p -> hardness = ( I[on][20]*I[on][22]/(I[on][26]*4) ).to_fix();
	p -> hardness = ( I[on][20]*I[on][22]/(I[on][26]*HARD_FAC) ).to_fix();
	p -> mass = I[on][26].to_fix();
	p -> gravity = I[on][25].to_fix();
        p -> cyber_space = I[on][10].to_int();
	p -> height = (I[on][6] + I[on][22]).to_fix();

}





//	And the compression test for terrain "traps..."
//	===============================================
fix EDMS_get_pelvis_damage( physics_handle ph, fix delta_t ) {

int	object;
Q	worker_bee_buzz_buzz = 0;

	object = ph2on[ph];			//As stupid as it gets...
        worker_bee_buzz_buzz = I[object][14];
        I[object][14] = 0;
 
	return fix_mul( delta_t, I[object][14].to_fix() );

}





//}	//End of extern "C" for the &^%$@% compiler...

