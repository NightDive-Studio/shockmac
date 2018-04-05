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
//		COMPOSEW.C		Routines for handling wide8 compose buffer
//		Rex E. Bradford (REX)
//
/*
* $Header: r:/prj/lib/src/afile/RCS/compose.c 1.4 1994/10/18 08:19:13 rex Exp $
* $Log: compose.c $
 * Revision 1.4  1994/10/18  08:19:13  rex
 * Added BMF_TRANS flag to computed RSD bitmaps, so they'll work properly with 2d.
 * 
 * Revision 1.3  1994/09/13  12:23:27  rex
 * Push and pop stack canvas, instead of just set
 * 
 * Revision 1.2  1994/09/06  18:02:56  rex
 * Fixed check for same size bitmap as compose buffer (no need for row to match)
 * 
 * Revision 1.1  1994/07/22  13:20:04  rex
 * Initial revision
 * 
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lg.h"
#include "compose.h"

//	-------------------------------------------------------
//
//	ComposeInit() allocates memory and inits bitmap.

void ComposeInit(grs_bitmap *pcompose, int bmtype, int w, int h)
{
//	Compute rowbytes based on type

	switch (bmtype)
		{
		case BMT_FLAT8:
			pcompose->row = sizeof(uchar) * w;
			break;
//		case BMT_FLAT24:
//			pcompose->row = 3 * w;
//			break;
		default:
			printf("ComposeInit: invalid bitmap type: %d\n", bmtype);
			return;
		}

//	Init bitmap

	pcompose->bits = (uchar *)malloc((long) pcompose->row * h);
	pcompose->type = bmtype;
	pcompose->flags = 0;
	pcompose->align = 0;
	pcompose->w = w;
	pcompose->h = h;
}

//	-------------------------------------------------------
//
//	ComposeAdd() adds a bitmap to the compose buffer.

void ComposeAdd(grs_bitmap *pcompose, grs_bitmap *pbm)
{
	switch (pcompose->type)
		{
		case BMT_FLAT8:
			ComposeFlat8Add(pcompose, pbm);
			break;

//		case BMT_FLAT24:
//			ComposeFlat24Add(pcompose, pbm);
//			break;

		default:
			printf("ComposeAdd: can't handle compose buffer type: %d\n",
				pcompose->type);
			break;
		}
}

//	-------------------------------------------------------
//
//	ComposeFlat8Add() adds a bitmap to a flat8 compose buffer.

void ComposeFlat8Add(grs_bitmap *pcompose, grs_bitmap *pbm)
{
	grs_canvas cv;

	if ((pcompose->w != pbm->w) || (pcompose->h != pbm->h))
	{
		printf("ComposeFlat8Add: not same size bitmaps!\n");
		return;
	}

	switch (pbm->type)
		{
//		case BMT_RSD24:
//			printf("ComposeFlat8Add: can't add RSD24 to compose buffer\n");
//			break;

		default:
			gr_make_canvas(pcompose, &cv);
			gr_push_canvas(&cv);
			gr_bitmap(pbm, 0, 0);
			gr_pop_canvas();
			break;
		}
}
/*
//	-------------------------------------------------------
//
//	ComposeFlat24Add() adds a bitmap to a flat8 compose buffer.

void gr_flat24_flat24_ubitmap(grs_bitmap *pbm, int x, int y)
{
	uchar *ps,*pd;
	int iy;

	ps = pbm->bits;
	pd = grd_canvas->bm.bits + (grd_canvas->bm.row * y) + (x * 3);
	for (iy = y; iy < (y + pbm->h); iy++)
		{
		memcpy(pd, ps, pbm->w * 3);
		ps += pbm->row;
		pd += grd_canvas->bm.row;
		}
}

void ComposeFlat24Add(grs_bitmap *pcompose, grs_bitmap *pbm)
{
	grs_canvas cv;

	if ((pcompose->w != pbm->w) || (pcompose->h != pbm->h) ||
		(pcompose->row != pbm->row))
		{
		Warning(("ComposeFlat24Add: not same size bitmaps!\n"));
		return;
		}

	switch (pbm->type)
		{
		case BMT_RSD24:
			Warning(("ComposeFlat24Add: can't add RSD24 to compose buffer!\n"));
			break;

		case BMT_FLAT24:
			gr_make_canvas(pcompose, &cv);
			gr_push_canvas(&cv);
			gr_flat24_flat24_ubitmap(pbm, 0, 0);
			gr_pop_canvas();
			break;

		default:
			gr_make_canvas(pcompose, &cv);
			gr_push_canvas(&cv);
			gr_bitmap(pbm, 0, 0);
			gr_pop_canvas();
			break;
		}
}
*/
//	--------------------------------------------------------
//
//	ComposeDiff() computes difference between compose buffer & bitmap.

long ComposeDiff(grs_bitmap *pcompose, grs_bitmap *pbmNew,
	grs_bitmap *pbmDiff)
{
	switch (pcompose->type)
		{
		case BMT_FLAT8:
			return(ComposeFlat8Diff(pcompose, pbmNew, pbmDiff));

//		case BMT_FLAT24:
//			return(ComposeFlat24Diff(pcompose, pbmNew, pbmDiff));

		default:
			printf("ComposeDiff: can't handle compose buffer type: %d\n",
				pcompose->type);
			return(0);
		}
}

//	-----------------------------------------------------------
//
//	ComposeFlat8Diff() computes diff between flat8 compose buff & bitmap.

long ComposeFlat8Diff(grs_bitmap *pcompose, grs_bitmap *pbmNew,
	grs_bitmap *pbmDiff)
{
	long numPixels,len;

//	Error-check

	if (pbmNew->type != BMT_FLAT8)
	{
		printf("ComposeFlat8Diff: new bitmap wrong type: %d\n", pbmNew->type);
		return(-1);
	}

//	Init rsd8 bitmap

	gr_init_bm(pbmDiff, pbmDiff->bits, BMT_RSD8, BMF_TRANS,
		pcompose->w, pcompose->h);
	pbmDiff->row = pbmDiff->w;

//	Try rsd compression

	numPixels = (long) pbmDiff->row * pbmDiff->h;
	len = 0;
//	len = RsdCompressDiff(pbmDiff->bits, numPixels, pcompose->bits,
//		pbmNew->bits, numPixels);

//	If failed, revert to flat8 bitmap

	if (len <= 0)
	{
		pbmDiff->type = BMT_FLAT8;
		memcpy(pbmDiff->bits, pcompose->bits, numPixels);
		len = numPixels;
	}

//	Return length

	return(len);
}
/*
//	-----------------------------------------------------------
//
//	ComposeFlat24Diff() computes diff between flat24 compose buff & bitmap.

long ComposeFlat24Diff(grs_bitmap *pcompose, grs_bitmap *pbmNew,
	grs_bitmap *pbmDiff)
{
	long numPixels,numPixels3,len;

//	Error-check

	if (pbmNew->type != BMT_FLAT24)
		{
		Warning(("ComposeFlat8Diff: new bitmap wrong type: %d\n", pbmNew->type));
		return(-1);
		}

//	Init rsd24 bitmap

	pbmDiff->type = BMT_RSD24;
	pbmDiff->flags = BMF_TRANS;
	pbmDiff->align = 0;
	pbmDiff->w = pcompose->w;
	pbmDiff->h = pcompose->h;
	pbmDiff->row = pcompose->row;

//	Try rsd compression

	numPixels = (long) pbmDiff->row * pbmDiff->h;
	numPixels3 = numPixels * 3;
	len = Rsd24CompressDiff(pbmDiff->bits, numPixels3, pcompose->bits,
		pbmNew->bits, numPixels);

//	If failed, revert to flat24 bitmap

	if (len <= 0)
		{
		pbmDiff->type = BMT_FLAT24;
		memcpy(pbmDiff->bits, pcompose->bits, numPixels3);
		len = numPixels3;
		}

//	Return length

	return(len);
}
*/
//	-----------------------------------------------------------
//
//	ComposeConvert() converts compose buffer into bitmap.

long ComposeConvert(grs_bitmap *pcompose, grs_bitmap *pbm)
{
	switch (pcompose->type)
		{
		case BMT_FLAT8:
			return(ComposeFlat8Convert(pcompose, pbm));

//		case BMT_FLAT24:
//			return(ComposeFlat24Convert(pcompose, pbm));

		default:
			printf("ComposeConvert: can't handle compose buffer type: %d\n",
				pcompose->type);
			return(0);
		}
}

//	---------------------------------------------------------
//
//	ComposeFlat8Convert() converts flat8 compose buffer into bitmap.

long ComposeFlat8Convert(grs_bitmap *pcompose, grs_bitmap *pbm)
{
	long numPixels,len;
	grs_canvas cv;

	if ((pcompose->w != pbm->w) || (pcompose->h != pbm->h) ||
		(pcompose->row != pbm->row))
	{
		printf("ComposeFlat8Convert: not same size bitmaps!\n");
		return(0);
	}

	numPixels = (long) pcompose->w * pcompose->h;

CONVERT:

	switch (pbm->type)
		{
		case BMT_FLAT8:
			memcpy(pbm->bits, pcompose->bits, numPixels);
			return(numPixels);

		case BMT_RSD8:
//			len = RsdCompress(pbm->bits, numPixels, pcompose->bits, -1,
//				numPixels);
			len = -1;
			if (len < 0)
				{
				pbm->type = BMT_FLAT8;
				goto CONVERT;
				}
			return(len);

//		case BMT_FLAT24:
//			gr_make_canvas(pbm, &cv);
//			gr_push_canvas(&cv);
//			gr_bitmap(pcompose, 0, 0);
//			gr_pop_canvas();
//			return(numPixels * 3);

		default:
			printf("ComposeFlat8Convert: can't convert to bm type: %d\n",
				pbm->type);
			return(0);
		}
}
/*
//	---------------------------------------------------------
//
//	ComposeFlat24Convert() converts flat24 compose buffer into bitmap.

long ComposeFlat24Convert(grs_bitmap *pcompose, grs_bitmap *pbm)
{
	long numPixels;
	grs_canvas cv;

	if ((pcompose->w != pbm->w) || (pcompose->h != pbm->h) ||
		(pcompose->row != pbm->row))
		{
		Warning(("ComposeFlat24Convert: not same size bitmaps!\n"));
		return(0);
		}

	numPixels = (long) pcompose->w * pcompose->h;

	switch (pbm->type)
		{
		case BMT_FLAT8:
			gr_make_canvas(pbm, &cv);
			gr_push_canvas(&cv);
			gr_bitmap(pcompose, 0, 0);
			gr_pop_canvas();
			return(numPixels);

		case BMT_FLAT24:
			memcpy(pbm->bits, pcompose->bits, numPixels * 3);
			return(numPixels * 3);

		default:
			Warning(("ComposeFlat24Convert: can't convert to bm type: %d\n",
				pbm->type));
			return(0);
		}
}
*/
//	---------------------------------------------------------
//
//	ComposeFree() frees up compose buffer.

void ComposeFree(grs_bitmap *pcompose)
{
	if (pcompose->bits)
	{
		free(pcompose->bits);
		pcompose->bits = NULL;
	}
}


