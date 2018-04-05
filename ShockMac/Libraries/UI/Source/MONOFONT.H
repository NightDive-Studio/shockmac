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
// fake zanyness for mono fonts in texttools
int mono_c_wid(char c);
int mono_s_wid(char *s);
int mono_c_draw(char c, int xpos, int ypos);
int mono_s_draw(char *s, int xpos, int ypos);
int mono_s_clr(char *s, int x, int y);
int mono_l_clr(int len, int x, int y);
int mono_c_clr(char c, int x, int y);
int mono_cursor(int x, int y);
int anal_s_draw(char *s, int xpos, int ypos);
#ifdef nOAF
TTFontInfo *build_mono_TTFI(TextToolFontInfo *mfont);
#endif

