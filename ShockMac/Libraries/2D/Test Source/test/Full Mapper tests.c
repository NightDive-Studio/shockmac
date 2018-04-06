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
	// these will run through all the combinations of a mapper
	// (although only for the transparent or opaque mode at once)
	// the also don't exercise the nonlog2 wide bitmaps for the linear mapper

//  stick this line in for the normal init_bm to test nonlog2 maps,
//  and change TempTM so Per_UMap doesn't ignore nonlog2 maps
//	gr_init_bm (&bm, (uchar *) flat8_testbm_log2, BMT_FLAT8, 0, 17, 16);

	// linear
	SetVertexLinear(points);
	gr_poly(2, 4, points);
	gr_per_umap(&bm, 4, points);
	gr_clut_per_umap(&bm, 4, points, (uchar *) test_clut);
	gr_lit_per_umap(&bm, 4, points);
	gr_clear(0xff);
	
	// wall	
	SetVertexWall(points);
	gr_poly(2, 4, points);
	gr_per_umap(&bm, 4, points);
	gr_clut_per_umap(&bm, 4, points, (uchar *) test_clut);
	gr_lit_per_umap(&bm, 4, points);
	gr_clear(0xff);

	// floor
	SetVertexFloor(points);
	gr_poly(2, 4, points);
	gr_per_umap(&bm, 4, points);
	gr_clut_per_umap(&bm, 4, points, (uchar *) test_clut);
	gr_lit_per_umap(&bm, 4, points);
	gr_clear(0xff);

	// perspective(hscan)
	SetVertexPerHScan(points);
	gr_poly(2, 4, points);
	gr_per_umap(&bm, 4, points);
	gr_clut_per_umap(&bm, 4, points, (uchar *) test_clut);
	gr_lit_per_umap(&bm, 4, points);
	gr_clear(0xff);

	// perspective(vscan)
	SetVertexPerVScan(points);
	gr_poly(2, 4, points);
	gr_per_umap(&bm, 4, points);
	gr_clut_per_umap(&bm, 4, points, (uchar *) test_clut);
	gr_lit_per_umap(&bm, 4, points);
	gr_clear(0xff);
#endif
