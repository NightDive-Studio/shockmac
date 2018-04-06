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
 * $Source: r:/prj/cit/src/RCS/hflip.c $
 * $Revision: 1.6 $
 * $Author: mahk $
 * $Date: 1994/10/31 22:08:14 $
 */

#include "gr2ss.h"
#include "cybmem.h"

// MLA #define REAL_HFLIP
#ifdef REAL_HFLIP
// asm is good
// tmp should be half a max row
// we copy to half row, then mirrow backwards, then copy half row in
void do_flip_in_place(uchar *bits, uchar *tmp, int w, int h , int row);
#pragma aux do_flip_in_place =                                          \
/* end of the loop thing, so we need to do it to start out */           \
   "mov eax, ecx"                                                       \
/* edi ptr tmp, esi ptr bits line, eax+ecx width, edx is height */      \
"per_line:"                                                             \
   "shl edx, 16"       /* get height back up there */                   \
   "mov ebx, esi"                                                       \
/* first copy half row */                                               \
"move_left_to_tmp:"                                                     \
   "shr ecx, 1"                                                         \
   "and ecx, 3"                                                         \
   "rep movsb"                                                          \
   "mov ecx, eax"                                                       \
   "shr ecx, 3"                                                         \
   "rep movsd"                                                          \
   "dec edi"        /* get back to end of tmp stream */                 \
/* now mirror right half back to left */                                \
   "mov ecx, eax"                                                       \
   "mov esi, ebx"   /* get to left of bits */                           \
   "add esi, ecx"   /* get to right of bits */                          \
   "dec esi"        /* correct pixel is w-1 */                          \
/* should inline this a bunch, eh? */                                   \
"rev_right_to_left_loop:"                                               \
   "mov dl,[esi]"                                                       \
   "mov [ebx],dl"                                                       \
   "dec esi"                                                            \
   "inc ebx"                                                            \
   "cmp esi, ebx"                                                       \
   "jg  rev_right_to_left_loop"                                         \
/* now take back out of temp */                                         \
   "jne even_size"                                                      \
   "inc ebx"        /* if odd, need do nothing to middle pixel */       \
"even_size:"        /* ebx now points at next to fill */                \
   "shr ecx, 1"     /* note now edi is source, bx dest */               \
"rev_temp_to_right_loop:"                                               \
   "mov dl,[edi]"                                                       \
   "mov [ebx],dl"                                                       \
   "dec edi"                                                            \
   "inc ebx"                                                            \
   "dec ecx"                                                            \
   "jnz rev_temp_to_right_loop"                                         \
   "inc edi"    /* edi is left pointing one before start, thus inc */   \
   "mov esi, ebx"      /* store final pixel addr back into esi */       \
   "add esi,[esp]"   /* esi is pointing one past the end of line */     \
   "mov ecx, eax"                                                       \
   "shr edx, 16"                                                        \
   "dec edx"                                                            \
   "jnz per_line"                                                       \
   "add esp, 4"     /* get rid of the row_size on the stack */          \
parm [esi] [edi] [ecx] [edx] modify [eax ebx];


#pragma disable_message(202) 
// row skip is used implicitly above
void shock_hflip_in_place(grs_bitmap *bm)
{
   int row_skip=bm->row-bm->w;
   uchar tmp[320];

   do_flip_in_place(bm->bits, tmp, bm->w, bm->h, row_skip);
}
#pragma enable_message(202)

void _flip_in_place(uchar* bits, uchar* tmp, int w, int h, int row)
{
   do_flip_in_place(bits,tmp,w,h,row-w);
}


#else // !REAL_HFLIP

void shock_hflip_in_place(grs_bitmap* bm)
{
   grs_canvas big_canvas;
   grs_canvas bm_canvas;
   gr_init_canvas(&big_canvas, big_buffer, BMT_FLAT8, bm->w, bm->h);
   gr_init_canvas(&bm_canvas, bm->bits, BMT_FLAT8, bm->w, bm->h);
   gr_push_canvas(&big_canvas);
   gr_hflip_bitmap(bm,0,0);
   gr_pop_canvas();
   gr_push_canvas(&bm_canvas);
   ss_bitmap(&big_canvas.bm,0,0);
   gr_pop_canvas();
}

#endif
