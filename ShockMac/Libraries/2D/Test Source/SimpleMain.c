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
// simple test code for 2d library 

#define Mac 1

#include <2d.h>
#include <fix.h>
#include "lg.h"
#include <Stdio.h>

#if Mac
#include <InitMac.h>
#include "ShockBitmap.h"
#endif

#if Mac
WindowPtr gMainWindow;
#endif

// prototypes
void SetVertexLinear(grs_vertex **points);
void SetVertexFloor(grs_vertex **points);
void SetVertexWall(grs_vertex **points);
void SetVertexPerHScan(grs_vertex **points);
void SetVertexPerVScan(grs_vertex **points);
void WaitKey(void);

#if Mac
void DoTest(void);
#endif

#define make_vertex(_vertex,_x,_y,_u,_v,_w,_i) \
	 _vertex.x = fix_make(_x,0), \
	 _vertex.y = fix_make(_y,0), \
	 _vertex.u = fix_make(_u,0),             \
	 _vertex.v = fix_make(_v,0),_vertex.w = _w, _vertex.i = _i; 

#if Mac
#define clear_color 0xff
#else
#define clear_color 0
#endif

#if Mac
void main (void)
 {    
	InitMac();
	CheckConfig();
	
	SetupWindows(&gMainWindow);               // setup everything
	SetupOffscreenBitmaps();      
				
	DoTest();     
 
	ExitToShell();
 }
#endif

uchar pal_buf[768];
uchar bitmap_buf[17000];
uchar shade_buf[4096];

#if Mac
void DoTest(void)
#else
void main(void)
#endif
 {      
	FILE        *fp;
	grs_screen  *screen;
	char        *fd;
	grs_bitmap  bm;
	grs_vertex  v0,v1,v2,v3;
	grs_vertex  *points[4];

	gr_init();
	gr_set_mode (GRM_640x480x8, TRUE);
	screen = gr_alloc_screen (grd_cap->w, grd_cap->h);
	gr_set_screen (screen);

	fp = fopen("test.img","rb");
	fread (bitmap_buf, 1, 16412, fp);
	fclose (fp);

	bm = * (grs_bitmap *) bitmap_buf;
	bm.bits = bitmap_buf+28;
	
	fp = fopen("test.pal","rb");
	fread (pal_buf, 1, 768, fp);
	fclose (fp);

	gr_set_pal(0, 256, pal_buf);
		
	gr_alloc_ipal();
	gr_init_blend(1);
	gr_clear(clear_color);
	
	points[0] = &v0;
	points[1] = &v1;
	points[2] = &v2;
	points[3] = &v3;

	fp = fopen("test.shd","rb");
	fread (shade_buf, 1, 4096, fp);
	fclose (fp);
	gr_set_light_tab(shade_buf);

// ==
	// linear
	SetVertexLinear(points);
	gr_poly(2, 4, points);
	WaitKey();
	gr_per_umap(&bm, 4, points);
	WaitKey();
//  gr_clut_per_umap(&bm, 4, points, (uchar *) test_clut);
	gr_lit_per_umap(&bm, 4, points);
	WaitKey();
	gr_clear(clear_color);
	
	// wall 
	SetVertexWall(points);
	gr_poly(2, 4, points);
	WaitKey();
	gr_per_umap(&bm, 4, points);
	WaitKey();
	gr_lit_per_umap(&bm, 4, points);
	WaitKey();
	gr_clear(clear_color);

	// floor
	SetVertexFloor(points);
	gr_poly(2, 4, points);
	WaitKey();
	gr_per_umap(&bm, 4, points);
	WaitKey();
	gr_lit_per_umap(&bm, 4, points);
	WaitKey();
	gr_clear(clear_color);

	// perspective(hscan)
	SetVertexPerHScan(points);
	gr_poly(2, 4, points);
	WaitKey();
	gr_per_umap(&bm, 4, points);
	WaitKey();
	gr_lit_per_umap(&bm, 4, points);
	WaitKey();
	gr_clear(clear_color);

	// perspective(vscan)
	SetVertexPerVScan(points);
	gr_poly(2, 4, points);
	WaitKey();
	gr_per_umap(&bm, 4, points);
	WaitKey();
	gr_lit_per_umap(&bm, 4, points);
	WaitKey();
	gr_clear(clear_color);
		
// ===== test code     
/*  SetVertexPerHScan(points);
	SetVertexLinear(points);
	bm.flags = 0;
		
//  gr_set_fill_type(FILL_SOLID);
//  gr_set_fill_parm(33);
	time = TickCount();  
	for (i=0; i<100; i++)
//    gr_clear(i);
//    gr_per_umap(&bm, 4, points);
//    gr_clut_per_umap(&bm, 4, points, (uchar *) test_clut);
		gr_lit_per_umap(&bm, 4, points);

	time = TickCount()-time;
	NumToString(time,str);
//  DebugStr(str);  
		 */   
 
	gr_close();
}

void WaitKey(void)
 {
#if Mac
 {EventRecord evt; while (true){if (GetNextEvent(mDownMask+keyDownMask,&evt))
	{if (evt.what==mouseDown) break;}}}
#else
	while (!getch());
#endif
 }
 

void SetVertexLinear(grs_vertex **points)
 {
	make_vertex((*(points[0])),100,   100,    0,    0,    fix_div(FIX_UNIT,fix_make(10,0)), 0);
	make_vertex((*(points[1])),200,   120,    128,    0,    fix_div(FIX_UNIT,fix_make(10,0)), 0);
	make_vertex((*(points[2])),200,   180,    128,    128,    fix_div(FIX_UNIT,fix_make(10,0)), 16*FIX_UNIT-1);
	make_vertex((*(points[3])),100,   200,    0,    128,    fix_div(FIX_UNIT,fix_make(10,0)), 0);
 }
 

void SetVertexFloor(grs_vertex **points)
 {
	make_vertex((*(points[0])),100,   100,    0,    0,    fix_div(FIX_UNIT,fix_make(10,0)), 0);
	make_vertex((*(points[1])),200,   100,    128,    0,    fix_div(FIX_UNIT,fix_make(10,0)), 0);
	make_vertex((*(points[2])),180,   200,    128,    128,    fix_div(FIX_UNIT,fix_make(20,0)), 16*FIX_UNIT-1);
	make_vertex((*(points[3])),120,   200,    0,    128,    fix_div(FIX_UNIT,fix_make(20,0)), 0);
 }
 

void SetVertexWall(grs_vertex **points)
 {
	make_vertex((*(points[0])),100,   100,    0,    0,    fix_div(FIX_UNIT,fix_make(10,0)), 0);
	make_vertex((*(points[1])),200,   120,    128,    0,    fix_div(FIX_UNIT,fix_make(20,0)), 0);
	make_vertex((*(points[2])),200,   180,    128,    128,    fix_div(FIX_UNIT,fix_make(20,0)), 16*FIX_UNIT-1);
	make_vertex((*(points[3])),100,   200,    0,    128,    fix_div(FIX_UNIT,fix_make(10,0)), 0);
 }
 

void SetVertexPerHScan(grs_vertex **points)
 {
	make_vertex((*(points[0])),100,   100,    0,    0,    fix_div(FIX_UNIT,fix_make(10,0)), 0);
	make_vertex((*(points[1])),200,   120,    128,    0,    fix_div(FIX_UNIT,fix_make(12,0)), 0);
	make_vertex((*(points[2])),180,   200,    128,    128,    fix_div(FIX_UNIT,fix_make(20,0)), 16*FIX_UNIT-1);
	make_vertex((*(points[3])),105,   200,    0,    128,    fix_div(FIX_UNIT,fix_make(14,0)), 0);
 }
 
void SetVertexPerVScan(grs_vertex **points)
 {
	make_vertex((*(points[0])),100,   100,    0,    0,    fix_div(FIX_UNIT,fix_make(10,0)), 0);
	make_vertex((*(points[1])),200,   120,    128,    0,    fix_div(FIX_UNIT,fix_make(20,0)), 0);
	make_vertex((*(points[2])),180,   200,    128,    128,    fix_div(FIX_UNIT,fix_make(20,0)), 16*FIX_UNIT-1);
	make_vertex((*(points[3])),105,   200,    0,    128,    fix_div(FIX_UNIT,fix_make(12,0)), 0);
 }
 
