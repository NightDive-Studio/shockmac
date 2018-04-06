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
 * $Header: r:/prj/lib/src/fixpp/RCS/fixpp.cc 1.28 1994/08/30 11:32:42 jak Exp $
 */

// еее For now, turn debugging on, so we can run the test programs.
#define FIXDEBUG 1


#include <math.h>

#include "fixpp.h"





#ifdef FIXDEBUG

// =========================================================
// touch() is a function that does nothing except cause the
// variable passed in to "escape" in the optimization sense,
// so that the compiler cannot optimize it as much.
// =========================================================

void touch( Fixpoint& a )
{
   a = a;
}

// ===========================================================
// bitdump() dumps the fixpoint to a string and returns
// the address of the string.
// ===========================================================

char *bitdump( Fixpoint & a )
{
   static char string[30];

   sprintf( string, "[%#lx]", a.val );

   return string;
}


#endif /* FIXDEBUG */

// Moved here from header file - KC
Fixpoint rawConstruct( long l )
{
	Fixpoint f;
	f.val = l;
	return f;
}
#define f2Fixpoint(x) (rawConstruct( (long)((x)*SHIFTMULTIPLIER) ))


Fixpoint Fixpoint_one_over_two_pi = f2Fixpoint(0.159154943);
Fixpoint Fixpoint_two_pi          = f2Fixpoint(6.283185306);
  



#ifdef FIXDEBUG

bool  Fixpoint::click_bool = 1;

ulong Fixpoint::constructor_void     = 0,
      Fixpoint::constructor_Fixpoint = 0,
      Fixpoint::constructor_int      = 0,
      Fixpoint::constructor_uint     = 0,
      Fixpoint::constructor_lint     = 0,
      Fixpoint::constructor_ulint    = 0,
      Fixpoint::constructor_double   = 0;

ulong Fixpoint::ass_Fixpoint = 0,
      Fixpoint::ass_int      = 0,
      Fixpoint::ass_lint     = 0,
      Fixpoint::ass_uint     = 0,
      Fixpoint::ass_ulint    = 0,
      Fixpoint::ass_double   = 0;

ulong Fixpoint::binary_add = 0,
      Fixpoint::binary_div = 0,
      Fixpoint::binary_sub = 0,
      Fixpoint::binary_mul = 0;

ulong Fixpoint::add_eq = 0,
      Fixpoint::sub_eq = 0,
      Fixpoint::mul_eq = 0,
      Fixpoint::div_eq = 0;

ulong Fixpoint::unary_minus = 0,
      Fixpoint::unary_plus  = 0;

ulong Fixpoint::cond_l   = 0,
      Fixpoint::cond_g   = 0,
      Fixpoint::cond_le  = 0,
      Fixpoint::cond_ge  = 0,
      Fixpoint::cond_eq  = 0,
      Fixpoint::cond_neq = 0;

void Fixpoint::report( void ) { report( cout ); }

void Fixpoint::report( ostream& os )
{
   os << "Constructor     void: " << constructor_void     << '\n' ;
   os << "Constructor Fixpoint: " << constructor_Fixpoint << '\n' ;
   os << "Constructor      int: " << constructor_int      << '\n' ;
   os << "Constructor     lint: " << constructor_lint     << '\n' ;
   os << "Constructor     uint: " << constructor_uint     << '\n' ;
   os << "Constructor    ulint: " << constructor_ulint    << '\n' ;
   os << "Constructor   double: " << constructor_double   << '\n' ;

   os << "Assign to Fixpoint:   " << ass_Fixpoint << '\n';
   os << "Assign to int:        " << ass_int      << '\n'; 
   os << "Assign to uint:       " << ass_uint     << '\n'; 
   os << "Assign to lint:       " << ass_lint     << '\n'; 
   os << "Assign to ulint:      " << ass_ulint    << '\n'; 
   os << "Assign to double:     " << ass_double   << '\n';

   os << "Binary Add:           " << binary_add << '\n';
   os << "Binary Sub:           " << binary_sub << '\n';
   os << "Binary Div:           " << binary_div << '\n';
   os << "Binary Mul:           " << binary_mul << '\n';

   os << "Add-equals            " << add_eq << '\n';
   os << "Sub-equals            " << sub_eq << '\n';
   os << "Mul-equals            " << mul_eq << '\n';
   os << "Div-equals            " << div_eq << '\n';

   os << "Unary minus           " << unary_minus << '\n';
   os << "Unary  plus           " << unary_plus  << '\n';

   os << "<                     " << cond_l   << '\n' ;
   os << ">                     " << cond_g   << '\n' ;
   os << "<=                    " << cond_le  << '\n' ;
   os << ">=                    " << cond_ge  << '\n' ;
   os << "==                    " << cond_eq  << '\n' ;
   os << "!=                    " << cond_neq << '\n' ;
}

void Fixpoint::reset_report( void )
{
   constructor_void =
   constructor_Fixpoint =
   constructor_int =
   constructor_uint =
   constructor_lint =
   constructor_ulint =
   constructor_double =

   ass_Fixpoint =
   ass_int =
   ass_uint =
   ass_lint =
   ass_ulint =
   ass_double =

   binary_add =
   binary_sub =
   binary_div =
   binary_mul =

   add_eq =
   sub_eq =
   mul_eq =
   div_eq =

   unary_minus =
   unary_plus =

   cond_l =
   cond_g =
   cond_le =
   cond_ge =
   cond_eq =
   cond_neq =

   0;
}

#endif /* FIXDEBUG */

