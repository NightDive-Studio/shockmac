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
#	Macintosh HD:MPW:MotoTools:cor
	#	"Motorola C Front End 1.6; Back End 1.6"
	.file	"SimpleTest.cp"
#	Options: -O4 -Atarg=601,inllev=5,sinllev=5

	.toc
T.FD:	.tc	FD.4330000080000000[tc]	,1127219200,-2147483648

#	This function came from file: "SimpleFixpp.h"
	.toc
T..__ct__8FixpointFRC8Fixpoint:
	.tc	..__ct__8FixpointFRC8Fixpoint[tc], __ct__8FixpointFRC8Fixpoint[ds]

	.align	2
	.csect	__ct__8FixpointFRC8Fixpoint[ds]
	.ualong	.__ct__8FixpointFRC8Fixpoint[pr], TOC[tc0], 0
	.csect	.__ct__8FixpointFRC8Fixpoint[pr]
.__ct__8FixpointFRC8Fixpoint:

#	start of prologue, stack size = 80

	mfspr	r0,lr
	stw	r31,-4(sp)
	stw	r0,8(sp)
	stwu	sp,-80(sp)

#	end of prologue

__ct__8FixpointFRC8Fixpoint.b:
	or.	r5,r3,r3
	or	r31,r4,r4
	bc	0x5,0x2,L..3
	addi	r3,r0,4
	bl	.__nw__FUi
	nop
	or.	r5,r3,r3
	bc	0xc,0x2,L..4
L..3:
	lwz	r3,0(r31)
	stw	r3,0(r5)
L..4:
	or	r3,r5,r5
	lwz	r0,88(sp)
	addi	sp,sp,80
	mtspr	lr,r0
	lwz	r31,-4(sp)
	bclr	0x14,0x0
FE_MOT_RESVD.__ct__8FixpointFRC8Fixpoint:

#	This function came from file: "SimpleFixpp.h"
	.toc
T..__pl__F8FixpointT1:
	.tc	..__pl__F8FixpointT1[tc], __pl__F8FixpointT1[ds]

	.align	2
	.csect	__pl__F8FixpointT1[ds]
	.ualong	.__pl__F8FixpointT1[pr], TOC[tc0], 0
	.csect	.__pl__F8FixpointT1[pr]
.__pl__F8FixpointT1:

#	start of prologue, stack size = 80

	mfspr	r0,lr
	stw	r0,8(sp)
	stwu	sp,-80(sp)

#	end of prologue

__pl__F8FixpointT1.b:
	stw	r4,108(sp)
	lwz	r6,0(r4)
	lwz	r5,0(r5)
	add	r6,r6,r5
	stw	r6,0(r4)
	lwz	r4,108(sp)
	bl	.__ct__8FixpointFRC8Fixpoint
	lwz	r0,88(sp)
	addi	sp,sp,80
	mtspr	lr,r0
	bclr	0x14,0x0
FE_MOT_RESVD.__pl__F8FixpointT1:

#	This function came from file: "SimpleFixpp.h"
	.toc
T..__ct__8FixpointFl:
	.tc	..__ct__8FixpointFl[tc], __ct__8FixpointFl[ds]

	.align	2
	.csect	__ct__8FixpointFl[ds]
	.ualong	.__ct__8FixpointFl[pr], TOC[tc0], 0
	.csect	.__ct__8FixpointFl[pr]
.__ct__8FixpointFl:

#	start of prologue, stack size = 80

	mfspr	r0,lr
	stw	r31,-4(sp)
	stw	r0,8(sp)
	stwu	sp,-80(sp)

#	end of prologue

__ct__8FixpointFl.b:
	or.	r5,r3,r3
	or	r31,r4,r4
	bc	0x5,0x2,L..8
	addi	r3,r0,4
	bl	.__nw__FUi
	nop
	or.	r5,r3,r3
	bc	0xc,0x2,L..9
L..8:
	rlwinm	r3,r31,0x10,0x0,0xf
	stw	r3,0(r5)
L..9:
	or	r3,r5,r5
	lwz	r0,88(sp)
	addi	sp,sp,80
	mtspr	lr,r0
	lwz	r31,-4(sp)
	bclr	0x14,0x0
FE_MOT_RESVD.__ct__8FixpointFl:

#	This function came from file: "SimpleFixpp.h"
	.toc
T..__ct__8FixpointFi:
	.tc	..__ct__8FixpointFi[tc], __ct__8FixpointFi[ds]

	.align	2
	.csect	__ct__8FixpointFi[ds]
	.ualong	.__ct__8FixpointFi[pr], TOC[tc0], 0
	.csect	.__ct__8FixpointFi[pr]
.__ct__8FixpointFi:

#	start of prologue, stack size = 80

	mfspr	r0,lr
	stw	r31,-4(sp)
	stw	r0,8(sp)
	stwu	sp,-80(sp)

#	end of prologue

__ct__8FixpointFi.b:
	or.	r5,r3,r3
	or	r31,r4,r4
	bc	0x5,0x2,L..12
	addi	r3,r0,4
	bl	.__nw__FUi
	nop
	or.	r5,r3,r3
	bc	0xc,0x2,L..13
L..12:
	rlwinm	r3,r31,0x10,0x0,0xf
	stw	r3,0(r5)
L..13:
	or	r3,r5,r5
	lwz	r0,88(sp)
	addi	sp,sp,80
	mtspr	lr,r0
	lwz	r31,-4(sp)
	bclr	0x14,0x0
FE_MOT_RESVD.__ct__8FixpointFi:

#	This function came from file: "SimpleTest.cp"
	.toc
T..main:
	.tc	..main[tc], main[ds]

	.align	2
	.globl	main[ds]
	.csect	main[ds]
	.ualong	.main[pr], TOC[tc0], 0
	.globl	.main[pr]

	.csect	.main[pr]
.main:

#	start of prologue, stack size = 96

	mfspr	r0,lr
	stw	r29,-12(sp)
	stw	r30,-8(sp)
	stw	r31,-4(sp)
	stw	r0,8(sp)
	stwu	sp,-96(sp)

#	end of prologue

main.b:
	addi	r31,sp,64
	or	r3,r31,r31
	addi	r4,r0,7
	addi	r30,r0,10
	bl	.__ct__8FixpointFi
	addi	r29,sp,68
	or	r4,r30,r30
	or	r3,r29,r29
	bl	.__ct__8FixpointFl
	addi	r30,sp,72
	addi	r3,sp,60
	or	r4,r31,r31
	bl	.__ct__8FixpointFRC8Fixpoint
	or	r31,r3,r3
	addi	r3,sp,56
	or	r4,r29,r29
	bl	.__ct__8FixpointFRC8Fixpoint
	or	r5,r3,r3
	or	r3,r30,r30
	or	r4,r31,r31
	bl	.__pl__F8FixpointT1
	addi	r3,r0,0
	lwz	r0,104(sp)
	addi	sp,sp,96
	mtspr	lr,r0
	lwz	r29,-12(sp)
	lwz	r30,-8(sp)
	lwz	r31,-4(sp)
	bclr	0x14,0x0
FE_MOT_RESVD.main:
	.csect	[rw]
	.align	2
.LDATA:

#	External Functions ...
	.extern	.__nw__FUi
#	End Of External Functions.

	.csect	[rw]
	.align	2
.LRDATA:
