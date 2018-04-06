#
# Copyright (C) 2015-2018 Night Dive Studios, LLC.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#   File:       3D.make
#   Target:     3D.xcoff  (static library)


MAKEFILE     = 3D.make
¥MondoBuild¥ = ""
Sym¥PPC      = -sym off 
ObjDir¥PPC   = :Obj:
Source		 = :Source:
Clip		 = :Source:Clip:
Flat8		 = :Source:Flat 8:
Gen			 = :Source:Gen:
GR			 = :Source:GR:
RSD			 = :Source:RSD:
FixDir	 	 = ::FIX:Source:
ResDir		 = ::RES:Source:
2dDir		 = ::2D:Source:
LGDir		 = ::LG:Source:
DStructDir	 = ::DSTRUCT:Source:
LIB			 = :::MrC LIB:
Includes     = -i :Source: ¶
			   -i ::H: ¶
			   -i ":::Mac Src:" ¶
			   -i {FixDir} ¶
			   -i {DStructDir} ¶
			   -i {ResDir} ¶
			   -i {2dDir} ¶
			   -i {LGDir}

PPCCOptions  = {Includes} {Sym¥PPC} -align mac68k -inline on -opt speed -load "{LIB}MacHeaders.MrC"

.o Ä .C {¥MondoBuild¥}
	MrCPP {depDir}{Default}.C -o {Targ} {PPCCOptions}

Objects¥PPC  = ¶
		{ObjDir¥PPC}ALLOC.o ¶
		{ObjDir¥PPC}Bitmap.o ¶
		{ObjDir¥PPC}CLIP.o ¶
		{ObjDir¥PPC}DETAIL.o ¶
		{ObjDir¥PPC}FOV.o ¶
		{ObjDir¥PPC}GlobalV.o ¶
		{ObjDir¥PPC}INSTANCE.o ¶
		{ObjDir¥PPC}INTERP.o ¶
		{ObjDir¥PPC}LIGHT.o ¶
		{ObjDir¥PPC}MATRIX.o ¶
		{ObjDir¥PPC}POINTS.o ¶
		{ObjDir¥PPC}POLYGON.o ¶
		{ObjDir¥PPC}SLEW.o ¶
		{ObjDir¥PPC}TMAP.o ¶
		{ObjDir¥PPC}VECTOR.o

:Obj: Ä :Source:		# Objects in Obj folder depend on sources in Source folder

3D ÄÄ {¥MondoBuild¥} {Objects¥PPC}
	PPCLink ¶
		-o "{LIB}"{Targ}.xcoff {Sym¥PPC} ¶
		{Objects¥PPC} ¶
		-t 'XCOF' ¶
		-c 'MPS ' ¶
		-xm l ¶
		-mf
