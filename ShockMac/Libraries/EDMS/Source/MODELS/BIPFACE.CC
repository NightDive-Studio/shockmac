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
//	Here is the bridge routine for maintenance and upkeep of the biped models...
//	============================================================================


//#include <conio.h>
#include "fixpp.h"
#include "EDMS_int.h"



#ifdef EDMS_SHIPPABLE
//#include <mout.h>
#endif



//	Here we need include files for each and every model that we'll be using...
//	==========================================================================
#include "biped.h"


//	The physics handles definitions...
//	==================================
#include "physhand.h"


//	Data...
//	=======
extern EDMS_Argblock_Pointer	A;
extern Q	S[MAX_OBJ][7][4],
		I[MAX_OBJ][DOF_MAX];

extern int	no_no_not_me[MAX_OBJ];

//	Pointers to skeletons (for bipeds, as it were and will be)...
//	=============================================================
extern Q	*utility_pointer[MAX_OBJ];


//	Utilities...
//	============
extern void	write_object( int ),				//Collisions...
		delete_object( int );


//	Structs...
//	==========
//typedef struct
//{
//	fix X, Y, Z, alpha, beta, gamma;
//	fix X_dot, Y_dot, Z_dot, alpha_dot, beta_dot, gamma_dot;
//} State;



//	Bipeds...
//	---------
typedef struct {

fix	mass,
	max_speed,
	skill,
	gravity;

fix     hip_radius,
        thigh,
        shin,
        torso;

fix     shoulders,                                        //Arms have 1/2 length segments, i.e. elbow always at 1/2*arms
        arms;

} Biped;



//	We need to link to c...
//	=======================
//extern "C" {





//	Here are the bridge routines to the models...
//	=============================================






//      Biped routines...
//	==================
physics_handle EDMS_make_biped( Biped *b, State *s, fix skeleton[50] ) {


Q		params[10],
		init_state[6][3];

Q		mass,
		max_speed,
		skill,
		gravity;

Q               hips,
                thighs,
                shins,
                torsos;

int		on = 0;
physics_handle	ph = 0;

	init_state[0][0].fix_to( s->X );		init_state[0][1].fix_to( s->X_dot );
	init_state[1][0].fix_to( s->Y );		init_state[1][1].fix_to( s->Y_dot );    
	init_state[2][0].fix_to( s->Z );		init_state[2][1].fix_to( s->Z_dot );    
	init_state[3][0].fix_to( s-> alpha );		init_state[3][1].fix_to( s->alpha_dot );
	init_state[4][0].fix_to( s-> beta );		init_state[4][1].fix_to( s->beta_dot );
	init_state[5][0].fix_to( s-> gamma );		init_state[5][1].fix_to( s->gamma_dot );


	mass.fix_to( b -> mass );
	max_speed.fix_to( b -> max_speed );
	skill.fix_to( b -> skill );
	gravity.fix_to( b -> gravity );

   hips.fix_to( b -> hip_radius );
	thighs.fix_to( b -> thigh );
	shins.fix_to( b -> shin );
	torsos.fix_to( b -> torso );

	params[0] = mass;			//Mass
	params[1] = 250.*mass;			//Kappa_leg
	params[2] = 1.5*sqrt(mass*params[1]);	//Delta_leg
	params[3] = hips;			//l_hip
	params[4] = thighs;			//l_thigh
	params[5] = shins;			//l_shin
	params[6] = torsos;			//l_torso
	params[7] = 5.*mass;			//m_bal
	params[8] = skill;			//balance skill
	params[9] = gravity;			//gravity


		on = make_biped( init_state, params, (Q*)&skeleton[0] );

		utility_pointer[on] = (Q*)&skeleton[0];

		ph = EDMS_bind_object_number( on );

		return ph;

}




//	Here we can set biped parameters...
//	===================================
void	EDMS_set_biped_parameters( physics_handle ph, Biped *b ) {


Q		mass,
		max_speed,
		skill,
		gravity;

Q               hips,
                thighs,
                shins,
                torsos;

int		on = ph2on[ph];


	mass.fix_to( b -> mass );
	max_speed.fix_to( b -> max_speed );
	skill.fix_to( b -> skill );
	gravity.fix_to( b -> gravity );

   hips.fix_to( b -> hip_radius );
	thighs.fix_to( b -> thigh );
	shins.fix_to( b -> shin );
	torsos.fix_to( b -> torso );

	I[on][20] = mass;			//Mass
	I[on][21] = 250.*mass;			//Kappa_leg
	I[on][22] = 1.5*sqrt(mass*I[on][21]);	//Delta_leg
	I[on][23] = hips;			//l_hip
	I[on][24] = thighs;			//l_thigh
	I[on][25] = shins;			//l_shin
	I[on][26] = torsos;			//l_torso
	I[on][27] = 5.*mass;			//m_bal
	I[on][28] = skill;			//balance skill
	I[on][29] = gravity;			//gravity


}





//	Here we control the silly thing. Here "mode" is 0 for heading control, 1 for
//	sidestepping...
//	===============
void EDMS_control_biped( physics_handle ph, fix forward, fix side_rotate, int mode ) {

int	on = ph2on[ph];
Q	F,
	SR;

	F.fix_to( forward );

Q	speed = sqrt( S[on][0][1]*S[on][0][1] + S[on][1][1]*S[on][1][1] );

	F = 20*( .2*F - speed );

	SR.fix_to( side_rotate );

//	mout << "Biped physics handle " << ph << " controlled with " << F << ", " << SR << ", and "
//	     << mode << ".\n";

	if (mode==1) {	I[on][18] = SR;	I[on][20] = 0; }		//Simple for now...
	else	     {	I[on][20] = SR;	I[on][18] = 0; }

			I[on][19] = F;

	no_no_not_me[on] = no_no_not_me[on] ||
                         ( abs(I[on][18]) + abs(I[on][19]) + abs(I[on][20]) > 0 );



}




//	After calling an integrator, jointed objects such as the Biped must have their
//	skeletons updated before rendering.  This allows the caller to save time if the
//	object is not visible during a frame.  The skeleton is a vector specifyied upon
//	creaton of the model by the caller.  That is all.  Good grammar.
//	================================================================
void EDMS_make_biped_skeleton( physics_handle ph ) {

int	on = physics_handle_to_object_number( ph );
	make_biped_skeleton( on );
	
}




//}	//End of extern "C" for the &^%$@% compiler...

