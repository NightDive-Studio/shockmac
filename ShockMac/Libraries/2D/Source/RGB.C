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
/*
 * $Source: r:/prj/lib/src/2d/RCS/rgb.c $
 * $Revision: 1.5 $
 * $Author: kaboom $
 * $Date: 1994/07/15 20:15:43 $
 *
 * RGB color manipulation routines.
 *
 * This file is part of the 2d library.
 */

#include "grs.h"
#include "grmalloc.h"
#include "rgb.h"
#include "scrdat.h"

// prototypes
static int _redloop();

/* Static Globals for his and her pleasure */
static int bcenter, gcenter, rcenter;
static long gdist, rdist, cdist;
static long cbinc, cginc, crinc;
static ulong *gdp, *rdp, *cdp;
static uchar *grgbp, *rrgbp, *crgbp;
static int gstride, rstride;
static long x, xsqr, colormax;
static int cindex;

static void inv_cmap_2(int colors, uchar *colormap[3],int bits,ulong *dist_buf, uchar *rgbmap);
int redloop();
static int _greenloop(int restart);
static int _blueloop(int restart);
static void _maxfill(ulong *buffer);

/* The routines in this file operate on grs_rgb color values.  The color
   values are encoded so that each r,g, and b has 8 bits of integer, 2
   bits of fraction, and one bit of padding. */
/* split a grs_rgb into its component 8-bit r, g, and b values. */
void gr_split_rgb (grs_rgb c, uchar *r, uchar *g, uchar *b)
{
   *r = (c>>2)&0xff;
   *g = (c>>13)&0xff;
   *b = (c>>24)&0xff;
}

/* This routine allocates a 15 bit inverse palette for the
   current screen palette (that 32768 bytes) and initializes
	it for the current palette.  Returns OUT_OF_MEMORY
	if it can't allocate anything.  If ipal is currently non-null
	it tries to delete it first, in the interests of robustness */

int gr_alloc_ipal(void)
{
	int err;

	if (grd_ipal != NULL) {
		if ((err = gr_free_ipal())<0) return err;
	}

	if ((grd_ipal = (uchar *) NewPtr(32768))==NULL) return RGB_OUT_OF_MEMORY;		// was gr_malloc

	if ((err = gr_init_ipal())<0) return err;
	return RGB_OK;
}

int gr_free_ipal(void)
{
	if (grd_ipal==NULL) return RGB_CANT_DEALLOCATE;
	DisposePtr((Ptr) grd_ipal);	// was gr_free
   grd_ipal=NULL;
	return RGB_OK;
}

/* Initializes the inverse palette to the current screen
   palette.  */

int gr_init_ipal(void)
{
	int 	i;
  uchar r,g,b;
	uchar *data,*colormap[3];
	ulong *dist_buf;

	if (grd_ipal == NULL) return RGB_IPAL_NOT_ALLOCATED;

  if ((data = (uchar *) NewPtr(3*256)) == NULL) return RGB_OUT_OF_MEMORY;	// was gr_malloc
	/* needs to be split up into r,g,b planes */
	colormap[0] = data;
	colormap[1] = data + 256;
  colormap[2] = data + 2*256;

  for(i=0;i<256;++i) {
    gr_split_rgb(grd_bpal[i],&r,&g,&b);
   	colormap[2][i] = r;
   	colormap[1][i] = g;
   	colormap[0][i] = b;
   }

	if ((dist_buf = (ulong *) NewPtr(sizeof(ulong) * 32768))==NULL) return RGB_OUT_OF_MEMORY;	// was gr_malloc

	inv_cmap_2(256,colormap,5,dist_buf,grd_ipal);

 	DisposePtr((Ptr) dist_buf);	// was gr_free
 	DisposePtr((Ptr) data);	// was gr_free

	return RGB_OK;
}


static void inv_cmap_2(int colors, uchar *colormap[3],int bits,ulong *dist_buf, uchar *rgbmap )
{
	int nbits = 8 - bits;

	colormax = 1 << bits;
	x = 1 << nbits;
	xsqr = 1 << (2 * nbits);

	/* Compute strides for accessing the arrays. */

	gstride = colormax;
	rstride = colormax * colormax ;

	_maxfill(dist_buf);

	for(cindex = 0;cindex<colors; cindex++) {
		/* The initial position is the cell containing the colormap
		 * entry.  We get this by quantiziing the colormap values.
		 */
		rcenter = colormap[0][cindex] >> nbits;
		gcenter = colormap[1][cindex] >> nbits;
		bcenter = colormap[2][cindex] >> nbits;

		rdist = colormap[0][cindex] - (rcenter * x + x/2);
		gdist = colormap[1][cindex] - (gcenter * x + x/2);
		cdist = colormap[2][cindex] - (bcenter * x + x/2);
		cdist = rdist*rdist + gdist*gdist + cdist*cdist;

		crinc = 2 * ((rcenter+1) * xsqr - (colormap[0][cindex] * x));
		cginc = 2 * ((gcenter+1) * xsqr - (colormap[1][cindex] * x));
		cbinc = 2 * ((bcenter+1) * xsqr - (colormap[2][cindex] * x));

		/* Array starting points. */
		cdp = dist_buf + rcenter*rstride + gcenter*gstride + bcenter;
		crgbp = rgbmap + rcenter*rstride + gcenter*gstride + bcenter;

		_redloop();
	}
}

/* redloop -- loop up and down from red center. */
static int _redloop()
{
	int detect, r, first;
	long txsqr = xsqr + xsqr;
	static long rxx;

	detect = 0;

	/* Basic loop up */
	for (r = rcenter, rdist = cdist, rxx = crinc,
			rdp = cdp, rrgbp = crgbp, first = 1;
		r<colormax;
		r++, rdp += rstride, rrgbp += rstride,
			rdist += rxx, rxx += txsqr, first = 0) {
		if (_greenloop(first))
			detect = 1;
		else if (detect)
			break;
	}

	/* Basic loop down */
	for (r = rcenter - 1, rxx = crinc - txsqr, rdist = cdist - rxx,
			rdp = cdp - rstride, rrgbp = crgbp - rstride, first = 1;
		r >= 0;
		r--, rdp -= rstride, rrgbp -= rstride,
			rxx -= txsqr, rdist -= rxx, first = 0) {
		if (_greenloop(first))
			detect = 1;
		else if (detect)
			break;
	}

	return detect;
}


/* greenloop -- loop up and down from green center. */
static int _greenloop(int restart)
{
	int detect, g, first;
	long txsqr = xsqr + xsqr;
	static int here, min, max;
	static int prevmax, prevmin;
	int thismax, thismin;
	static long ginc, gxx, gcdist;
	static ulong *gcdp;
	static uchar *gcrgbp;

	/* Red loop restarted, reset variables to "center" position */
	if (restart) {
		here = gcenter;
		min = 0;
		max = colormax - 1;
		ginc = cginc;
		prevmax = 0;
		prevmin = colormax;
	}

	/* finding actual min and max on this line. */
	thismin = min;
	thismax = max;
	detect = 0;

	/* Basic loop up. */
	for (g=here, gcdist = gdist = rdist, gxx = ginc,
			gcdp = gdp = rdp, gcrgbp = grgbp = rrgbp, first = 1;
		g <= max;
		g++, gdp += gstride, gcdp += gstride,
			grgbp += gstride, gcrgbp += gstride,
			gdist += gxx, gcdist += gxx, gxx += txsqr, first = 0) {
		if (_blueloop(first)) {
			if (!detect) {
				/* remember here and associated data! */
				if (g>here) {
					here = g;
					rdp = gcdp;
					rrgbp = gcrgbp;
					rdist = gcdist;
					ginc = gxx;
					thismin = here;
				}
				detect = 1;
			}
		}
		else if (detect) {
			thismax = g - 1;
			break;
		}
	}

	/* Basic loop down */
	for (g=here - 1, gxx = ginc - txsqr, gcdist = gdist = rdist - gxx,
			gcdp = gdp = rdp - gstride, gcrgbp = grgbp = rrgbp - gstride,
			first = 1;
		g >= min;
		g--, gdp -= gstride, gcdp -= gstride,
			grgbp -= gstride, gcrgbp -= gstride,
			gxx -= txsqr, gdist -= gxx, gcdist -= gxx, first = 0) {
		if (_blueloop(first)) {
			if (!detect) {
				/* remember here! */
				here = g;
				rdp = gcdp;
				rrgbp = gcrgbp;
				rdist = gcdist;
				ginc = gxx;
				thismax = here;
				detect = 1;
			}
		}
		else if (detect) {
			thismin = g + 1;
			break;
		}
	}

	/* If we saw something, update the edge tracers.  Only
	 * tracks edges that are "shrinking" (min increating, max
	 * decreasing.
  	 */

	if (detect) {
		if (thismax < prevmax)
			max = thismax;
		prevmax = thismax;

		if (thismin > prevmin )
				min = thismin;
		prevmin = thismin;
	}
		
	return detect;
}

/* blueloop -- loop up and down from blue center. */
static int _blueloop(int restart)
{
	int detect;
	/* These are all registers on a Sun 3. Your mileage may differ. */
	ulong *dp;
	uchar *rgbp;
	long bdist, bxx;
	int b, i=cindex;
	long txsqr = xsqr + xsqr;
	int lim; 	/* for min and max, avoid extra registers. */
	static int here, min, max;
	static int prevmin, prevmax;	/* For tracking min and max. */
	int thismin, thismax;
	static long binc;

	if (restart) {
		here = bcenter;
		min = 0;
		max = colormax - 1;
		binc = cbinc;
		prevmin = colormax;
		prevmax = 0;
	}

	detect = 0;
	thismin = min;
	thismax = max;

	/* Basic loop up. */
	/* First loop just finds first applicable cell. */
	for (b = here, bdist = gdist, bxx = binc, dp = gdp, rgbp = grgbp,
			lim = max;
		b <= lim;
		b++, dp++, rgbp++, bdist += bxx, bxx += txsqr) {
		if (*dp > bdist) {
			/* Remember new here and associated data! */
			if (b>here) {
				here = b;
				gdp = dp;
				grgbp = rgbp;
				gdist = bdist;
				binc = bxx;
				thismin = here;
			}
			detect = 1;
			break;
		}
	}
	/* Second loop fills in a run of closer cells. */
	for (;
		b <= lim;
		b++, dp++, rgbp++, bdist += bxx, bxx += txsqr) {
		if (*dp > bdist ) {
			*dp = bdist;
			*rgbp = i;
		} else {
			thismax = b - 1;
			break;
		}
	}

	/* Basic loop down */
	/* Do initializations here, since the 'find' loop might not get
	 * executed.
	 */
	lim = min;
	b = here - 1;
	bxx = binc - txsqr;
	bdist = gdist - bxx;
	dp = gdp - 1;
	rgbp = grgbp - 1;
	/* The 'find' loop ios executed on ly if we didn't already find
	 * something.
	 */
	if (!detect)
		for(;
			b >= lim;
			b--, dp--, rgbp--, bxx -= txsqr, bdist -= bxx) {
			if ( *dp > bdist) {
				/* Remember here! */
				/* No test for b against here necessary because b <
				 * here by definition.
				 */
				here = b;
				gdp = dp;
				grgbp = rgbp;
				gdist = bdist;
				binc = bxx;
				thismax = here;
				detect = 1;
				break;
			}
		}
	/* the 'update' loop */
	for (;
			b>= lim;
			b--, dp--, rgbp--, bxx -= txsqr, bdist -= bxx) {
		if ( *dp > bdist) {
			*dp = bdist;
			*rgbp = i;
		} else {
			thismin = b + 1;
			break;
		}
	}

	/* If we saw something, update the edge trackers. */
	if (detect) {
		/* Only tracks edges that are 'shrinking' (*min increasin, max
		 * decreasing).
	    */
		if (thismax < prevmax)
		 	max = thismax;
		if (thismin > prevmin )
			min = thismin;

		/* Remember the min and max values. */
		prevmax = thismax;
		prevmin = thismin;
	}

	return detect;
}

/* Fill a buffer with the largest unsigned long. */
static void _maxfill(ulong *buffer)
{
	ulong maxv = (long)-1;
	long i;
	ulong *bp;

	for(i=colormax * colormax * colormax, bp = buffer;
			i > 0;
			i--, bp++ )
		*bp = maxv;
}
