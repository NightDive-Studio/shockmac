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
/* fmaketab.cc
**
** Makes the tables for fix.c
**
** This is perhaps the worst possible application for C++
**
** $Header: r:/prj/lib/src/fix/RCS/fmaketab.cc 1.6 1994/08/11 12:11:48 dfan Exp $
**
** $Log: fmaketab.cc $
**Revision 1.6  1994/08/11  12:11:48  dfan
**has to output a little different now that it makes a c file
**
**Revision 1.5  1993/12/31  09:38:41  dfan
**I was wrong.  It rounds towards 0, not to the nearest integer.
**So we add .5 in the appropriate direction
**
**Revision 1.4  1993/12/30  17:42:08  dfan
**Hey wow, watcom rounds doubles to ints, rather than truncates
**which means that I shouldn't have added .5
**Note that to be completely compiler/independent, I should
**switch on the value of FLT_ROUNDS (in float.h), but forget that
**
**Revision 1.3  1993/07/30  12:43:07  dfan
**Exponential tables
**
** Revision 1.2  1993/06/03  11:52:21  dfan
** round instead of truncate
** 
** Revision 1.1  1993/06/01  14:15:33  dfan
** Initial revision
** 
*/

#include <iostream.h>
//#include <iomanip.h>
#include <math.h>
#include <stdlib.h>
#include "lg_types.h"

const int entries_per_line = 8;
const double two_pi = 6.283185306 ;

ushort sins[256];
ushort asins[129];

void do_exp_table (void);

main ()
{

   cout << "#include \"fix.h\"\n";
   cout << "#include \"trigtab.h\"\n";
   cout << "\n";
	cout << "// First, the sine table.\n";
	cout << "// The sine table is indexed by the top 8 bits of a fixang.\n";
	cout << "// Its units are fix >> 2 (so -1 = 0xc000, 0 = 0x0000, 1 = 0x4000)\n";
	cout << "// This means that the high bit is almost useless, but otherwise\n";
	cout << "//   we get results like sin(PI/2) = 0.fffe rather than 1.0000.\n";
	cout << "//\n";
	cout << "// cos[x] = sin[x + 64].\n";
	cout << "\n";

	int i;
	for (i = 0; i < 256; i++)
	{
		double th = sin (i * two_pi / 256.0);
		double nth = th * 16384.0;			// normalized theta
		nth += 0.5; 						// Do always to match PC output table
		sins[i] = (ushort) nth;		
	}
	
	cout.setf (ios::showbase);
	cout.setf (ios::internal, ios::adjustfield);

	cout << "ushort sintab[256+64+1] = { // indexed by fixang" << endl;

	i = 0;
	while (i < 256)
	{
		int j;
		
		cout << "\t";
		for (j = 0; j < entries_per_line; j++)
		{
			cout.width(6);
			cout.fill('0');
			cout << hex << sins[i+j] << ", ";
//			cout << hex << cout.width(6) << cout.fill('0') << sins[i+j] << ", ";
		}
		cout << endl;
		i += j;
	}

	i = 0;
	while (i < 64)
	{
		int j;
		
		cout << "\t";
		for (j = 0; j < entries_per_line; j++)
		{
			cout.width(6);
			cout.fill('0');
			cout << hex << sins[i+j] << ", ";
//			cout << hex <<	cout.width(6) << cout.fill ('0') << sins[i+j] << ", ";
		}
		cout << endl;
		i += j;
	}
	cout << "\t";
	cout.width(6);
	cout.fill('0');
	cout << hex << sins[i] << endl
		 << "};" << endl << endl;
//	cout << "\t" << hex << cout.width(6) << cout.fill ('0') << sins[i] << endl
//		<< "};" << endl << endl;

	cout << "// Now the arcsin table.\n";
	cout << "// The arcsin table is indexed by (((fix >> 2) + 0x4000) & 0xffff).\n";
	cout << "// That means -1 = 0xc000 + 0x4000 = 0x0000,\n";
	cout << "//             0 = 0x0000 + 0x4000 = 0x4000,\n";
	cout << "//             1 = 0x4000 + 0x4000 = 0x8000.\n";
	cout << "// So the high bit is almost useless, but otherwise we have problems\n";
	cout << "//   trying to differentiate between 1 and -1.\n";
	cout << "// Its units are fixangs.\n";
	cout << "//\n";
	cout << "// acos(x) = PI/2 - asin(x).   (PI/2 is fixang 0x4000)\n";
	cout << "\n";

	for (i = 0; i <= 128; i++)
	{
		double th = asin (-1.0 + i / 64.0);	// the argument ranges from -1.0 to 1.0
		double nth = (th / two_pi) * 65536.0;
		nth += 0.5;							// Do this always to match PC table output.
		asins[i] = (ushort) nth;
	}

	cout << "// indexed by (high 8 bits of (fix >> 2 + 0x4000)" << endl;
	cout << "fixang asintab[128+1] = { " << endl;
	i = 0;
	while (i < 128)
	{
		int j;
		
		cout << "\t";
		for (j = 0; j < entries_per_line; j++)
		{
			cout.width(6);
			cout.fill('0');
			cout << hex << asins[i+j] << ", ";
		}
		cout << endl;
		i += j;
	}
	cout << "\t";
	cout.width(6);
	cout.fill('0');
	cout << hex << asins[i] << endl
		 << "};" << endl << endl;
	
	do_exp_table ();

	exit (0);
}

#define INTEGER_EXP_OFFSET 11

void do_exp_table (void)
{
	cout << "// There are two exp tables.  The first is for integer exponents.\n";
	cout << "// The table only goes from -11 to 11 because that's all that will\n";
	cout << "// fit in a 16:16 fixed point number.  Add INTEGER_EXP_OFFSET to\n";
	cout << "// your exponent before looking it up in the table.\n\n";

	cout.unsetf (ios::showbase);
	cout << "#define INTEGER_EXP_OFFSET " << dec << INTEGER_EXP_OFFSET << endl << endl;

	ulong exps[INTEGER_EXP_OFFSET*2 + 1];
	int i;
	for (i = -INTEGER_EXP_OFFSET; i <= INTEGER_EXP_OFFSET; i++)
	{
		double e = exp (i) * 65536.0 + 0.5;
		exps[i+INTEGER_EXP_OFFSET] = (ulong) e;
	}

	cout.setf (ios::showbase);
	cout.setf (ios::internal, ios::adjustfield);

	cout << "ulong expinttab[INTEGER_EXP_OFFSET*2+1] = {" << endl;
	i = 0;
	int exp_entries_per_line = 4;
	while (i < INTEGER_EXP_OFFSET * 2 + 1)
	{
		int j;
		
		cout << "\t";
		for (j = 0; j < exp_entries_per_line && j+i < INTEGER_EXP_OFFSET * 2 + 1; j++)
		{
			cout.width(10);
			cout.fill('0');
			cout << hex << exps[i+j] << ", ";
		}
		cout << endl;
		i += j;
	}
	cout << "};\n\n";

	cout << "// Now for the fractional table, which currently has 16+1 values,\n";
	cout << "// which should be interpolated between.  We can crank up the\n";
	cout << "// accuracy later if we need it.  So this input to this table goes\n";
	cout << "// from 0 to 1 by sixteenths.\n\n";

	ulong fracexps[16+1];
	for (i = 0; i <= 16; i++)
	{
		double e = exp (i/16.0) * 65536.0 + 0.5;
		fracexps[i] = (ulong) e;
	}

	cout.setf (ios::showbase);
	cout.setf (ios::internal, ios::adjustfield);

	cout << "ulong expfractab[16+1] = {" << endl;
	i = 0;
	while (i < 16+1)
	{
		int j;
		
		cout << "\t";
		for (j = 0; j < exp_entries_per_line && j+i < 16+1; j++)
		{
			cout.width(10);
			cout.fill('0');
			cout << hex << fracexps[i+j] << ", ";
		}
		cout << endl;
		i += j;
	}
	cout << "};\n\n";
}
