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
// ===== test code 		 
	do
	 {
	 	gr_set_fcolor(Random()%0xff);
	 	gr_point(abs(Random() % 640), abs(Random() % 480));
	 }
	while (!GetNextEvent(mDownMask,&evt));

	do
	 {
	 	gr_set_fcolor(Random()%0xff);
	 	gr_int_circle(abs(Random() % 640), abs(Random() % 480), abs(Random() % 70));
	 }
	while (!GetNextEvent(mDownMask,&evt));

	do
	 {
	 	gr_set_fcolor(Random()%0xff);
	 	gr_rect(abs(Random() % 640), abs(Random() % 480), abs(Random() % 640), abs(Random() % 480));
	 }
	while (!GetNextEvent(mDownMask,&evt));


// end test code
