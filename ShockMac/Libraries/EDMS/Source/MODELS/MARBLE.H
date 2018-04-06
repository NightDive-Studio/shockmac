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
 * $Header: r:/prj/lib/src/new_edms/models/RCS/marble.h 1.1 1994/08/05 12:08:49 roadkill Exp $
 */


int make_marble( Q init_state[6][3], Q params[10] );
void	marble_set_control( int marble, Q X, Q Y, Q Z );

