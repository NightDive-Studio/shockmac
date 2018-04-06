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
#   File:       2D.make
#   Target:     2D.xcoff  (static library)


MAKEFILE     = 2D.make
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
LGDir		 = ::LG:Source:
DStructDir	 = ::DSTRUCT:Source:
LIB			 = :::MrC LIB:
Includes     = -i :Source: ¶
			   -i {Clip} ¶
			   -i "{Flat8}" ¶
			   -i {Gen} ¶
			   -i {GR} ¶
			   -i {RSD} ¶
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
		{ObjDir¥PPC}BIT.o ¶
		{ObjDir¥PPC}BITMAP.o ¶
		{ObjDir¥PPC}BLEND.o ¶
		{ObjDir¥PPC}BUFFER.o ¶
		{ObjDir¥PPC}CANVAS.o ¶
		{ObjDir¥PPC}CHAIN.o ¶
		{ObjDir¥PPC}ChnFuncs.o ¶
		{ObjDir¥PPC}CHRSIZ.o ¶
		{ObjDir¥PPC}CHRWID.o ¶
		{ObjDir¥PPC}CLOSE.o ¶
		{ObjDir¥PPC}CONTEXT.o ¶
		{ObjDir¥PPC}CNVTAB.o ¶
		{ObjDir¥PPC}DETECT.o ¶
		{ObjDir¥PPC}FCNTAB.o ¶
		{ObjDir¥PPC}INIT.o ¶
		{ObjDir¥PPC}INVTAB.o ¶
		{ObjDir¥PPC}LINTAB.o ¶
		{ObjDir¥PPC}MODE.o ¶
		{ObjDir¥PPC}PAL.o ¶
		{ObjDir¥PPC}PERMAP.o ¶
		{ObjDir¥PPC}PERSETUP.o ¶
		{ObjDir¥PPC}PERTOL.o ¶
		{ObjDir¥PPC}PIXFILL.o ¶
		{ObjDir¥PPC}RGB.o ¶
		{ObjDir¥PPC}SCREEN.o ¶
		{ObjDir¥PPC}SSCRN.o ¶
		{ObjDir¥PPC}StateStk.o ¶
		{ObjDir¥PPC}STRSCL.o ¶
		{ObjDir¥PPC}STRUSCL.o ¶
		{ObjDir¥PPC}STRNSIZ.o ¶
		{ObjDir¥PPC}STRSIZ.o ¶
		{ObjDir¥PPC}STRWID.o ¶
		{ObjDir¥PPC}STRWRAP.o ¶
		{ObjDir¥PPC}TEMPBM.o ¶
		{ObjDir¥PPC}TEMPTM.o ¶
		{ObjDir¥PPC}TLUCDAT.o ¶
		{ObjDir¥PPC}TLUCTAB.o ¶
		{ObjDir¥PPC}VALLOC.o ¶
		{ObjDir¥PPC}VTAB.o ¶
		{ObjDir¥PPC}WIRPOLY.o ¶
		¶
		{ObjDir¥PPC}FL8BL.o ¶
		{ObjDir¥PPC}FL8BLDBL.o ¶
		{ObjDir¥PPC}FL8CHFL8.o ¶
		{ObjDir¥PPC}FL8CLEAR.o ¶
		{ObjDir¥PPC}FL8CLIN.o ¶
		{ObjDir¥PPC}FL8CNV.o ¶
		{ObjDir¥PPC}FL8CPLY.o ¶
		{ObjDir¥PPC}FL8DBL.o ¶
		{ObjDir¥PPC}FL8FL8.o ¶
		{ObjDir¥PPC}FL8FL8C.o ¶
		{ObjDir¥PPC}FL8FL8M.o ¶
		{ObjDir¥PPC}FL8FLTR2.o ¶
		{ObjDir¥PPC}FL8FT.o ¶
		{ObjDir¥PPC}FL8G24.o ¶
		{ObjDir¥PPC}FL8GFL8.o ¶
		{ObjDir¥PPC}FL8GPIX.o ¶
		{ObjDir¥PPC}FL8HFL8.o ¶
		{ObjDir¥PPC}FL8HLIN.o ¶
		{ObjDir¥PPC}FL8LIN.o ¶
		{ObjDir¥PPC}FL8LP.o ¶
		{ObjDir¥PPC}FL8MSCL.o ¶
		{ObjDir¥PPC}FL8MONO.o ¶
		{ObjDir¥PPC}FL8NS.o ¶
		{ObjDir¥PPC}FL8NTRP2.o ¶
		{ObjDir¥PPC}FL8P.o ¶
		{ObjDir¥PPC}FL8P24.o ¶
		{ObjDir¥PPC}FL8PIX.o ¶
		{ObjDir¥PPC}FL8PLY.o ¶
		{ObjDir¥PPC}FL8PNT.o ¶
		{ObjDir¥PPC}FL8RECT.o ¶
		{ObjDir¥PPC}FL8ROW.o ¶
		{ObjDir¥PPC}FL8RSD8.o ¶
		{ObjDir¥PPC}FL8S.o ¶
		{ObjDir¥PPC}FL8SLIN.o ¶
		{ObjDir¥PPC}FL8SPLY.o ¶
		{ObjDir¥PPC}FL8SUB.o ¶
		{ObjDir¥PPC}FL8TL8.o ¶
		{ObjDir¥PPC}FL8VLIN.o ¶
		{ObjDir¥PPC}FL8WCLIN.o ¶
		{ObjDir¥PPC}FL8WLIN.o ¶
		¶
		{ObjDir¥PPC}FL8LNOP.o ¶
		{ObjDir¥PPC}FL8LL.o ¶
		{ObjDir¥PPC}FL8NL.o ¶
		{ObjDir¥PPC}Fl8F.o ¶
		{ObjDir¥PPC}FL8LF.o ¶
		¶
		{ObjDir¥PPC}FL8W.o ¶
		{ObjDir¥PPC}FL8LW.o ¶
		¶
		{ObjDir¥PPC}FL8TSMAP.o ¶
		{ObjDir¥PPC}FL8COP.o ¶
		{ObjDir¥PPC}FL8CTP.o ¶
		{ObjDir¥PPC}FL8LOP.o ¶
		{ObjDir¥PPC}FL8LTP.o ¶
		{ObjDir¥PPC}FL8OPL.o ¶
		{ObjDir¥PPC}FL8TPL.o ¶
		¶
		{ObjDir¥PPC}GENBOX.o ¶
		{ObjDir¥PPC}GENCHFL8.o ¶
		{ObjDir¥PPC}GENCHR.o ¶
		{ObjDir¥PPC}GENCIRC.o ¶
		{ObjDir¥PPC}GENCLIN.o ¶
		{ObjDir¥PPC}GENCNV.o ¶
		{ObjDir¥PPC}GENCWLIN.o ¶
		{ObjDir¥PPC}GENDISK.o ¶
		{ObjDir¥PPC}GENEL.o ¶
		{ObjDir¥PPC}GENERAL.o ¶
		{ObjDir¥PPC}GENF24.o ¶
		{ObjDir¥PPC}GENFL8.o ¶
		{ObjDir¥PPC}GENFL8C.o ¶
		{ObjDir¥PPC}GENGFL8.o ¶
		{ObjDir¥PPC}GENHFL8.o ¶
		{ObjDir¥PPC}GENHLIN.o ¶
		{ObjDir¥PPC}GENLIN.o ¶
		{ObjDir¥PPC}GENMONO.o ¶
		{ObjDir¥PPC}GENOV.o ¶
		{ObjDir¥PPC}GENPIX.o ¶
		{ObjDir¥PPC}GENRECT.o ¶
		{ObjDir¥PPC}GENRSD8.o ¶
		{ObjDir¥PPC}GENRSDBM.o ¶
		{ObjDir¥PPC}GENRSDTM.o ¶
		{ObjDir¥PPC}GENSLIN.o ¶
		{ObjDir¥PPC}GENSTR.o ¶
		{ObjDir¥PPC}GENTE.o ¶
		{ObjDir¥PPC}GENTL8.o ¶
		{ObjDir¥PPC}GENTM.o ¶
		{ObjDir¥PPC}GENUCHR.o ¶
		{ObjDir¥PPC}GENUCLIN.o ¶
		{ObjDir¥PPC}GENUHLIN.o ¶
		{ObjDir¥PPC}GENULIN.o ¶
		{ObjDir¥PPC}GENUSLIN.o ¶
		{ObjDir¥PPC}GENUSTR.o ¶
		{ObjDir¥PPC}GENUVLIN.o ¶
		{ObjDir¥PPC}GENVCPLY.o ¶
		{ObjDir¥PPC}GENVLIN.o ¶
		{ObjDir¥PPC}GENVPOLY.o ¶
		{ObjDir¥PPC}GENVRECT.o ¶
		{ObjDir¥PPC}GENWCLIN.o ¶
		{ObjDir¥PPC}GENWLIN.o ¶
		¶
		{ObjDir¥PPC}GRD.o ¶
		{ObjDir¥PPC}GRILIN.o ¶
		{ObjDir¥PPC}GRMALLOC.o ¶
		{ObjDir¥PPC}GRNULL.o ¶
		{ObjDir¥PPC}GRUILIN.o ¶
		¶
		{ObjDir¥PPC}devtab.o ¶
		{ObjDir¥PPC}MacDev.o ¶
		{ObjDir¥PPC}SVGAINIT.o ¶
		¶
		{ObjDir¥PPC}RSDCVT.o ¶
		{ObjDir¥PPC}RSDUnpack.o ¶
		¶
		{ObjDir¥PPC}CLPCLIN.o ¶
		{ObjDir¥PPC}CLPF24.o ¶
		{ObjDir¥PPC}CLPLIN.o ¶
		{ObjDir¥PPC}CLPLIN2.o ¶
		{ObjDir¥PPC}CLPLTAB.o ¶
		{ObjDir¥PPC}CLPMONO.o ¶
		{ObjDir¥PPC}CLPPLY.o ¶
		{ObjDir¥PPC}CLPPOLY.o ¶
		{ObjDir¥PPC}CLPRECT.o ¶
		{ObjDir¥PPC}CLPSLIN.o 

2DObjects¥Asm  = ¶
		{Source}LinearLoop.s.o ¶
		{Source}FloorLoop.s.o ¶
		{Source}WallLitLoop1D.s.o ¶
		{Source}PermapLoop.s.o

:Obj: Ä :Source:		# Objects in Obj folder depend on sources in Source folder
:Obj: Ä {Clip}
:Obj: Ä "{Flat8}"
:Obj: Ä {Gen}
:Obj: Ä {GR}
:Obj: Ä {RSD}

2D ÄÄ {¥MondoBuild¥} {Objects¥PPC} {2DObjects¥Asm}
	PPCLink ¶
		-o "{LIB}"{Targ}.xcoff {Sym¥PPC} ¶
		{Objects¥PPC} ¶
		{2DObjects¥Asm} ¶
		-t 'XCOF' ¶
		-c 'MPS ' ¶
		-xm l ¶
		-mf
