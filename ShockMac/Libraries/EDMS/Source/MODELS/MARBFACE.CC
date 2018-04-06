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
 * $Header: r:/prj/lib/src/new_edms/models/RCS/marbface.cc 1.2 1994/12/07 19:38:28 dfan Exp $
 */

// Marble interface

#include "fixpp.h"
#include "EDMS_int.h"
#include "idof.h"
#include "marble.h"

//	Marbles...
//	----------
typedef struct
{
   fix	mass,
	      size,
      	pep,
         gravity;
} Marble;


//extern "C" {

//	Marble routines...
//	==================
void EDMS_get_marble_parameters( physics_handle ph, Marble *m )
{
	int on = physics_handle_to_object_number( ph );

	m -> pep = I[on][23].to_fix();
	m -> size = I[on][22].to_fix();
	m -> mass = I[on][26].to_fix();
   m -> gravity = I[on][25].to_fix();
}

void EDMS_control_marble( physics_handle ph, fix X, fix Y, fix Z )
{
   Q	XX,			//Silly, no?
	   YY,
	   ZZ;

	XX.fix_to( X );
	YY.fix_to( Y );
	ZZ.fix_to( Z );

	int on = physics_handle_to_object_number( ph );
	marble_set_control( on, XX, YY, ZZ );

}

physics_handle EDMS_make_marble( Marble *m, State *s )
{

   Q	params[10],
		init_state[6][3];

   Q	mass,
		pep,
		size,
      gravity;

   int on = 0;
   physics_handle	ph = 0;

	init_state[0][0].fix_to( s->X );		init_state[0][1].fix_to( s->X_dot );
	init_state[1][0].fix_to( s->Y );		init_state[1][1].fix_to( s->Y_dot );    
	init_state[2][0].fix_to( s->Z );		init_state[2][1].fix_to( s->Z_dot );    
	init_state[3][0] =				init_state[3][1] =
	init_state[4][0] =				init_state[4][1] = 
	init_state[5][0] = 				init_state[5][1] = END;

	mass.fix_to( m -> mass );
	size.fix_to( m -> size );
	pep.fix_to( m -> pep );
   gravity.fix_to (m->gravity);

	params[OFFSET(IDOF_MARBLE_K)]          = 100. * mass;
	params[OFFSET(IDOF_MARBLE_D)]          = 2. * sqrt( params[0] * mass );
	params[OFFSET(IDOF_MARBLE_RADIUS)]     = size;
	params[OFFSET(IDOF_MARBLE_ROLL_DRAG)]  = 10./(pep+0.1);
	params[OFFSET(IDOF_MARBLE_MASS_RECIP)] = 1. / mass;
	params[OFFSET(IDOF_MARBLE_GRAVITY)]    = gravity;
	params[OFFSET(IDOF_MARBLE_MASS)]       = mass;
	params[OFFSET(IDOF_MARBLE_27)]         = 0;
	params[OFFSET(IDOF_MARBLE_28)]         = 0;
	params[OFFSET(IDOF_MARBLE_29)]         = 0;

	on = make_marble( init_state, params );
	ph = EDMS_bind_object_number( on );
	return ph;
}


void EDMS_set_marble_parameters( physics_handle ph, Marble *m )
{
   Q	mass,
	   size,
   	pep,
      gravity;

	mass.fix_to( m -> mass );
	size.fix_to( m -> size );
	pep.fix_to( m -> pep );
   gravity.fix_to (m->gravity);   

	int on = physics_handle_to_object_number( ph );

	I[on][IDOF_MARBLE_K]          = 100. * mass;
	I[on][IDOF_MARBLE_D]          = 2. * sqrt( I[on][20] *  mass );
	I[on][IDOF_MARBLE_RADIUS]     = size;
	I[on][IDOF_MARBLE_ROLL_DRAG]  = 10./(pep+0.1);
	I[on][IDOF_MARBLE_MASS_RECIP] = 1. / mass;
	I[on][IDOF_MARBLE_GRAVITY]    = gravity;
	I[on][IDOF_MARBLE_MASS]       = mass;
	I[on][IDOF_MARBLE_27]         = 1;
	I[on][IDOF_MARBLE_28]         = 0;
	I[on][IDOF_MARBLE_29]         = 0;
}

//} // extern "C"
