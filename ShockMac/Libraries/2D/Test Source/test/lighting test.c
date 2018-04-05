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
#if 1
	big_buffer = NewPtr(100000);
  gr_set_unpack_buf((uchar *) big_buffer);			// Do this somewhere
	StandardGetFile(0L, -1, 0L, &reply);
	PaintRect(&gMainWindow->portRect);
	if (reply.sfGood)
	 {
	 	resNum = ResOpenFile(&reply.sfFile);
		if (resNum != -1)
		 {
	   	p = (Ptr) RefLock(MKREF(1270, 0));
	   	FrameDesc	*fd = (FrameDesc *)p;
	   	fd->bm.bits = (uchar *)(fd+1);
	   	if (fd->bm.type == 4)
	   	 {
				gr_rsd8_convert(&fd->bm, &RealBitmap);
				RealBitmapPtr = &RealBitmap;
			 }
			else
				RealBitmapPtr = &fd->bm;
		 }
	 }
	else
	 {
		gr_close();
		ExitToShell();
	 }

// 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=-----
  make_vertex(v0, 0,			0,		0,		0,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
  make_vertex(v1, 200,		0,		128,		0,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
  make_vertex(v2, 200,		200,		128,		128,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
  make_vertex(v3, 0,			200,		0,		128,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
  
  do
   {
 	 	Point pt;
 	 	fix		mousex,mousey,dist,intens;
 	 	int		i;
 	 	
	 	points[0]->x = fix_make(0,0);
	 	points[1]->x = fix_make(200,0);
	 	points[2]->x = fix_make(200,0);
	 	points[3]->x = fix_make(0,0);

 	 	GetMouse(&pt);
 	 	mousex = pt.h<<16;
 	 	mousey = pt.v<<16;
 	 		 	
 	 	for (j=0; j<2; j++)
 	 	 {
	 	 	for (i=0; i<4; i++)
	 	 	 {
	 	 	 	dist = fix_safe_pyth_dist(fix_abs(mousex-points[i]->x), fix_abs(mousey-points[i]->y));
	 	 		points[i]->i = FixDiv(dist, fix_make(18,0));
	 	 		if (points[i]->i>=(16*FIX_UNIT-1)) points[i]->i = 16*FIX_UNIT-1;
	 	 		if (points[i]->i<0) points[i]->i = 0;
	 	 	 } 	 	
	
	  	gr_lit_per_umap(RealBitmapPtr, 4, points);
	  	
	  	for (i=0; i<4; i++)
	  	 	points[i]->x+=fix_make(200,0);
	   }
   }
  while (!Button()); 
  
	ResCloseFile(resNum);
	gr_close();
	ExitToShell();
// 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=----- 0-=-----
#endif
