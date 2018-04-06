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
//		Rect.H		Rectangle routines header file
//		Rex E. Bradford (REX)
/*
* $Header: n:/project/lib/src/dstruct/RCS/rect.h 1.5 1994/04/05 11:02:15 rex Exp $
* $Log: rect.h $
 * Revision 1.5  1994/04/05  11:02:15  rex
 * One should endeavor to spend less time writing diatribes about the compiler,.
 * and more time ensuring the veracity of one's code.
 * 
 * Revision 1.4  1994/04/05  04:04:24  dc
 * how about a make point that isnt a function and works better and so on....
 * 
 * Revision 1.3  1993/11/08  18:53:28  mahk
 * WHOOOPS
 * 
 * Revision 1.2  1993/11/08  18:33:47  mahk
 * Added RECT_FILL and MakePoint
 * 
 * Revision 1.1  1993/04/22  13:06:17  rex
 * Initial revision
 * 
 * Revision 1.2  1993/04/19  18:35:01  rex
 * Added macro versions of most functions
 * 
 * Revision 1.1  1992/08/31  17:03:04  unknown
 * Initial revision
 * 
*/


#ifndef RECT_H
#define RECT_H

//	Here are the Point and LGRect structs

typedef struct {
	short x;
	short y;
} LGPoint;

typedef struct {
	LGPoint ul;
	LGPoint lr;
} LGRect;

//	Point macros

#define PointsEqual(p1,p2) (*(long*)(&(p1)) == *(long*)(&(p2)))
#define PointSetNull(p) do {(p).x = -1; (p).y = -1;} while (0);
#define PointCheckNull(p) ((p).x == -1 && (p).y == -1)

//	LGRect macros: get width & height

#define RectWidth(pr) ((pr)->lr.x - (pr)->ul.x)
#define RectHeight(pr) ((pr)->lr.y - (pr)->ul.y)

// Unwrap macros

#define RECT_UNWRAP(pr)  ((pr)->ul.x),((pr)->ul.y),((pr)->lr.x),((pr)->lr.y)
#define PT_UNWRAP(pt)    ((pt).x),((pt).y)

//	These macros are faster, fatter versions of their function counterparts

#define RECT_TEST_SECT(pr1,pr2) ( \
	((pr1)->ul.y < (pr2)->lr.y) && \
	((pr1)->lr.y > (pr2)->ul.y) && \
	((pr1)->ul.x < (pr2)->lr.x) && \
	((pr1)->lr.x > (pr2)->ul.x))

#define RECT_UNION(pr1,pr2,prunion) { \
	(prunion)->ul.x = (pr1)->ul.x < (pr2)->ul.x ? (pr1)->ul.x : (pr2)->ul.x; \
	(prunion)->lr.x = (pr1)->lr.x > (pr2)->lr.x ? (pr1)->lr.x : (pr2)->lr.x; \
	(prunion)->ul.y = (pr1)->ul.y < (pr2)->ul.y ? (pr1)->ul.y : (pr2)->ul.y; \
	(prunion)->lr.y = (pr1)->lr.y > (pr2)->lr.y ? (pr1)->lr.y : (pr2)->lr.y; \
	}

#define RECT_ENCLOSES(pr1,pr2) ( \
	((pr1)->ul.y <= (pr2)->ul.y) && \
	((pr1)->lr.y >= (pr2)->lr.y) && \
	((pr1)->ul.x <= (pr2)->ul.x) && \
	((pr1)->lr.x >= (pr2)->lr.x))

#define RECT_TEST_PT(prect,pt) ( \
	((pt).y >= (prect)->ul.y) && ((pt).y < (prect)->lr.y) && \
	((pt).x >= (prect)->ul.x) && ((pt).x < (prect)->lr.x))

#define RECT_MOVE(prect,pt) { \
	(prect)->ul.x += pt.x; \
	(prect)->ul.y += pt.y; \
	(prect)->lr.x += pt.x; \
	(prect)->lr.y += pt.y; \
	}

#define RECT_OFFSETTED_RECT(pr1,pt,proff) { \
	(proff)->ul.x = (pr1)->ul.x + (pt).x; \
	(proff)->ul.y = (pr1)->ul.y + (pt).y; \
	(proff)->lr.x = (pr1)->lr.x + (pt).x; \
	(proff)->lr.y = (pr1)->lr.y + (pt).y; \
	}

#define RECT_FILL(pr,x,y,x2,y2) \
   { \
      (pr)->ul.x = (x);  \
      (pr)->ul.y = (y);  \
      (pr)->lr.x = (x2); \
      (pr)->lr.y = (y2); \
   }

//	These are the functional versions of the above macros

int RectTestSect(LGRect *pr1, LGRect *pr2);
void RectUnion(LGRect *pr1, LGRect *pr2, LGRect *prunion);
int RectEncloses(LGRect *pr1, LGRect *pr2);
int RectTestPt(LGRect *prect, LGPoint pt);
void RectMove(LGRect *pr, LGPoint delta);
void RectOffsettedRect(LGRect *pr, LGPoint delta, LGRect *proff);

//	These functions have no macro counterparts
int RectSect(LGRect *pr1, LGRect *pr2, LGRect *prsect);
int RectClipCode(LGRect *prect, LGPoint pt);

// guess why this isnt a macro        // hah, you cant
//Point MakePoint(short x, short y);  // Guess what this does. 
//#define MakePoint(x,y) (Point)(((ushort)y<<16)+((ushort)x))
// oh, doug is mocked, you cant cast to a non-scaler type
LGPoint MakePoint(short x, short y);

/*
// take this, note ax and bx passed but use whole thing... oooooh
Point MakePointInline(ushort x, ushort y);
#pragma aux MakePointInline = \
   "shl     ebx,10H"          \
   "and     eax,0000ffffH"    \
   "add     eax,ebx"          \
   parm [ax] [bx]             \
   modify [eax ebx];
// and this
#define MakePoint(x,y) MakePointInline((ushort)x,(ushort)y)
// curse you
// note i had to specify ax and bx even though i dont care what is really used
// we would like to say [ax bx cx dx] and then have the code use arg1 and arg2
// however, we cant, because there is no way of doing that, because lifeispain
// so the compiler generates things like mov eax,ebx;mov ebx,ecx;code as above
// which is dumb, since we could do the above with bx and cx just as well. ick
*/
#endif
