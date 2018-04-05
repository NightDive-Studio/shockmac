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
#include "Shock.h"


#define CreditsID		7900

void ScrollCredits(void)
{
	RGBColor	black = {0, 0, 0};
	RGBColor	white = {0xffff, 0xffff, 0xffff};
	RGBColor	greenish = {0, 0x9999, 0};

	RGBBackColor(&black);			// Set background to black while scrolling

	// prepare the draw rects
	
	PicHandle pict = GetPicture(CreditsID);
	short picWidth = (**pict).picFrame.right - (**pict).picFrame.left;
	short picHeight = (**pict).picFrame.bottom - (**pict).picFrame.top;

	Rect txtRect;							// This is the rect you want to draw everything into
	
	txtRect.left = 160;
	txtRect.top = 130;
	txtRect.right = txtRect.left + picWidth;
	txtRect.bottom = txtRect.top + 164;		

	Rect picFrame;							// This is the frame of the picture.  It's based on txtRect.
	
	picFrame.top = txtRect.top;
	picFrame.left = txtRect.left;
	picFrame.bottom = picFrame.top + picHeight;
	picFrame.right = picFrame.left + picWidth;

	// save off old clip
	
	RgnHandle saveClipRgn = NewRgn();
	GetClip(saveClipRgn);
		
	// draw a quickie green frame
	
	RGBForeColor(&greenish);
	InsetRect(&txtRect, -1, -1);
	FrameRect(&txtRect);
	InsetRect(&txtRect, -1, -1);
	FrameRect(&txtRect);
	InsetRect(&txtRect, 2, 2);
	RGBForeColor(&black);

	ClipRect(&txtRect);	

	RgnHandle dud = NewRgn();
	RgnHandle save = NewRgn();
	
	// Draw the initial pict and wait for a few seconds.
	
	DrawPicture(pict, &picFrame);
	long	stupid;
	Delay(120, &stupid);
	
	do
	{
		// scroll up a pixel
									
		ScrollRect(&txtRect, 0, -1, dud);
		picFrame.top -= 1;
		picFrame.bottom -= 1;
	
		GetClip(save);
		SetClip(dud);
	
		DrawPicture(pict, &picFrame); 	
	
		SetClip(save);
		
		if (Button())
			goto cleanup;
	}
	while (picFrame.bottom > txtRect.bottom);
	Delay(90, &stupid);
			
cleanup:
	DisposeRgn(dud);
	DisposeRgn(save);
	
	// restore old clip & port
	
	SetClip(saveClipRgn);
	DisposeRgn(saveClipRgn);

	// Redraw what was there.	
	InsetRect(&txtRect, -2, -2);
	InvalRect(&txtRect);

	RGBBackColor(&white);			// Restore background color.
}
