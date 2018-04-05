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
#if 0
 {
 	char 	side;
 	fix		total,amt1,amt2;
 	char	whichverts[][2] = {{0,1},{1,2},{2,3},{3,0}};
 	
 	side = 0;
 	total = 16*FIX_UNIT-1;
 	amt1 = 0;
 	amt2 = total;

  make_vertex(v0,100,		100,		0,		0,		FixDiv(FIX_UNIT,fix_make(10,0)), total);
  make_vertex(v1,200,		120,		16,		0,		FixDiv(FIX_UNIT,fix_make(10,0)), total);
  make_vertex(v2,200,		180,		16,		16,		FixDiv(FIX_UNIT,fix_make(10,0)), total);
  make_vertex(v3,100,		200,		0,		16,		FixDiv(FIX_UNIT,fix_make(10,0)), total);
 	
	do
	 {
	 	amt1 += FIX_UNIT;
	 	amt2 -= FIX_UNIT;
	 	if (amt1>16*FIX_UNIT-1)
	 	 {
	 	 	points[whichverts[side][0]]->i = total;
	 	 	if (++side==4) side = 0;
		 	amt1 = 0;
		 	amt2 = total;
	 	 }
	 	
	 	points[whichverts[side][0]]->i = amt1;
	 	points[whichverts[side][1]]->i = amt2;
		gr_lit_per_umap(&bm, 4, points);
	 }
	while (!Button());
 }
#endif
