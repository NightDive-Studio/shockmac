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
#ifndef __EMAILBIT_H
#define __EMAILBIT_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/emailbit.h $
 * $Revision: 1.2 $
 * $Author: tjs $
 * $Date: 1994/08/12 21:04:42 $
 *
 */

// bit masks for the player_struct email inventory


#define EMAIL_GOT          0x80  // we've got the email
#define EMAIL_READ         0x40  // we've read it.
#define EMAIL_SEQ          0x3F  /* sequence number of emails
                                    from this sender */
#define EMAIL_SEQ_SHF      0


// flavors of data

#define EMAIL_VER    0
#define LOG_VER      1
#define DATA_VER     2

#endif // __EMAILBIT_H

