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
 * $Source: n:/project/lib/src/2d/RCS/genslin.c $
 * $Revision: 1.7 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 02:26:11 $
 *
 * Routine to draw an rgb shaded line to a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: genslin.c $
 * Revision 1.7  1994/06/11  02:26:11  lmfeeney
 * moved unclipped drawer, now contains two versions of
 * clipped line drawer, one for each i\f - call clipper
 * and call unclipped line drawer
 * 
 * Revision 1.6  1994/05/06  18:19:52  lmfeeney
 * rewritten for greater accuracy and speed
 * 
 * Revision 1.5  1993/10/02  01:04:01  kaboom
 * Fixed clipping include files.
 * 
 * Revision 1.4  1993/07/03  22:51:02  spaz
 * Bugfix; treated clipper return code spastically
 *
 * Revision 1.3  1993/07/01  20:32:29  spaz
 * Set last pixel explicitly to i1 in h and vlines
 *
 * Revision 1.2  1993/06/30  00:43:13  spaz
 * Changed a check for (x0>x1) to fix_int(x0) version,
 * because it was negating the delta uselessly for
 * near-vertical lines.
 *
 * Revision 1.1  1993/06/22  20:14:28  spaz
 * Initial revision
 */

#include <stdlib.h>
#include "ctxmac.h"
#include "plytyp.h"
#include "clpcon.h"
#include "clpltab.h"
#include "grlin.h"
#include "general.h"

/* Draw a gouraud-shaded line, specified by indices into the palette.
   be warned, weird precision bugs abound (check out test programs
   in /project/lib/src/2d/test). 
   
   5/94: Precision errors have been (entirely?) eliminated.  See
   correctness argument in note.txt.

*/

/* This routine draws clipped goroud-shaded lines as specified by
   intensities.  returns a clip value */

int gen_fix_sline (fix x0, fix y0, fix i0, fix x1, fix y1, fix i1)
{
   int r;
   grs_vertex v0, v1;
   
   v0.x = x0; v0.y = y0; v0.i = i0;
   v1.x = x1; v1.y = y1; v1.i = i1;

   r = grd_sline_clip_fill (gr_get_fcolor(), gr_get_fill_parm(), &v0, &v1);

   return r;
}


int gri_sline_clip_fill (long c, long parm, grs_vertex *v0, grs_vertex *v1)
{
   int r;
   grs_vertex u0, u1;

   /* save inputs (don't really need whole struct) */

   u0 = *v0;   
   u1 = *v1;

   r = gri_sline_clip (&u0, &u1);

   if (r != CLIP_ALL) 
     grd_usline_fill (c, parm, &u0, &u1);

   return r;
}
