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
#	ClearCache.s
#
#	Written by: Jeff Robbin
#
#	Date updated:	6/26/95
#
#	void ClearCache (unsigned char* theAddress, unsigned long numBlocks);
#
#	Call this routine to either clear a cacheable block of memory
#	that is aligned, or before filling a block of memory that is
#	going to be completely overwritten.  This clears the designated
#	cache block so that the processor doesn't have to reload the information
#	from main memory.

	EXPORT .ClearCache			# export the code symbol

.ClearCache:
	mtctr	r4
loop:
	dcbz	r0, r3
	addi	r3, r3, 32			# move to the next cache block
	bdnz	loop
	blr
