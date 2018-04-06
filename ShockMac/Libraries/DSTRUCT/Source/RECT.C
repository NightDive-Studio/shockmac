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
//		Rect.C		Rectangle-handling routines.
//		Rex E. Bradford (REX)
//
//		This module implements a bunch of rectangle-manipulation
//		routines.  Rectangles are defined such that the upper-left
//		point is inside the rectangle, and the lower-right point is
//		outside.
//
//		Thus, if the rect.ul.x == rect.lr.x OR rect.ul.y == rect.lr.y,
//		the rectangle is empty.
//
//		Also, a rect's width = rect.lr.x - rect.ul.x,
//		and its height = rect.lr.y - rect.ul.y.
/*
* $Header: n:/project/lib/src/dstruct/RCS/rect.c 1.3 1994/04/05 04:04:13 dc Exp $
* $log$
*/

#include "rect.h"

//	--------------------------------------------------------
//
//	RectTestSect() tests if two rectangles intersect.
//
//		pr1 = ptr to 1st rectangle
//		pr2 = ptr to 2nd rectangle
//
//	returns: TRUE if rectangles intersect, FALSE if disjoint.

int RectTestSect(LGRect *pr1, LGRect *pr2)
{
	return(RECT_TEST_SECT(pr1,pr2));
}

//	--------------------------------------------------------
//
//	RectSect() finds intersection of two rects.
//
//		pr1    = ptr to 1st rectangle
//		pr2    = ptr to 2nd rectangle
//		prsect = ptr to rectangle, filled with intersection of the
//			two rects (caution: if no intersection, this rect is undefined!)
//
//	returns: TRUE IF rectangles intersect, FALSE if disjoint (in this
//		case, *prsect is undefined)

int RectSect(LGRect *pr1, LGRect *pr2, LGRect *prsect)
{
	if (!RECT_TEST_SECT(pr1, pr2))
		return(FALSE);

	prsect->ul.x = pr1->ul.x > pr2->ul.x ? pr1->ul.x : pr2->ul.x;
	prsect->lr.x = pr1->lr.x < pr2->lr.x ? pr1->lr.x : pr2->lr.x;
	prsect->ul.y = pr1->ul.y > pr2->ul.y ? pr1->ul.y : pr2->ul.y;
	prsect->lr.y = pr1->lr.y < pr2->lr.y ? pr1->lr.y : pr2->lr.y;

	return(TRUE);
}

//	---------------------------------------------------------
//
//	RectUnion() finds union of two rects.
//
//		pr1     = ptr to 1st rectangle
//		pr2     = ptr to 2nd rectangle
//		prunion = ptr to rectangle, filled with union of the two rects

void RectUnion(LGRect *pr1, LGRect *pr2, LGRect *prunion)
{
	RECT_UNION(pr1, pr2, prunion);
}

//	---------------------------------------------------------
//
//	RectEncloses() tests whether first rect fully encloses second.
//
//		pr1 = ptr to 1st rectangle
//		pr2 = ptr to 2nd rectangle
//
//	returns: TRUE if *pr1 encloses *pr2, FALSE otherwise

int RectEncloses(LGRect *pr1, LGRect *pr2)
{
	return(RECT_ENCLOSES(pr1, pr2));
}

//	---------------------------------------------------------
//
//	RectTestPt() tests whether point is inside rect.
//
//		prect = ptr to rectangle
//		pt    = point to be tested
//
//	returns: TRUE if point is within rectangle, FALSE if outside

int RectTestPt(LGRect *prect, LGPoint pt)
{
	return(RECT_TEST_PT(prect, pt));
}

//	---------------------------------------------------------
//
//	RectMove() moves a rectangle by a delta.
//
//		pr    = ptr to rectangle
//		delta = point to move rectangle by

void RectMove(LGRect *pr, LGPoint delta)
{
	RECT_MOVE(pr, delta);
}

//	---------------------------------------------------------
//
//	RectOffsettedRect() creates a rectangle, offsetted from another.
//
//		pr    = ptr to original rect
//		delta = pt to offset by
//		proff = rectangle to fill in with offsetted rect

void RectOffsettedRect(LGRect *pr, LGPoint delta, LGRect *proff)
{
	RECT_OFFSETTED_RECT(pr, delta, proff);
}

//	---------------------------------------------------------
//
//	RectClipCode() calculates 4-bit clipcode for pt vs. rect.
//
//		prect = ptr to rectangle
//		pt    = point to be tested
//
//	Returns: a 4-bit clipcode, bits set as follows:
//
//	000x: set to 1 if pt.x < rect.ul.x
//	00x0: set to 1 if pt.x >= rect.lr.x
//	0x00: set to 1 if pt.y < rect.ul.y
//	x000: set to 1 if pt.y >= rect.lr.y
//
//	thus set to 0 if point is inside rect, although a cheaper test can be
//	done (via RectTestPt()).

int RectClipCode(LGRect *prect, LGPoint pt)
{
	short flag;

	flag = 0;
	if (pt.x < prect->ul.x)
		flag = 1;
	if (pt.x >= prect->lr.x)
		flag |= 2;
	if (pt.y < prect->ul.y)
		flag |= 4;
	if (pt.y >= prect->lr.y)
		flag |= 8;

	return(flag);
}

//	---------------------------------------------------------
LGPoint MakePoint(short x, short y)
{
	LGPoint	pt;
	
	pt.x = x;
	pt.y = y;
	return (pt);
}
