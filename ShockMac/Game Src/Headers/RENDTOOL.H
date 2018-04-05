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
#ifndef __RENDTOOL_H
#define __RENDTOOL_H

/*
 * $Source: n:/project/cit/src/inc/RCS/rendtool.h $
 * $Revision: 1.5 $
 * $Author: dc $
 * $Date: 1994/05/09 06:06:01 $
 *
 */

void fr_show_rate(int color);
void game_fr_startup(void);
void game_fr_shutdown(void);
uchar *get_free_frame_buffer_bits(int size); // to get bitmap bits
void *get_scr_canvas_from_frame_buffer(int x, int y, int wid, int hgt); // to get an actual canvas
void game_fr_reparam(int is_128s, int full_scrn, int show_all);
#endif // __RENDTOOL_H
