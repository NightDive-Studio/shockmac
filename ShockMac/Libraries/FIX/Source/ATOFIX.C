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
** atofix.c
**
** $Header: n:/project/lib/src/fix/RCS/atofix.c 1.2 1993/11/11 13:50:29 rex Exp $
** $Log: atofix.c $
 * Revision 1.2  1993/11/11  13:50:29  rex
 * Fixed bug in atofix24
 * 
 * Revision 1.1  1993/11/11  13:30:53  rex
 * Initial revision
 * 
 * 
*/

#include <stdlib.h>
#include <stdio.h>

#include "fix.h"

static void fixgetab(char *p, short fracshift, short *a, short *b, short *sign);

//	----------------------------------------------------------
//		CONVERSION ROUTINES
//	----------------------------------------------------------
//
//	atofix() converts an ascii string into a fixed-point number

fix atofix(char *p)
{
	short a, b, sign;

	fixgetab(p, 16, &a, &b, &sign);
	return(sign*fix_make(a,b));
}

//	----------------------------------------------------------
//
//	atofix24() converts an ascii string into a fix24
/*
fix24 atofix24(char *p)
{
	int a,b,sign;

	fixgetab(p,8,&a,&b,&sign);
	return(sign*fix24_make(a,b));
}
*/

//	-----------------------------------------------------------
//		INTERNAL ROUTINES
//	-----------------------------------------------------------
//
//	fixgetab() gets integer and fractional part from ascii buffer

static void fixgetab(char *p, short fracshift, short *a, short *b, short *sign)
{
	short divis;

//	sign is +1 or -1

	*sign = 1;
	if (*p == '-')
	{
		*sign = -1;
		++p;
	}

//	Get integer portion

	*a = *b = 0;
	while ((*p >= '0') && (*p <= '9'))
		*a = (*a * 10) + (*p++ - '0');

//	If period, get fractional portion

	if (*p == '.')
	{
		++p;
		divis = 10;
		while ((*p >= '0') && (*p <= '9'))
		{
			*b += ((*p++ - '0') << fracshift) / divis;
			divis *= 10;
			if (divis > 655360)
				break;
		}
	}
}
