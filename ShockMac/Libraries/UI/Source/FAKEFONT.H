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
#ifndef __FAKEFONT_H
#define __FAKEFONT_H

// prototypes
// drawing - return the width drawn
int fnt_c_draw(char  c, int xpos, int ypos);
int fnt_s_draw(char *s, int xpos, int ypos);
int fnt_c_width(char  c);
int fnt_s_width(char *s);
int fnt_l_clr(int len, int xpos, int ypos);
int fnt_s_clr(char *s, int xpos, int ypos);
int fnt_c_clr(char c, int xpos, int ypos);
int fnt_no_cursor(int x, int y);

// load/system
int fnt_init(char *def_fname);      // ret handle of def_fname, -1 fail,
                                    //  0 if def_fname NULL + succesful
int fnt_init_from_style(void *style_ptr);
bool fnt_free(void);                // free the font system
int fnt_load(char *fnt_fname);      // returns the handle
int fnt_load_from_style(void *style_ptr);
bool fnt_unload(int fnt_handle);    // these both
bool fnt_select(int fnt_handle);    //   return success or not

// for texttools...
#ifdef NOT
TTFontInfo *build_font_TTFI(TextToolFontInfo *tfont);
#endif

// contstants
#define FNT_MAX_CHARS 128

#endif
