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
** fix24.c
**
** fix24 functions moved here from fix.c
**
** see fix.c for descriptions
**
** $Header: r:/prj/lib/src/fix/RCS/fix24.c 1.1 1994/08/11 12:12:11 dfan Exp $
** $Log: fix24.c $
 * Revision 1.1  1994/08/11  12:12:11  dfan
 * Initial revision
 * 
*/

#include <fix.h>
#include <trigtab.h>

fix24 fix24_pyth_dist (fix24 a, fix24 b)
{
	return fix24_sqrt (fix24_mul (a, a) + fix24_mul (b, b));
}

//////////////////////////////
//
// We can use the fix function because the difference in scale doesn't matter.
//

fix24 fix24_fast_pyth_dist (fix24 a, fix24 b)
{
   return (fix_fast_pyth_dist (a, b));
}

//////////////////////////////
//
// We can use the fix function because the difference in scale doesn't matter.
//

fix24 fix24_safe_pyth_dist (fix24 a, fix24 b)
{
   return (fix_safe_pyth_dist (a, b));
}

void fix24_sincos (fixang theta, fix24 *sin, fix24 *cos)
{
	fix_sincos (theta, sin, cos);
	*sin >>= 8;
	*cos >>= 8;
}

fix24 fix24_sin (fixang theta)
{
	return (fix_sin (theta) >> 8);
}

fix24 fix24_cos (fixang theta)
{
	return (fix_cos (theta) >> 8);
}

void fix24_fastsincos (fixang theta, fix24 *sin, fix24 *cos)
{
	*sin = (((short) (sintab[theta >> 8])) >> 6);
	*cos = (((short) (sintab[(theta >> 8) + 64])) >> 6);

	return;
}

fix24 fix24_fastsin (fixang theta)
{
	return (((short) (sintab[theta >> 8])) >> 6);
}

fix24 fix24_fastcos (fixang theta)
{
	return (((short) (sintab[(theta >> 8) + 64])) >> 6);
}

fixang fix24_asin (fix24 x)
{
	// Here and in fix24_acos, we don't need to worry about shifting out
	// important bits of x, since we require x to be between -1.0 and 1.0
	// anyways.

	return (fix_asin (x << 8));
}

fixang fix24_acos (fix24 x)
{
	return (fix_acos (x << 8));
}

fixang fix24_atan2 (fix24 y, fix24 x)
{
	fix24 hyp;										// hypotenuse
	fix24 s, c;									// sine, cosine
	fixang th;									// our answer

	// Get special cases out of the way so we don't have to deal
	// with things like making sure 1 gets converted to 0x7fff and
	// not 0x8000.  Note that we grab the y = x = 0 case here
	if (y == 0)
	{
		if (x >= 0) return 0x0000;
		else return 0x8000;
	}
	else if (x == 0)
	{
		if (y >= 0) return 0x4000;
		else return 0xc000;
	}

	if ((hyp = fix24_pyth_dist (x, y)) == 0)
	{
//		printf ("hey, dist was 0\n");
		return 0;
	}

	// Use fix24_asin or fix24_acos depending on where we are.  We don't want to use
	// fix24_asin if the sin is close to 1 or -1
	s = fix24_div (y, hyp);
	if ((ulong) s < 0x00004000 || (ulong) s > 0xffffc000)
	{												// range is good, use asin
		th = fix24_asin (s);
		if (x < 0)
		{
			if (th < 0x4000) th = 0x8000 - th;
			else th = ~th + 0x8000;			// that is, 0xffff - th + 0x8000
		}
	}
	else
	{												// use acos instead
		c = fix24_div (x, hyp);
		th = fix24_acos (c);
		if (y < 0)
		{
			th = ~th;							// that is, 0xffff - th
		}
	}

#ifdef NO_NEED
	// set high bits based on what quadrant we are in
	th &= 0x3fff;
	th |= (y > 0 ? ( x > 0 ? 0x0000 : 0x4000)
		          : ( x > 0 ? 0xc000 : 0x8000));
#endif

	return th;
}
