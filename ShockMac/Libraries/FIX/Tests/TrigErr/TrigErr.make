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
#   File:       TrigErr.make
#   Target:     TrigErr  (app)


MAKEFILE     = TrigErr.make
¥MondoBuild¥ = {MAKEFILE}
Includes     = -i :::Source: ¶
			   -i ::::H:
Sym¥PPC      = -sym off 
ObjDir¥PPC   = 
FixDir		 = :::Source:
LIB			 = :::::MrC LIB:

PPCCOptions  = {Includes} {Sym¥PPC} -align mac68k -inline on -opt speed -load "{LIB}MacHeaders.MrC"

.o Ä .C {¥MondoBuild¥}
	MrCPP {Default}.C -o {Targ} {PPCCOptions}

Objects¥PPC  = ¶
		{ObjDir¥PPC}TrigErr.o

TrigErr ÄÄ {¥MondoBuild¥} {Objects¥PPC}
	PPCLink ¶
		-o {Targ} {Sym¥PPC} ¶
		{Objects¥PPC} ¶
		-t 'APPL' ¶
		-c '????' ¶
		"{LIB}"FIX.xcoff ¶
		"{PPCLibraries}"PPCSIOW.o ¶
		"{SharedLibraries}"InterfaceLib ¶
		"{SharedLibraries}"StdCLib ¶
		"{PPCLibraries}"MathLib.xcoff ¶
#		"{PPCLibraries}"StdCRuntime.o ¶
		"{PPCLibraries}"MrCPlusLib.o ¶
#		"{PPCLibraries}"MrCIOStreams.o ¶
		"{PPCLibraries}"PPCToolLibs.o ¶
		"{PPCLibraries}"PPCCRuntime.o ¶
		-librename MathLib.xcoff=MathLib


TrigErr ÄÄ {¥MondoBuild¥} "{RIncludes}"SIOW.r
	Rez -a "{RIncludes}"SIOW.r -o {Targ}



TrigErr.o Ä TrigErr.c {FixDir}fix.h
