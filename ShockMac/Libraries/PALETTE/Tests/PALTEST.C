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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Timer.h>

#include "InitMac.h"
#include "ShockBitmap.h"
#include "palette.h"

WindowPtr	gMainWindow;
grs_screen	*screen;

void DrawMsg(uchar *msg);


void main ()
{
   uchar Black_Palette[768];
   uchar Save_Palette[768];
   int  i, j, k;
   RGBColor	white = {0xffff, 0xffff, 0xffff};
   char	buff[10];
   byte	id;
   Handle	pal;
   long	dl;

	// Mac setup
	InitMac();
	CheckConfig();
	
	SetupWindows(&gMainWindow);
	SetupOffscreenBitmaps();

	gr_init();
	gr_set_mode( GRM_640x480x8, TRUE );
	screen = gr_alloc_screen( grd_cap->w, grd_cap->h );
	gr_set_screen( screen );
	
	// Get our default palette.
	
	pal = GetResource('pal ',1000);
	BlockMove(*pal, Save_Palette, 768L);
    gr_set_pal(0, 256, Save_Palette);

	// Draw the whole dang palette.
	
	gr_clear(255);
	k = 0;
	TextFont(geneva);
	TextSize(9);
	TextMode(srcOr);
	RGBForeColor(&white);
	for (i=0; i < 8; i++)
		for (j=0; j < 32; j++)
		{
			int	x, y;
			
			x = j * 20;
			y = i * 60;
			gr_set_fcolor(k);
			gr_rect(x, y, x+18, y+45);
			
			sprintf(buff, "%d", k);
		
			MoveTo(x + ((18 - TextWidth(buff,0,strlen(buff))) / 2), y+57);
			DrawText(buff, 0, strlen(buff));
			k++;
		}
	
	// Setup the palette effects system.
	
	palette_initialize(8);
	palette_set_rate(1);
	for (i = 0; i < 768; i++)
		Black_Palette[i] = 0;
	
	// Fade to black.
	
	DrawMsg("\pClick to fade to black.");
	id = palette_install_effect(SHIFT, REAL_TIME, 0, 255, 1, 200, Save_Palette, Black_Palette);
	
	while (!Button()) ;
	
	while (palette_query_effect(id) == ACTIVE) 
		palette_advance_effect(id, 1);
	
	// Fade back.
	
	DrawMsg("\pClick to fade back.");
	id = palette_install_effect(SHIFT, REAL_TIME, 0, 255, 0, 100, Black_Palette, Save_Palette);
	
	while (!Button()) ;
	
	while (palette_query_effect(id) == ACTIVE) 
		palette_advance_effect(id, 1);
	
	// Do a CBANK test.
	
	DrawMsg("\pClick to CBANK.");
	palette_install_cbank(REAL_TIME, 0x03, 0x07, 17);	// 80
	palette_install_cbank(REAL_TIME, 0x0b, 0x0f, 10);	// 50
	palette_install_cbank(REAL_TIME, 0x10, 0x14, 5);	// 25
	palette_install_cbank(REAL_TIME, 0x15, 0x17, 27);	// 125
	palette_install_cbank(REAL_TIME, 0x18, 0x1a, 21);	// 100
	palette_install_cbank(REAL_TIME, 0x1b, 0x1f, 16);	// 75
	while (!Button()) ;
	Delay(30, &dl);
	
	DrawMsg("\pClick to stop CBANK.");
	while (!Button())
		palette_advance_all_fx(TickCount());
	
	// Click and we're done.
	
//	DrawMsg("\pClick to quit.");
//	Delay(30, &dl);
//	while (!Button()) ;
	
	palette_shutdown();
	gr_close();
	
	FlushEvents(everyEvent,0);
	CleanupAndExit();
}

void DrawMsg(uchar *msg)
{
	RGBColor	black = {0, 0, 0};
	RGBColor	white = {0xffff, 0xffff, 0xffff};
	Rect		r;
	
	RGBForeColor(&black);
	SetRect(&r, 0, -35, 300, -15);
	PaintRect(&r);
	MoveTo(1, -20);
	RGBForeColor(&white);
	DrawString(msg);
}
