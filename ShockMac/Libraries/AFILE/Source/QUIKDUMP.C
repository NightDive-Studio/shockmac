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
//	===============================================================
//
//		QUIKDUMP		QuickTime movie file dumper
//		Ken Cobb - Adapted from program by Rex E. Bradford
//
//	===============================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lg_types.h"
#include "fix.h"
#include "quiktime.h"


// Prototypes
void WriteChunk(ulong ctype, ulong length, long offset, char *indent);
void MakeIndentString(char *str, int istack);

bool QuikReadChunkHdr(FILE *fp, QT_ChunkHdr *phdr);
QT_ChunkInfo *QuikFindChunkInfo(QT_ChunkHdr *phdr);
void QuikSkipChunk(FILE *fp, QT_ChunkHdr *phdr);

void PrintSTCO(uchar *data, char *indent);
void PrintSTTS(uchar *data, char *indent);


// Globals
QT_ChunkInfo chunkInfo[] =
{
	QT_CLIP,FALSE,
	QT_CRGN,TRUE,
	QT_DINF,FALSE,
	QT_DREF,TRUE,
	QT_EDTS,FALSE,
	QT_ELST,TRUE,
	QT_HDLR,TRUE,
	QT_KMAT,TRUE,
	QT_MATT,FALSE,
	QT_MDAT,TRUE,
	QT_MDIA,FALSE,
	QT_MDHD,TRUE,
	QT_MINF,FALSE,
	QT_MOOV,FALSE,
	QT_MVHD,TRUE,
	QT_SMHD,TRUE,
	QT_STBL,FALSE,
	QT_STCO,TRUE,
	QT_STSC,TRUE,
	QT_STSD,TRUE,
	QT_STSH,TRUE,
	QT_STSS,TRUE,
	QT_STSZ,TRUE,
	QT_STTS,TRUE,
	QT_TKHD,TRUE,
	QT_TRAK,FALSE,
	QT_UDTA,FALSE,
	QT_VMHD,TRUE,
	0,0
};

TrackType currTrackType;


//	--------------------------------------------------------------
//		MAIN PROGRAM
//	--------------------------------------------------------------

void main(int argc, char **argv)
{
	FILE *fp;
	int iarg,istack;
	bool badArgs,dumpChunks;
	uchar *dbuff;
	ulong dbuffLen;
	QT_ChunkHdr chunkHdr;
	QT_ChunkInfo *pinfo;
	char filename[128];
	ulong offsetStack[64];
	char indent[128];

	dumpChunks = TRUE;
	filename[0] = 0;

// 	Get the input file

	printf ("File to dump: ");
	fgets (filename, sizeof(filename), stdin);
	filename[strlen(filename)-1] = 0;
	if (strchr(filename, '.') == NULL)
		strcat(filename, ".mov");

//	Open file

	fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		printf("Can't open: %s\n", filename);
		exit(1);
	}

//	Allocate initial data buffer

	if (dumpChunks)
	{
		dbuffLen = 64000;
		dbuff = (uchar *)malloc(dbuffLen);
	}

//	Dump chunks

	istack = 0;
	MakeIndentString(indent, istack);

	while (TRUE)
	{
		if (!QuikReadChunkHdr(fp, &chunkHdr))
			break;
		WriteChunk(chunkHdr.ctype, chunkHdr.length,
			ftell(fp) - sizeof(QT_ChunkHdr), indent);
		if (chunkHdr.length == 0)
			break;
		pinfo = QuikFindChunkInfo(&chunkHdr);
		if (pinfo->isleaf)
		{
			if ((chunkHdr.ctype != QT_MDAT) && dumpChunks)
			{
				if (chunkHdr.length > dbuffLen)
				{
					dbuffLen = chunkHdr.length;
					dbuff = (uchar *)realloc(dbuff, dbuffLen);
				}
				fread(dbuff, chunkHdr.length - sizeof(QT_ChunkHdr), 1, fp);
				MakeIndentString(indent, istack + 1);
				if (chunkHdr.ctype == QT_STTS)
					PrintSTTS(dbuff, indent);
				if (chunkHdr.ctype == QT_STCO)
					PrintSTCO(dbuff, indent);
				MakeIndentString(indent, istack);
			}
			else
				QuikSkipChunk(fp, &chunkHdr);
		}
		else
		{
			if (chunkHdr.length > sizeof(QT_ChunkHdr))
			{
				offsetStack[istack++] = ftell(fp) +
					chunkHdr.length - sizeof(QT_ChunkHdr);
				MakeIndentString(indent, istack);
			}
		}
		while ((istack > 0) && (ftell(fp) >= offsetStack[istack - 1]))
			MakeIndentString(indent, --istack);
	}

	fclose(fp);
}

//	-------------------------------------------------------------
//		SUPPORT ROUTINES
//	-------------------------------------------------------------

void WriteChunk(ulong ctype, ulong length, long offset, char *indent)
{
	printf("%s%c%c%c%c: {offset: $%x  size: $%x}\n", indent,
		ctype >> 24, (ctype >> 16) & 0xFF, (ctype >> 8) & 0xFF, ctype & 0xFF,
		offset, length);
}

#define NUM_SPACES_PER_INDENT 3

void MakeIndentString(char *str, int istack)
{
	int i;

	for (i = 0; i < (istack * NUM_SPACES_PER_INDENT); i++)
		str[i] = ' ';
	str[i] = 0;
}

//	===============================================================
//
//	QuickTime file dumping routines.
//
//	===============================================================

//	--------------------------------------------------------------
//
//	QuikReadChunkHdr() reads in the next chunk header, returns TRUE if ok.

bool QuikReadChunkHdr(FILE *fp, QT_ChunkHdr *phdr)
{
	fread(phdr, sizeof(QT_ChunkHdr), 1, fp);

	switch (phdr->ctype)
	{
		case QT_TRAK:
			currTrackType = TRACK_OTHER;
			break;

		case QT_VMHD:
			currTrackType = TRACK_VIDEO;
			break;

		case QT_SMHD:
			currTrackType = TRACK_AUDIO;
			break;
	}

	return(feof(fp) == 0);
}

//	--------------------------------------------------------------
//
//	QuikFindChunkInfo() finds info for a chunk.

QT_ChunkInfo *QuikFindChunkInfo(QT_ChunkHdr *phdr)
{
static QT_Ctype lastType = 0;
static QT_ChunkInfo *lastInfoPtr = NULL;

	QT_ChunkInfo *pinfo;

	if (lastType == phdr->ctype)
		return((QT_ChunkInfo *) lastInfoPtr);

	pinfo = chunkInfo;
	while (pinfo->ctype)
		{
		if (pinfo->ctype == phdr->ctype)
			{
			lastType = phdr->ctype;
			lastInfoPtr = pinfo;
			return((QT_ChunkInfo *) pinfo);
			}
		++pinfo;
		}
	return NULL;
}

//	--------------------------------------------------------------
//
//	QuikSkipChunk() skips over the data in the current chunk.

void QuikSkipChunk(FILE *fp, QT_ChunkHdr *phdr)
{
	fseek(fp, phdr->length - sizeof(QT_ChunkHdr), SEEK_CUR);
}

//	--------------------------------------------------------------
//
//	PrintSTCO() prints the Chunk Offset Table.

void PrintSTCO(uchar *data, char *indent)
{
	QTS_STCO *p;
	int i;

	p = (QTS_STCO *)data;

	printf("%sversion: %d\n", indent, p->version);
	printf("%sflags: $%x $%x $%x\n", indent, p->flags[0], p->flags[1], p->flags[2]);
	printf("%snumEntries: %d\n", indent, p->numEntries);
	for (i = 0; i < p->numEntries; i++)
		printf("%s   offset[%d]: %d ($%x)\n",
			indent, i, p->offset[i], p->offset[i]);
}

//	--------------------------------------------------------------
//
//	PrintSTTS() prints the Sample to Time table.

void PrintSTTS(uchar *data, char *indent)
{
	QTS_STTS *p;
	int i;

	p = (QTS_STTS *)data;

	printf("%sversion: %d\n", indent, p->version);
	printf("%sflags: $%x $%x $%x\n", indent, p->flags[0], p->flags[1], p->flags[2]);
	printf("%snumEntries: %d\n", indent, p->numEntries);
	for (i = 0; i < p->numEntries; i++)
		printf("%s   count: %4d   duration: %6d\n", indent,
			p->time2samp[i].count, p->time2samp[i].duration);
}
