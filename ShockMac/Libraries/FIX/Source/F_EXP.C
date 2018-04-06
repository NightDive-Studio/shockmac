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
** f_exp.c
**
** fix_exp
**
** $Header: r:/prj/lib/src/fix/RCS/f_exp.c 1.1 1994/08/11 12:12:16 dfan Exp $
** $Log: f_exp.c $
 * Revision 1.1  1994/08/11  12:12:16  dfan
 * Initial revision
 * 
*/

#include "fix.h"
#include "trigtab.h"

//////////////////////////////
//
// returns e to the x
// does no range checking whatsoever
//
fix fix_exp (fix x)
{
	int int_part = fix_int (x);
	fix exp_int_part;
	int basex, fracx;
	fix loy, hiy;
	fix exp_frac_part;
	
	// If our exponent is so small that it goes off the small end of the table,
	// just return 0.
	
	if (int_part + INTEGER_EXP_OFFSET < 0) return 0;
	
	exp_int_part = expinttab[int_part + INTEGER_EXP_OFFSET];
	
	basex = fix_frac (x) >> 12;
	fracx = x & 0x0fff;
	
	loy = expfractab[basex];
	hiy = expfractab[basex+1];
	
	exp_frac_part = loy + (hiy - loy) * fracx / 0x1000;
	return (fix_mul (exp_int_part, exp_frac_part));
}
