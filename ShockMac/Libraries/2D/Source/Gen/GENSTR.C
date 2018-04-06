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
 * $Source: n:/project/lib/src/2d/RCS/genstr.c $
 * $Revision: 1.7 $
 * $Author: lmfeeney $
 * $Date: 1994/06/15 01:18:46 $
 *
 * Generic clipped string drawer.
 *
 * This file is part of the 2d library.
 *
 * $Log: genstr.c $
 * Revision 1.7  1994/06/15  01:18:46  lmfeeney
 * support extended ascii (c > 127) w\ uchars, don't change fn i\f
 * 
 * Revision 1.6  1994/04/09  07:23:14  lmfeeney
 * mostly rewritten for improved efficiency, doesn't clip
 * each character bitmap
 * 
 * Revision 1.5  1993/10/19  09:57:54  kaboom
 * Replaced #include   new headers.
 * 
 * Revision 1.4  1993/10/02  01:17:35  kaboom
 * Changed include of clip.h to include of clpcon.h and/or clpfcn.h.
 * 
 * Revision 1.3  1993/06/02  16:19:44  kaboom
 * Now handles strings with hard and soft carriage returns and soft
 * spaces, from line wrapping.
 * 
 * Revision 1.2  1993/04/29  18:40:53  kaboom
 * Changed include of gr.h to smaller more specific grxxx.h.
 * 
 * Revision 1.1  1993/04/08  16:26:02  kaboom
 * Initial revision
 */

#include "bitmap.h"
#include "clpcon.h"
#include "ctxmac.h"
#include "grdbm.h"
#include "chr.h"
#include "general.h"


int gen_font_string (grs_font *f, char *s, short x0, short y0)
{
	grs_bitmap bm;        /* character bitmap */
	short *offset_tab;		/* table of character offsets */
	uchar *char_buf;			/* font pixel data */
	short offset;				  /* offset of current character */
	short x, y;					 	/* position of current character */
	uchar c;						  /* current character */
	short yok;					  /* current line in t/b clip */

	if (x0 > grd_clip.right || y0 > grd_clip.bot)
	  return CLIP_NONE;

	char_buf = (uchar *)f + f->buf;
	offset_tab = f->off_tab;
	gr_init_bm (&bm, NULL, (f->id==0xcccc)? BMT_FLAT8: BMT_MONO,
					BMF_TRANS, 0, f->h);
	bm.row = f->w;

	x = x0; y = y0;	  

	while (1) {  

	  /* y in clip region */

	  if ((y+f->h >= grd_clip.top && y+f->h <= grd_clip.bot) ||
	 			(y >= grd_clip.top && y <= grd_clip.bot)) {
		 
		 yok = (y >= grd_clip.top && y+f->h <= grd_clip.bot);
		 
		/* line coming into range */
		while ((c= (uchar)(*s++)) != CHAR_SOFTCR && c != '\n') {
	 		if (c == '\0')
				return CLIP_NONE;
			 if (c>f->max || c<f->min || c==CHAR_SOFTSP)
				continue;
	 		offset = offset_tab[c-f->min];
			 bm.w = offset_tab[c-f->min+1]-offset;
	 		if (x+bm.w >= grd_clip.left) 
				break;
			x+=bm.w;
		 } 
		 
	if (c=='\n' || c==CHAR_SOFTCR) {
		x = x0; y += f->h;
	 		continue;
	 }
		 
		 /* clip boundary character */
	if (bm.type == BMT_MONO) {
	 	bm.bits = char_buf + (offset>>3);
	 	bm.align = offset&7;
	 	gr_mono_bitmap (&bm, x, y);
	 }
	else {
	 	bm.bits = char_buf + offset;
	 	gr_flat8_bitmap (&bm, x, y);
	 }
	x+=bm.w; 

	/* line in range */
	while ((c=*s++) != CHAR_SOFTCR && c != '\n') {
	 	if (c == '\0')
			return CLIP_NONE;
	 	if (c>f->max || c<f->min || c==CHAR_SOFTSP)
			continue;
		offset = offset_tab[c-f->min];
		bm.w = offset_tab[c-f->min+1]-offset;
	 	if (x+bm.w > grd_clip.right) 
			break;
	 	if (bm.type == BMT_MONO) {
		  bm.bits = char_buf + (offset>>3);
		  bm.align = offset&7;
		  if (yok)
			 	gr_mono_ubitmap (&bm, x, y);
		  else 
			 	gr_mono_bitmap (&bm, x, y);
		}
	 else {
		bm.bits = char_buf + offset;
		if (yok) 
		  gr_flat8_ubitmap (&bm, x, y);
		else 
		  gr_flat8_bitmap (&bm, x, y);
	 }
	 x+=bm.w;
  }

	if (c=='\n' || c==CHAR_SOFTCR) {
	 	x = x0; y += f->h; 
	 	continue;
	 }

		 /* clip boundary character */
	if (bm.type == BMT_MONO) {
	 	bm.bits = char_buf + (offset>>3);
	 	bm.align = offset&7;
	 	gr_mono_bitmap (&bm, x, y);
	 }
	else {
	 	bm.bits = char_buf + offset;
	 	gr_flat8_bitmap (&bm, x, y);
	 }

		 /* end of line */
	while ((c=*s++) != CHAR_SOFTCR && c != '\n') {
	 	if (c == '\0') return CLIP_NONE;
	 }
	x = x0; y += f->h; 
 }

	  /* not yet in y-range */
	else if (y < grd_clip.top) { 
		while ((c=*s++) != CHAR_SOFTCR && c != '\n') {
	 	if (c == '\0') return CLIP_NONE;
	 }
	x = x0; y += f->h; 
 }
 /* can't be in range */
	else 
	return CLIP_NONE;
	}
 }
	  
