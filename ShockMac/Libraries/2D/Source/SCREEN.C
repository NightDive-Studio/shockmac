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
 * $Source: r:/prj/lib/src/2d/RCS/screen.c $
 * $Revision: 1.5 $
 * $Author: kevin $
 * $Date: 1994/10/19 17:57:31 $
 *
 * Screen handling routines.
 *
 * This file is part of the 2d library.
 *
 * $Log: screen.c $
 * Revision 1.5  1994/10/19  17:57:31  kevin
 * No pal, bpal, and ipal are no longer used in the screen structure.
 * 
 * Revision 1.4  1993/10/08  01:16:21  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.3  1993/05/25  18:53:14  kaboom
 * Fixed bug in gr_free_screen---was erroneously freeing bpal.
 * 
 * Revision 1.2  1993/04/29  19:08:03  kaboom
 * Cleaned up memory allocation in gr_alloc_screen.
 * 
 * Revision 1.1  1993/02/04  17:44:20  kaboom
 * Initial revision
 */

#include "grs.h"
#include "grd.h"
#include "grmalloc.h"
#include "bitmap.h"
#include "canvas.h"
#include "rgb.h"
#include "valloc.h"
#include "scrdat.h"
#include "screen.h"


/* allocate enoguh video memory for a screen of the specified size.  then set
   up the screen structure describing this screen and the 2 system canvases
   for drawing on it.  return a pointer to the new screen structure. */
grs_screen *gr_alloc_screen (short w, short h)
 {
	grs_screen *s=0L;
  grs_canvas *c;
  uchar *p;
  uchar *b;

  /* get memory for screen structure itself and 2 system canvases,
        and video ram for the screen itself. */
  if ((p=(uchar *)NewPtr (sizeof (*s)+2*sizeof (*c))) == NULL)	// was gr_malloc
     goto bailout2;
  if ((b = valloc (w, h)) == (uchar *)-1)      
  	goto bailout1;

  /* set up bitmap. */
  s = (grs_screen *)p;
  c = (grs_canvas *)(p+sizeof (*s));
  gr_init_bm (&s->bm, b, BMT_DEVICE, 0, w, h);

  /* start with upper left visible. */
  s->x = 0;
  s->y = 0;

  /* set up global canvases. */
  s->c = c;
  gr_init_canvas (c, s->bm.bits, BMT_DEVICE, w, h);
  gr_init_canvas (c+1, s->bm.bits, BMT_DEVICE, grd_cap->w, grd_cap->h);
  return s;

bailout1:
  DisposePtr ((Ptr) s);	// was gr_free
bailout2:
  return NULL;
}

/* free memory for screen and its related data structures. */
void gr_free_screen (grs_screen *s)
{
	vfree (s->bm.bits);
	if (s->c->ytab)
  	DisposePtr ((Ptr) s->c->ytab);	// was gr_free
  if ((s->c+1)->ytab)
  	DisposePtr ((Ptr) (s->c+1)->ytab);	// was gr_free
  DisposePtr ((Ptr) s->c);	// was gr_free
  DisposePtr ((Ptr) s);	// was gr_free
}
