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
//		DBG.H		Error/debug system
//		Rex E. Bradford (REX)
/*
* $Header: r:/prj/lib/src/lg/rcs/dbg.h 1.24 1994/08/12 17:03:24 jak Exp $
* $Log: dbg.h $
 * Revision 1.24  1994/08/12  17:03:24  jak
 * Split DbgHandleC() off DbgHandle()
 * Externed some things for use by dbgpp.cc
 * Split up decl of DbgSetReportRoutine() to make
 * C++ parser happy
 * 
 * Revision 1.23  1994/08/11  10:32:00  dfan
 * Make C++ compatible
 * 
 * Revision 1.22  1994/03/11  11:32:52  dfan
 * DBGS shouldn't have been do/whiled
 * 
 * Revision 1.21  1994/03/10  15:26:52  eric
 * Fixed bug in Spew -- wasn't wrapped in do {} while (0), so statements like
 * if (condition) Spew(("Hello\n")); else Error(1, "Bye\n");
 * would be evaluated incorrectly.  (else would be bound to wrong if() ).
 * 
 * Revision 1.20  1993/09/22  19:05:42  jak
 * Oops.  Wrong # of args for Assrt().
 * 
 * Revision 1.19  1993/09/22  18:51:34  jak
 * Added null 'Assrt()' macro,
 * 
 * Revision 1.18  1993/09/16  10:02:35  dfan
 * When warnings and spews not on, replace them by do{}while(0), not nothing
 * Otherwise strange things happen, for instance in conditionals
 * 
 * Revision 1.17  1993/08/11  14:54:23  dfan
 * There was no DBGS macro if DBG_ON wasn't defined
 * Removed some sarcastic comments in the interest of promoting love and harmony
 * 
 * Revision 1.16  1993/08/10  22:44:44  dc
 * move around stuff to get stuff working for assembler code
 * 
 * Revision 1.15  1993/08/10  21:36:19  dc
 * attempt to fix broken ifdef nesting from r1.13 on the 9th of July 
 * when dbg macros for asm source files were broken
 * but i cant test it since i need to make install to really do an h2i
 * so we will see what happens
 * 
 * Revision 1.14  1993/07/26  10:27:45  jak
 * Modified Assert() macro to have an ELSE clause so that it does not
 * swallow up an ELSE clause in the caller's code.
 * Added Assrt() macro to call Assert() with a default message
 * for the lazy among us.
 * 
 * Revision 1.13  1993/07/09  09:32:56  rex
 * Added Assert(), made dummy macro set when DBG_ON is not defined
 * 
 * Revision 1.12  1993/04/22  13:58:55  rex
 * Changed mono config key install thingy from flag to func ptr
 * 
 * Revision 1.11  1993/04/22  11:39:23  rex
 * Added macro DbgUseKblib()
 * 
 * Revision 1.10  1993/03/25  10:50:41  rex
 * Made AtExit() into a macro, instead of function.
 * 
 * Revision 1.9  1993/03/24  12:19:53  matt
 * Fixed another stupid bug.  You would think I would test these files
 * before I checked them in.
 * 
 * Revision 1.8  1993/03/24  12:15:37  matt
 * Fixed stupid mistake
 * 
 * Revision 1.7  1993/03/24  12:12:25  matt
 * Added include for assembly macros
 * 
 * Revision 1.6  1993/03/04  11:51:41  rex
 * Fixed macros: DbgSetDbg(), DbgSetMono(), DbgSetFunc()
 * 
 * Revision 1.5  1993/02/25  12:51:30  rex
 * Changed exit-handling functions
 * 
 * Revision 1.4  1993/02/17  11:17:49  matt
 * Added new macro DBGS(), like DBG(), but based on spew flags
 * 
 * Revision 1.3  1993/02/04  20:04:32  rex
 * Changed DbgExit() to Exit(), etc.
 * 
 * Revision 1.2  1993/01/29  17:30:25  rex
 * Added arg to Error()
 * 
 * Revision 1.1  1993/01/29  09:47:52  rex
 * Initial revision
 * 
*/


// Define the only warning I want to do right now - KC
/*
#ifdef DBG_ON
#define Warning(msg) DoWarningMsg msg
void DoWarningMsg(char *msg);
#else
#define Warning(msg) do {} while (0)
#endif
*/
#define Warning(msg) DoWarningMsg msg
void DoWarningMsg(char *msg);

/*
#include <stdio.h>
#include "types.h"

//	The 4 levels of reporting

#define DBG_ERROR 3			// report & go to DOS
#define DBG_WARNUSER 2		// alert user somehow
#define DBG_WARNING 1		// warn developer
#define DBG_SPEW 0			// be happy

//	'sources' combine a bank (0-31 in high 5 bits) and one or many of 27 lower
//	'slot' bits.  You can make your own sources with DBGSRC() macro.
//	The bank portion of a source can be extracted using DBGBANK().

#define NUM_DBG_BANKS 32
#define NUM_DBG_SLOTS 27
#define DBG_SLOT_MASK 0x07FFFFFF

#define DBGSRC(bank,bits) (((bank)<<NUM_DBG_SLOTS)|((bits)&DBG_SLOT_MASK))
#define DBGBANK(src) ((ulong)(src)>>NUM_DBG_SLOTS)

//	If DBG_ON is defined, debug system is defined, else macros
//	are used to compile calls and macros out

//	The DbgBank structure maintains most information about the
//	run-time debugging desires of banks & slots.

#define MAX_BANKNAMELEN 8
#define MAX_SLOTNAMELEN 8

#define DG_DBG 0
#define DG_MONO 1
#define DG_FUNC 2
#define DG_FILE 3
#define DG_NUM 4

typedef struct {
	ulong gate[DG_NUM];		// gates for: dbg, spew, warn, mono
	uchar file_index[NUM_DBG_SLOTS];		// which log file for each slot
	char bank_name[MAX_BANKNAMELEN+1];	// bank's name
	char **ppBankSlotNames;	// ptr to bank slot names
	char pad[8];				// padding
} DbgBank;		// 64 bytes each

#ifdef DBG_ON
// this is here for C, and in ASM we have it hand coded into
// the dbgmacro.inc, because h2inc is so cool
extern DbgBank dbgBank[NUM_DBG_BANKS];		// 2K total bank storage

//	These macros set & get gates.

#define DbgSetDbg(bank,slots) dbgBank[bank].gate[DG_DBG] = ((slots) & DBG_SLOT_MASK)
#define DbgGetDbg(bank) (dbgBank[bank].gate[DG_DBG])
#define DbgSetMono(bank,slots) dbgBank[bank].gate[DG_MONO] = ((slots) & DBG_SLOT_MASK)
#define DbgGetMono(bank) (dbgBank[bank].gate[DG_MONO])
#define DbgSetFunc(bank,slots) dbgBank[bank].gate[DG_FUNC] = ((slots) & DBG_SLOT_MASK)
#define DbgGetFunc(bank) (dbgBank[bank].gate[DG_FUNC])

//	Each bank can be given a name, and an array of slot names also.
//	This is used by human-operated config routines.

#define DbgSetBankName(bank,name) strncpy(dbgBank[bank].bank_name,name,MAX_BANKNAMELEN);
void DbgSetSlotNames(int bank, char **namelist);
void DbgSetSlotName(int bank, int slot, char *name);
int DbgFindBankName(char *name);
int DbgFindSlotName(int bank, char *name);

//	Debug info can be directed to one of several files.  This data structure
//	keeps track of the debug files which are in use.  All logfiles must be
//	in the same directory.

#define MAX_DBG_LOGFILENAME 12

typedef struct {
	char name[MAX_DBG_LOGFILENAME+1];
	FILE *fp;
	short refCount;
} DbgLogFile;

#define NUM_DBG_LOGFILES 16
extern DbgLogFile dbgLogFile[NUM_DBG_LOGFILES];
extern char dbgLogPath[128];
extern int errErrCode;

//	The DBG() macro is used to conditionally compile debugging code,
//	based on DBG_ON, which we checked at the top of the file.
#define DBG(src,stuff) if ((dbgBank[DBGBANK(src)].gate[DG_DBG]&(src))==((src)&DBG_SLOT_MASK)) stuff

// This macro is like DBG(), in that it conditionally compiles code, except
// it is based on the spew flag.  It is useful, for example, if you have a
// for loop to print out an array.

#ifdef SPEW_ON
#define DBGS(src,stuff) if (DbgSpewTest(src)) stuff
#else
#define DBGS(src,stuff) 
#endif

//	The important macros:
//
//		Error(char *msg, ...)		  - Fatal error
//		WarnUser(char *msg, ...)	  - Warn User
//		Warning(char *msg, ...)		  - Warn developer
//		Assert(expr, char *msg, ...) - Test expression, warn if false
//		Spew(flags, char *msg, ...)  - Spew message

#define Error DbgReportError
#define WarnUser DbgReportWarnUser

#ifdef WARN_ON
#define Warning(msg) DbgReportWarning msg
#define Assert(expr,msg) if (!(expr)) DbgReportWarning msg ; else
#else
#define Warning(msg)          do {} while (0)
#define Assert(expr,msg)      do {} while (0)
#endif

#define Assrt(expr) Assert(expr,("Assert in %s at line %d in %s\n", #expr, __LINE__, __FILE__))

#ifdef SPEW_ON
#define Spew(src,msg) do { if (DbgSpewTest(src)) DbgDoSpew msg; } while (0)
#else
#define Spew(src,msg)         do {} while (0)
#endif

//	These are prototypes for the 4 reporting routines, which should be
//	called via the macros given above, and not directly.

#ifdef __cplusplus
extern "C" {
#endif

void DbgReportError(int errcode, char *msg, ...);
void DbgReportWarnUser(char *msg, ...);
void DbgReportWarning(char *msg, ...);
bool DbgSpewTest(ulong src);
void DbgDoSpew(char *msg, ...);

//	Set debug config screen to use function for getting keys

extern int (*f_getch)();
#define DbgInstallGetch(f) (f_getch = (f))

//	All logfiles are written to the same directory, which defaults to
//	the current directory, but can be changed.

void DbgSetLogPath(char *path);

//	This routine sets the log file associated with a source.
//	Opening, writing, and closing of the log file is automatic, as
//	is sharing of files with the same name.

bool DbgSetLogFile(ulong src, char *name);

//	These are really internal things

bool DbgOpenLogFile(int index);
void DbgCloseLogFiles();
void DbgHandle(int reportType, ulong src, char *buff);
extern char *dbgTags[];

//	Set a routine to be called to present reports.

// C++ doesn't like the following:
//
// void DbgSetReportRoutine(void (*f_warn)(int reportType, char *msg));
//
// so instead we break it up into two parts:
//

typedef void ReportRoutine(int reportType, char *msg);
void DbgSetReportRoutine(ReportRoutine *);

//	Allows user to configure debug system

void DbgInit();							// auto-loads settings from "debug.dbg"
void DbgMonoConfig();					// let operator config on mono screen
bool DbgAddConfigPath(char *path);	// add path for finding config files
int DbgLoadConfig(char *fname);		// load config file
int DbgSaveConfig(char *fname);		// save config file

#ifdef __cplusplus
}
#endif

// note this is the else DBG_ON from the top of the file, sitting here all 
// alone and lonely in the middle of the file
#else

//	If DBG_ON not defined, most macros and functions are macro'ed to
//	nothing or (0).  A few functions remain

#define DbgSetDbg(bank,slots)
#define DbgGetDbg(bank) (0)
#define DbgSetMono(bank,slots)
#define DbgGetMono(bank) (0)
#define DbgSetFunc(bank,slots)
#define DbgGetFunc(bank) (0)
#define DbgSetBankName(bank,name)
#define DbgSetSlotNames(bank, namelist)
#define DbgSetSlotName(bank, slot, name)
#define DbgFindBankName(name) (0)
#define DbgFindSlotName(bank, name) (0)
#define DBG(src,stuff)
#define DBGS(src,stuff)
#define Error DbgReportError
#define WarnUser DbgReportWarnUser
#define Warning(msg)
#define Assert(expr,msg)
#define Assrt(expr)
#define Spew(src,msg)
void DbgReportError(int errcode, char *msg, ...);
void DbgReportWarnUser(char *msg, ...);
#define DbgInstallGetch(f)
#define DbgSetLogPath(path)
#define DbgSetLogFile(src, name) (0)
#define DbgOpenLogFile(index) (0)
#define DbgCloseLogFiles()
//#define DbgHandle(reportType,src,buff)
typedef void ReportRoutine(int reportType, char *msg);
void DbgSetReportRoutine(ReportRoutine *);
#define DbgInit()
#define DbgMonoConfig()
#define DbgAddConfigPath(path) (0)
#define DbgLoadConfig(fname) (0)
#define DbgSaveConfig(fname) (0)

#endif

// look ma, an important thing to do
#ifdef _H2INC						//include assembly macros
#include "dbgmacro.h"		  //dummy file; converts to 'include dbgmacro.inc'
#endif

//	These routines are in exit.c, and handle exit functions.

void Exit(int errcode, char *msg);	// shut down with msg
#define AtExit(func) atexit(func);	// add func to atexit list
void PrintExitMsg();						// prints exit message

#define SetExitMsg(str) pExitMsg=str
extern char *pExitMsg;
*/
