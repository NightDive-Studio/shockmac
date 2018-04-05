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
//		Rnd.C		Random stream implementation
//		Rex E. Bradford (REX)
//
//	INTRODUCTION
//
//		Random streams constitute an interface for the functionality
//		of a set of deterministic "random number" streams.  These have
//		the property of producing an unvarying set of values given the
//		same starting "seed".  This file includes implementations for
//		several random number algorithms, which vary in their speed of
//		calculation and "randomness" of their output.
//
//		Using random streams over ad-hoc random number approaches has
//		the following advantages.
//
//		1. Switching to a new, better random number algorithm involves
//			only changing the random stream's declaration, not any calls
//			to get new random values.
//
//		2. Several independent random streams may be operating concurrently,
//			so that a module which needs a deterministic flow of random
//			values will not be disturbed by other modules' needs.
//
//		3. By saving and restoring a random stream's seed, a repeatable
//			flow of random values can be guaranteed.  Each stream is
//			independently controllable in this way.
//
//		4. Handy macros and functions are available to get a random
//			value scaled into a range, converted to fixed-point format, etc.
//
//	USING RANDOM STREAMS
//
//		To use a random stream, first declare it, for instance:
//
//		static RNDSTREAM_LC16(myRs);	// usually static or global, but not necc.
//
//		The random stream must be seeded!  The declaration sets the seed
//		value to 0, which may be inappropriate for some random streams,
//		especially those which transform the seed into an alternate range
//		or use seed time to create helper tables.  You may reseed at any
//		time:
//
//		RndSeed(&myRs,savedSeed);		// any ulong value will do
//
//		You can get the next random value produced by the stream via a variety
//		of macros and functions, depending on the type and range of random
//		value you want:
//
//		ulong rval = Rnd(&myRs);		// get next value (16-bit generators
//												// use high 16 bits, low 16 bits set to 0)
//
//		long rval = RndRange(&myRs,low,high);	// get next value scaled into
//												// range from low to high, inclusive
//
//		fix rval = RndFix(&myRs);		// get next value as fix, range 0 to .9999
//
//		fix rval = RndRangeFix(&myRs,low,high);	// get next value as fix,
//												// scaled into range from low to high
//
//	CREATING A NEW RANDOM STREAM CLASS
//
//		To create a new random stream class, you only need to define one
//		macro (in rnd.h) and two functions (in rnd.c, prototyped in rnd.h).
//		For example:
//
//		(in rnd.h):
//
//		#define RNDSTREAM_WHIZ(name) RndStream name = {0,RndWhiz,RndWhizSeed};
//		ulong RndWhiz(RndStream *prs);
//		void RndWhizSeed(RndStream *prs, ulong seed);
//
//		(in rnd.c):
//
//		ulong RndWhiz(RndStream *prs)
//			{
//			return(.....);
//			}
//
//		void RndWhizSeed(RndStream *prs, ulong seed)
//			{
//			..... (any 32-bit value should be acceptable, transform if needed)
//			prs->curr = ...;
//			}
//
//		If your random number generator normally works in 32-bit values, fine.
//		If it works in values less than 32 bits wide, you must ensure that
//		the values returned by your generator move those bits into the high
//		bits of the ulong.  Generators which use more than 32 bits are not
//		currently supported.
/*
* $Header: n:/project/lib/src/rnd/RCS/rnd.c 1.2 1993/06/01 10:59:38 rex Exp $
* $Log: rnd.c $
 * Revision 1.2  1993/06/01  10:59:38  rex
 * Turned stack checking off
 * 
 * Revision 1.1  1993/04/06  09:56:44  rex
 * Initial revision
 * 
*/

#include "lg.h"
#include "rnd.h"

//	For gruesome interrupt routines, let 'em have their way:

//еее#pragma off(check_stack);


//	---------------------------------------------------------------
//  Get the high 32-bit result of the unsigned multiply of 2 32-bit numbers.
//	---------------------------------------------------------------
/*
#pragma aux high_umpy =\								// Original 386 code.
   "mul    edx"      \
	"mov    eax,edx"  \
   parm [eax] [edx]  \
   modify [eax edx];
*/
#if defined(powerc) || defined(__powerc)

ulong high_umpy(ulong a, ulong b);					//  Code in RndAsm.s

#else
ulong high_umpy(ulong a, ulong b);					// Proto
ulong asm high_umpy(ulong a, ulong b)
 {
 	move.l	4(A7), d0
	dc.w		0x4C2F,0x0401,0x0008		// 	mulu.l	8(A7),d1:d0
	move.l	d1,d0
 	rts
 }
#endif

//	---------------------------------------------------------------
//		ROUTINES WHICH SCALE RNUMS INTO RANGE
//	---------------------------------------------------------------
//
//	RndRange() returns the next random value, scaled into an integer range.
//
//		prs  = ptr to random stream
//		low  = low value of range
//		high = high value of range
//
//	Returns: next random value scaled into range low->high, inclusive

long RndRange(RndStream *prs, long low, long high)
{
	return(low + high_umpy(Rnd(prs), (high - low) + 1));
}

//	----------------------------------------------------------------
//
//	RndRangeFix() returns the next random value, scaled into fixed-point range.
//
//		prs  = ptr to random stream
//		low  = low value of fixed-point range
//		high = high value of fixed-point range
//
//	Returns: next random value scaled into range low->high

fix RndRangeFix(RndStream *prs, fix low, fix high)
{
	return(low + (fix_mul(RndFix(prs), high - low)));
}

//	-----------------------------------------------------------------
//		RANDOM GENERATORS
//	-----------------------------------------------------------------
//
//	RndLc16() uses a 16-bit linear conguential method.

#define LC16_MULT 2053
#define LC16_ADD 13849

ulong RndLc16(RndStream *prs)
{
	prs->curr = (prs->curr * LC16_MULT) + LC16_ADD;	// only low 16 bits matter
	return(prs->curr << 16);								// move them to high 16
}

void RndLc16Seed(RndStream *prs, ulong seed)
{
	prs->curr = seed ^ (seed >> 16);		// make sure something in low 16 bits
}

//	-----------------------------------------------------------------
//
//	RndGauss16() uses multiple passes of a linear conguential method to
//	generate a random number with gaussian distribution.  It is slow.

#define NUM_PASSES

ulong RndGauss16(RndStream *prs)
{
	long gauss;
	ushort rnum;
	int i;

	gauss = 0;
	rnum = prs->curr;						// prs->curr uses only low 16 bits

	for (i = 0; i < 6; i++)				// add 6 rnums & subtract 6
	{
		rnum = (rnum * LC16_MULT) + LC16_ADD;
		gauss += rnum;
		rnum = (rnum * LC16_MULT) + LC16_ADD;
		gauss -= rnum;
	}

	gauss /= 12;							// scale by 12 since we did 12 rnums's
	if (gauss < -32768)					// and clamp to 16-bit short range
		gauss = -32768;
	else if (gauss > 32767)
		gauss = 32767;

	prs->curr = gauss + 32768;			// set our current rnum as ushort

	return(prs->curr << 16);			// return in high 16 bits
}

void RndGauss16Seed(RndStream *prs, ulong seed)
{
	prs->curr = seed ^ (seed >> 16);	// make sure something in low 16 bits
}

//	----------------------------------------------------------------
//
//	RndGauss16Fast() creates a gaussian distribution table the first
//	time it is called, and then does a single lc random number to
//	look up into the table with.

#define NUM_GAUSSBITS 13			// we'll use 13 bits of our 16 bit rnums
#define SIZE_GAUSSTABLE (1<<NUM_GAUSSBITS)	// # entries in table = 8192
#define MASK_GAUSSTABLE (SIZE_GAUSSTABLE-1)	// mask for lookup
static ushort *gaussTable;			// ptr to our 16K (gasp) table

ulong RndGauss16Fast(RndStream *prs)
{
//	Compute 16-bit lc rnum & look up 16-bit gaussian rnum in table.
//	Return it in high 16 bits.

	prs->curr = (prs->curr * LC16_MULT) + LC16_ADD;
	return((ulong)(gaussTable[prs->curr & MASK_GAUSSTABLE]) << 16);
}

void RndGauss16FastSeed(RndStream *prs, ulong seed)
{
	int i;
	ushort *pg;

//	If table not allocated, allocate it and fill it using the RndGauss16
//	generator.

	if (gaussTable == NULL)
	{
		pg = gaussTable = (ushort *)NewPtr(SIZE_GAUSSTABLE * sizeof(short));
		for (i = 0; i < SIZE_GAUSSTABLE; i++)
		{
			prs->curr = i;
			*pg++ = RndGauss16(prs) >> 16;
		}
	}

	prs->curr = seed ^ (seed >> 16);		// make sure something in low 16 bits
}
