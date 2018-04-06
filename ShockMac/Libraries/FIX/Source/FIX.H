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
 * $Source: r:/prj/lib/src/fix/RCS/fix.h $
 * $Revision: 1.41 $
 * $Author: jaemz $
 * $Date: 1994/08/18 18:10:21 $
 *
 * Code, prototypes and types for fixed-point routines.
 *
 * $Log: fix.h $
 * Revision 1.41  1994/08/18  18:10:21  jaemz
 * Added sloppy_sqrt
 * 
 * Revision 1.40  1994/08/11  12:11:14  dfan
 * multiple source directories
 * 
 * Revision 1.39  1994/03/16  10:29:25  lmfeeney
 * ,
 * added extern fn fix_pow
 * 
 * Revision 1.38  1994/03/16  10:29:25  dfan
 * fix_safe_pyth_dist etc.
 * also conversions to and from degrees
 * 
 * Revision 1.37  1994/01/22  19:05:46  dc
 * fast_fix_mul_int
 * 
 * Revision 1.36  1994/01/22  19:04:22  dc
 * shrd not shr\shl\or for fix_mul
 * neat
 * 
 * Revision 1.34  1993/11/11  13:50:46  rex
 * Changed fix_from_float() to much simpler macro, added atofix() and atofix24()
 * 
 * Revision 1.33  1993/11/05  13:56:16  dfan
 * 2pi was wrong.
 * 
 * Revision 1.32  1993/11/04  11:13:42  dfan
 * long_fast_pyth_dist
 * 
 * Revision 1.31  1993/09/17  13:03:36  dfan
 * fast_pyth_dist
 * 
 * Revision 1.30  1993/08/17  15:10:04  kaboom
 * Added fix_cint and fix_fint macros.
 * 
 * Revision 1.29  1993/07/30  12:42:58  dfan
 * fix_exp
 * 
 * Revision 1.28  1993/07/07  12:26:58  xemu
 * fixed a numerical error in fixang_to_fixrad
 * 
 * Revision 1.27  1993/07/02  15:10:01  xemu
 * conversion from fixed-point radians to fixangs
 * 
 * Revision 1.26  1993/06/27  02:30:47  dc
 * added char return to fix_sprint prototypes, added fix_sprint_hex prototypes
 * 
 * Revision 1.25  1993/06/07  10:29:36  jak
 * Reversed #pragma and C decls for some functions
 * so that C++ parser will be happy.
 * 
 * Revision 1.24  1993/06/05  07:38:36  mahk
 * Added abs, sgn, FIXANG_PI
 * 
 * Revision 1.23  1993/04/19  13:31:42  dfan
 * individual sin & cos functions
 * 
 * Revision 1.22  1993/04/14  17:13:23  jaemz
 * Added FIX_MIN and FIX_MAX
 * 
 * Revision 1.21  1993/04/14  16:57:23  jaemz
 * Added floor and ceil
 * 
 * Revision 1.20  1993/04/07  10:15:06  matt
 * Removed include of math.h, which seemed unneccesary.
 * 
 * Revision 1.19  1993/03/15  16:55:45  matt
 * Added include of "types.h"
 * 
 * Revision 1.18  1993/03/03  14:46:59  dfan
 * fix_from_float: short should have been ushort to prevent nasty sign-extend
 * 
 * Revision 1.17  1993/03/03  11:50:53  dfan
 * float conversion
 * 
 * Revision 1.16  1993/02/16  10:44:33  matt
 * Added parens around macro args
 * 
 * 
 * Revision 1.15  1993/02/15  12:15:29  dfan
 * more fix24 functions
 * 
 * Revision 1.14  1993/02/15  11:40:22  dfan
 * fix24 support
 * 
 * Revision 1.13  1993/02/04  16:25:33  matt
 * Added new fix_mul_div() function
 * 
 * Revision 1.12  1993/01/29  11:10:45  dfan
 * hey, fix_sqrt returns a 32-bit value
 * 
 * Revision 1.11  1993/01/29  10:42:34  dfan
 * type in fix_sqrt pragma
 * 
 * Revision 1.10  1993/01/27  16:47:16  dfan
 * sqrt functions
 * by the way, the arctrig functions did work after all
 * 
 * Revision 1.9  1993/01/22  15:52:36  dfan
 * Asin, acos, and atan2 do work after all
 * 
 * Revision 1.8  1993/01/22  09:57:19  dfan
 * Added lots of functions, including trig ones
 * Arctrig doesn't work yet
 * 
 * Revision 1.7  1992/10/14  23:37:46  kaboom
 * Added fix_rint macro to round & take integer part.
 * 
 * Revision 1.6  1992/10/13  22:34:24  kaboom
 * Added #ifdef __FIX_H clause around body of file.
 * 
 * Revision 1.5  1992/09/16  19:52:06  kaboom
 * Modified the fix_int macro not to round off before converting fix to int.
 * Added fix_trunc to remove fractional part, fix_frac to return fractional
 * part.
 * 
 * Revision 1.4  1992/09/16  19:28:26  kaboom
 * Changed fix_mul and fix_div to be inline assembly-language functions
 * instead of called functions.  That makes this the only file in the
 * fixed-point math library and fix.asm is obsolete.
 * 
 * Revision 1.3  1992/09/15  14:08:22  kaboom
 * Made typedef for fixed point variables.  Added fix_make macro.
 * 
 * Revision 1.2  1992/08/24  17:27:17  kaboom
 * Added RCS keywords and header at top of file.
 * 
 * Revision 1.1  1992/08/24  17:27:17  kaboom
 * Initial revision.
 */

#ifndef __FIX_H
#define __FIX_H

#include "lg_types.h"

// Globals
extern int	gOVResult;


//////////////////////////////
//
// First some math functions that don't use fixes.
//
/*  ��� Change this
// Returns 0 if x < 0
ushort long_sqrt (long x);
#pragma aux long_sqrt parm [eax] value [ax] modify [eax ebx ecx edx esi edi]
*/

//////////////////////////////
//
// fix.c
//

long long_fast_pyth_dist (long a, long b);
long long_safe_pyth_dist (long a, long b);


//========================================
//
// Now for fixes themselves.  Macros.
//
//========================================

/* these functions operate on fixed-point numbers with one bit of sign, 15
   bits of integer, and 16 bits of fraction.  thus, a rational number a is
   represented as a 32-bit number as a*2^16. */

typedef long fix;
typedef fix fix16;

// define min and max
#define FIX_MAX (0x7fffffff)
#define FIX_MIN (0x80000000)

// A fixang (fixed-point angle) can be converted to radians by multiplying
// by 2 * PI and dividing by 2^16.
//
// 0x0000 -> 0
// 0x4000 -> PI/2
// 0x8000 -> PI
// 0xc000 -> 3PI/2
//

#define FIXANG_PI 0x8000
#define fix_2pi fix_make(6,18559) // that's 6 + 18559/65536 = 6.28319

typedef ushort fixang;

/* makes a fixed point number with integral part a and fractional part b. */
#define fix_make(a,b) ((((long)(a))<<16)|(b))

#define FIX_UNIT fix_make(1,0)

/* lops off the fractional part of a fixed point number. */
#define fix_trunc(n) ((n)&0xffff0000)

/* Does a floor */
#define fix_floor(n) ((n)&0xffff0000)

/* Does a ceil */
#define fix_ceil(n) (((n)+65535)&0xffff0000)

/* round a fix to the nearest integer, leaving in fix format. */
#define fix_round(n) (((n)+32768)&0xffff0000)

/* returns the integral part of a fixed point number. */
#define fix_int(n) ((n)>>16)

// Absolute value and signum
#define fix_abs(n) (((n) < 0) ? -(n) : (n))
#define fix_sgn(n) (((n) < 0) ? -FIX_UNIT : (((n) == 0) ? 0 : FIX_UNIT)) 

/* converts the floor of n to an integer. */
#define fix_fint(n) ((n)>>16)

/* converts the ceiling of n to an integer. */
#define fix_cint(n) (((n)+0xffff)>>16)

/* returns the integral part of a fixed point number rounded up. */
// #define fix_rint(n) (fix_int (fix_round (n)))
// the following macro does it all explictly to avoid the spurious & in fix_round
#define fix_rint(n) (((n)+0x8000)>>16)

/* returns the fractional part of a fixed point number. */
#define fix_frac(n) ((n)&0xffff)

// fixrad_to_fixang converts a fixed-point in radians to a fixang
// fixang_to_fixrad converts a fixang to a fixed point radians
// degrees_to_fixang converts an integer number of degrees to a fixang
// fixang_to_degrees converts a fixang to an integer number of degrees

#define fixrad_to_fixang(fixradian) (fix_frac(fix_div((fixradian),fix_2pi)))
#define fixang_to_fixrad(ang) fix_div(fix_mul(ang,fix_2pi),0x10000)
#define degrees_to_fixang(d)    ((fixang)(((d)*FIXANG_PI)/180))
#define fixang_to_degrees(ang)  (((long)(ang)*180)/FIXANG_PI)

// turns a fixed point into a float.
#define fix_float(n) ((float)(fix_int(n)) + (float)(fix_frac(n))/65536.0)

// makes a fixed point from a float.
#define fix_from_float(n) ((fix)(65536.0*(n)))

//========================================
//
// Multiplication and division.
//
//========================================

// For Mac version: The PowerPC version uses two assembly language routines
// to do the multiply and divide.
#if defined(powerc) || defined(__powerc)

extern "C"
{
fix fix_mul_asm(fix a, fix b);
fix fast_fix_mul_int_asm(fix a, fix b);
fix fix_div_asm(fix a, fix b);
fix fix_mul_div_asm (fix m0, fix m1, fix d);
}
#define fix_mul fix_mul_asm
#define fast_fix_mul fix_mul_asm
#define fast_fix_mul_int fast_fix_mul_int_asm
#define fix_div fix_div_asm
#define fix_mul_div fix_mul_div_asm

#else

fix fix_mul(fix a, fix b);
fix fix_div(fix a, fix b);
fix fix_mul_div (fix m0, fix m1, fix d);
fix fast_fix_mul_int(fix a, fix b);
#define fast_fix_mul fix_mul

#endif


//========================================
//
//  Square rooty kind of stuff.
//
//========================================

// Returns sqrt (a^2 + b^2)
fix fix_pyth_dist (fix a, fix b);

// Returns approximately sqrt (a^2 + b^2)
// Is never off by more than 12% (it's worst at 45 deg)
fix fix_fast_pyth_dist (fix a, fix b);

// pyth_dist with less fear of overflow.  Either number
// can be up to 0x2fffffff.
fix fix_safe_pyth_dist (fix a, fix b);

// Now in FIX_SQRT.C
// Returns 0 if x < 0
fix fix_sqrt (fix x);

#if defined(powerc) || defined(__powerc)
long quad_sqrt(long hi, long lo);
#else
asm long quad_sqrt(long hi, long lo);
#endif


// Returns 0 if x < 0
/* ��� Fix this
fix fix_sloppy_sqrt (fix x);
#pragma aux fix_sloppy_sqrt parm [eax] value [eax] modify [eax ebx ecx edx esi edi]
*/

//========================================
//
//  Trigonometric functions.
//
//========================================

// Computes sin and cos of theta
void fix_sincos (fixang theta, fix *sin, fix *cos);

fix fix_sin (fixang theta);

fix fix_cos (fixang theta);

// Computes sin and cos of theta
// Faster than fix_sincos() but not as accurate (does not interpolate)
void fix_fastsincos (fixang theta, fix *sin, fix *cos);

fix fix_fastsin (fixang theta);

fix fix_fastcos (fixang theta);

// Computes the arcsin of x
fixang fix_asin (fix x);

// Computes the arccos of x
fixang fix_acos (fix x);

// Computes the atan of y/x, in the correct quadrant and everything
fixang fix_atan2 (fix y, fix x);


//========================================
//
//  String/fix conversions.
//
//========================================

//	Converts string into fixed-point
fix atofix(char *p);

// Puts a decimal representation of x into str
char *fix_sprint (char *str, fix x);
char *fix_sprint_hex (char *str, fix x);

/* fixpoint x ^ y */
extern fix fix_pow (fix x, fix y);

//////////////////////////////
//
// f_exp.c

// Computes e to the x
//
fix fix_exp (fix x);

//////////////////////////////
//
// fix24 - 24 bits integer, 8 bits fraction
//
// fix24.c

typedef long fix24;

#define fix24_make(a,b) ((((long)(a))<<8)|(b))
#define fix24_trunc(n) ((n)&0xffffff00)
#define fix24_round(n) (((n)+128)&0xffffff00)
#define fix24_int(n) ((n)>>8)
#define fix24_rint(n) (((n)+128)>>8)
#define fix24_frac(n) ((n)&0xff)
#define fix24_float(n) ((float)(fix24_int(n)) + (float)(fix24_frac(n))/256.0)
#define fix24_from_float(n) (fix24_make((short)(floor(n)), (ushort)((n-floor(n))*256.0)))

#define fix24_from_fix16(n) ((n)>>8)
#define fix16_from_fix24(n) ((n)<<8)

#define fix16_to_fix24(n) (fix24_from_fix16(n))
#define fix24_to_fix16(n) (fix16_from_fix24(n))

// For Mac version: The PowerPC version uses an assembly language routine
// to do the multiply.
#if defined(powerc) || defined(__powerc)

extern "C"
{
fix24 fix24_mul_asm(fix24 a, fix24 b);
fix24 fix24_div_asm(fix24 a, fix24 b);
}
#define fix24_mul fix24_mul_asm
#define fix24_div fix24_div_asm

#else

fix24 asm fix24_mul(fix24 a, fix24 b);
fix24 asm fix24_div (fix24 a, fix24 b);

#endif

/* ��� Fix this
fix24 fix24_mul_div (fix24 m0, fix24 m1, fix24 d);
#pragma aux fix24_mul_div =\
   "imul    edx"     \
   "idiv    ebx"     \
   parm [eax] [edx] [ebx]  \
   modify [eax edx];
*/

fix24 fix24_pyth_dist (fix24 a, fix24 b);
fix24 fix24_fast_pyth_dist (fix24 a, fix24 b);
fix24 fix24_safe_pyth_dist (fix24 a, fix24 b);
/* ��� Fix this
fix24 fix24_sqrt (fix24 x);
#pragma aux fix24_sqrt parm [eax] value [eax] modify [eax ebx ecx edx esi edi]
*/
void fix24_sincos (fixang theta, fix24 *sin, fix24 *cos);
fix24 fix24_sin (fixang theta);
fix24 fix24_cos (fixang theta);
void fix24_fastsincos (fixang theta, fix24 *sin, fix24 *cos);
fix24 fix24_fastsin (fixang theta);
fix24 fix24_fastcos (fixang theta);
fixang fix24_asin (fix24 x);
fixang fix24_acos (fix24 x);
fixang fix24_atan2 (fix24 y, fix24 x);
fix24 atofix24(char *p);
char *fix24_sprint (char *str, fix24 x);
char *fix24_sprint_hex (char *str, fix24 x);


//============================================
//
//  Other multiply/div/add variants used by 2D and 3D.
//
//============================================
struct AWide
{
	long			hi;
	unsigned long	lo;
};
typedef struct AWide AWide;

#if defined(powerc) || defined(__powerc)
extern "C"
{
extern fix fix_mul_3_3_3_asm (fix a, fix b);
extern fix fix_mul_3_32_16_asm (fix a, fix b);
extern fix fix_mul_3_16_20_asm (fix a, fix b);
extern fix fix_mul_16_32_20_asm (fix a, fix b);
extern AWide *AsmWideAdd(AWide *target, AWide *source);
extern AWide *AsmWideSub(AWide *target, AWide *source);
extern AWide *AsmWideMultiply(long multiplicand, long multiplier, AWide *target);
extern long AsmWideDivide(long hi, long lo, long den);

// since these aren't implemented yet in our PPC code yet, we just call the fixMath versions
// extern AWide *AsmWideNegate(AWide *target);
// extern AWide *AsmWideBitShift(AWide *src, long shift);
#define AsmWideNegate(target) (AWide *) WideNegate((wide *) target)
#define AsmWideBitShift(target,count) (AWide *) WideBitShift((wide *) target,count)
}

#else
extern asm AWide *AsmWideAdd(AWide *target, AWide *source);
extern asm AWide *AsmWideMultiply(long multiplicand, long multiplier, AWide *target);
extern asm long AsmWideDivide(long hi, long lo, long divisor);
extern asm AWide *AsmWideNegate(AWide *target);
extern asm AWide *AsmWideBitShift(AWide *src, long shift);
#endif


#endif /* !__fix24_H */
