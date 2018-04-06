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
//		ResTypes.C		Resource type names
//		Rex E. Bradford (REX)
//
//		See the doc RESOURCE.DOC for information.
/*
* $Header: r:/prj/lib/src/res/rcs/restypes.c 1.10 1994/09/01 12:03:09 rex Exp $
* $log: $
*/

#include "res.h"

//	Resource type names

char *resTypeNames[NUM_RESTYPENAMES] = 
{
	"UNKNOWN",		// RTYPE_UNKNOWN	(aka BIN)
	"STRING",			// RTYPE_STRING
	"IMAGE",			// RTYPE_IMAGE		(aka IMG)
	"FONT",			// RTYPE_FONT
	"ANIM",			// RTYPE_ANIM
	"PALL",			// RTYPE_PALL
	"SHADTAB",		// RTYPE_SHADTAB
	"VOC",			// RTYPE_VOC
	"SHAPE",			// RTYPE_SHAPE
	"PICT",			// RTYPE_PICT
	"B2EXTERN",		// RTYPE_B2EXTERN
	"B2RELOC",		// RTYPE_B2RELOC
	"B2CODE",			// RTYPE_B2CODE
	"B2HEADER",		// RTYPE_B2HEADER
	"hey!",			// RTYPE_B2RESRVD
	"OBJ3D",			// RTYPE_OBJ3D
	"STENCIL",			// RTYPE_STENCIL
	"MOVIE",			// RTYPE_MOVIE
	"RECT",			// RTYPE_RECT
	"",				// 19
	"",				// 20
	"",				// 21
	"",				// 22
	"",				// 23
	"",				// 24
	"",				// 25
	"",				// 26
	"",				// 27
	"",				// 28
	"",				// 29
	"",				// 30
	"",				// 31
	"",				// 32
	"",				// 33
	"",				// 34
	"",				// 35
	"",				// 36
	"",				// 37
	"",				// 38
	"",				// 39
	"",				// 40
	"",				// 41
	"",				// 42
	"",				// 43
	"",				// 44
	"",				// 45
	"",				// 46
	"",				// 47
	"APP1",			// RTYPE_APP
	"APP2",
	"APP3",
	"APP4",
	"APP5",
	"APP6",
	"APP7",
	"APP8",
	"APP9",
	"APP10",
	"APP11",
	"APP12",
	"APP13",
	"APP14",
	"APP15",
	"APP16",
};

// For Mac version, table of 4-char ResTypes corresponding to each Shock resType.

ResType resMacTypes[NUM_RESTYPENAMES] =
{
	's???',			// RTYPE_UNKNOWN	(aka BIN)
	'sSTR',			// RTYPE_STRING
	'sIMG',			// RTYPE_IMAGE		(aka IMG)
	'sFNT',			// RTYPE_FONT
	'sANI',			// RTYPE_ANIM
	'sPAL',			// RTYPE_PALL
	'sSTB',			// RTYPE_SHADTAB
	'snd ',			// RTYPE_VOC
	'sSHP',			// RTYPE_SHAPE
	'sPIC',			// RTYPE_PICT
	'sB2X',			// RTYPE_B2EXTERN
	'sB2R',			// RTYPE_B2RELOC
	'sB2C',			// RTYPE_B2CODE
	'sB2H',			// RTYPE_B2HEADER
	'sB2!',			// RTYPE_B2RESRVD
	'sO3D',			// RTYPE_OBJ3D
	'sSTN',			// RTYPE_STENCIL
	'sMOV',			// RTYPE_MOVIE
	'sRCT',			// RTYPE_RECT
	'    ',				// 19
	'    ',				// 20
	'    ',				// 21
	'    ',				// 22
	'    ',				// 23
	'    ',				// 24
	'    ',				// 25
	'    ',				// 26
	'    ',				// 27
	'    ',				// 28
	'    ',				// 29
	'    ',				// 30
	'    ',				// 31
	'    ',				// 32
	'    ',				// 33
	'    ',				// 34
	'    ',				// 35
	'    ',				// 36
	'    ',				// 37
	'    ',				// 38
	'    ',				// 39
	'    ',				// 40
	'    ',				// 41
	'    ',				// 42
	'    ',				// 43
	'    ',				// 44
	'    ',				// 45
	'    ',				// 46
	'    ',				// 47
	'sA01',			// RTYPE_APP
	'sA02',
	'sA03',
	'sA04',
	'sA05',
	'sA06',
	'sA07',
	'sA08',
	'sA09',
	'sA10',
	'sA11',
	'sA12',
	'sA13',
	'sA14',
	'sA15',
	'sA16',
};

