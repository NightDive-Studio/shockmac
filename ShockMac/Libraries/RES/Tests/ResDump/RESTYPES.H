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
//		Restypes.H		Resource types
//		Rex E. Bradford (REX)

/*
* $Header: r:/prj/lib/src/res/rcs/restypes.h 1.12 1994/12/07 14:19:07 mahk Exp $
* $Log: restypes.h $
 * Revision 1.12  1994/12/07  14:19:07  mahk
 * Added rect resource type.
 * 
 * Revision 1.11  1994/09/01  12:00:22  rex
 * Added RTYPE_MOVIE
 * 
 * Revision 1.10  1994/05/02  17:47:02  rex
 * Added RTYPE_STENCIL
 * 
 * Revision 1.9  1994/05/02  14:49:23  rex
 * Added RTYPE_SHADTAB
 * 
 * Revision 1.8  1994/02/17  11:26:54  rex
 * Changed #ifdef to use double-underscores
 * 
 * Revision 1.7  1993/09/23  16:43:50  rex
 * Added RTYPE_OBJ3D
 * 
 * Revision 1.6  1993/07/23  16:41:17  rex
 * Added RTYPE_B2HEADER
 * 
 * Revision 1.5  1993/07/22  19:17:30  rex
 * Added B2xxx types
 * 
 * Revision 1.4  1993/06/17  10:00:41  rex
 * Added RTYPE_PICT
 * 
 * Revision 1.3  1993/06/02  13:50:42  dfan
 * Added SHAPE types
 * 
 * Revision 1.2  1993/05/11  18:51:02  rex
 * Added RTYPE_VOC
 * 
 * Revision 1.1  1993/03/04  18:48:04  rex
 * Initial revision
 * 
 * Revision 1.1  1993/03/02  18:42:05  rex
 * Initial revision
 * 
*/

#ifndef __RESTYPES_H
#define __RESTYPES_H

#define NUM_RESTYPENAMES 64

//	Resource types 0-47 are reserved for system-wide use

#define RTYPE_UNKNOWN	0		// unknown resource type, or mix in compound
#define RTYPE_STRING		1		// string (usually in compound table)
#define RTYPE_IMAGE		2		// bitmapped image (usually in compound table)
#define RTYPE_FONT		3		// bitmapped font (usually non-compound)
#define RTYPE_ANIM		4		// animation script (usually in compound table)
#define RTYPE_PALL		5		// color pallette (usually non-compound)
#define RTYPE_SHADTAB	6		// shading table (usually non-compound)
#define RTYPE_VOC			7		// sound .voc file (usually non-compound)
#define RTYPE_SHAPE		8		// shape (usually compound)
#define RTYPE_PICT		9		// picture (usually compound)
#define RTYPE_B2EXTERN	10		// BABL2 extern records (always compound)
#define RTYPE_B2RELOC	11		// BABL2 relocation records (always non-compound)
#define RTYPE_B2CODE		12		// BABL2 object code (always compound)
#define RTYPE_B2HEADER	13		// BABL2 linked resource header
#define RTYPE_B2RESRVD	14		// BABL2 reserved
#define RTYPE_OBJ3D		15		// 3d object (always compound)
#define RTYPE_STENCIL	16		// stencil, with offsets (usually non-compound)
#define RTYPE_MOVIE		17		// movie (lg .mov format)
#define RTYPE_RECT      18    // list of bounding rects for images (usually in compound table)

//	Resource types 48-63 are application-specific

#define RTYPE_APP			48		// 16 application-specific resource types

//	Type names can be found thru this array (array kept in res.c)

extern char *resTypeNames[NUM_RESTYPENAMES];
extern ResType resMacTypes[NUM_RESTYPENAMES];		// Table of Mac resID's - KC

#endif

