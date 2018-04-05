#   File:       AsmSample.make
#   Target:     AsmSample
#   Sources:    asm.s
#               AsmSample.c
#               AsmSample.r
#   Created:    Wednesday, October 13, 1993 05:20:00 PM
#
#  By default, builds the sample program without debugging info.
#  To build with debug info:
#     make -e -f AsmSample.make -d SymOpt=on
#

AppName = AsmSample
MakeFile = {AppName}.make

OBJECTS = ¶
		asm.s.o ¶
		{AppName}.c.o 

SymOpt = off

#------------------------------------------------------------------------

.s.o Ä .s {Makefile}
	 PPCAsm -o asm.s.o asm.s 

.c.o Ä .c {Makefile}
	 PPCC -w conformance -appleext on -sym {SymOpt} {default}.c -o {Targ}

#------------------------------------------------------------------------

{AppName}	ÄÄ {AppName}.xcoff
	makePEF {AppName}.xcoff -o {Targ} ¶
		-l InterfaceLib.xcoff=InterfaceLib ¶
		-l MathLib.xcoff=MathLib ¶
		-l StdCLib.xcoff=StdCLib ¶
		-ft APPL -fc '????'
		
{AppName}  ÄÄ  {AppName}.r
	Rez  {AppName}.r -append -o {Targ}
	

{AppName}.xcoff ÄÄ {OBJECTS}
	PPCLink {OBJECTS} ¶
		"{PPCLibraries}"InterfaceLib.xcoff ¶
		"{PPCLibraries}"MathLib.xcoff ¶
		"{PPCLibraries}"StdCLib.xcoff ¶
		"{PPCLibraries}"StdCRuntime.o ¶
		"{PPCLibraries}"PPCCRuntime.o ¶
		-sym {SymOpt} ¶
		-main __start ¶
		-o {Targ}
	If "{SymOpt}" =~ /[oO][nN]/
   	   MakeSYM {AppName}.xcoff
	End
