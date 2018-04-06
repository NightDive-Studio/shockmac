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
#if !(defined(powerc) || defined(__powerc))	

// This file contains a nasty macro define that is used to simulate the
// Fix_Div call, so its inline for 68K routines, and handles the divide
// by zero case correctly

// this macro does a divs.l d3,d1:d0, with the fix_div setup for reg and zero checking
#define fix_div_68k_d3(reg) \
	tst.l		d3;							\
	dc.w		0x6712;		    	\
 	moveq		#0,d1; 					\
 	move.l	reg,d0;					\
 	swap		d0; 						\
 	move.w	d0,d1; 					\
 	ext.l		d1;							\
 	clr.w		d0; 						\
	dc.l		0x4C430C01;			\
	dc.w		0x680C;		    	\
	move.l	#0x7FFFFFFF,d0;	\
	tst.l		reg;						\
	dc.w		0x6A02;					\
	neg.l		d0;							
		
/*
// this macro does a divs.l d2,d1:d0, with the fix_div setup for reg and zero checking
#define fix_div_68k_d2(reg) \
	tst.l		d2;							\
	dc.w		0x6712;					\
 	moveq		#0,d1; 					\
 	move.l	reg,d0;					\
 	swap		d0; 						\
 	move.w	d0,d1; 					\
 	ext.l		d1;							\
 	clr.w		d0; 						\
	dc.l		0x4C420C01;			\
	dc.w		0x680C;					\
	move.l	#0x7FFFFFFF,d0;	\
	tst.l		reg;						\
	dc.w		0x6A02;					\
	neg.l		d0;							
*/

// this macro does a divs.l d2,d1:d0, with the fix_div setup for reg and zero checking
#define fix_div_68k_d2_d0(tempreg) \
	move.l	d0,tempreg;			\
	tst.l		d2;							\
	dc.w		0x6710;					\
 	moveq		#0,d1; 					\
 	swap		d0; 						\
 	move.w	d0,d1; 					\
 	ext.l		d1;							\
 	clr.w		d0; 						\
	dc.l		0x4C420C01;			\
	dc.w		0x680C;					\
	move.l	#0x7FFFFFFF,d0;	\
	tst.l		tempreg;				\
	dc.w		0x6A02;					\
	neg.l		d0;							

#define fix_mul_d2_d0(tempreg) 	\
	dc.l		0x4C020C01;						\
	move.w	d1,d0;								\
 	swap		d0; 									\
	move.l	d0,tempreg;

#endif
