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
 * $Source: n:/project/lib/src/h/RCS/error.h $
 * $Revision: 1.8 $
 * $Author: kaboom $
 * $Date: 1993/09/28 01:12:45 $
 *
 * $Log: error.h $
 * Revision 1.8  1993/09/28  01:12:45  kaboom
 * Converted #include "xxx" to #include <xxx> for watcom.
 * 
 * Revision 1.7  1993/06/03  06:52:13  mahk
 * Added file errors
 * 
 * Revision 1.6  1993/03/22  11:44:05  matt
 * Moved home of this file to the \project\lib\src\h (from src\lg).
 * 
 * Revision 1.5  1993/03/21  01:51:17  mahk
 * Added ERR_NOMEM
 * 
 * Revision 1.4  1993/03/19  18:44:05  mahk
 * Added ERR_NULL and ERR_NOEFFECT, and an RCS header.
 * 
 * Revision 1.3  1993/03/19  17:09:35  mahk
 * added ERR_NULL. 
 *
 * Revision 1.2  1993/03/15  18:17:27  mahk
 * cast each error value explicitly
 *
 * Revision 1.1  1993/02/25  12:52:09  rex
 * Initial revision
 */

#ifndef __ERROR_H
#define __ERROR_H

typedef short errtype;

#define OK              					((errtype)0)       // Normal execution 
#define ERR_NODEV       			((errtype)1)       // No such device
#define ERR_DUNDERFLOW  	((errtype)2)       // Data underflow (stack, queue, etc) 
#define ERR_DOVERFLOW   		((errtype)3)       // Data overflow  (stack, queue, etc)
#define ERR_RANGE       			((errtype)4)       // Arg out of range
#define ERR_NULL        			((errtype)5)       // Unexpected NULL pointer
#define ERR_NOEFFECT    		((errtype)6)       // Operation had no effect
#define ERR_NOMEM       		((errtype)7)       // Not enough memory
#define ERR_FOPEN       			((errtype)8)       // Error opening file
#define ERR_FCLOSE      			((errtype)9)       // Error closing file
#define ERR_FREAD       			((errtype)10)      // Error reading file
#define ERR_FWRITE      			((errtype)11)      // Error writing file
// more here

#endif // __ERROR_H
