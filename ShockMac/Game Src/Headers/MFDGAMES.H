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
 * $Source: n:/project/cit/src/inc/RCS/mfdgames.h $
 * $Revision: 1.1 $
 * $Author: dc $
 * $Date: 1994/04/22 07:02:03 $
 */

// yo, word up
errtype mfd_games_init(MFD_Func* f);
bool mfd_games_handler(MFD *m, uiEvent *e);
void mfd_games_expose(MFD *m, ubyte control);

