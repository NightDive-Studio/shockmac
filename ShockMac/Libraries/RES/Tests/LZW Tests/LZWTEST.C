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
//		LZWTEST.C - Computes compression ratio for data
//		Rex E. Bradford

#include <stdio.h>
#include <stdlib.h>

#include "lzw.h"

void main(void)
{
	StandardFileReply	reply;
	SFTypeList				typeList;
	short						filenum;
	Handle					resHdl, exHdl;
	Ptr						compBuff;
	long						len, clen, exlen;
	float						ratio;

	// Open a resource file.
	
	printf("Select the LzwTest resource file...\n\n");

	typeList[0] = 'rsrc';
	StandardGetFile(nil, 1, typeList, &reply);
	if (reply.sfGood)
	{
		filenum = FSpOpenResFile(&reply.sfFile, fsRdWrPerm);
		if (filenum == -1)
		{
			printf("Couldn't open the resource file.\n");
			exit(-1);
		}

		// Get the text resource from the file and its length.
		
		resHdl = Get1Resource('TEXT', 128);
		if (resHdl == NULL)
		{
			printf("Couldn't get the text resource.\n");
			exit(-2);
		}
		len = GetHandleSize(resHdl);
		HLock(resHdl);

		// Allocate buffer to hold compressed data.
		
		compBuff = NewPtr(len);
		if (compBuff == NULL)
		{
			printf("Couldn't allocate compBuff.\n");
			exit(-3);
		}
		
		// Allocate handle to hold expanded data.
		
		exHdl = NewHandle(len+100);						// Just to be safe
		if (exHdl == NULL)
		{
			printf("Couldn't allocate exHdl.\n");
			exit(-4);
		}
		HLock(exHdl);
		
		//	Compress into bit bucket

		LzwInit();
		clen = LzwCompressBuff2Buff(*resHdl, len, compBuff, len);
		exlen = LzwExpandBuff2Buff(compBuff, *exHdl, 0, 0);
		LzwTerm();
		
		// Add the expanded handle to the resource file, for later comparison.
		
		resHdl = Get1Resource('TEXT', 1000);			// Delete if already there.
		if (resHdl != NULL)
		{
			RmveResource(resHdl);
			DisposeHandle(resHdl);
		}
		HUnlock(exHdl);
		SetHandleSize(exHdl, exlen);
		AddResource(exHdl, 'TEXT', 1000, "\pExpanded");
		ChangedResource(exHdl);
		WriteResource(exHdl);
		
		CloseResFile(filenum);

		//	Compute compression ratio

		ratio = (float) len / (float) clen;

		//	Issue report

		printf("Org length: %d  compressed length: %d  comp ratio: %f:1\n",
					len, clen, ratio);
		printf("Expanded length: %d\n", exlen);
		printf("\nCompare the original and expanded resources with ResEdit.\n");
	}
	exit(0);
}
