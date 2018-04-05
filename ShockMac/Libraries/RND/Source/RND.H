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
//		Rnd.H		Random stream header file (see rnd.c for more info)
//		Rex E. Bradford (REX)
/*
* $Header: n:/project/lib/src/rnd/RCS/rnd.h 1.2 1993/04/06 10:33:57 rex Exp $
* $Log: rnd.h $
 * Revision 1.2  1993/04/06  10:33:57  rex
 * Fixed RndSeed() macro to pass seed!
 * 
 * Revision 1.1  1993/04/06  09:56:35  rex
 * Initial revision
 * 
*/

#ifndef RND_H
#define RND_H

#include "lg_types.h"
#include "fix.h"

//	A random stream

typedef struct RndStream_ {
	ulong curr;
	ulong (*f_Next)(struct RndStream_ *prs);
	void (*f_Seed)(struct RndStream_ *prs, ulong seed);
} RndStream;

//	To use a random stream, instantiate one (usually statically),
//	seed it, and then make calls to get rnums, like so:
//
//		static RNDSTREAM_STD(rs);			// declare a stream
//		RndSeed(&rs,22);						// or maybe 23
//		rval = Rnd(&rs);						// get any old rnum
//		rval = RndRange(&rs,1,6);			// throw dice
//		rfix = RndFix(&rs);					// maybe you'd like 0 to .9999
//		rfix = RndRangeFix(&rs,fl,fh);	// or fixed point in a range

//	Here are the random stream type declaration macros

#define RNDSTREAM_LC16(name) RndStream name = {0,RndLc16,RndLc16Seed}
#define RNDSTREAM_GAUSS16(name) RndStream name = {0,RndGauss16,RndGauss16Seed}
#define RNDSTREAM_GAUSS16FAST(name) RndStream name = {0,RndGauss16Fast,RndGauss16FastSeed}

#define RNDSTREAM_STD(name) RNDSTREAM_LC16(name)

//	Seed a random stream

#define RndSeed(prs,seed) ((prs)->f_Seed(prs,seed))

//	Get next random #

#define Rnd(prs) ((prs)->f_Next(prs))

//	Get next random # and scale into fixed range 0.0 to .9999

#define RndFix(prs) (fix_make(0,Rnd(prs)>>16))

//	Get next random # and scale into low->high range (high value included)

long RndRange(RndStream *prs, long low, long high);

//	Get next random # and scale into low->high range

fix RndRangeFix(RndStream *prs, fix low, fix high);

//	Prototypes for current set of random stream classes

ulong RndLc16(RndStream *prs);
void RndLc16Seed(RndStream *prs, ulong seed);

ulong RndGauss16(RndStream *prs);
void RndGauss16Seed(RndStream *prs, ulong seed);

ulong RndGauss16Fast(RndStream *prs);
void RndGauss16FastSeed(RndStream *prs, ulong seed);

#endif
