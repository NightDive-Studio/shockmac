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

#include <StdLib.h>
#include <string.h>

#include "InitMac.h"
#include "ShockBitmap.h"

#include "lg.h"
#include "fix.h"
#include "2d.h"
#include "2dRes.h"
#include "3d.h"
#include "pal.h"
#include "res.h"
#include "vox.h"
#include "kb.h"
#include "kbcook.h"


WindowPtr	gMainWindow;

#define build_fix_angle(ang) ((65536*(ang))/360)

// prototypes
void voxel_convert(grs_bitmap *bmp);
Boolean IsOptKeyDown(void);
Boolean IsCmdKeyDown(void);
Boolean IsShiftKeyDown(void);

// globals
uchar pal_buf[768];
     

void main (void)
{
 	OSErr				err;
	grs_screen 	*screen;
	Str255			str;
	Handle			testRes,testRes2,testRes3;
  short 			w,h;
	long				time,i,j;
	grs_bitmap  bm1, bm2;
	char				temp[256];
  grs_vertex 	v0,v1,v2,v3;
  grs_vertex 	*points[4]= {&v0,&v1,&v2,&v3};
	Handle	 		pal;
	Handle			shade1,shade2;
	Rect				r;
	StandardFileReply	 reply;
	int					resNum=-1;
	Ptr					p1, p2;
	Ptr					big_buffer;
	FrameDesc		*fd;
	EventRecord evt;
	grs_canvas 	*off_canvas;
	
	InitMac();
	CheckConfig();
	ResInit();
	
	SetupWindows(&gMainWindow);								// setup everything
	SetupOffscreenBitmaps();			
				
	gr_init();
	gr_set_mode (GRM_640x480x8, TRUE);
	screen = gr_alloc_screen (grd_cap->w, grd_cap->h);
	gr_set_screen (screen);
	off_canvas = gr_alloc_canvas(BMT_FLAT8,512,480);

	pal = GetResource('pal ',1001);
	BlockMove(*pal, pal_buf, 768L);
  gr_set_pal(0, 256, pal_buf);
  
  HideCursor();
  
 	gr_alloc_ipal();
	gr_init_blend(1);
  gr_clear(0xff);

	PicHandle	pic = GetPicture(9998);
	if (pic)
	{
		Rect r = (*pic)->picFrame;
		OffsetRect(&r, -r.left, -r.top);
		OffsetRect(&r, 0, 300);
		DrawPicture(pic, &r);
		
		ReleaseResource((Handle)pic);
	}
	
  g3_init(80,AXIS_RIGHT,AXIS_DOWN,AXIS_IN);

	{
		WDPBRec		wdpb;
		FSSpec		spec;
		Ref				rid;
		RefTable	*refTbl;
		
		memset(&wdpb, 0x00, sizeof(wdpb));					// Find out about the current directory
		PBHGetVol(&wdpb, false);
		err = FSMakeFSSpec(wdpb.ioWDVRefNum, wdpb.ioWDDirID, "\pobjart.rsrc", &spec);	
		resNum = ResOpenFile(&spec);
		if (resNum != -1)
		{	
			rid = MKREF(1350, 0);
 			refTbl = ResReadRefTable(REFID(rid));

			rid = MKREF(1350,1631);										// Bitmap
			p1 = NewPtrClear(RefSize(refTbl,REFINDEX(rid)));
  		RefExtract(refTbl, rid, p1);
			fd = (FrameDesc *)p1;
			fd->bm.bits = (uchar *)(fd+1);
			bm1 = fd->bm;
			gr_scale_ubitmap(&bm1, 0, 40, bm1.w * 2, bm1.h * 2);
			
			rid = MKREF(1350,1632);										// Heights
			p2 = NewPtrClear(RefSize(refTbl,REFINDEX(rid)));
  		RefExtract(refTbl, rid, p2);
			fd = (FrameDesc *)p2;
			fd->bm.bits = (uchar *)(fd+1);
			bm2 = fd->bm;
			gr_scale_ubitmap(&bm2, 0, 160, bm2.w * 2, bm2.h * 2);
			voxel_convert(&bm2);	

			ResCloseFile(resNum);
		}
	}	
  
  // Voxel stuff
	kb_startup(NULL);
	kb_set_flags(KBF_BLOCK);
  {
		vxs_vox 		vvv;
		g3s_vector	vec;
		g3s_angvec	ang;
		int					dx = 0, dy = 0, dz = 0;
		fix					spc = fix_make(0,0x1000);
		fix					siz = fix_make(0,0x0400);
		fix					vecX = 0, vecY = 0, vecZ = 0;
		fix					vec2X = 0, vec2Y = 0;
		ushort 			cooked = 0;
		bool 				result;
		Boolean			done = FALSE;
		Rect				clrRect;
		Boolean			optDown, cmdDown, shiftDown;
		
		SetRect(&clrRect, 128, 0, 640, 480);
		vx_init(16);
		do
		{
			gr_set_canvas(off_canvas);
			gr_clear(0xff);     

		  g3_start_frame();
			vec.gX = vecX;
			vec.gY = vecY;
			vec.gZ = vecZ;
			ang.tx = 0;
			ang.ty = 0;
			ang.tz = 0;
	   	g3_set_view_angles(&vec, &ang, ORDER_YXZ, g3_get_zoom('X',build_fix_angle(80),640,480));
		
			vec.gX = vec2X;
			vec.gY = vec2Y;
			vec.gZ = fix_make(2,0);
			g3_start_object_angles_xyz(&vec, 
				build_fix_angle(dx), build_fix_angle(dy), build_fix_angle(dz), ORDER_YXZ);
	
			vx_init_vox(&vvv, spc, siz, 16, &bm1, &bm2);
			// PaintRect(&clrRect);
			vx_render(&vvv);
	
			g3_end_object();
			g3_end_frame();
		 
			gr_set_canvas(grd_screen_canvas);
			gr_bitmap(&off_canvas->bm,128,0);	
	
			optDown = IsOptKeyDown();
			cmdDown = IsCmdKeyDown();
			shiftDown = IsShiftKeyDown();
				if (kb_state(0x0C))
					done = TRUE;
				else
				{
					if (kb_state(0x7B))
					{
							if (shiftDown)
								vecX -= 0x0600;
							else if (optDown)
								dz -= 5;												// If alt key down, change dz
							else
								dy += 5;
					}
					if (kb_state(0x7C))
					{
							if (shiftDown)
								vecX += 0x0600;
							else if (optDown)
								dz += 5;												// If alt key down, change dz
							else
								dy -= 5;
					}
					if (kb_state(0x7D))
					{
							if (shiftDown)
								vecY += 0x0600;
							else if (cmdDown)
								spc -= 0x0080;									// If cmd key down, change spacing
							else if (optDown)
								siz -= 0x0100;									// If alt key down, change size
							else
								dx += 5;												// else rotate.
					}
					if (kb_state(0x7E))
					{
							if (shiftDown)
								vecY -= 0x0600;
							else if (cmdDown)
								spc += 0x0080;									// If cmd key down, change spacing
							else if (optDown)
								siz += 0x0100;									// If alt key down, change size
							else
								dx -= 5;
					}
			}
		} while (!done);
	}
	kb_flush();
	kb_close();
	
// {EventRecord evt; while (true){if (GetNextEvent(mDownMask+keyDownMask,&evt))
//  {if (evt.what==mouseDown) break;}}}
 
	vx_close();
	gr_close();
  ShowCursor();
	CleanupAndExit();
}


void voxel_convert(grs_bitmap *bmp)
{
   int x,y;
   for (x=0; x < bmp->w; x++)
      for (y=0; y < bmp->h; y++)
         if (bmp->bits[(y * bmp->w) + x] != 0)
            bmp->bits[(y * bmp->w) + x] -= 208;
}

Boolean IsShiftKeyDown(void)
 {
	long		keys[4];

	GetKeys(keys);
	return((keys[1] & 0x00000001) != 0L);
 }

Boolean IsOptKeyDown(void)
 {
	long		keys[4];

	GetKeys(keys);
	return((keys[1] & 0x00000004) != 0L);
 }

Boolean IsCmdKeyDown(void)
 {
	long		keys[4];

	GetKeys(keys);
	return((keys[1] & 0x00008000) != 0L);
 }
