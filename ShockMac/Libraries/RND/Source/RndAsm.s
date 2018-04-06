;
; Copyright (C) 2015-2018 Night Dive Studios, LLC.
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.
;
	
	; Need to export both the function descriptor and an entry point for
	; all routines.
	
	export high_umpy[DS]
	export .high_umpy__FUlUl[PR]

	; create a TOC entry for the the function descriptor for each function.

	toc
		tc high_umpy[TC], high_umpy[DS]

	; The function descriptor contains definitions used for runtime linkage
	; of the function.  The address of the function entry point must be in 
	; the first longword.  The address of the start of the TOC for the  
	; module that contains the function (use the predefined PPCAsm
	; global tc0) must be in the second longword.  The third longword 
	; contains the environment pointer, which is not used by C and so is set
	; to 0.
	
	csect	high_umpy[DS]
		dc.l	.high_umpy__FUlUl[PR]
		dc.l	TOC[tc0]
		dc.l	0
		
	; This is the actual code of the high_umpy function

	csect	.high_umpy__FUlUl[PR]

;
;  ulong high_umpy(ulong a, ulong b)
;
		mulhwu	r3,r3,r4				; oooh, this is tough.
		blr
