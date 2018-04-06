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
#include <Quickdraw.h>
#include <FixMath.h>

typedef long Fixed;
extern Fixed AsmFixedDiv(Fixed a, Fixed b);

static void TestAsmFixDiv(unsigned long testCount);

void main (void)
{
	printf("Test for FixDiv\n\n");
	InitGraf(&qd);
	
	TestAsmFixDiv(10000000);
	
}

void TestAsmFixDiv(unsigned long testCount)
{
	Fixed 				a, b, qAsm, qReal;
	Fixed				diff;
	unsigned long		curCount = testCount;
	
	curCount = 0;
	
	while (curCount < testCount) 
	{
		a = (Random() << 16) + Random();
		b = (Random() << 16) + Random();
		
		qAsm = AsmFixedDiv(a, b);
		qReal = FixDiv(a, b);
		
		diff = qAsm - qReal;
		if (diff < 0) diff = -diff;

		if (diff > 0x1) // is it close?
			printf("0x%lx divided by 0x%lx is incorrect 0x%lx (vs. 0x%lx)\n", a, b, qAsm, qReal);
		
		curCount++;
		if (curCount % 100000 == 0)
			printf("%d tests complete.\n", curCount);
	}
	
	printf("\n\n%d tests complete.\n\n", testCount);
}





