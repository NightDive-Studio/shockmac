#
# Copyright (C) 2015-2018 Night Dive Studios, LLC.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# 
#  void BlitLargeAlign(uchar *draw_buffer, int dstRowBytes, void *dstPtr, long w, long h, long modulus)
#                      r3                  r4               r5            r6       r7       r8
# 
	csect	.BlitLargeAlign[PR]
	
	export	.BlitLargeAlign
	
fpTemp:		EQU	fp0
rSrcPtr:		EQU	r3
rDblDStrd:	EQU	r4
rDst1Ptr:	EQU	r5
rWidth:		EQU	r6
rHeight:		EQU	r7
rModulus:	EQU	r8
rSStrd:		EQU	r9
rLDblPtr:	EQU	r30
rDst2Ptr:	EQU	r31

.BlitLargeAlign:
	stw		r31,-4(SP)				# store non-volatile reg in red zone
	addi	 	rDst1Ptr,rDst1Ptr,-8	# subtract 8 from dst
	stw		r30,-8(SP)				# store non-volatile reg in red zone

	la     	rLDblPtr,-16(SP)		# calculate copy of local 8-byte variable
	sub      rSStrd,rModulus,rWidth
											# rSStrd = modulus - w
	add		rDst2Ptr,rDst1Ptr,r4	# dst2 = dstRowBytes + dst1
	sub      r4,r4,rWidth			# r4 = dstRowBytes - w
	addi     rHeight,rHeight,-1	# subtract 1 from height count
	srawi    rWidth,rWidth,2		# rWidth = w >> 2
	addi	 	rSrcPtr,rSrcPtr,-4	# subtract 4 from src
	addi		rWidth,rWidth,-1		# subtract 1 from width count
	add      rDblDStrd,r4,r4		# rDblDStrd = 2 * r4

BlitLargeAlignY:						# y count is in rHeight
	lwzu     r10,4(rSrcPtr)			# load a long into r10
	mr       r0,r10  					# put a copy in r0
	mr       r11,r10
	inslwi   r0,r10,16,8
	insrwi   r11,r10,16,8
	rlwimi   r0,r10,16,24,31
	stw      r0,0(rLDblPtr)
	rlwimi   r11,r10,16,0,7
	stw      r11,4(rLDblPtr)
	mtctr	 	rWidth					# copy x count into the counter
	lfd      fpTemp,0(rLDblPtr)

BlitLargeAlignX:
	lwzu     r10,4(rSrcPtr)			# load a long into r10
	stfdu    fpTemp,8(rDst1Ptr)
	mr       r0,r10  					# put a copy in r0
	mr       r11,r10
	inslwi   r0,r10,16,8
	insrwi   r11,r10,16,8
	rlwimi   r0,r10,16,24,31
	stw      r0,0(rLDblPtr)
	rlwimi   r11,r10,16,0,7
	stw      r11,4(rLDblPtr)
	stfdu    fpTemp,8(rDst2Ptr)
	lfd      fpTemp,0(rLDblPtr)
	bdnz	 	BlitLargeAlignX		# loop over all x

	stfdu    fpTemp,8(rDst1Ptr)
	addic.   rHeight,rHeight,-1	# decrement the counter
	add      rSrcPtr,rSrcPtr,rSStrd
											# src += sstride
	add      rDst1Ptr,rDst1Ptr,rDblDStrd		
											# dst1 += dstride
	stfdu    fpTemp,8(rDst2Ptr)
	add      rDst2Ptr,rDst2Ptr,rDblDStrd
											# dst2 += dstride
	bne      BlitLargeAlignY      # loop for all y

	lwz		r30,-8(SP)				# restore non-volatile regs
	lwz		r31,-4(SP)				# restore non-volatile regs
	blr									# return to caller



# 
#  void BlitLargeAlignSkip(uchar *draw_buffer, int dstRowBytes, void *dstPtr, long w, long h, long modulus)
#                      r3                  r4               r5            r6       r7       r8
# 
# only does even lines
#
	csect	.BlitLargeAlignSkip[PR]
	
	export	.BlitLargeAlignSkip
	
.BlitLargeAlignSkip:
#	stw		r31,-4(SP)				# store non-volatile reg in red zone
	addi	 	rDst1Ptr,rDst1Ptr,-8	# subtract 8 from dst
	stw		r30,-4(SP)				# store non-volatile reg in red zone

	la     	rLDblPtr,-16(SP)		# calculate copy of local 8-byte variable
	sub      rSStrd,rModulus,rWidth
											# rSStrd = modulus - w
#	add		rDst2Ptr,rDst1Ptr,r4	# dst2 = dstRowBytes + dst1
	sub      r4,r4,rWidth			# r4 = dstRowBytes - w
	addi     rHeight,rHeight,-1	# subtract 1 from height count
	srawi    rWidth,rWidth,2		# rWidth = w >> 2
	addi	 	rSrcPtr,rSrcPtr,-4	# subtract 4 from src
	addi		rWidth,rWidth,-1		# subtract 1 from width count
	add      rDblDStrd,r4,r4		# rDblDStrd = 2 * r4

BlitLargeAlignSkipY:					# y count is in rHeight
	lwzu     r10,4(rSrcPtr)			# load a long into r10
	mr       r0,r10  					# put a copy in r0
	mr       r11,r10
	inslwi   r0,r10,16,8
	insrwi   r11,r10,16,8
	rlwimi   r0,r10,16,24,31
	stw      r0,0(rLDblPtr)
	rlwimi   r11,r10,16,0,7
	stw      r11,4(rLDblPtr)
	mtctr	 	rWidth					# copy x count into the counter
	lfd      fpTemp,0(rLDblPtr)

BlitLargeAlignSkipX:
	lwzu     r10,4(rSrcPtr)			# load a long into r10
	stfdu    fpTemp,8(rDst1Ptr)
	mr       r0,r10  					# put a copy in r0
	mr       r11,r10
	inslwi   r0,r10,16,8
	insrwi   r11,r10,16,8
	rlwimi   r0,r10,16,24,31
	stw      r0,0(rLDblPtr)
	rlwimi   r11,r10,16,0,7
	stw      r11,4(rLDblPtr)
#	stfdu    fpTemp,8(rDst2Ptr)
	lfd      fpTemp,0(rLDblPtr)
	bdnz	 	BlitLargeAlignSkipX	# loop over all x

	stfdu    fpTemp,8(rDst1Ptr)
	addic.   rHeight,rHeight,-1	# decrement the counter
	add      rSrcPtr,rSrcPtr,rSStrd
											# src += sstride
	add      rDst1Ptr,rDst1Ptr,rDblDStrd		
											# dst1 += dstride
#	stfdu    fpTemp,8(rDst2Ptr)
#	add      rDst2Ptr,rDst2Ptr,rDblDStrd
											# dst2 += dstride
	bne      BlitLargeAlignSkipY  # loop for all y

	lwz		r30,-4(SP)				# restore non-volatile regs
#	lwz		r31,-4(SP)				# restore non-volatile regs
	blr									# return to caller


