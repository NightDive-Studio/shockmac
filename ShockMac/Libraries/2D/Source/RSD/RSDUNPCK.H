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
 * $Source: n:/project/lib/src/2d/RCS/rsdunpck.h $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1993/12/28 16:29:09 $
 * 
 * Declarations and error codes for gr_unpack_rsd8.
 * Uses memory provided externally.
 *
 * This file is part of the 2d library.
 *
 * $Log: rsdunpck.h $
 * Revision 1.2  1993/12/28  16:29:09  kevin
 * Added assembly unpacker, changed some names.
 * 
 * Revision 1.1  1993/12/06  13:09:47  kevin
 * Initial revision
 * 
 */

#ifndef __RSDUNPCK_H
#define __RSDUNPCK_H

#define kSkipColor 0

//�MLA - removed so we have the prototypes
// #ifndef _RSDCVT_C
extern uchar *grd_unpack_buf;
extern int gr_rsd8_convert(grs_bitmap *sbm, grs_bitmap *dbm);
// #endif

#if defined(powerc) || defined(__powerc)
uchar *gr_rsd8_unpack(uchar* src, uchar *dst);
#else
asm uchar *gr_rsd8_unpack(uchar* src, uchar *dst);
#endif
//#pragma aux gr_rsd8_unpack parm [esi] [edi] value [edi] modify [eax ecx edx esi edi]

#define gr_set_unpack_buf(buf) grd_unpack_buf=buf
#define gr_get_unpack_buf() grd_unpack_buf

/* gr_unpack_rsd8 return codes */

#define GR_UNPACK_RSD8_OK 0
#define GR_UNPACK_RSD8_NOBUF 1
#define GR_UNPACK_RSD8_NOTRSD 2
#endif
