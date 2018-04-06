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
//====================================================================================
//
//		System Shock - ©1994-1995 Looking Glass Technologies, Inc.
//
//		Shock.h	-	Mac-specific initialization and main event loop.
//
//====================================================================================

//--------------------
// Defines
//--------------------
//#define TESTING		1

//--------------------
// Menus
//--------------------
#ifdef TESTING
#define kNumMenus			6
#else
#define kNumMenus			4
#endif

#define mApple					128					// Menu IDs
#define mFile						129
#define mEdit						130
#define mOptions				131
#ifdef TESTING
#define mTests					132
#define mTests2					133
#endif

#define fileNewGame			1						// File menu item IDs
#define fileOpenGame			2
#define fileSaveGame			3
#define fileSaveGameAs		4
#define filePlayIntro			6
#define fileResumeGame		8
#define fileEndGame			9
#define fileQuit					11

#define editUndo					1						// Edit menu item IDs
#define editCut					3
#define editCopy					4
#define editPaste				5
#define editClear				6

#define optGameOptions		1						// Options menu item IDs
#define optGraphicsOptions	2
#define optSoundOptions		3

#ifdef TESTING
#define testBrowseImages		1					// Tests menu item IDs
#define testBrowseFonts			2
#define testLoadPalette			3
#define testMoveKeys				5
#define testMouse					6
#define testPlayMovie				8
#define testPlayMovie2x			9
#define testPlayMovieDblSpd	10
#define testPlayMovieHalfSpd	11
#define testPlayIntro				13
#define testPlayDeath				14
#define testPlayEndGame			15
#define testPlayCitadelVM		17
#define testPlayDetachVM		18
#define testPlayJettisonVM		19
#define testPlayLaserMalVM	20
#define testPlayShieldsVM		21
#define testPlayAutoDesVM		22
#define testPlayBark1			24
#define testPlayBark2			25
#define testPlayAlog1				26
#define testPlayAlog2				27

#define testLoadLevelR			1
#define testLoadLevel1			2
#define testLoadLevel2			3
#define testLoadLevel3			4
#define testZoomIn					6
#define testZoomOut				7
#define testRender				9
#endif

//--------------------
// Graphics Defines
//--------------------
#define screenMaxX			640L
#define screenMaxY			480L

//--------------------
// File Types
//--------------------
#define kAppFileType 			'Shok'
#define kGameFileType 		'Sgam'
#define kRsrcFileType 		'Sres'

//--------------------
//  Function Prototypes
//--------------------
void main(void);
void HandleEvents(void);
void UpdateWindow(WindowPtr wind);
void DoCommand(unsigned long mResult);
Boolean DoSaveGame(void);
Boolean DoSaveGameAs(void);
void DoQuit(void);
void HandleAEOpenGame(FSSpec *openSpec);
void ScrollCredits(void);

//--------------------
// Public Globals
//--------------------
extern WindowPtr		gMainWindow;
extern MenuHandle		gMainMenus[kNumMenus];
extern RgnHandle		gCursorRgn;
extern short				gCursorSet;
extern Boolean			gDone;
extern Boolean			gInForeground;
extern Boolean			gIsNewGame;
extern FSSpec				gSavedGameFile;
extern long					gGameSavedTime;

extern Handle				gExtraMemory;
extern ColorSpec*		gOriginalColors;
extern unsigned long	gRandSeed;
extern short				gMainVRef;
extern CursHandle		gWatchCurs;

extern short				gOriginalDepth;
extern short				gStartupDepth;
extern short				gLastAlertDepth;
extern Ptr					gScreenAddress;
extern long					gScreenRowbytes;
extern short				gScreenWide, gScreenHigh;
extern short				gActiveWide, gActiveHigh;
extern short				gActiveLeft, gActiveTop;
extern Rect				gActiveArea, gOffActiveArea;

extern Boolean			gIsPowerPC;

extern long					gDataDirID;
extern short				gDataVref;
extern long					gCDDataDirID;
extern short				gCDDataVref;
extern long					gAlogDirID;
extern short				gAlogVref;
extern long					gBarkDirID;
extern short				gBarkVref;
