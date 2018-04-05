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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <fixmath.h>
#include "fix.h"

#define PI 3.14159265358979323846

void main()
{
   fixang i = 0;
   fix s, cs, a, b;
   double id;                          // in radians
   double sd, cd;
   double serr = 0;
   double cerr = 0;
   double sinerr, coserr;
   double maxsinerr = 0;
   double maxcoserr = 0;
   int	c;
   long t;

   do
   {
      s = fix_sin(i);
      cs = fix_cos(i);

      id = (i * PI) / ((double) 0x8000);
      sd = sin(id);
      cd = cos(id);

      sinerr = fabs ((double) fix_float (s) - sd);
      if (sinerr > maxsinerr) maxsinerr = sinerr;
      serr += sinerr;

      coserr = fabs ((double) fix_float (cs) - cd);
      if (coserr > maxcoserr) maxcoserr = coserr;
      cerr += coserr;

      i++;
   }
   while (i != 0);

   printf ("fix_sin() results:\n");
   printf ("  total err %lf, avg err %lf, max err %lf\n\n", serr, serr / 65536, maxsinerr);

   printf ("fix_cos() results:\n");
   printf ("  total err %lf, avg err %lf, max err %lf\n\n", cerr, cerr / 65536, maxcoserr);

/*
   i = 0; err = 0; maxerr = 0;
   do
   {
      s = fix_fastsin (i);

      id = (i * PI) / ((double) 0x8000);
      sd = sin (id);

      thiserr = fabs ((double) fix_float (s) - sd);
      if (thiserr > maxerr) maxerr = thiserr;
      err += thiserr;

      i++;
   }
   while (i != 0);

   printf ("fix_fastsin() results:\n");
   printf ("  total err %lf, avg err %lf, max err %lf\n\n", err, err / 65536, maxerr);
*/
	printf ("\nCompare fix_div with native double divides:\n");
	
	a = fix_make(101,0xf43e);
	b = fix_make(7,0x8000);
	t = TickCount();
	for (c=0; c < 100000; c++)
	{
		s = fix_div(a, b);
	}
	t = TickCount() - t;
	printf ("  time for 100,000 fix divides: %d ticks\n", t);

	id = 101.8559;
	sd = 7.5;
	t = TickCount();
	for (c=0; c < 100000; c++)
	{
		serr = id / sd;
	}
	t = TickCount() - t;
	printf ("  time for 100,000 double divides: %d ticks\n", t);

	t = TickCount();
	for (c=0; c < 100000; c++)
	{
		id = Fix2X((Fixed)a);
		sd = Fix2X((Fixed)b);
		serr = id / sd;
		s = (fix)X2Fix(serr);
	}
	t = TickCount() - t;
	printf ("  time for 100,000 double divides with fix conversion: %d ticks\n", t);
}
