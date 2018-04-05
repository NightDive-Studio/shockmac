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
// Miscellaneous code-references quest variables

#define MUSIC_VOLUME_QVAR  0x29
#define GAMMACOR_QVAR      0x2a
#define SFX_VOLUME_QVAR    0x2b
#define MOUSEHAND_QVAR     0x2c
#define DCLICK_QVAR        0x2f
#define LANGUAGE_QVAR      0x30
#define ALOG_VOLUME_QVAR   0x31
#define SCREENMODE_QVAR    0x32
#define JOYSENS_QVAR       0x33
#define FULLSCRN_ICON_QVAR 0x34
#define ALOG_OPT_QVAR      0x35
#define FULLSCRN_VITAL_QVAR 0x36
#define AMAP_NOTES_QVAR    0x37
#define HUDCOLOR_QVAR      0x39
#define DIGI_CHANNELS_QVAR 0x3A

#define QVAR_TO_VOLUME(x) (long_sqrt(100*(x)))
#define VOLUME_TO_QVAR(x) ((x)*(x)/100)
#define QVAR_TO_GAMMA(x) (fix_mul((FIX_UNIT-fix_make(0,(x))),(FIX_UNIT-fix_make(0,(x))))+FIX_UNIT/2)

#define QVAR_TO_DCLICK(v,t) ((((t)==0)?30:100)*(USHRT_MAX+3*((ulong)(v)))/(2*USHRT_MAX))

#define QVAR_TO_JOYSENS(x) ((FIX_UNIT*3*(x)/(256*2))+(FIX_UNIT/4))
