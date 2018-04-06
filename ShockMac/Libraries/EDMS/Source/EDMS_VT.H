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
//	EDMS vector tools: Some vector macros for EDMS that are fast...
//	===============================================================

//	Components of cross products...
//	-------------------------------
#define	XP_0( Q, W ) ( Q[1]*W[2] - Q[2]*W[1] )
#define	XP_1( Q, W ) ( Q[2]*W[0] - Q[0]*W[2] )
#define	XP_2( Q, W ) ( Q[0]*W[1] - Q[1]*W[0] )

#define	XPT_0( Q ) ( Q[1]*gamma_dot - Q[2]*beta_dot  )
#define	XPT_1( Q ) ( Q[2]*alpha_dot - Q[0]*gamma_dot )
#define	XPT_2( Q ) ( Q[0]*beta_dot  - Q[1]*alpha_dot )

//	Components of cross products with state elements...
//	---------------------------------------------------
#define	XPA_0( Q, object ) ( Q[1]*A[object][5][1] - Q[2]*A[object][4][1] )
#define	XPA_1( Q, object ) ( Q[2]*A[object][3][1] - Q[0]*A[object][5][1] )
#define	XPA_2( Q, object ) ( Q[0]*A[object][4][1] - Q[1]*A[object][3][1] )

#define	XPS_0( Q, object ) ( Q[1]*S[object][5][1] - Q[2]*S[object][4][1] )
#define	XPS_1( Q, object ) ( Q[2]*S[object][3][1] - Q[0]*S[object][5][1] )
#define	XPS_2( Q, object ) ( Q[0]*S[object][4][1] - Q[1]*S[object][3][1] )

//	Print a 3D vector using mout...
//	-------------------------------
#define	PRINT3D( N )  mout << #N << ": " << N[0] << " : " << N[1] << " : " << N[2] << "\n"; 

//	Print one using cout for the laptop...
//	--------------------------------------
#define	PRINT3Dl( N )  cout << #N << ": " << N[0] << " : " << N[1] << " : " << N[2] << "\n"; 

//	Rotate a vector in an easy-to-read way...
//	-----------------------------------------
#define	R1( sin, cos, X, Y )	( cos*X + sin*Y )
#define	R2( sin, cos, X, Y )	( cos*Y - sin*X )
