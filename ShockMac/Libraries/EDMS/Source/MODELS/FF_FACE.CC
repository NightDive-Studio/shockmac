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
//	One time only, FF appearance...
//	===============================

//	Here is the bridge routine for maintenance and upkeep of the FTL models...
//	==========================================================================



#include <fixpp.h>
#include <EDMS_int.h>


#ifdef EDMS_SHIPPABLE
#include <mout.h>
#endif



//	Here we need include files for each and every model that we'll be using...
//	==========================================================================
#include <ff_ftl.h>


//	The physics handles definitions...
//	==================================
#include <physhand.h>


//	We need to link to c...
//	=======================
extern "C" {

//	Here are the bridge routines to the models...
//	=============================================



//	This one uses the Freefall raycaster
//	
physics_handle	EDMS_FF_beam_weapon_new(    fix X[3],
					fix D[3],
					fix speed,
					fix mass,
					fix size,
					fix range,
					physics_handle exclude,
					physics_handle shooter,
					long &g_info,
					long &w_info,
					bool &hit )
{

//	Return me, baby...
//	------------------
physics_handle	ph = -1;
int	EXCLUDE = 0;

Q      	DD[3];

Q	Speed,
	Mass,
	Size,
	Range;

Q	*XX = (Q*)&X[0];	

	DD[0].fix_to( D[0] );
	DD[1].fix_to( D[1] );
	DD[2].fix_to( D[2] );

	Speed.fix_to( speed );
	Mass.fix_to( mass );
	Size.fix_to( size );
	Range.fix_to( range );

	EXCLUDE = ph2on[exclude];


//	Do it...
//	========
	ph = EDMS_FF_cast_projectile_new( XX, DD, Speed, Mass, Size, Range, EXCLUDE, shooter, g_info, w_info, hit );

//      Convert back to the goofbakk fixpoint system...
//      -----------------------------------------------
        X[0] = XX[0].to_fix();
        X[1] = XX[1].to_fix();
        X[2] = XX[2].to_fix();

//	Return a physics handle...
//	==========================
	return ph;
}








#ifdef OBSOLETE

//	Beam weapon ray caster...
//	=========================
physics_handle	EDMS_FF_beam_weapon(    fix X[3],
					fix D[3],
					fix speed,
					fix mass,
					fix size,
					fix range,
					physics_handle exclude,
					physics_handle shooter,
					long &g_info,
					long &w_info,
					bool &hit )
{

//	Return me, baby...
//	------------------
physics_handle	ph = -1;
int	EXCLUDE = 0;

Q      	DD[3];

Q	Speed,
	Mass,
	Size,
	Range;

Q	*XX = (Q*)&X[0];	

	DD[0].fix_to( D[0] );
	DD[1].fix_to( D[1] );
	DD[2].fix_to( D[2] );

	Speed.fix_to( speed );
	Mass.fix_to( mass );
	Size.fix_to( size );
	Range.fix_to( range );

	EXCLUDE = ph2on[exclude];


//	Do it...
//	========
	ph = EDMS_FF_cast_projectile( XX, DD, Speed, Mass, Size, Range, EXCLUDE, shooter, g_info, w_info, hit );

//      Convert back to the goofbakk fixpoint system...
//      -----------------------------------------------
        X[0] = XX[0].to_fix();
        X[1] = XX[1].to_fix();
        X[2] = XX[2].to_fix();


//	Return a physics handle...
//	==========================
	return ph;


}

#endif // OBSOLETE

}	//End of extern "C" for the &^%$@% compiler...
