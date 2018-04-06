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
	testRes = GetResource('sFNT',608);
	HLock(testRes);
	testRes2 = GetResource('sFNT',602);
	HLock(testRes2);
	testRes3 = GetResource('sFNT',609);
	HLock(testRes3);

  gr_set_font ((grs_font *) *testRes2);
  for (i=0; i<4; i++)
   {
  	gr_set_fcolor((i+1)*25);
  	gr_string("A monochrome font, in several colors", 8, 32+(i*8));
 	 }

  gr_string_size("A scaled monochrome font, in several colors",&w,&h);
  for (i=0; i<4; i++)
   {
  	gr_set_fcolor((i+1)*30);
		gr_scale_ustring ("A scaled monochrome font, in several colors", 8, 64+(i*16),w<<1, h<<1);
 	 }
 	 
  gr_set_font ((grs_font *) *testRes3);
  gr_string("A.COLOR.FONT.012345", 8, 150);
  gr_string("SCALED.COLOR.FONTS.BELOW:", 8, 180);

  gr_set_font ((grs_font *) *testRes);
  
  gr_ustring("0123456789", 8, 250);
    
  gr_string_size("0123456789",&w,&h);
	gr_scale_ustring ("0123456789", 8, 300,w>>1, h>>1);
	gr_scale_ustring ("0123456789", 8, 320,w<<1, h<<1);

// end test code
