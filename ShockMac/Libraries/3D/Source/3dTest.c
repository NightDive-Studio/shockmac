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
// main test code for 3d library (remove if not building test app)

#include <fixmath.h>
#include <string.h>
#include <StdLib.h>
#include "lg.h"
#include "fix.h"
#include "2d.h"
#include "InitMac.h"
#include "ShockBitmap.h"
#if __profile__
#include "Profiler.h"
#endif

WindowPtr	gMainWindow;

extern void test_3d(void);
 
void main (void)
 {
	grs_screen 	*screen;
	Str255			str;
  short 			w,h;
	long				time,i,j;
	grs_bitmap  bm;
	char				temp[256];
	Rect				r;
	Ptr					p;
												
	InitMac();
	CheckConfig();	
	
	SetupWindows(&gMainWindow);								// setup everything
	SetupOffscreenBitmaps();			
					
#if __profile__
	if (!ProfilerInit(collectDetailed, bestTimeBase, 20, 10))
	{
#endif

	test_3d();
	
#if __profile__
		ProfilerDump("\p3dtest.prof");
		ProfilerTerm();
	}
#endif
 
	CleanupAndExit();
}

