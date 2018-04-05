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
	gr_set_mode(GRM_640x480x8,TRUE);
	screen=gr_alloc_screen(grd_cap->w,grd_cap->h);
	gr_set_screen(screen);
  gr_set_font ((grs_font *) *testRes);

	points[0] = &v0;
	points[1] = &v1;
	points[2] = &v2;
	 	
	do
	 {
	 	Point	pt;
	 	
	 	GetMouse(&pt);
	 	pt.h -= 50;
	 	pt.v -= 50;
	  make_vertex(v0,100+pt.h,100+pt.v,0,0);
	  make_vertex(v1,150+pt.h,150+pt.v,0,0);
	  make_vertex(v2,50+pt.h,150+pt.v,0,0);
	 	
	 	gr_clear(0xff);
		gr_cpoly(1,3,points);
	 }
	while (!Button());
*/
