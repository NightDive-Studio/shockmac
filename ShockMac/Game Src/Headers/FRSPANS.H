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
 * $Source: n:/project/cit/src/inc/RCS/frspans.h $
 * $Revision: 1.1 $
 * $Author: dc $
 * $Date: 1994/01/02 17:16:44 $
 *
 * Citadel Renderer
 *  clipped and coned spans
 */

//
#ifndef __FRPIPE_SRC
extern uchar *x_span_lists;
extern uchar *cone_span_list;
#endif

void cone_span_set(int y, int lx, int rx);
void store_x_span (int y, int lx, int rx);


// maximum spans on a database scan line
#define MAX_SPANS  7
#define SPAN_MEM  (2*MAX_SPANS+2)
#define SPAN_SHIFT 4 // 8 spans, each 2 entries, 16 values
#define SPAN_LEFT  0 
#define SPAN_RIGHT 1 

#define span_count(y)         (x_span_lists[(y<<SPAN_SHIFT)+(MAX_SPANS<<1)])
#define span_left(y,s)        (x_span_lists[((y)<<SPAN_SHIFT)+(s<<1)+SPAN_LEFT])
#define span_right(y,s)       (x_span_lists[((y)<<SPAN_SHIFT)+(s<<1)+SPAN_RIGHT])

#define cone_span_left(y)     (cone_span_list[y<<1])
#define cone_span_right(y)    (cone_span_list[(y<<1)+1])

#ifdef CLIPPER_CRACK_CHECK
#define SPAN_NOCRACK 0xff
#define span_crack(y)   (x_span_lists[(y<<SPAN_SHIFT)+(MAX_SPANS<<1)+1])
#endif


