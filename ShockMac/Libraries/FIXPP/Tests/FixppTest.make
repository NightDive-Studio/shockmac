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
#   File:       FixppTest.make
#   Target:     FixppTest  (app)


MAKEFILE     = FixppTest.make
¥MondoBuild¥ = {MAKEFILE}
Includes     = -i ::Source: ¶
			   -i :::H: ¶
			   -i :::FIX:Source:
Sym¥PPC      = -sym off 
ObjDir¥PPC   = 
FixDir		 = :::FIX:Source:
Source		 = ::Source:
LIB			 = ::::MrC LIB:

PPCCOptions  = {Includes} {Sym¥PPC} -align mac68k -inline on -opt speed -load "{LIB}MacHeaders.MrC"
#-saveil -xo

.o Ä .CC {¥MondoBuild¥}
	MrCPP {Default}.CC -o {Targ} {PPCCOptions}

Objects¥PPC  = ¶
		{ObjDir¥PPC}FixppTest.o

FixppTest ÄÄ {¥MondoBuild¥} {Objects¥PPC}
	PPCLink ¶
		-o {Targ} {Sym¥PPC} ¶
		{Objects¥PPC} ¶
		-t 'APPL' ¶
		-c '????' ¶
		"{LIB}"FIX.xcoff ¶
		"{LIB}"FIXPP.xcoff ¶
		"{PPCLibraries}"PPCSIOW.o ¶
		"{SharedLibraries}"InterfaceLib ¶
		"{SharedLibraries}"StdCLib ¶
		"{PPCLibraries}"MathLib.xcoff ¶
#		"{PPCLibraries}"StdCRuntime.o ¶
		"{PPCLibraries}"MrCPlusLib.o ¶
		"{PPCLibraries}"MrCIOStreams.o ¶
		"{PPCLibraries}"PPCToolLibs.o ¶
		"{PPCLibraries}"PPCCRuntime.o ¶
		-librename MathLib.xcoff=MathLib


FixppTest ÄÄ {¥MondoBuild¥} "{RIncludes}"SIOW.r
	Rez -a "{RIncludes}"SIOW.r -o {Targ}


FixppTest.o Ä FixppTest.CC {Source}fixpp.h {FixDir}fix.h 

