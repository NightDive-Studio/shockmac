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
#   File:       3dTest.make
#   Target:     3dTest  (app)


MAKEFILE     = 3dTest.make
¥MondoBuild¥ = {MAKEFILE}
Sym¥PPC      = -sym off 
ObjDir¥PPC   = :Obj:
Source		 = :Source:
FixDir	 	 = ::FIX:Source:
LGDir		 = ::LG:Source:
ResDir		 = ::RES:Source:
2dDir		 = ::2D:Source:
3dDir		 = ::3D:Source:
DStructDir	 = ::DSTRUCT:Source:
LIB			 = :::MrC LIB:
Includes     = -i :Source: ¶
			   -i ::H: ¶
			   -i ":::Mac Src:" ¶
			   -i {FixDir} ¶
			   -i {ResDir} ¶
			   -i {DStructDir} ¶
			   -i {2dDir} ¶
			   -i {3dDir} ¶
			   -i {LGDir}


PPCCOptions  = {Includes} {Sym¥PPC} -align mac68k -inline on -opt speed -load "{LIB}MacHeaders.MrC"

.o Ä .C {¥MondoBuild¥}
	MrCPP {depDir}{Default}.C -o {Targ} {PPCCOptions}

Objects¥PPC  = ¶
		{ObjDir¥PPC}3dTest.o ¶
		{ObjDir¥PPC}GenBoxTest.o ¶
		{ObjDir¥PPC}TestInitMac.o ¶
		{ObjDir¥PPC}ShockBitmap.o 

:Obj: Ä :Source:
:Obj: Ä :Tests:
:Obj: Ä ":::Mac Src:"
 
3dTest ÄÄ {¥MondoBuild¥} {Objects¥PPC} "{LIB}"3D.xcoff
	PPCLink ¶
		-o {Targ} {Sym¥PPC} ¶
		-main __cplusstart ¶
		{Objects¥PPC} ¶
		-t 'APPL' ¶
		-c '????' ¶
		"{LIB}"FIX.xcoff ¶
		"{LIB}"3D.xcoff ¶
		"{LIB}"2D.xcoff ¶
		"{LIB}"LG.xcoff ¶
		"{SharedLibraries}"InterfaceLib ¶
		"{SharedLibraries}"StdCLib ¶
		"{PPCLibraries}"MathLib.xcoff ¶
		"{PPCLibraries}"StdCRuntime.o ¶
		"{PPCLibraries}"MrCPlusLib.o ¶
#		"{PPCLibraries}"MrCIOStreams.o ¶
		"{PPCLibraries}"PPCCRuntime.o ¶
		-librename MathLib.xcoff=MathLib
