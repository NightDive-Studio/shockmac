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
#   File:       2dTest.make
#   Target:     2dTest  (app)


MAKEFILE     = 2dTest.make
¥MondoBuild¥ = {MAKEFILE}
Sym¥PPC      = -sym off 
ObjDir¥PPC   = :Obj:
Source		 = :Source:
FixDir	 	 = ::FIX:Source:
ResDir		 = ::RES:Source:
LGDir		 = ::LG:Source:
DStructDir	 = ::DSTRUCT:Source:
LIB			 = :::MrC LIB:
Includes     = -i :Source: ¶
			   -i ::H: ¶
			   -i ":::Mac Src:" ¶
			   -i {FixDir} ¶
			   -i {DStructDir} ¶
			   -i {ResDir} ¶
			   -i {LGDir}


PPCCOptions  = {Includes} {Sym¥PPC} -align mac68k -inline on -opt speed -load "{LIB}MacHeaders.MrC"

.o Ä .C {¥MondoBuild¥}
	MrCPP {depDir}{Default}.C -o {Targ} {PPCCOptions}

Objects¥PPC  = ¶
		{ObjDir¥PPC}2dTest.o ¶
		{ObjDir¥PPC}TestInitMac.o ¶
		{ObjDir¥PPC}ShockBitmap.o 

:Obj: Ä :
:Obj: Ä :Source:
:Obj: Ä ":::Mac Src:"

2dTest ÄÄ {¥MondoBuild¥} {Objects¥PPC}
	PPCLink ¶
		-o {Targ} {Sym¥PPC} ¶
		{Objects¥PPC} ¶
		-t 'APPL' ¶
		-c '????' ¶
		"{LIB}"FIX.xcoff ¶
		"{LIB}"2D.xcoff ¶
		"{LIB}"LG.xcoff ¶
#		"{PPCLibraries}"PPCSIOW.o ¶
		"{SharedLibraries}"InterfaceLib ¶
		"{SharedLibraries}"StdCLib ¶
#		"{PPCLibraries}"InterfaceLib.xcoff ¶
#		"{PPCLibraries}"StdCLib.xcoff ¶
		"{PPCLibraries}"MathLib.xcoff ¶
		"{PPCLibraries}"StdCRuntime.o ¶
		"{PPCLibraries}"MrCPlusLib.o ¶
#		"{PPCLibraries}"MrCIOStreams.o ¶
		"{PPCLibraries}"PPCCRuntime.o ¶
		-librename MathLib.xcoff=MathLib


2dTest ÄÄ {¥MondoBuild¥} 2dTest.¹.rsrc
	Rez -a 2dTest.¹.rsrc -o {Targ}
