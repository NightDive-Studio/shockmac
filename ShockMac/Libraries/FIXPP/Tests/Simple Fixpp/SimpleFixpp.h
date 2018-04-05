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

#define SHIFTUP 16                     // 16:16 default format.


class Fixpoint
{
public:

   // The data is stored here.
   // ========================
   long	val;

   // Constructors.
   // =============

   Fixpoint();
   Fixpoint( const Fixpoint & );
   Fixpoint( int );
   Fixpoint( long );

   // Arithmetic operators (homogeneous)!!
   // ====================================
   Fixpoint& operator+=( Fixpoint );
   Fixpoint& operator-=( Fixpoint );


   Fixpoint operator-( void ) const ;   
   Fixpoint operator+( void ) const ;
};


// Constructors
// ============

inline Fixpoint::Fixpoint()  						{}

inline Fixpoint::Fixpoint( const Fixpoint & fp )	{ val = fp.val; }

inline Fixpoint::Fixpoint( int i )					{ val = i<<SHIFTUP; }

inline Fixpoint::Fixpoint( long i )					{ val = i<<SHIFTUP; }





// ======================================
//
// Math functions.
//
// ======================================

inline Fixpoint& Fixpoint::operator+=(Fixpoint fp2 )
{
   val += fp2.val;
   return *this;
}

inline Fixpoint& Fixpoint::operator-=(Fixpoint fp2 )
{
   val -= fp2.val;
   return *this;
}

inline Fixpoint operator+(Fixpoint a,Fixpoint b)
{
   a.val+=b.val;
   return a;
}

inline Fixpoint operator-(Fixpoint a,Fixpoint b)
{
   a.val-=b.val;
   return a;
}
