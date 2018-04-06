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
/*	This file created by RESTOOL */

#ifndef __GAMESCR_H
#define __GAMESCR_H

#define RES_helpscreen_english 0x543		// (1347)
#define REF_IMG_bmHelpOverlayEnglish 0x5430000		
#define RES_helpscreen_french 0x544		// (1348)
#define REF_IMG_bmHelpOverlayFrench 0x5440000		
#define RES_helpscreen_german 0x545		// (1349)
#define REF_IMG_bmHelpOverlayGerman 0x5450000		
#define RES_gamescrGfx 0x259		// (601)
#define REF_IMG_bmBlankMessageLine 0x2590000		
#define REF_IMG_bmBlankInventoryPanel 0x2590001		
#define REF_IMG_bmBlankMFD 0x2590002		
#define REF_IMG_bmDiffBio 0x2590003		
#define REF_IMG_bmBiorhythm 0x2590004		
#define REF_IMG_bmVitalInnardsTop 0x2590005		
#define REF_IMG_bmVitalInnardsBottom 0x2590006		
#define REF_IMG_bmStatusAngle1 0x2590007		
#define REF_IMG_bmStatusAngle2 0x2590008		
#define REF_IMG_bmStatusAngle3 0x2590009		
#define REF_IMG_bmStatusAngle4 0x259000a		
#define REF_IMG_bmHealthIcon1 0x259000b		
#define REF_IMG_bmHealthIcon2 0x259000c		
#define REF_IMG_bmHealthIcon3 0x259000d		
#define REF_IMG_bmEnergyIcon1 0x259000e		
#define REF_IMG_bmEnergyIcon2 0x259000f		
#define REF_IMG_bmEnergyIcon3 0x2590010		
#define REF_IMG_bmCyberIcon1 0x2590011		
#define REF_IMG_bmCyberIcon2 0x2590012		
#define REF_IMG_bmCyberIcon3 0x2590013		
#define REF_IMG_bm3dBackground1 0x2590014		
#define REF_IMG_bm3dBackground2 0x2590015		
#define REF_IMG_bm3dBackground3 0x2590016		
#define REF_IMG_bm3dBackground4 0x2590017		
#define REF_IMG_bm3dBackground5 0x2590018		
#define REF_IMG_bm3dBackground6 0x2590019		
#define REF_IMG_bmMFDButtonBackground 0x259001a		
#define REF_IMG_bmInventoryButtonBackground 0x259001b		
#define RES_smallTechFont 0x25a		// (602)
#define RES_tinyTechFont 0x25b		// (603)
#define RES_mediumTechFont 0x25c		// (604)
#define RES_largeTechFont 0x25d		// (605)
#define RES_citadelFont 0x25e		// (606)
#define RES_mediumLEDFont 0x25f		// (607)
#define RES_bigLEDFont 0x260		// (608)
#define RES_graffitiFont 0x261		// (609)
#define RES_mfdFont 0x262		// (610)
#define RES_cutsceneFont 0x263		// (611)
#define RES_readingFont 0x264		// (612)
#define RES_coloraliasedFont 0x265		// (613)
#define RES_editorGfx 0x266		// (614)
#define REF_IMG_bmBitsIcon 0x2660000		
#define REF_IMG_bmHeightIcon 0x2660001		
#define REF_IMG_bmPokeIcon 0x2660002		
#define REF_IMG_bmPaintIcon 0x2660003		
#define REF_IMG_bmParamsIcon 0x2660004		
#define REF_IMG_bmStringsIcon 0x2660005		
#define REF_IMG_bmToggle3DIcon 0x2660006		
#define REF_IMG_bmBlankIcon1 0x2660007		
#define REF_IMG_bmStopGoIcon 0x2660008		
#define REF_IMG_bmDemoIcon 0x2660009		
#define REF_IMG_bmMusicIcon 0x266000a		
#define REF_IMG_bmClearHighlightsIcon 0x266000b		
#define REF_IMG_bmRubberbandIcon 0x266000c		
#define REF_IMG_bmDebugIcon 0x266000d		
#define REF_IMG_bmRenderIcon 0x266000e		
#define REF_IMG_bmQuitIcon 0x266000f		
#define REF_IMG_bmBlankIcon2 0x2660010		
#define REF_IMG_bmEmptyIcon1 0x2660011		
#define REF_IMG_bmFindIcon 0x2660012		
#define REF_IMG_bmTextureIcon 0x2660013		
#define REF_IMG_bmZoomIcon 0x2660014		
#define REF_IMG_bmRaiseIcon 0x2660015		
#define REF_IMG_bmLoadLevelIcon 0x2660016		
#define REF_IMG_bmPhysicsIcon 0x2660017		
#define REF_IMG_bmTilePopupIcon 0x2660018		
#define REF_IMG_bmBlankIcon3 0x2660019		
#define REF_IMG_bmEmptyIcon2 0x266001a		
#define REF_IMG_bmEyeballIcon 0x266001b		
#define REF_IMG_bmObjectIcon 0x266001c		
#define REF_IMG_bmUnzoomIcon 0x266001d		
#define REF_IMG_bmLowerIcon 0x266001e		
#define REF_IMG_bmSaveLevelIcon 0x266001f		
#define REF_IMG_bmTmapSelectIcon 0x2660020		
#define REF_IMG_bmEditPaintIcon 0x2660021		
#define REF_IMG_bmBlankIcon5 0x2660022		
#define REF_IMG_bmEmptyIcon3 0x2660023		
#define REF_IMG_bmLoadIcon 0x2660024		
#define REF_IMG_bmSaveIcon 0x2660025		
#define REF_IMG_bmCutpasteIcon 0x2660026		
#define REF_IMG_bmGunPistolIcon 0x2660027		
#define REF_IMG_bmGunAutoIcon 0x2660028		
#define REF_IMG_bmGunSpecialIcon 0x2660029		
#define REF_IMG_bmGunHandtohandIcon 0x266002a		
#define REF_IMG_bmGunBeamIcon 0x266002b		
#define REF_IMG_bmGunBeamprojIcon 0x266002c		
#define REF_IMG_bmAmmoPistolIcon 0x266002d		
#define REF_IMG_bmAmmoNeedleIcon 0x266002e		
#define REF_IMG_bmAmmoMagnumIcon 0x266002f		
#define REF_IMG_bmAmmoRifleIcon 0x2660030		
#define REF_IMG_bmAmmoFlechetteIcon 0x2660031		
#define REF_IMG_bmAmmoAutoIcon 0x2660032		
#define REF_IMG_bmAmmoProjIcon 0x2660033		
#define REF_IMG_bmPhysicsTracerIcon 0x2660034		
#define REF_IMG_bmPhysicsSlowIcon 0x2660035		
#define REF_IMG_bmPhysicsCameraIcon 0x2660036		
#define REF_IMG_bmGrenadeDirectIcon 0x2660037		
#define REF_IMG_bmGrenadeTimedIcon 0x2660038		
#define REF_IMG_bmDrugStatsIcon 0x2660039		
#define REF_IMG_bmHardwareGoggleIcon 0x266003a		
#define REF_IMG_bmHardwareHardwareIcon 0x266003b		
#define REF_IMG_bmSoftwareOffensiveIcon 0x266003c		
#define REF_IMG_bmSoftwareDefensiveIcon 0x266003d		
#define REF_IMG_bmSoftwareMiscIcon 0x266003e		
#define REF_IMG_bmSoftwareOneShotIcon 0x266003f		
#define REF_IMG_bmSoftwareDataIcon 0x2660040		
#define REF_IMG_bmBigstuffElectronicsIcon 0x2660041		
#define REF_IMG_bmBigstuffFurnishingsIcon 0x2660042		
#define REF_IMG_bmBigstuffOnthewallIcon 0x2660043		
#define REF_IMG_bmBigstuffLightIcon 0x2660044		
#define REF_IMG_bmBigstuffLabgearIcon 0x2660045		
#define REF_IMG_bmBigstuffTechnoIcon 0x2660046		
#define REF_IMG_bmBigstuffDecor 0x2660047		
#define REF_IMG_bmBigstuffTerrain 0x2660048		
#define REF_IMG_bmSmallstuffUselessIcon 0x2660049		
#define REF_IMG_bmSmallstuffBrokenIcon 0x266004a		
#define REF_IMG_bmSmallstuffCorpselikeIcon 0x266004b		
#define REF_IMG_bmSmallstuffGearIcon 0x266004c		
#define REF_IMG_bmSmallstuffCardsIcon 0x266004d		
#define REF_IMG_bmSmallstuffCyberIcon 0x266004e		
#define REF_IMG_bmSmallstuffOnthewall 0x266004f		
#define REF_IMG_bmSmallstuffPlot 0x2660050		
#define REF_IMG_bmFixtureControlIcon 0x2660051		
#define REF_IMG_bmFixtureReceptacleIcon 0x2660052		
#define REF_IMG_bmFixtureTerminalIcon 0x2660053		
#define REF_IMG_bmFixturePanelIcon 0x2660054		
#define REF_IMG_bmFixtureVendingIcon 0x2660055		
#define REF_IMG_bmFixtureCyberIcon 0x2660056		
#define REF_IMG_bmDoorNormalIcon 0x2660057		
#define REF_IMG_bmDoorDoorwaysIcon 0x2660058		
#define REF_IMG_bmDoorForceIcon 0x2660059		
#define REF_IMG_bmDoorElevatorIcon 0x266005a		
#define REF_IMG_bmDoorSpecialIcon 0x266005b		
#define REF_IMG_bmAnimatingObjectsIcon 0x266005c		
#define REF_IMG_bmAnimatingTransitoryIcon 0x266005d		
#define REF_IMG_bmAnimatingExplosionIcon 0x266005e		
#define REF_IMG_bmTrapTriggerIcon 0x266005f		
#define REF_IMG_bmTrapFeedbackIcon 0x2660060		
#define REF_IMG_bmTrapSecretIcon 0x2660061		
#define REF_IMG_bmContainerActualIcon 0x2660062		
#define REF_IMG_bmContainerWasteIcon 0x2660063		
#define REF_IMG_bmContainerLiquidIcon 0x2660064		
#define REF_IMG_bmContainerMutantCorpseIcon 0x2660065		
#define REF_IMG_bmContainerRobotCorpseIcon 0x2660066		
#define REF_IMG_bmContainerCyborgCorpseIcon 0x2660067		
#define REF_IMG_bmContainerOtherCorpseIcon 0x2660068		
#define REF_IMG_bmCritterMutantIcon 0x2660069		
#define REF_IMG_bmCritterRobotIcon 0x266006a		
#define REF_IMG_bmCritterCyborgIcon 0x266006b		
#define REF_IMG_bmCritterCyberIcon 0x266006c		
#define REF_IMG_bmCritterRobobabeIcon 0x266006d		
#define RES_leanmeterstub 0x267		// (615)
#define RES_popups 0x268		// (616)
#define RES_gamescrFull 0x269		// (617)
#define REF_IMG_bmGamescreenBackground 0x2690000		
#define RES_gamescrSHODAN 0x26a		// (618)
#define REF_IMG_bmSHODANEndgame 0x26a0000		
#define REF_IMG_bmSHODANEndgameFull 0x26a0001		
#define RES_SideIconArt 0x550		// (1360)
#define RES_FullAmapBack 0x551		// (1361)
#define REF_IMG_bmTriLogoBack 0x5510000		
#define RES_cursorNormal 0x552		// (1362)
#define REF_IMG_bmTargetCursor 0x5520000		
#define REF_IMG_bmUpCursor 0x5520001		
#define REF_IMG_bmUpLeftCursor 0x5520002		
#define REF_IMG_bmUpRightCursor 0x5520003		
#define REF_IMG_bmCircLeftCursor 0x5520004		
#define REF_IMG_bmCircRightCursor 0x5520005		
#define REF_IMG_bmLeftCursor 0x5520006		
#define REF_IMG_bmRightCursor 0x5520007		
#define REF_IMG_bmDownCursor 0x5520008		
#define REF_IMG_bmSprintCursor 0x5520009		
#define REF_IMG_bmMfdPhaserCursor 0x552000a		
#define REF_IMG_bmVmailCursor 0x552000b		
#define RES_cursorMisc 0x553		// (1363)
#define REF_IMG_bmWaitCursor 0x5530000		
#define REF_IMG_bmOptionCursor 0x5530001		
#define REF_IMG_bmFireCursor 0x5530002		
#define REF_IMG_bmXCursor 0x5530003		
#define RES_cursorCyber 0x554		// (1364)
#define REF_IMG_bmCyberUpLeftCursor 0x5540000		
#define REF_IMG_bmCyberUpCursor 0x5540001		
#define REF_IMG_bmCyberUpRightCursor 0x5540002		
#define REF_IMG_bmCyberBankLeftCursor 0x5540003		
#define REF_IMG_bmCyberForwardCursor 0x5540004		
#define REF_IMG_bmCyberBankRightCursor 0x5540005		
#define REF_IMG_bmCyberRollLeftCursor 0x5540006		
#define REF_IMG_bmCyberDownCursor 0x5540007		
#define REF_IMG_bmCyberRollRightCursor 0x5540008		
#define RES_leanMeterBase 0x555		// (1365)
#define RES_leanMeterBio 0x556		// (1366)
#define RES_leanMeterRad 0x557		// (1367)
#define RES_leanShield1 0x558		// (1368)
#define RES_leanShield2 0x559		// (1369)
#define RES_leanShield3 0x55a		// (1370)
#define RES_leanShield4 0x55b		// (1371)
#define RES_leanMeterBack 0x55c		// (1372)
#define REF_IMG_bmLeanBkgnd 0x55c0000		
#define REF_IMG_bmEyeIconL 0x55c0001		
#define REF_IMG_bmEyeIconLdark 0x55c0002		
#define REF_IMG_bmEyeIconR 0x55c0003		
#define REF_IMG_bmEyeIconRdark 0x55c0004		
#define REF_IMG_bmLeanBkgndTransp 0x55c0005		
#define RES_doubleTinyTechFont 0x730		// (1840)
#define RES_megaTinyTechFont 0x731		// (1841)
#define RES_doubleMediumLEDFont 0x732		// (1842)
#define RES_megaMediumLEDFont 0x733		// (1843)
#define RES_tallTinyTechFont 0x734		// (1844)
#define RES_doubleCutsceneFont 0x735		// (1845)

#endif
