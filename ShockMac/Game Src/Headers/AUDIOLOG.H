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
 * $Source: r:/prj/cit/src/inc/RCS/audiolog.h $
 * $Revision: 1.4 $
 * $Author: dc $
 * $Date: 1994/11/19 20:44:42 $
 */

extern errtype audiolog_init();
extern errtype audiolog_play(int email_id);
extern errtype audiolog_bark_play(int bark_id);
extern void    audiolog_stop();
extern errtype audiolog_loop_callback();
extern bool    audiolog_playing(int email_id);

extern uchar   audiolog_setting;
