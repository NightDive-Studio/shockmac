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
#   File:       SimpleTest2.make
#   Target:     SimpleTest2
#   Sources:    SimpleTest.cp
#   Created:    Thursday, June 1, 1995 06:12:26 PM


OBJECTS = SimpleTest.o



SimpleTest2 �� SimpleTest2.make  {OBJECTS}
	PPCLink  �
		{OBJECTS} �
		"{PPCLibraries}"InterfaceLib.xcoff �
		"{PPCLibraries}"MathLib.xcoff �
		"{PPCLibraries}"StdCLib.xcoff �
		"{PPCLibraries}"CPlusLib.o �
		"{PPCLibraries}"StdCRuntime.o �
		"{PPCLibraries}"PPCCRuntime.o �
		-main __cplusstart �
		-o SimpleTest2.xcoff
	makePEF SimpleTest2.xcoff -o SimpleTest2 �
		-l InterfaceLib.xcoff=InterfaceLib �
		-l MathLib.xcoff=MathLib �
		-l StdCLib.xcoff=StdCLib �
		-ft APPL -fc '????'
SimpleTest.o � SimpleTest2.make SimpleTest.cp
	 PPCC -w conformance -appleext on -dialect cplus  SimpleTest.cp -o SimpleTest.o
