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
// virtualized mouse support
extern errtype ui_mouse_get_xy(short *pmx, short *pmy);
extern errtype ui_mouse_put_xy(short pmx, short pmy);
extern errtype ui_mouse_constrain_xy(short xl, short yl, short xh, short yh);
extern errtype ui_mouse_do_conversion(short *pmx, short *pmy, bool down);
