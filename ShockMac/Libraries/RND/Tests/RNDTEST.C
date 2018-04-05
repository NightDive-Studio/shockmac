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
//		Rndtest.C		Random stream tester
//		Rex E. Bradford (REX)
//
/*
* $Header: n:/project/lib/src/rnd/RCS/rndtest.c 1.1 1993/04/06 09:56:48 rex Exp $
* $Log: rndtest.c $
 * Revision 1.1  1993/04/06  09:56:48  rex
 * Initial revision
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "rnd.h"

//	--------------------------------------------------------------
void RunRaw(RndStream *prs);
void RunRange(RndStream *prs, long low, long high);
void RunFix(RndStream *prs);
void RunRangeFix(RndStream *prs, fix flow, fix fhigh);
void TestDistribution(RndStream *prs);

//	--------------------------------------------------------------

#define RS_LC16 0
#define RS_GAUSS16 1
#define RS_GAUSS16FAST 2
#define NUM_RS 3

static RNDSTREAM_LC16(rsLc16);
static RNDSTREAM_GAUSS16(rsGauss16);
static RNDSTREAM_GAUSS16FAST(rsGauss16Fast);

static RndStream *rstab[] = {&rsLc16,&rsGauss16,&rsGauss16Fast};
static RndStream *prs;

main()
{
	char		ans[10];
	int 		c;
	ulong		seed;
	long		low,high;
	fix		flow,fhigh;

	prs = rstab[0];

LOOP:
	printf("Q = quit, P = pick stream, S = seed, R = run, T = test\n");
	fgets (ans, sizeof(ans), stdin);
	c = toupper(ans[0]);
	switch (c)
	{
		case 27:
		case 'Q':
			exit(0);

		case 'P':
			printf("0 = Lc16, 1 = Gauss16, 2 = Gauss16Fast\n");
			fgets(ans, sizeof(ans), stdin);
			c = atoi(ans);
			if ((c >= 0) && (c < NUM_RS))
				prs = rstab[c];
			else
				printf("???\n");
			break;

		case 'S':
			printf("Enter seed: ");
			fgets(ans, sizeof(ans), stdin);
			seed = atol(ans);
			RndSeed(prs, seed);
			break;

		case 'R':
RUNLOOP:
			printf("0 = raw, 1 = range, 2 = fix, 3 = fixrange, Q = end\n");
			fgets (ans, sizeof(ans), stdin);
			c = toupper(ans[0]);
			switch (c)
			{
				case '0':
					RunRaw(prs);
					break;
				case '1':
					printf("Enter low\n");
					fgets(ans, sizeof(ans), stdin);
					low = atol(ans);
					printf("Enter high\n");
					fgets(ans, sizeof(ans), stdin);
					high = atol(ans);
					RunRange(prs, low, high);
					break;
				case '2':
					RunFix(prs);
					break;
				case '3':
					printf("Enter low\n");
					fgets(ans, sizeof(ans), stdin);
					low = atol(ans);
					printf("Enter high\n");
					fgets(ans, sizeof(ans), stdin);
					high = atol(ans);
					flow = fix_make(low, 0);
					fhigh = fix_make(high, 0);
					RunRangeFix(prs, flow, fhigh);
					break;
				case 'Q':
					break;
				default:
					printf("???\n");
			}
			if (c != 'Q')
				goto RUNLOOP;
			break;

		case 'T':
TESTLOOP:
			printf("Test type: 0 = distribution, Q = end\n");
			fgets (ans, sizeof(ans), stdin);
			c = toupper(ans[0]);
			switch (c)
			{
				case '0':
					TestDistribution(prs);
					break;
				case 'Q':
					break;
				default:
					printf("???\n");
					break;
			}
			if (c != 'Q')
				goto TESTLOOP;
			break;

		default:
			printf("???\n");
			break;
		}
	goto LOOP;
}

void RunRaw(RndStream *prs)
{
	int i;

	for (i = 0; i < 8; i++)
		printf("$%x\n", Rnd(prs));
}

void RunRange(RndStream *prs, long low, long high)
{
	int i;

	for (i = 0; i < 8; i++)
		printf("%d\n", RndRange(prs, low, high));
}

void RunFix(RndStream *prs)
{
	int i;
	char buff[32];

	for (i = 0; i < 8; i++)
		{
		fix_sprint(buff, RndFix(prs));
		printf("%s\n", buff);
		}
}

void RunRangeFix(RndStream *prs, fix flow, fix fhigh)
{
	int i;
	char buff[32];

	for (i = 0; i < 8; i++)
		{
		fix_sprint(buff, RndRangeFix(prs, flow, fhigh));
		printf("%s\n", buff);
		}
}


void TestDistribution(RndStream *prs)
{
#define NUMBINS 100
#define NUMPERBIN 1000
#define NUMRANDS (NUMBINS * NUMPERBIN)
static ulong bins[NUMBINS];
	int i,ibin;
	ulong bmin,bmax;
	fix fpct;
	char buff1[32],buff2[32];

printf("putting %d random values into %d bins...\n", NUMRANDS, NUMBINS);
printf("bins should average 1 pct each...\n");

	memset(bins, 0, sizeof(bins));

	for (i = 0; i < NUMRANDS; i++)
		{
		ibin = RndRange(prs, 0, NUMBINS - 1);
		bins[ibin]++;
		}

	bmin = +99999;
	bmax = 0;

	for (i = 0; i < NUMBINS; i++)
		{
		if (bins[i] < bmin)
			bmin = bins[i];
		if (bins[i] > bmax)
			bmax = bins[i];
		}

	fpct = fix_make(0, (bmin << 16) / NUMRANDS);
	fpct *= 100;
	fix_sprint(buff1, fpct);
	fpct = fix_make(0, (bmax << 16) / NUMRANDS);
	fpct *= 100;
	fix_sprint(buff2, fpct);
	printf("Minimum bin pct = %s, maximum bin pct = %s\n", buff1, buff2);
}
