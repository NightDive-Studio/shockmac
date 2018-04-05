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
// main test code for 2d library (remove if not building test app)

#include <2d.h>
#include <2dRes.h>
#include <fix.h>
#include <FixMath.h>
#include <InitMac.h>
#include <StdLib.h>
#include "lg.h"
#include "ShockBitmap.h"
#include <string.h>
#include <pal.h>

WindowPtr	gMainWindow;

// prototypes
extern void v_umap(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti);

void SetVertexLinear(grs_vertex **points);
void SetVertexFloor(grs_vertex **points);
void SetVertexWall(grs_vertex **points);
void SetVertexPerHScan(grs_vertex **points);
void SetVertexPerVScan(grs_vertex **points);
void Rotate90(grs_vertex **points);

#define make_vertex(_vertex,_x,_y,_u,_v,_w,_i) \
   _vertex.x = fix_make(_x,0), \
   _vertex.y = fix_make(_y,0), \
   _vertex.u = fix_make(_u,0),             \
   _vertex.v = fix_make(_v,0),_vertex.w = _w, _vertex.i = _i;	

uchar pal_buf[768];
uchar flat8_testbm2[4][4] =
   { { 1, 2, 3, 4},
   	 { 5, 6, 7, 8},
   	 { 1, 2, 3, 4},
   	 { 1, 2, 3, 4}};

char test_clut[111] = {0,0,0,0,0,0,0,0,0,0,
											 0,0,0,0,0,0,0,0,0,0,
											 0,0,0,0,0,0,0,0,0,0,
											 0,0,0,0,54,0,0,0,0,0,
											 0,0,0,0,0,0,0,0,0,0,
											 0,0,0,0,0,0,0,0,0,0,
											 0,0,0,0,0,0,0,0,0,0,
											 0,0,0,0,0,0,0,0,0,0,
											 0,0,0,0,0,0,0,0,0,0,
											 0,0,0,0,0,0,0,0,0,0,
											 0,0,0,0,0,0,0,0,0,0,
											 80
											};

uchar flat8_testbm[16][16] =
   { { 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34 },
     { 34, 34,  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,  110, 34, 34 },
     { 34,  110, 34,  110,  110,  110,  110,  110,  110,  110,  110,  110,  110, 34,  110, 34 },
     { 34,  110,  110, 34,  110,  110,  110,  110,  110,  110,  110,  110, 34,  110,  110, 34 },
     { 34,  110,  110,  110, 34,  110,  110,  110,  110,  110,  110, 34,  110,  110,  110, 34 },
     { 34,  110,  110,  110,  110, 34,  110,  110,  110,  110, 34,  110,  110,  110,  110, 34 },
     { 34,  110,  110,  110,  110,  0, 34,  110,  110, 34,  0,  110,  110,  110,  110, 34 },
     { 34,  110,  110,  110,  110,  0,  0, 34, 34,  0,  0,  110,  110,  110,  110, 34 },
     { 34,  110,  110,  110,  110,  0,  0, 34, 34,  0,  0,  110,  110,  110,  110, 34 },
     { 34,  110,  110,  110,  110,  0, 34,  110,  110, 34,  0,  110,  110,  110,  110, 34 },
     { 34,  110,  110,  110,  110, 34,  110,  110,  110,  110, 34,  110,  110,  110,  110, 34 },
     { 34,  110,  110,  110, 34,  110,  110,  110,  110,  110,  110, 34,  110,  110,  110, 34 },
     { 34,  110,  110, 34,  110,  110,  110,  110,  110,  110,  110,  110, 34,  110,  110, 34 },
     { 34,  110, 34,  110,  110,  110,  110,  110,  110,  110,  110,  110,  110, 34,  110, 34 },
     { 34, 34,  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,  110, 34, 34 },
     { 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34 } };

   
char bigblock[100000];

extern "C" {extern fix fix_mul_asm_safe(fix a, fix b);}
     
void main (void)
 {
	grs_screen 	*screen;
	Str255			str;
	Handle			testRes,testRes2,testRes3;
  short 			w,h;
	long				time,i,j;
	grs_bitmap  bm;
	char				temp[256];
  grs_vertex 	v0,v1,v2,v3,v4;
//  grs_vertex 	*points[5]= {&v0,&v1,&v2,&v3,&v4};
  grs_vertex 	*points[5];
	Handle	 		pal;
	Handle			shade1,shade2;
	Rect				r;
	Ptr					p;
	grs_bitmap	*RealBitmapPtr,RealBitmap;
	Ptr					big_buffer;
	FrameDesc		*fd;
	EventRecord evt;
	char				bogus_clut[256];
	grs_canvas 	*cnv;
  grs_tmap_info ti;
	RGBColor	col = {0xFFFF, 0x3333, 0x3333};
	
	points[0] = &v0;
	points[1] = &v1;
	points[2] = &v2;
	points[3] = &v3;
	points[4] = &v4;
	
	InitMac();
	CheckConfig();
	
	SetupWindows(&gMainWindow);								// setup everything
	SetupOffscreenBitmaps();			
				
	gr_init();
	gr_set_mode (GRM_640x480x8, TRUE);
	screen = gr_alloc_screen (grd_cap->w, grd_cap->h);
	gr_set_screen (screen);
	gr_init_bm (&bm, (uchar *) flat8_testbm, BMT_FLAT8, 0, 16, 16);

	testRes = GetResource('sIMG',1000);
	HLock(testRes);
	fd = (FrameDesc *) *testRes;
	fd->bm.bits = (uchar *)(fd+1);
	grd_unpack_buf = (uchar *) bigblock;
//	gr_rsd8_convert(&fd->bm, &bm);
	bm = fd->bm;
	
	pal = GetResource('pal ',1000);
	BlockMove(*pal, pal_buf, 768L);
  gr_set_pal(0, 256, pal_buf);
  
  SetRect(&r,0,0,420,16);
  EraseRect(&r);
  MoveTo(10,12);  DrawString("\pCreating Inverse palette(normally preloaded in the real game)");
  
 	gr_alloc_ipal();
	gr_init_blend(1);
  gr_clear(0xff);
	
	for (i=0; i<256; i++)
	 bogus_clut[i] = 256-i;
	 
	points[0] = &v0;
	points[1] = &v1;
	points[2] = &v2;
	points[3] = &v3;
	points[3] = &v4;

	shade1 = GetResource('shad',1000);
	HLock(shade1);
	gr_set_light_tab((uchar *) *shade1);

	cnv = gr_alloc_canvas(BMT_FLAT8,640,480);
	gr_set_canvas(cnv);
	gr_clear(0xff);

// ===== test code 		 
/*	 
	gr_set_canvas(grd_screen_canvas);
	gr_bitmap(&bm, 0,300);
{	
extern void flat8_flat8_smooth_hv_double_ubitmap(grs_bitmap *src, grs_bitmap *dst);
extern void flat8_flat8_smooth_h_double_ubitmap(grs_bitmap *src, grs_bitmap *dst);
grs_bitmap dst;

	dst.row=2*bm.w;
	dst.bits=grd_unpack_buf;

	flat8_flat8_smooth_h_double_ubitmap(&bm,&dst);
	gr_bitmap(&dst, 32,32);
}
*/

	
  gr_set_fcolor(33);
//  SetVertexFloor(points);
//  SetVertexLinear(points);
  SetVertexPerHScan(points);
//  SetVertexPerVScan(points);
//  SetVertexWall(points);
  cnv->bm.flags = 0;
  
//	gr_set_canvas(grd_screen_canvas);

//      ti.tmap_type=GRC_LIT_WALL2D;
//      ti.flags=TMF_WALL;
//      v_umap(&bm,4,points,&ti);
     
//  gr_set_fill_type(FILL_SOLID);
//  gr_set_fill_parm(33);

	// ��Setup for forcing 1D wall mapper
//  ti.tmap_type=GRC_LIT_WALL1D;
//  ti.flags=TMF_WALL;

  time = TickCount();  
  for (i=0; i<1000; i++)
//	  gr_per_umap(&bm, 4, points);
		gr_clut_per_umap(&bm, 4, points, (uchar *) bogus_clut);
//		gr_lit_per_umap(&bm, 4, points);
//  		v_umap(&bm,4,points,&ti);		//�� for wall 1D
 /*{
extern void per_umap (grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti);
      grs_tmap_info ti;
      ti.tmap_type=GRC_CLUT_PER;
      ti.flags=TMF_CLUT;
      ti.clut=(uchar *) bogus_clut;
      per_umap(&bm,4,points,&ti);
 }
 */
/*
 {
	extern int h_map(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti);
  grs_tmap_info ti;
	ti.tmap_type = GRC_TRANS_LIT_BILIN;	
	ti.flags = 0;
	h_map(&bm, 4, points, &ti);
	
}*/


//  	gr_clear(i);
 /* {
extern void v_umap(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti);
      grs_tmap_info ti;
      ti.clut = (uchar *) bogus_clut;
      ti.tmap_type=GRC_CLUT_WALL1D;
      ti.flags=TMF_WALL|TMF_CLUT;
      v_umap(&bm,4,points,&ti);
  }*/

  	
  time = TickCount()-time;

	gr_set_canvas(grd_screen_canvas);
	gr_bitmap(&cnv->bm,0,0);

  NumToString(time,str);
//  DebugStr(str);  
  MoveTo(10,420);
	RGBForeColor(&col);
  DrawString(str);
        
       
 {EventRecord evt; while (true){if (GetNextEvent(mDownMask+keyDownMask,&evt))
  {if (evt.what==mouseDown) break;}}}
 
	gr_close();
	CleanupAndExit();
}

void Rotate90(grs_vertex **points)
 {
 	int i;
	fix	u,v;
	
	u = points[0]->u; v = points[0]->v;
	
	for (i=0; i<3; i++)
	 {points[i]->u = points[i+1]->u; points[i]->v = points[i+1]->v;}
	 
 	points[3]->u = u; points[3]->v = v;
 }
/*
void SetVertexLinear(grs_vertex **points)
 {
  make_vertex((*(points[0])),101,		100,		0,		0,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
  make_vertex((*(points[1])),200,		120,		128,		0,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
  make_vertex((*(points[2])),200,		180,		128,		128,		FixDiv(FIX_UNIT,fix_make(10,0)), 16*FIX_UNIT-1);
  make_vertex((*(points[3])),101,		200,		0,		128,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
 } */
 
void SetVertexLinear(grs_vertex **points)
 {
  make_vertex((*(points[0])),250,		148,		0,		0,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
  make_vertex((*(points[1])),327,		148,		128,		0,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
  make_vertex((*(points[2])),327,		222,		128,		128,		FixDiv(FIX_UNIT,fix_make(10,0)), 16*FIX_UNIT-1);
  make_vertex((*(points[3])),250,		222,		0,		128,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
 } 



void SetVertexFloor(grs_vertex **points)
 {
  make_vertex((*(points[0])),101,		100,		0,		0,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
  make_vertex((*(points[1])),200,		100,		128,		0,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
  make_vertex((*(points[2])),181,		200,		128,		128,		FixDiv(FIX_UNIT,fix_make(20,0)), 16*FIX_UNIT-1);
  make_vertex((*(points[3])),121,		200,		0,		128,		FixDiv(FIX_UNIT,fix_make(20,0)), 0);
 }
 

void SetVertexWall(grs_vertex **points)
 {
  make_vertex((*(points[0])),101,		100,		0,		0,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
  make_vertex((*(points[1])),220,		120,		128,		0,		FixDiv(FIX_UNIT,fix_make(20,0)), 0);
  make_vertex((*(points[2])),220,		210,		128,		128,		FixDiv(FIX_UNIT,fix_make(20,0)), 16*FIX_UNIT-1);
  make_vertex((*(points[3])),101,		230,		0,		128,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
 }
 

void SetVertexPerHScan(grs_vertex **points)
 {
  make_vertex((*(points[0])),101,		100,		0,		0,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
  make_vertex((*(points[1])),200,		120,		128,		0,		FixDiv(FIX_UNIT,fix_make(12,0)), 0);
  make_vertex((*(points[2])),180,		200,		128,		128,		FixDiv(FIX_UNIT,fix_make(20,0)), 16*FIX_UNIT-1);
  make_vertex((*(points[3])),105,		200,		0,		128,		FixDiv(FIX_UNIT,fix_make(14,0)), 0);
 }
 
void SetVertexPerVScan(grs_vertex **points)
 {
  make_vertex((*(points[0])),101,		100,		0,		0,		FixDiv(FIX_UNIT,fix_make(10,0)), 0);
  make_vertex((*(points[1])),200,		120,		128,		0,		FixDiv(FIX_UNIT,fix_make(20,0)), 0);
  make_vertex((*(points[2])),180,		200,		128,		128,		FixDiv(FIX_UNIT,fix_make(20,0)), 16*FIX_UNIT-1);
  make_vertex((*(points[3])),105,		200,		0,		128,		FixDiv(FIX_UNIT,fix_make(12,0)), 0);
 }
 