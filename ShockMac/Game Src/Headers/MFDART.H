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

#ifndef __MFDART_H
#define __MFDART_H

#define RES_mfdArtOverlays 0x25		// (37)
#define RES_mfdSpecial 0x26		// (38)
#define REF_IMG_LitLamp0 0x260000		
#define REF_IMG_LitLamp1 0x260001		
#define REF_IMG_LitLamp2 0x260002		
#define REF_IMG_UnlitLamp0 0x260003		
#define REF_IMG_UnlitLamp1 0x260004		
#define REF_IMG_UnlitLamp2 0x260005		
#define REF_IMG_EmailButt0 0x260006		
#define REF_IMG_EmailButt1 0x260007		
#define REF_IMG_EmailButt2 0x260008		
#define REF_IMG_LitEmailButt0 0x260009		
#define REF_IMG_LitEmailButt1 0x26000a		
#define REF_IMG_LitEmailButt2 0x26000b		
#define REF_IMG_PrevPage 0x26000c		
#define REF_IMG_Near 0x26000d		
#define REF_IMG_NextPage 0x26000e		
#define REF_IMG_LitShield0 0x26000f		
#define REF_IMG_LitShield1 0x260010		
#define REF_IMG_LitShield2 0x260011		
#define REF_IMG_LitShieldSuper 0x260012		
#define REF_IMG_UnlitShield0 0x260013		
#define REF_IMG_UnlitShield1 0x260014		
#define REF_IMG_UnlitShield2 0x260015		
#define REF_IMG_UnlitShieldSuper 0x260016		
#define REF_IMG_UnlitMotion0 0x260017		
#define REF_IMG_UnlitMotion1 0x260018		
#define REF_IMG_UnlitMotion2 0x260019		
#define REF_IMG_LitMotion0 0x26001a		
#define REF_IMG_LitMotion1 0x26001b		
#define REF_IMG_LitMotion2 0x26001c		
#define REF_IMG_BullFrame 0x26001d		
#define REF_IMG_BullFrame2 0x26001e		
#define REF_IMG_BullFrame3 0x26001f		
#define REF_IMG_BullRightArrow 0x260020		
#define REF_IMG_BullLeftArrow 0x260021		
#define REF_IMG_NoAmmo 0x260022		
#define REF_IMG_Active 0x260023		
#define REF_IMG_Inactive 0x260024		
#define REF_IMG_Apply 0x260025		
#define REF_IMG_Activate 0x260026		
#define REF_IMG_BeamOverload 0x260027		
#define REF_IMG_BeamOverloadOn 0x260028		
#define REF_IMG_BeamOverloadOff 0x260029		
#define REF_IMG_BeamTemperature 0x26002a		
#define REF_IMG_BeamSetting 0x26002b		
#define REF_IMG_TargetButton 0x26002c		
#define REF_IMG_DiscardButton 0x26002d		
#define REF_IMG_BioIcon1 0x26002e		
#define REF_IMG_BioIconNot 0x260036		
#define REF_IMG_TinyArrowUp 0x260037		
#define REF_IMG_TinyArrowDown 0x260038		
#define REF_IMG_QuestionCursor 0x260039		
#define REF_IMG_CircuitBack 0x26003a		
#define REF_IMG_RookSymbol 0x26003b		
#define REF_IMG_KingSymbol 0x26003c		
#define REF_IMG_BishopSymbol 0x26003d		
#define REF_IMG_QueenSymbol 0x26003e		
#define REF_IMG_SimpleSymbol 0x26003f		
#define REF_IMG_ViewIcon1 0x260040		
#define REF_IMG_Use 0x260043		
#define REF_IMG_GridHelpSwitch 0x260044		
#define REF_IMG_MFDButtonBack 0x260045		
#define REF_IMG_On 0x260046		
#define REF_IMG_Off 0x260047		
#define RES_GamesBitmaps 0x27		// (39)
#define REF_IMG_LittleGuy 0x270000		
#define REF_IMG_DepthCharge 0x270001		
#define REF_IMG_Destroyer 0x270002		
#define REF_IMG_RAservbot 0x270003		
#define REF_IMG_RAsec1bot 0x270004		
#define REF_IMG_RAhopper 0x270005		
#define REF_IMG_DiegoAnim15 0x270012		
#define REF_IMG_TriopLogo15 0x270016		
#define REF_IMG_ttt_Player 0x270017		
#define REF_IMG_ttt_Shodan 0x270018		
#define REF_IMG_GoofyNed 0x270019		
#define REF_IMG_wing_bad1 0x27001c		
#define REF_IMG_wing_bad2 0x270022		
#define REF_IMG_wing_bad3 0x270028		
#define REF_IMG_wing_ship 0x27002e		
#define RES_EmailMugShots 0x28		// (40)
#define REF_IMG_EmailMugShotBase 0x280000		
#define RES_mfdClass_1 0x29		// (41)
#define RES_mfdClass_2 0x2a		// (42)
#define RES_mfdClass_3 0x2b		// (43)
#define RES_mfdClass_4 0x2c		// (44)
#define RES_mfdClass_5 0x2d		// (45)
#define RES_mfdClass_6 0x2e		// (46)
#define RES_mfdClass_7 0x2f		// (47)
#define RES_mfdClass_8 0x30		// (48)
#define RES_mfdClass_9 0x31		// (49)
#define RES_mfdClass_10 0x32		// (50)
#define RES_mfdClass_11 0x33		// (51)
#define RES_mfdClass_12 0x34		// (52)
#define RES_mfdClass_13 0x35		// (53)
#define RES_mfdClass_14 0x36		// (54)
#define RES_mfdClass_15 0x37		// (55)

#endif
