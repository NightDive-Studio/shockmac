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
//	Here is the bridge routine for maintenance and upkeep of death...
//	=================================================================


//#include <conio.h>
#include "fixpp.h"
#include "EDMS_int.h"



//#ifdef EDMS_SHIPPABLE
//#include <mout.h>
//#endif



//	Here we need include files for each and every model that we'll be using...
//	==========================================================================
#include "death.h"


//	The physics handles definitions...
//	==================================
#include "physhand.h"

//	Pointers to skeletons (for bipeds, as it were and will be)...
//	=============================================================
extern Q	*utility_pointer[MAX_OBJ];

//	Death...
//	--------
typedef struct {

fix	mass,
	size,
	gravity,
   drag;

} Death;




//	We need to link to c...
//	=======================
//extern "C" {





//	Here are the bridge routines to the models...
//	=============================================






//      Utility routines...
//	===================
physics_handle EDMS_make_death( Death *d, State *s ) {


Q		params[10],
		init_state[6][3];

Q		mass,
		size,
		gravity,
      drag;

int		on = 0;

physics_handle	ph = 0;

	init_state[0][0].fix_to( s->X );		init_state[0][1].fix_to( s->X_dot );
	init_state[1][0].fix_to( s->Y );		init_state[1][1].fix_to( s->Y_dot );    
	init_state[2][0].fix_to( s->Z );		init_state[2][1].fix_to( s->Z_dot );    

        mass.fix_to( d -> mass );
	size.fix_to( d -> size );
//	if ( size > .45/hash_scale ) size = .45/hash_scale;
	gravity.fix_to( d -> gravity );
   drag.fix_to (d->drag);

	params[0] = mass;
	params[1] = 1/mass;
	params[2] = 0;
	params[3] = 0;
	params[4] = 0;
	params[5] = drag * 2 * size;
	params[6] = gravity;
	params[7] = size;
	params[8] = 
	params[9] = END;


                on = make_death( init_state, params );
                ph = EDMS_bind_object_number( on );

		return ph;

}




//	Utilities for the weak spirited...
//	==================================
void EDMS_set_death_parameters( physics_handle ph, Death *d )
{
Q	mass,
	size,
	gravity,
   drag;

	mass.fix_to( d -> mass );
	size.fix_to( d -> size );
	gravity.fix_to( d -> gravity );
   drag.fix_to (d->drag);

	int on = physics_handle_to_object_number( ph );

	I[on][20] = mass;
	I[on][21] = 1/mass;
	I[on][22] = 0;
	I[on][23] = 0;
	I[on][24] = 0;
	I[on][25] = drag*2*size;
	I[on][26] = gravity;
	I[on][27] = size;
	I[on][28] = 
	I[on][29] = END;


}



//	And the weak minded...
//	======================
void EDMS_get_death_parameters( physics_handle ph, Death *d )
{
	int on = physics_handle_to_object_number( ph );

	d -> size = (I[on][27]).to_fix();
	d -> mass = I[on][20].to_fix();
	d -> gravity = I[on][26].to_fix();
   d -> drag = (I[on][25] / (I[on][27] * 2)).to_fix();
}



//}	//End of extern "C" for the &^%$@% compiler...

