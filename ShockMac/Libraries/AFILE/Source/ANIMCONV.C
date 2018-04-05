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
//		ANIMCONV		- Convert one anim to another
//		Rex E. Bradford

/*
* $Header: r:/prj/lib/src/afile/RCS/animconv.c 1.5 1994/10/18 16:01:31 rex Exp $
* $Log: animconv.c $
 * Revision 1.5  1994/10/18  16:01:31  rex
 * Inserts frame pal get/set code
 * 
 * Revision 1.4  1994/10/04  20:31:15  rex
 * Added -16 option for 16-bit quiktime movies
 * 
 * Revision 1.3  1994/10/03  18:06:18  rex
 * Added scaling of anim
 * 
 * Revision 1.2  1994/09/29  10:31:32  rex
 * Added time arg to read/write frame calls
 * 
 * Revision 1.1  1994/07/22  13:20:51  rex
 * Initial revision
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lg.h"
#include "afile.h"
//#include <timer.h>
//#include <quiktime.h>
#include "InitMac.h"
#include "ShockBitmap.h"

Afile afi;
Afile afo;

WindowPtr	gMainWindow;

//	---------------------------------------------------------------
//		MAIN PROGRAM
//	---------------------------------------------------------------

void main(void)
{
	grs_screen 	*screen;
	static grs_bitmap bm;
	char infile[64],outfile[64];
	long len;
	int iarg;
	int iframe,numFrames;
	int outWidth,outHeight;
	bool depth16;
	fix tFrame;
	grs_bitmap bmscale;
	grs_canvas cvscale;
	Apalette pal;

//	Init graphics system

	InitMac();
	CheckConfig();

	SetupWindows(&gMainWindow);								// setup everything
	SetupOffscreenBitmaps();			

	gr_init();
	gr_set_mode (GRM_640x480x8, TRUE);
	screen = gr_alloc_screen (grd_cap->w, grd_cap->h);
	gr_set_screen (screen);

//	Check args

	numFrames = 32767;
	outWidth = 0;
	outHeight = 0;
	depth16 = FALSE;

//	Open input anim file

	printf ("Input file: ");
	fgets (infile, sizeof(infile), stdin);
	if (AfileOpen(&afi, infile) < 0)
	{
		printf("Can't open %s or bad file type\n", infile);
		gr_close();
		exit(1);
	}

//	Open output anim file

	printf ("Output file: ");
	fgets (outfile, sizeof(outfile), stdin);
	if (AfileCreate(&afo, outfile, afi.v.frameRate) < 0)
	{
		printf("Can't open %s or bad file type\n", outfile);
		gr_close();
		exit(1);
	}

//	Set palette and depth

	AfileSetPal(&afo, &afi.v.pal);
//	if (depth16)
//		{
//		if (afo.type != AFILE_QTM)
//			{
//			printf("Can't use -16 option with non-.qtm output file\n");
//			exit(1);
//			}
//		QuikSetDepth16((QTM *) afo.pspec);	// oooh, nasty!!
//		}

//	Copy audio if any

	if ((len = AfileAudioLength(&afi)) > 0)
	{
		void *paudio = malloc(len);
		AfileGetAudio(&afi, paudio);
		AfilePutAudio(&afo, &afi.a, paudio);
	}

//	If scaling, set it all up

	if (outWidth && (outHeight == 0))
		outHeight = afi.v.width;
	else if (outHeight && (outWidth == 0))
		outWidth = afi.v.width;
	if (outWidth)
		{
		gr_init_bitmap(&bmscale, (uchar *)malloc(outWidth * outHeight), BMT_FLAT8, 0,
			outWidth, outHeight);
		gr_make_canvas(&bmscale, &cvscale);
		}

//	Copy the frames

	iframe = 0;
	while ((len = AfileReadFullFrame(&afi, &bm, &tFrame)) > 0)
	{
		if (AfileGetFramePal(&afi, &pal))
			AfileSetFramePal(&afo, &pal);
		if (outWidth)
		{
			gr_push_canvas(&cvscale);
			gr_scale_bitmap(&bm, 0, 0, outWidth, outHeight);
			gr_pop_canvas();
			AfileWriteFrame(&afo, &cvscale.bm, tFrame);
		}
		else
			AfileWriteFrame(&afo, &bm, tFrame);
		printf(".");
		fflush(stdout);
		if ((++iframe & 63) == 0)
			printf("\n");
		if (iframe == numFrames)
			break;
	}
	printf("\n");

//	Close files

	AfileClose(&afi);
	AfileClose(&afo);
	CleanupAndExit();
}

