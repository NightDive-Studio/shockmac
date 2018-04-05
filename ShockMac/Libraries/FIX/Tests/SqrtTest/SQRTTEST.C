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
#include <stdio.h>
#include <stdlib.h>
#include <FixMath.h>
#include "fix.h"

void test_sqrt (long x)
{
	long res;
	long d1, d2, d3;

	res = fix_sqrt(x);
	res = res >> 8;
	d1 = abs(res * res - x);
	d2 = abs((res+1)*(res+1)-x);
	d3 = abs((res-1)*(res-1)-x);

	if (d2 < d1 || d3 < d1)
	{
		printf ("Error: x = %ld sqrt = %ld\n", x, res);
		exit (0);
	}
}

main ()
{
/*	long i;
	
	for (i = 1; i <= 0x7fffffff; i++)
	{
		if ((i & 0xfffff) == 0)
			printf ("%08lx\n", i);
		test_sqrt (i);
	} */

/*
	AWide	a, b;	
	
	for (long i = 4; i < 100; i += 5)
	{
		a.hi = 100;
		a.lo = 0;
		b.hi = i;
		b.lo = i;
		AsmWideSub(&a, &b);
		printf("100:0 - %d = %d:%x\n", i, a.hi, a.lo);
	}
*/
	long	i, res, wres, ticks;
	
	ticks = TickCount();
	for (i = 0; i < 500000; i++)
		res = quad_sqrt(23, 2551);
	ticks = TickCount() - ticks;
	printf("quad_sqrt() time: %d\n", ticks);
//	char	str[256];
//	printf("Result: %s (%d)\n", fix_sprint(str, res), res);

	wide	a;
	a.hi = 23;
	a.lo = 2551;

	ticks = TickCount();
	for (i = 0; i < 500000; i++)
		wres = WideSquareRoot(&a);
	ticks = TickCount() - ticks;
	printf("WideSquareRoot() time: %d\n", ticks);
//	printf("Result: %s (%d)\n", fix_sprint(str, wres), wres);
}
