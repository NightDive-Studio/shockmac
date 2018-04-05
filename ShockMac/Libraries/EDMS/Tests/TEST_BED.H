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
 * $Header: n:/project/lib/src/edms/RCS/test_bed.h 1.1 1994/02/28 17:07:43 roadkill Exp $
 */

//	The temporary graphics routines...
//	==================================

extern void	EDMS_setup_graphics(),
		EDMS_kill_graphics(),
		EDMS_make_terrain( int harshness ),
		EDMS_draw_object( int page );
		EDMS_set_terrain_offsets( fix, fix );

fix		EDMS_test_bed_terrain( fix X, fix Y, int deriv );







