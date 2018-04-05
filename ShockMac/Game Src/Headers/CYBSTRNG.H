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

#ifndef __CYBSTRNG_H
#define __CYBSTRNG_H

#define RES_traps 0x867		// (2151)
#define REF_STR_TrapZeroMessage 0x8670000		
#define RES_words 0x868		// (2152)
#define REF_STR_WordZero 0x8680000		
#define RES_names 0x869		// (2153)
#define REF_STR_Name0 0x8690000		
#define RES_texnames 0x86a		// (2154)
#define RES_texuse 0x86b		// (2155)
#define RES_inventory 0x86c		// (2156)
#define REF_STR_Null 0x86c0000		
#define REF_STR_WeaponTitle 0x86c0001		
#define REF_STR_AmmoTitle 0x86c0002		
#define REF_STR_GrenadeTitle 0x86c0003		
#define REF_STR_DrugTitle 0x86c0004		
#define REF_STR_PistolCartTitle 0x86c0005		
#define REF_STR_RifleCartTitle 0x86c0006		
#define REF_STR_AutoCartTitle 0x86c0007		
#define REF_STR_NeedleCartTitle 0x86c0008		
#define REF_STR_SpeclCartTitle 0x86c0009		
#define REF_STR_HardwareTitle 0x86c000a		
#define REF_STR_SoftTitle 0x86c000b		
#define REF_STR_SoftComTitle 0x86c000c		
#define REF_STR_SoftDefTitle 0x86c000d		
#define REF_STR_SoftMiscTitle 0x86c000e		
#define REF_STR_GeneralTitle 0x86c000f		
#define REF_STR_EmailTitle 0x86c0010		
#define REF_STR_DataTitle 0x86c0011		
#define REF_STR_LogTitle 0x86c0012		
#define REF_STR_InvReject 0x86c0013		
#define REF_STR_InvNoRoom 0x86c0014		
#define REF_STR_EmailMoreLeft 0x86c0015		
#define REF_STR_EmailMoreRight 0x86c0016		
#define REF_STR_AmmoLoad 0x86c0017		
#define REF_STR_AmmoOver 0x86c0018		
#define REF_STR_GunHot 0x86c0019		
#define REF_STR_GunWarm 0x86c001a		
#define REF_STR_GunOK 0x86c001b		
#define REF_STR_View360Update 0x86c001c		
#define REF_STR_ClickToLoad 0x86c001d		
#define REF_STR_DClickToUnload 0x86c001e		
#define REF_STR_TimeSetting 0x86c001f		
#define REF_STR_EnergySetting 0x86c0020		
#define REF_STR_LowSetting 0x86c0021		
#define REF_STR_HighSetting 0x86c0022		
#define REF_STR_Overload 0x86c0023		
#define REF_STR_MessageTitle 0x86c0024		
#define REF_STR_MessageSubject 0x86c0025		
#define REF_STR_EmptyGump 0x86c0026		
#define REF_STR_BiowareTitle 0x86c0027		
#define REF_STR_BiowareHealth 0x86c0028		
#define REF_STR_BiowareFatigue 0x86c0029		
#define REF_STR_BioHelpBase 0x86c002a		
#define REF_STR_HudBase 0x86c0032		
#define REF_STR_ViewHelpBase 0x86c0035		
#define REF_STR_HudColorsTitle 0x86c0038		
#define REF_STR_AmmoMFDWeaps 0x86c0039		
#define REF_STR_AmmoMFDClips 0x86c003a		
#define REF_STR_InvCursor 0x86c003b		
#define REF_STR_MFDCursor 0x86c0041		
#define REF_STR_IconCursor 0x86c0046		
#define REF_STR_NoAutomap 0x86c0050		
#define REF_STR_AmmoTypeLetters 0x86c0051		
#define RES_objshortnames 0x86d		// (2157)
#define REF_STR_MINIPISTOL_SHT 0x86d0000		
#define REF_STR_DARTPISTOL_SHT 0x86d0001		
#define REF_STR_MAGNUM_SHT 0x86d0002		
#define REF_STR_ASSAULTRFL_SHT 0x86d0003		
#define REF_STR_RIOTGUN_SHT 0x86d0004		
#define REF_STR_FLECHETTE_SHT 0x86d0005		
#define REF_STR_SKORPION_SHT 0x86d0006		
#define REF_STR_MAGPULSE_SHT 0x86d0007		
#define REF_STR_RAILGUN_SHT 0x86d0008		
#define REF_STR_BATON_SHT 0x86d0009		
#define REF_STR_LASERAPIER_SHT 0x86d000a		
#define REF_STR_PHASER_SHT 0x86d000b		
#define REF_STR_BLASTER_SHT 0x86d000c		
#define REF_STR_IONBEAM_SHT 0x86d000d		
#define REF_STR_STUNGUN_SHT 0x86d000e		
#define REF_STR_PLASMABEAM_SHT 0x86d000f		
#define REF_STR_SPAMMO_SHT 0x86d0010		
#define REF_STR_TEFAMMO_SHT 0x86d0011		
#define REF_STR_NNAMMO_SHT 0x86d0012		
#define REF_STR_TNAMMO_SHT 0x86d0013		
#define REF_STR_HTAMMO_SHT 0x86d0014		
#define REF_STR_HSAMMO_SHT 0x86d0015		
#define REF_STR_RBAMMO_SHT 0x86d0016		
#define REF_STR_MRAMMO_SHT 0x86d0017		
#define REF_STR_PRAMMO_SHT 0x86d0018		
#define REF_STR_HNAMMO_SHT 0x86d0019		
#define REF_STR_SPLAMMO_SHT 0x86d001a		
#define REF_STR_SLGAMMO_SHT 0x86d001b		
#define REF_STR_BGAMMO_SHT 0x86d001c		
#define REF_STR_MAGAMMO_SHT 0x86d001d		
#define REF_STR_RGAMMO_SHT 0x86d001e		
#define REF_STR_BULLTRACE_SHT 0x86d001f		
#define REF_STR_ENERTRACE_SHT 0x86d0020		
#define REF_STR_AUTOTRACE_SHT 0x86d0021		
#define REF_STR_NEEDTRACE_SHT 0x86d0022		
#define REF_STR_GRENTRACE_SHT 0x86d0023		
#define REF_STR_RUBBTRACE_SHT 0x86d0024		
#define REF_STR_VIRUSSLOW_SHT 0x86d0025		
#define REF_STR_ELITESLOW_SHT 0x86d0026		
#define REF_STR_ASSASINSLOW_SHT 0x86d0027		
#define REF_STR_MUTANTSLOW_SHT 0x86d0028		
#define REF_STR_ZEROSLOW_SHT 0x86d0029		
#define REF_STR_MAGBURST_SHT 0x86d002a		
#define REF_STR_RAILSLOW_SHT 0x86d002b		
#define REF_STR_STUNSLOW_SHT 0x86d002c		
#define REF_STR_PLASMABLST_SHT 0x86d002d		
#define REF_STR_CYBERBOLT_SHT 0x86d002e		
#define REF_STR_CYBERSLOW_SHT 0x86d002f		
#define REF_STR_DRILLSLOW_SHT 0x86d0030		
#define REF_STR_DISCSLOW_SHT 0x86d0031		
#define REF_STR_SPEWSLOW_SHT 0x86d0032		
#define REF_STR_PLANTSLOW_SHT 0x86d0033		
#define REF_STR_INVISOSLOW_SHT 0x86d0034		
#define REF_STR_DRONECAM_SHT 0x86d0035		
#define REF_STR_EXPLCAM_SHT 0x86d0036		
#define REF_STR_FRAG_G_SHT 0x86d0037		
#define REF_STR_EMP_G_SHT 0x86d0038		
#define REF_STR_GAS_G_SHT 0x86d0039		
#define REF_STR_CONC_G_SHT 0x86d003a		
#define REF_STR_L_MINE_SHT 0x86d003b		
#define REF_STR_NITRO_G_SHT 0x86d003c		
#define REF_STR_EARTH_G_SHT 0x86d003d		
#define REF_STR_OBJ_G_SHT 0x86d003e		
#define REF_STR_STAMINA_DRUG_SHT 0x86d003f		
#define REF_STR_SIGHT_DRUG_SHT 0x86d0040		
#define REF_STR_LSD_DRUG_SHT 0x86d0041		
#define REF_STR_MEDI_DRUG_SHT 0x86d0042		
#define REF_STR_NINJA_DRUG_SHT 0x86d0043		
#define REF_STR_GENIUS_DRUG_SHT 0x86d0044		
#define REF_STR_DETOX_DRUG_SHT 0x86d0045		
#define REF_STR_INFRA_GOG_SHT 0x86d0046		
#define REF_STR_TARG_GOG_SHT 0x86d0047		
#define REF_STR_SENS_HARD_SHT 0x86d0048		
#define REF_STR_AIM_GOG_SHT 0x86d0049		
#define REF_STR_HUD_GOG_SHT 0x86d004a		
#define REF_STR_BIOSCAN_HARD_SHT 0x86d004b		
#define REF_STR_NAV_HARD_SHT 0x86d004c		
#define REF_STR_SHIELD_HARD_SHT 0x86d004d		
#define REF_STR_VIDTEX_HARD_SHT 0x86d004e		
#define REF_STR_LANTERN_HARD_SHT 0x86d004f		
#define REF_STR_FULLSCR_HARD_SHT 0x86d0050		
#define REF_STR_ENV_HARD_SHT 0x86d0051		
#define REF_STR_MOTION_HARD_SHT 0x86d0052		
#define REF_STR_JET_HARD_SHT 0x86d0053		
#define REF_STR_STATUS_HARD_SHT 0x86d0054		
#define REF_STR_DRILL_SHT 0x86d0055		
#define REF_STR_SPEW_SHT 0x86d0056		
#define REF_STR_MINE_SHT 0x86d0057		
#define REF_STR_DISC_SHT 0x86d0058		
#define REF_STR_PULSER_SHT 0x86d0059		
#define REF_STR_SCRAMBLER_SHT 0x86d005a		
#define REF_STR_VIRUS_SHT 0x86d005b		
#define REF_STR_SHIELD_SHT 0x86d005c		
#define REF_STR_OLD_FAKEID_SHT 0x86d005d		
#define REF_STR_ICE_SHT 0x86d005e		
#define REF_STR_TURBO_SHT 0x86d005f		
#define REF_STR_FAKEID_SHT 0x86d0060		
#define REF_STR_DECOY_SHT 0x86d0061		
#define REF_STR_RECALL_SHT 0x86d0062		
#define REF_STR_GAMES_SHT 0x86d0063		
#define REF_STR_MONITOR1_SHT 0x86d0064		
#define REF_STR_IDENTIFY_SHT 0x86d0065		
#define REF_STR_TRACE_SHT 0x86d0066		
#define REF_STR_TOGGLE_SHT 0x86d0067		
#define REF_STR_TEXT1_SHT 0x86d0068		
#define REF_STR_EMAIL1_SHT 0x86d0069		
#define REF_STR_MAP1_SHT 0x86d006a		
#define REF_STR_PHONE_SHT 0x86d006b		
#define REF_STR_VCR_SHT 0x86d006c		
#define REF_STR_MICROWAVE_OVN_SHT 0x86d006d		
#define REF_STR_STEREO_SHT 0x86d006e		
#define REF_STR_KEYBOARD_SHT 0x86d006f		
#define REF_STR_SMALL_CPU_SHT 0x86d0070		
#define REF_STR_TV_SHT 0x86d0071		
#define REF_STR_MONITOR2_SHT 0x86d0072		
#define REF_STR_LARGCPU_SHT 0x86d0073		
#define REF_STR_LDESK_SHT 0x86d0074		
#define REF_STR_FDESK_SHT 0x86d0075		
#define REF_STR_CABINET_SHT 0x86d0076		
#define REF_STR_SHELF_SHT 0x86d0077		
#define REF_STR_HIDEAWAY_SHT 0x86d0078		
#define REF_STR_CHAIR_SHT 0x86d0079		
#define REF_STR_ENDTABLE_SHT 0x86d007a		
#define REF_STR_COUCH_SHT 0x86d007b		
#define REF_STR_EXECCHR_SHT 0x86d007c		
#define REF_STR_COATTREE_SHT 0x86d007d		
#define REF_STR_SIGN_SHT 0x86d007e		
#define REF_STR_ICON_SHT 0x86d007f		
#define REF_STR_GRAF_SHT 0x86d0080		
#define REF_STR_WORDS_SHT 0x86d0081		
#define REF_STR_PAINTING_SHT 0x86d0082		
#define REF_STR_POSTER_SHT 0x86d0083		
#define REF_STR_SCREEN_SHT 0x86d0084		
#define REF_STR_TMAP_SHT 0x86d0085		
#define REF_STR_SUPERSCREEN_SHT 0x86d0086		
#define REF_STR_BIGSCREEN_SHT 0x86d0087		
#define REF_STR_REPULSWALL_SHT 0x86d0088		
#define REF_STR_DESKLAMP_SHT 0x86d0089		
#define REF_STR_FLOORLAMP_SHT 0x86d008a		
#define REF_STR_GLOWBULB_SHT 0x86d008b		
#define REF_STR_CHAND_SHT 0x86d008c		
#define REF_STR_GENE_SPLICER_SHT 0x86d008d		
#define REF_STR_TUBING_SHT 0x86d008e		
#define REF_STR_MED_CART_SHT 0x86d008f		
#define REF_STR_SURG_MACH_SHT 0x86d0090		
#define REF_STR_TTUBE_RACK_SHT 0x86d0091		
#define REF_STR_RSRCH_CHAIR_SHT 0x86d0092		
#define REF_STR_HOSP_BED_SHT 0x86d0093		
#define REF_STR_BROKLAB1_SHT 0x86d0094		
#define REF_STR_BROKLAB2_SHT 0x86d0095		
#define REF_STR_MICROSCOPE_SHT 0x86d0096		
#define REF_STR_SCOPE_SHT 0x86d0097		
#define REF_STR_LAB_PROBE_SHT 0x86d0098		
#define REF_STR_XRAY_MACHINE_SHT 0x86d0099		
#define REF_STR_CAMERA_SHT 0x86d009a		
#define REF_STR_CONTPAN_SHT 0x86d009b		
#define REF_STR_CONTPED_SHT 0x86d009c		
#define REF_STR_ENERGY_MINE_SHT 0x86d009d		
#define REF_STR_STATUE1_SHT 0x86d009e		
#define REF_STR_SHRUB1_SHT 0x86d009f		
#define REF_STR_GRASS_SHT 0x86d00a0		
#define REF_STR_PLANT1_SHT 0x86d00a1		
#define REF_STR_FUNG1_SHT 0x86d00a2		
#define REF_STR_FUNG2_SHT 0x86d00a3		
#define REF_STR_PLANT2_SHT 0x86d00a4		
#define REF_STR_VINE1_SHT 0x86d00a5		
#define REF_STR_VINE2_SHT 0x86d00a6		
#define REF_STR_PLANT3_SHT 0x86d00a7		
#define REF_STR_PLANT4_SHT 0x86d00a8		
#define REF_STR_LBOULDER_SHT 0x86d00a9		
#define REF_STR_BBOULDER_SHT 0x86d00aa		
#define REF_STR_SHRUB2_SHT 0x86d00ab		
#define REF_STR_VSHRUB1_SHT 0x86d00ac		
#define REF_STR_VSHRUB2_SHT 0x86d00ad		
#define REF_STR_BRIDGE_SHT 0x86d00ae		
#define REF_STR_CATWALK_SHT 0x86d00af		
#define REF_STR_WALL_SHT 0x86d00b0		
#define REF_STR_FPILLAR_SHT 0x86d00b1		
#define REF_STR_RAILING1_SHT 0x86d00b2		
#define REF_STR_RAILING2_SHT 0x86d00b3		
#define REF_STR_PILLAR_SHT 0x86d00b4		
#define REF_STR_FORCE_BRIJ_SHT 0x86d00b5		
#define REF_STR_NON_BRIDGE_SHT 0x86d00b6		
#define REF_STR_FORCE_BRIJ2_SHT 0x86d00b7		
#define REF_STR_BEV_CONT_SHT 0x86d00b8		
#define REF_STR_WRAPPER_SHT 0x86d00b9		
#define REF_STR_PAPERS_SHT 0x86d00ba		
#define REF_STR_WARECASING_SHT 0x86d00bb		
#define REF_STR_EXTING_SHT 0x86d00bc		
#define REF_STR_HELMET_SHT 0x86d00bd		
#define REF_STR_CLOTHES_SHT 0x86d00be		
#define REF_STR_BRIEFCASE_SHT 0x86d00bf		
#define REF_STR_BROKEN_GUN_SHT 0x86d00c0		
#define REF_STR_MCHUNK1_SHT 0x86d00c1		
#define REF_STR_MCHUNK2_SHT 0x86d00c2		
#define REF_STR_MCHUNK3_SHT 0x86d00c3		
#define REF_STR_CRATE_FRAG_SHT 0x86d00c4		
#define REF_STR_BROKEN_PAN_SHT 0x86d00c5		
#define REF_STR_BROKEN_CLK_SHT 0x86d00c6		
#define REF_STR_MSCRAP_SHT 0x86d00c7		
#define REF_STR_BROKEN_LEV1_SHT 0x86d00c8		
#define REF_STR_BROKEN_LEV2_SHT 0x86d00c9		
#define REF_STR_CORPSE1_SHT 0x86d00ca		
#define REF_STR_CORPSE2_SHT 0x86d00cb		
#define REF_STR_CORPSE3_SHT 0x86d00cc		
#define REF_STR_CORPSE4_SHT 0x86d00cd		
#define REF_STR_CORPSE5_SHT 0x86d00ce		
#define REF_STR_CORPSE6_SHT 0x86d00cf		
#define REF_STR_CORPSE7_SHT 0x86d00d0		
#define REF_STR_CORPSE8_SHT 0x86d00d1		
#define REF_STR_SKEL_RAGS_SHT 0x86d00d2		
#define REF_STR_BONES1_SHT 0x86d00d3		
#define REF_STR_BONES2_SHT 0x86d00d4		
#define REF_STR_SKULL_SHT 0x86d00d5		
#define REF_STR_LIMB_SHT 0x86d00d6		
#define REF_STR_HEAD_SHT 0x86d00d7		
#define REF_STR_HEAD2_SHT 0x86d00d8		
#define REF_STR_EPICK_SHT 0x86d00d9		
#define REF_STR_BATTERY2_SHT 0x86d00da		
#define REF_STR_ROD_SHT 0x86d00db		
#define REF_STR_AIDKIT_SHT 0x86d00dc		
#define REF_STR_TRACBEAM_SHT 0x86d00dd		
#define REF_STR_BATTERY_SHT 0x86d00de		
#define REF_STR_GENCARDS_SHT 0x86d00df		
#define REF_STR_STDCARD_SHT 0x86d00e0		
#define REF_STR_SCICARD_SHT 0x86d00e1		
#define REF_STR_STORECARD_SHT 0x86d00e2		
#define REF_STR_ENGCARD_SHT 0x86d00e3		
#define REF_STR_MEDCARD_SHT 0x86d00e4		
#define REF_STR_MAINTCARD_SHT 0x86d00e5		
#define REF_STR_ADMINCARD_SHT 0x86d00e6		
#define REF_STR_SECCARD_SHT 0x86d00e7		
#define REF_STR_COMCARD_SHT 0x86d00e8		
#define REF_STR_GROUPCARD_SHT 0x86d00e9		
#define REF_STR_PERSCARD_SHT 0x86d00ea		
#define REF_STR_MULTIPLEXR_SHT 0x86d00eb		
#define REF_STR_CYBERHEAL_SHT 0x86d00ec		
#define REF_STR_CYBERMINE_SHT 0x86d00ed		
#define REF_STR_CYBERCARD_SHT 0x86d00ee		
#define REF_STR_SHODO_SHRINE_SHT 0x86d00ef		
#define REF_STR_ICEWALL_SHT 0x86d00f0		
#define REF_STR_INFONODE_SHT 0x86d00f1		
#define REF_STR_CSPACE_EXIT_SHT 0x86d00f2		
#define REF_STR_DATALET_SHT 0x86d00f3		
#define REF_STR_BARRICADE_SHT 0x86d00f4		
#define REF_STR_TARGET_SHT 0x86d00f5		
#define REF_STR_ARROW_SHT 0x86d00f6		
#define REF_STR_BEAMBLST_SHT 0x86d00f7		
#define REF_STR_ACIDCORR_SHT 0x86d00f8		
#define REF_STR_BULLETHOLE_SHT 0x86d00f9		
#define REF_STR_EXBLAST_SHT 0x86d00fa		
#define REF_STR_BURNRES_SHT 0x86d00fb		
#define REF_STR_BLOODSTN_SHT 0x86d00fc		
#define REF_STR_CHEMSPLAT_SHT 0x86d00fd		
#define REF_STR_OILPUDDLE_SHT 0x86d00fe		
#define REF_STR_WASTESPILL_SHT 0x86d00ff		
#define REF_STR_ISOTOPE_X_SHT 0x86d0100		
#define REF_STR_CIRCBOARD1_SHT 0x86d0101		
#define REF_STR_PLASTIQUE_SHT 0x86d0102		
#define REF_STR_FAUX_X_SHT 0x86d0103		
#define REF_STR_CIRCBOARD4_SHT 0x86d0104		
#define REF_STR_CIRCBOARD5_SHT 0x86d0105		
#define REF_STR_CIRCBOARD6_SHT 0x86d0106		
#define REF_STR_CIRCBOARD7_SHT 0x86d0107		
#define REF_STR_SWITCH1_SHT 0x86d0108		
#define REF_STR_SWITCH2_SHT 0x86d0109		
#define REF_STR_BUTTON1_SHT 0x86d010a		
#define REF_STR_BUTTON2_SHT 0x86d010b		
#define REF_STR_LEVER1_SHT 0x86d010c		
#define REF_STR_LEVER2_SHT 0x86d010d		
#define REF_STR_BIGRED_SHT 0x86d010e		
#define REF_STR_BIGLEVER_SHT 0x86d010f		
#define REF_STR_DIAL_SHT 0x86d0110		
#define REF_STR_ACCESS_SLOT_SHT 0x86d0111		
#define REF_STR_CRCT_BD_SLOT_SHT 0x86d0112		
#define REF_STR_CHEM_RECEPT_SHT 0x86d0113		
#define REF_STR_ANTENNA_PAN_SHT 0x86d0114		
#define REF_STR_PLAS_ANTENNA_SHT 0x86d0115		
#define REF_STR_DEST_ANTENNA_SHT 0x86d0116		
#define REF_STR_RETSCANNER_SHT 0x86d0117		
#define REF_STR_CYB_TERM_SHT 0x86d0118		
#define REF_STR_ENRG_CHARGE_SHT 0x86d0119		
#define REF_STR_FIXUP_STATION_SHT 0x86d011a		
#define REF_STR_ACCPANEL1_SHT 0x86d011b		
#define REF_STR_ACCPANEL2_SHT 0x86d011c		
#define REF_STR_ACCPANEL3_SHT 0x86d011d		
#define REF_STR_ACCPANEL4_SHT 0x86d011e		
#define REF_STR_ELEPANEL1_SHT 0x86d011f		
#define REF_STR_ELEPANEL2_SHT 0x86d0120		
#define REF_STR_ELEPANEL3_SHT 0x86d0121		
#define REF_STR_KEYPAD1_SHT 0x86d0122		
#define REF_STR_KEYPAD2_SHT 0x86d0123		
#define REF_STR_ACCPANEL5_SHT 0x86d0124		
#define REF_STR_ACCPANEL6_SHT 0x86d0125		
#define REF_STR_AMMOVEND_SHT 0x86d0126		
#define REF_STR_HEALVEND_SHT 0x86d0127		
#define REF_STR_CYBERTOG1_SHT 0x86d0128		
#define REF_STR_CYBERTOG2_SHT 0x86d0129		
#define REF_STR_CYBERTOG3_SHT 0x86d012a		
#define REF_STR_BLAST_DOOR_SHT 0x86d012b		
#define REF_STR_ACCESS_DOOR_SHT 0x86d012c		
#define REF_STR_RESID_DOOR_SHT 0x86d012d		
#define REF_STR_MAINT_DOOR_SHT 0x86d012e		
#define REF_STR_HOSP_DOOR_SHT 0x86d012f		
#define REF_STR_LAB_DOOR_SHT 0x86d0130		
#define REF_STR_STOR_DOOR_SHT 0x86d0131		
#define REF_STR_REACTR_DOOR_SHT 0x86d0132		
#define REF_STR_EXEC_DOOR_SHT 0x86d0133		
#define REF_STR_NO_DOOR_SHT 0x86d0134		
#define REF_STR_LAB_DOORWAY_SHT 0x86d0135		
#define REF_STR_RES_DOORWAY_SHT 0x86d0136		
#define REF_STR_BRJ_DOORWAY_SHT 0x86d0137		
#define REF_STR_RCT_DOORWAY_SHT 0x86d0138		
#define REF_STR_GRATING1_SHT 0x86d0139		
#define REF_STR_GRATING2_SHT 0x86d013a		
#define REF_STR_GRATING3_SHT 0x86d013b		
#define REF_STR_GRATING4_SHT 0x86d013c		
#define REF_STR_NO_DOOR2_SHT 0x86d013d		
#define REF_STR_LABFORCE_SHT 0x86d013e		
#define REF_STR_BROKLABFORCE_SHT 0x86d013f		
#define REF_STR_RESFORCE_SHT 0x86d0140		
#define REF_STR_BROKRESFORCE_SHT 0x86d0141		
#define REF_STR_GENFORCE_SHT 0x86d0142		
#define REF_STR_CYBGENFORCE_SHT 0x86d0143		
#define REF_STR_NO_DOOR3_SHT 0x86d0144		
#define REF_STR_EXEC_ELEV_SHT 0x86d0145		
#define REF_STR_REG_ELEV1_SHT 0x86d0146		
#define REF_STR_REG_ELEV2_SHT 0x86d0147		
#define REF_STR_FREIGHT_ELEV_SHT 0x86d0148		
#define REF_STR_NO_DOOR4_SHT 0x86d0149		
#define REF_STR_DOUB_LEFTDOOR_SHT 0x86d014a		
#define REF_STR_DOUB_RITEDOOR_SHT 0x86d014b		
#define REF_STR_IRIS_SHT 0x86d014c		
#define REF_STR_VERT_OPEN_SHT 0x86d014d		
#define REF_STR_VERT_SPLIT_SHT 0x86d014e		
#define REF_STR_NO_DOOR5_SHT 0x86d014f		
#define REF_STR_SECRET_DOOR1_SHT 0x86d0150		
#define REF_STR_SECRET_DOOR2_SHT 0x86d0151		
#define REF_STR_SECRET_DOOR3_SHT 0x86d0152		
#define REF_STR_INVISO_DOOR_SHT 0x86d0153		
#define REF_STR_ALERT_PANEL_OFF_SHT 0x86d0154		
#define REF_STR_ALERT_PANEL_ON_SHT 0x86d0155		
#define REF_STR_HORZ_KLAXOFF_SHT 0x86d0156		
#define REF_STR_HORZ_KLAXON_SHT 0x86d0157		
#define REF_STR_SPARK_CABLE_SHT 0x86d0158		
#define REF_STR_TWITCH_MUT2_SHT 0x86d0159		
#define REF_STR_MACHINE_SHT 0x86d015a		
#define REF_STR_HOLOG_ANIM_SHT 0x86d015b		
#define REF_STR_TWITCH_MUT_SHT 0x86d015c		
#define REF_STR_BLOOD1_SHT 0x86d015d		
#define REF_STR_CAMEXPL_SHT 0x86d015e		
#define REF_STR_TVEXPL_SHT 0x86d015f		
#define REF_STR_SIMPLSMOKE_SHT 0x86d0160		
#define REF_STR_PLANTEXPL_SHT 0x86d0161		
#define REF_STR_BULLETWALLHIT_SHT 0x86d0162		
#define REF_STR_BEAMWALLHIT_SHT 0x86d0163		
#define REF_STR_IMPACT_ANIM_SHT 0x86d0164		
#define REF_STR_BULL_ROBOT_SHT 0x86d0165		
#define REF_STR_BEAM_ROBOT1_SHT 0x86d0166		
#define REF_STR_BEAM_ROBOT2_SHT 0x86d0167		
#define REF_STR_EXPLOSION1_SHT 0x86d0168		
#define REF_STR_EXPLOSION2_SHT 0x86d0169		
#define REF_STR_EXPLOSION3_SHT 0x86d016a		
#define REF_STR_LG_EXPLOSION_SHT 0x86d016b		
#define REF_STR_MAGPULSEHIT_SHT 0x86d016c		
#define REF_STR_STUNHIT_SHT 0x86d016d		
#define REF_STR_PLASMAHIT_SHT 0x86d016e		
#define REF_STR_SMOKEEXPL_SHT 0x86d016f		
#define REF_STR_CRATEEXPL_SHT 0x86d0170		
#define REF_STR_MNTR2EXPL_SHT 0x86d0171		
#define REF_STR_GASEXPL_SHT 0x86d0172		
#define REF_STR_EMPEXPL_SHT 0x86d0173		
#define REF_STR_CORP_HUM_EXPL_SHT 0x86d0174		
#define REF_STR_CORP_ROB_EXPL_SHT 0x86d0175		
#define REF_STR_ENTRY_TRIG_SHT 0x86d0176		
#define REF_STR_NULL_TRIG_SHT 0x86d0177		
#define REF_STR_FLOOR_TRIG_SHT 0x86d0178		
#define REF_STR_PLRDETH_TRIG_SHT 0x86d0179		
#define REF_STR_DETHWATCH_TRIG_SHT 0x86d017a		
#define REF_STR_AOE_ENT_TRIG_SHT 0x86d017b		
#define REF_STR_AOE_CON_TRIG_SHT 0x86d017c		
#define REF_STR_AI_HINT_SHT 0x86d017d		
#define REF_STR_LEVEL_TRIG_SHT 0x86d017e		
#define REF_STR_CONTIN_TRIG_SHT 0x86d017f		
#define REF_STR_REPULSOR_SHT 0x86d0180		
#define REF_STR_ECOLOGY_TRIG_SHT 0x86d0181		
#define REF_STR_SHODO_TRIG_SHT 0x86d0182		
#define REF_STR_TRIPBEAM_SHT 0x86d0183		
#define REF_STR_BIOHAZARD_SHT 0x86d0184		
#define REF_STR_RADHAZARD_SHT 0x86d0185		
#define REF_STR_CHEMHAZARD_SHT 0x86d0186		
#define REF_STR_MAPNOTE_SHT 0x86d0187		
#define REF_STR_MUSIC_MARK_SHT 0x86d0188		
#define REF_STR_SML_CRT_SHT 0x86d0189		
#define REF_STR_LG_CRT_SHT 0x86d018a		
#define REF_STR_SECURE_CONTR_SHT 0x86d018b		
#define REF_STR_RAD_BARREL_SHT 0x86d018c		
#define REF_STR_TOXIC_BARREL_SHT 0x86d018d		
#define REF_STR_CHEM_TANK_SHT 0x86d018e		
#define REF_STR_THERMOS_SHT 0x86d018f		
#define REF_STR_VIAL_CONT_SHT 0x86d0190		
#define REF_STR_FLASK_CONT_SHT 0x86d0191		
#define REF_STR_BEAKER_CONT_SHT 0x86d0192		
#define REF_STR_MUT_CORPSE1_SHT 0x86d0193		
#define REF_STR_MUT_CORPSE2_SHT 0x86d0194		
#define REF_STR_MUT_CORPSE3_SHT 0x86d0195		
#define REF_STR_MUT_CORPSE4_SHT 0x86d0196		
#define REF_STR_MUT_CORPSE5_SHT 0x86d0197		
#define REF_STR_MUT_CORPSE6_SHT 0x86d0198		
#define REF_STR_MUT_CORPSE7_SHT 0x86d0199		
#define REF_STR_MUT_CORPSE8_SHT 0x86d019a		
#define REF_STR_ROB_CORPSE1_SHT 0x86d019b		
#define REF_STR_ROB_CORPSE2_SHT 0x86d019c		
#define REF_STR_ROB_CORPSE3_SHT 0x86d019d		
#define REF_STR_ROB_CORPSE4_SHT 0x86d019e		
#define REF_STR_ROB_CORPSE5_SHT 0x86d019f		
#define REF_STR_ROB_CORPSE6_SHT 0x86d01a0		
#define REF_STR_ROB_CORPSE7_SHT 0x86d01a1		
#define REF_STR_ROB_CORPSE8_SHT 0x86d01a2		
#define REF_STR_ROB_CORPSE9_SHT 0x86d01a3		
#define REF_STR_ROB_CORPSE10_SHT 0x86d01a4		
#define REF_STR_ROB_CORPSE11_SHT 0x86d01a5		
#define REF_STR_ROB_CORPSE12_SHT 0x86d01a6		
#define REF_STR_ROB_CORPSE13_SHT 0x86d01a7		
#define REF_STR_CYB_CORPSE1_SHT 0x86d01a8		
#define REF_STR_CYB_CORPSE2_SHT 0x86d01a9		
#define REF_STR_CYB_CORPSE3_SHT 0x86d01aa		
#define REF_STR_CYB_CORPSE4_SHT 0x86d01ab		
#define REF_STR_CYB_CORPSE5_SHT 0x86d01ac		
#define REF_STR_CYB_CORPSE6_SHT 0x86d01ad		
#define REF_STR_CYB_CORPSE7_SHT 0x86d01ae		
#define REF_STR_OTH_CORPSE1_SHT 0x86d01af		
#define REF_STR_OTH_CORPSE2_SHT 0x86d01b0		
#define REF_STR_OTH_CORPSE3_SHT 0x86d01b1		
#define REF_STR_OTH_CORPSE4_SHT 0x86d01b2		
#define REF_STR_OTH_CORPSE5_SHT 0x86d01b3		
#define REF_STR_OTH_CORPSE6_SHT 0x86d01b4		
#define REF_STR_OTH_CORPSE7_SHT 0x86d01b5		
#define REF_STR_OTH_CORPSE8_SHT 0x86d01b6		
#define REF_STR_HUMAN_CRIT_SHT 0x86d01b7		
#define REF_STR_GOR_TIGER_SHT 0x86d01b8		
#define REF_STR_INSECT_CRIT_SHT 0x86d01b9		
#define REF_STR_AVIAN_CRIT_SHT 0x86d01ba		
#define REF_STR_PLANT_CRIT_SHT 0x86d01bb		
#define REF_STR_ZERO_CRIT_SHT 0x86d01bc		
#define REF_STR_PLAYER_CRIT_SHT 0x86d01bd		
#define REF_STR_INVISO_CRIT_SHT 0x86d01be		
#define REF_STR_VIRUS_CRIT_SHT 0x86d01bf		
#define REF_STR_LIFT_BOT_SHT 0x86d01c0		
#define REF_STR_REPAIRBOT_SHT 0x86d01c1		
#define REF_STR_SERVBOT_SHT 0x86d01c2		
#define REF_STR_EXECBOT_SHT 0x86d01c3		
#define REF_STR_LGTURRET_SHT 0x86d01c4		
#define REF_STR_HOPPER_SHT 0x86d01c5		
#define REF_STR_SECURITY_BOT1_SHT 0x86d01c6		
#define REF_STR_SECURITY_BOT2_SHT 0x86d01c7		
#define REF_STR_AUTOBOMB_SHT 0x86d01c8		
#define REF_STR_REPAIRBOT2_SHT 0x86d01c9		
#define REF_STR_FLIER_SHT 0x86d01ca		
#define REF_STR_SECURITY_BOT3_SHT 0x86d01cb		
#define REF_STR_CYBORG_DRONE_SHT 0x86d01cc		
#define REF_STR_WARRIOR_SHT 0x86d01cd		
#define REF_STR_ASSASSIN_SHT 0x86d01ce		
#define REF_STR_CYBERBABE_SHT 0x86d01cf		
#define REF_STR_ELITE_GUARD_SHT 0x86d01d0		
#define REF_STR_CORTEX_REAVER_SHT 0x86d01d1		
#define REF_STR_MUTANT_BORG_SHT 0x86d01d2		
#define REF_STR_CYBERDOG_SHT 0x86d01d3		
#define REF_STR_CYBERGUARD_SHT 0x86d01d4		
#define REF_STR_CYBER_CORTEX_SHT 0x86d01d5		
#define REF_STR_CYBER_DYN_ICE_SHT 0x86d01d6		
#define REF_STR_CYBER_HNT_KIL_SHT 0x86d01d7		
#define REF_STR_CYBER_SHODAN_SHT 0x86d01d8		
#define REF_STR_CYBERGUARD2_SHT 0x86d01d9		
#define REF_STR_ROBOBABE_SHT 0x86d01da		
#define REF_STR_DIEGO_SHT 0x86d01db		
#define RES_targeting 0x86e		// (2158)
#define REF_STR_NoTarget 0x86e0000		
#define REF_STR_NoTargetWare 0x86e0001		
#define REF_STR_TargetID 0x86e0002		
#define REF_STR_Condition 0x86e0003		
#define REF_STR_TargRange 0x86e0004		
#define REF_STR_NumKills 0x86e0005		
#define REF_STR_MutantDmg 0x86e0006		
#define REF_STR_RobotDmg 0x86e000c		
#define REF_STR_CyborgDmg 0x86e0012		
#define REF_STR_CyberDmg 0x86e0018		
#define REF_STR_BabeDmg 0x86e001e		
#define REF_STR_CritMoods 0x86e0024		
#define REF_STR_CritClasses 0x86e002c		
#define RES_HUDstrings 0x86f		// (2159)
#define REF_STR_InfraredOn 0x86f0000		
#define REF_STR_ExplosionDetect 0x86f0001		
#define REF_STR_oClock 0x86f0002		
#define REF_STR_RadiationZone 0x86f0003		
#define REF_STR_BiohazardZone 0x86f0004		
#define REF_STR_ShodanHud 0x86f0005		
#define REF_STR_HighFatigue 0x86f0006		
#define REF_STR_ShieldAbsorb 0x86f0007		
#define REF_STR_AbnormalGravity1 0x86f0008		
#define REF_STR_AbnormalGravity2 0x86f0009		
#define REF_STR_CyberDecoy 0x86f000a		
#define REF_STR_CyberFakeID 0x86f000b		
#define REF_STR_CyberTurbo 0x86f000c		
#define REF_STR_CyberTime 0x86f000d		
#define REF_STR_ShodanNow 0x86f000e		
#define REF_STR_CyberDanger 0x86f000f		
#define REF_STR_RadPoison 0x86f0010		
#define REF_STR_BioPoison 0x86f0011		
#define REF_STR_ExposureUnit 0x86f0012		
#define REF_STR_EnergyCritical 0x86f0013		
#define REF_STR_EnergyUsage 0x86f0014		
#define REF_STR_EnergyUnit 0x86f0015		
#define REF_STR_GametimeLeft 0x86f0016		
#define REF_STR_TargetDamageHit 0x86f0017		
#define REF_STR_TargetDamageBase 0x86f0018		
#define REF_STR_EnviroAbsorb 0x86f0020		
#define REF_STR_EnviroDrain 0x86f0021		
#define RES_lognames 0x870		// (2160)
#define REF_STR_LogName0 0x8700000		
#define REF_STR_LogName1 0x8700001		
#define REF_STR_LogName2 0x8700002		
#define REF_STR_LogName3 0x8700003		
#define REF_STR_LogName4 0x8700004		
#define REF_STR_LogName5 0x8700005		
#define REF_STR_LogName6 0x8700006		
#define REF_STR_LogName7 0x8700007		
#define REF_STR_LogName8 0x8700008		
#define REF_STR_LogName9 0x8700009		
#define REF_STR_LogName10 0x870000a		
#define REF_STR_LogName11 0x870000b		
#define REF_STR_LogName12 0x870000c		
#define REF_STR_LogName13 0x870000d		
#define REF_STR_LogName14 0x870000e		
#define RES_messages 0x871		// (2161)
#define REF_STR_InvCybFailSoft 0x8710000		
#define REF_STR_InvCybFailHard 0x8710001		
#define REF_STR_InvLiveGrenade 0x8710002		
#define REF_STR_ElevatorNoMove 0x8710003		
#define REF_STR_ElevatorMove 0x8710004		
#define REF_STR_ElevatorSameFloor 0x8710005		
#define REF_STR_ElevatorDoorOpen 0x8710006		
#define REF_STR_DoorLocked 0x8710007		
#define REF_STR_DoorLocked2 0x8710008		
#define REF_STR_DoorLocked3 0x8710009		
#define REF_STR_DoorNeedCard 0x871000a		
#define REF_STR_DoorNeedCard2 0x871000b		
#define REF_STR_DoorNeedcard3 0x871000c		
#define REF_STR_DoorWrongAccess 0x871000d		
#define REF_STR_DoorCardGoodButLocked 0x871000e		
#define REF_STR_DoorCardGoodButLocked2 0x871000f		
#define REF_STR_DoorCardGoodButLocked3 0x8710010		
#define REF_STR_DoorCardGood 0x8710011		
#define REF_STR_DoorCardGood2 0x8710012		
#define REF_STR_DoorCardGood3 0x8710013		
#define REF_STR_EmptyContainer 0x8710014		
#define REF_STR_NonemptyContainer 0x8710015		
#define REF_STR_KeypadGood 0x8710016		
#define REF_STR_KeypadBad 0x8710017		
#define REF_STR_PickupTooFar 0x8710018		
#define REF_STR_UseTooFar 0x8710019		
#define REF_STR_ReceiveEmail 0x871001a		
#define REF_STR_ReceiveLog 0x871001b		
#define REF_STR_ReceiveData 0x871001c		
#define REF_STR_AlreadyHaveOne 0x871001d		
#define REF_STR_More 0x871001e		
#define REF_STR_NoRecharge 0x871001f		
#define REF_STR_Recharge 0x8710020		
#define REF_STR_WareNoPower 0x8710021		
#define REF_STR_PowerRanOut 0x8710022		
#define REF_STR_TractorActivate 0x8710023		
#define REF_STR_TractorDeactivate 0x8710024		
#define REF_STR_MedikitUse 0x8710025		
#define REF_STR_Applied 0x8710026		
#define REF_STR_InvalidSave 0x8710027		
#define REF_STR_NoDataReader 0x8710028		
#define REF_STR_AntiGravActivate 0x8710029		
#define REF_STR_AntiGravDeactivate 0x871002a		
#define REF_STR_Pause 0x871002b		
#define REF_STR_Unpause 0x871002c		
#define REF_STR_NoTargetsAround 0x871002d		
#define REF_STR_PlastiqueOn 0x871002e		
#define REF_STR_OpenPanelFirst 0x871002f		
#define REF_STR_EnterSaveString 0x8710030		
#define REF_STR_LoadSlot 0x8710031		
#define REF_STR_SaveSlot 0x8710032		
#define REF_STR_QuitConfirm 0x8710033		
#define REF_STR_CyberspaceUse 0x8710034		
#define REF_STR_NoCyberSave 0x8710035		
#define REF_STR_CursorObjSave 0x8710036		
#define REF_STR_WrongHead 0x8710037		
#define REF_STR_FixtureAccessBad 0x8710038		
#define REF_STR_CspaceAcquire 0x8710039		
#define REF_STR_IceEncrusted 0x871003a		
#define REF_STR_CspaceHeal 0x871003b		
#define REF_STR_CspaceToggle 0x871003c		
#define REF_STR_SHODANFail 0x871003d		
#define REF_STR_AccessCardNoGain 0x8710048		
#define REF_STR_AccessCardNewGain 0x8710049		
#define REF_STR_KeypadForDoor 0x871004a		
#define REF_STR_EPickFailure 0x871004c		
#define REF_STR_SurgeryHeal 0x871004e		
#define REF_STR_HoldingGrenade 0x8710051		
#define REF_STR_WordPage 0x8710052		
#define REF_STR_ViewHelpOffMsg 0x8710053		
#define REF_STR_ViewHelpOnMsg 0x8710056		
#define REF_STR_CspaceMaxHealth 0x8710059		
#define REF_STR_NoEnergyWeapon 0x871005a		
#define REF_STR_GunTooHot 0x871005b		
#define REF_STR_NoEnergyFireWeapon 0x871005c		
#define REF_STR_NormalScreen 0x871005e		
#define REF_STR_WordLiveGrenade 0x871005f		
#define REF_STR_CantUse 0x8710060		
#define REF_STR_PresetSideicon 0x8710061		
#define REF_STR_InkyBlack 0x8710062		
#define REF_STR_InkyUse 0x8710063		
#define REF_STR_DataObj 0x8710064		
#define REF_STR_CspaceData 0x8710065		
#define REF_STR_CybWall 0x8710066		
#define REF_STR_CybWallUse 0x8710067		
#define REF_STR_NotAvailCspace 0x8710068		
#define REF_STR_SaveGameSaving 0x8710069		
#define REF_STR_SaveGameSaved 0x871006a		
#define REF_STR_SaveGameFail 0x871006b		
#define REF_STR_LoadGameLoading 0x871006c		
#define REF_STR_LoadGameLoaded 0x871006d		
#define REF_STR_LoadGameFail 0x871006e		
#define REF_STR_GameInitFail 0x871006f		
#define REF_STR_FSMode 0x8710070		
#define REF_STR_NoMapMessage 0x8710071		
#define REF_STR_DropMessage 0x8710072		
#define RES_intro 0x872		// (2162)
#define RES_death 0x873		// (2163)
#define RES_win 0x874		// (2164)
#define RES_plotware 0x875		// (2165)
#define REF_STR_pwPage0 0x8750000		
#define REF_STR_pwNull 0x8750003		
#define REF_STR_pwShodometer 0x8750004		
#define REF_STR_pwShodoOverall 0x8750005		
#define REF_STR_pwStatus 0x8750006		
#define REF_STR_pwStatus0 0x8750007		
#define REF_STR_pwLifePods 0x875000b		
#define REF_STR_pwEnabled 0x875000c		
#define REF_STR_pwComm 0x875000e		
#define REF_STR_pwComm0 0x875000f		
#define REF_STR_pwCooling 0x8750014		
#define REF_STR_pwCooling0 0x8750015		
#define REF_STR_pwNodes 0x8750018		
#define REF_STR_pwMulti 0x8750019		
#define REF_STR_pwCPUcool 0x875001a		
#define REF_STR_pwCPUcool0 0x875001b		
#define REF_STR_pwMainProgram 0x875001d		
#define REF_STR_pwMainProgram0 0x875001e		
#define REF_STR_pwPropulsion 0x8750023		
#define REF_STR_pwPropulsion0 0x8750024		
#define REF_STR_pwDest 0x8750027		
#define REF_STR_pwDest0 0x8750028		
#define REF_STR_pwLaser 0x875002a		
#define REF_STR_pwLaser0 0x875002b		
#define REF_STR_pwShield 0x875002d		
#define REF_STR_pwShield0 0x875002e		
#define REF_STR_pwTime2Dest 0x8750030		
#define REF_STR_pwDownLoadTime 0x8750031		
#define REF_STR_pwDestructTime 0x8750032		
#define REF_STR_pwBridgeTime 0x8750033		
#define REF_STR_pwVirusTime 0x8750034		
#define REF_STR_pwGroveStatus 0x8750035		
#define REF_STR_pwAlpha 0x8750036		
#define REF_STR_pwAlpha0 0x8750037		
#define REF_STR_pwBeta 0x8750039		
#define REF_STR_pwBeta0 0x875003a		
#define REF_STR_pwGamma 0x875003d		
#define REF_STR_pwGamma0 0x875003e		
#define REF_STR_pwDelta 0x875003f		
#define REF_STR_pwDelta0 0x8750040		
#define RES_objIconNames 0x876		// (2166)
#define REF_STR_IconName0 0x8760000		
#define REF_STR_IconName1 0x8760001		
#define REF_STR_IconName2 0x8760002		
#define REF_STR_IconName3 0x8760003		
#define REF_STR_IconName4 0x8760004		
#define REF_STR_IconName5 0x8760005		
#define REF_STR_IconName6 0x8760006		
#define REF_STR_IconName7 0x8760007		
#define REF_STR_IconName8 0x8760008		
#define REF_STR_IconName9 0x8760009		
#define REF_STR_IconName10 0x876000a		
#define REF_STR_IconName11 0x876000b		
#define REF_STR_IconName12 0x876000c		
#define REF_STR_IconName13 0x876000d		
#define REF_STR_IconName14 0x876000e		
#define REF_STR_IconName15 0x876000f		
#define REF_STR_IconName16 0x8760010		
#define REF_STR_IconName17 0x8760011		
#define RES_screenText 0x877		// (2167)
#define REF_STR_ScreenZero 0x8770000		
#define REF_STR_ScreenOne 0x8770001		
#define REF_STR_ScreenTwo 0x8770002		
#define RES_cyberspaceText 0x878		// (2168)
#define REF_STR_CspaceInfoBase 0x8780000		
#define RES_accessCards 0x879		// (2169)
#define RES_dataletText 0x87a		// (2170)
#define REF_STR_DataletZero 0x87a0000		
#define REF_STR_DataletBinaryZero 0x87a0012		
#define RES_wrapperPanelText 0x87b		// (2171)
#define REF_STR_WrapperText 0x87b0000		
#define REF_STR_MusicText 0x87b0008		
#define REF_STR_OptionsText 0x87b000d		
#define REF_STR_VerifyText 0x87b0013		
#define REF_STR_OffonText 0x87b0015		
#define REF_STR_MouseHand 0x87b0017		
#define REF_STR_TerseText 0x87b0019		
#define REF_STR_GammaCorr 0x87b001b		
#define REF_STR_DetailLvl 0x87b001e		
#define REF_STR_PopupCursFeedback 0x87b0022		
#define REF_STR_HandFeedback 0x87b0024		
#define REF_STR_TerseFeedback 0x87b0026		
#define REF_STR_GammaCorFeedback 0x87b0028		
#define REF_STR_DetailLvlFeedback 0x87b002b		
#define REF_STR_VolumeText 0x87b002f		
#define REF_STR_MusicFeedbackText 0x87b0032		
#define REF_STR_UnusedSave 0x87b003c		
#define REF_STR_ClickForOptions 0x87b003d		
#define REF_STR_DoubleClick 0x87b003e		
#define REF_STR_InputMenu 0x87b003f		
#define REF_STR_CenterJoy 0x87b0040		
#define REF_STR_CenterJoyPrompt 0x87b0041		
#define REF_STR_CenterJoyDone 0x87b0042		
#define REF_STR_OnlineHelp 0x87b0043		
#define REF_STR_Language 0x87b0044		
#define REF_STR_Languages 0x87b0045		
#define REF_STR_KeyEquivs0 0x87b0048		
#define REF_STR_KeyEquivs1 0x87b0049		
#define REF_STR_KeyEquivs2 0x87b004a		
#define REF_STR_InsufficientDisk 0x87b004b		
#define REF_STR_VideoText 0x87b004c		
#define REF_STR_ScreenModeText 0x87b004d		
#define REF_STR_ScreenModeFeedback 0x87b0052		
#define REF_STR_KeyEquivs3 0x87b0054		
#define REF_STR_KeyEquivs4 0x87b0055		
#define REF_STR_KeyEquivs5 0x87b0056		
#define REF_STR_AudiologState 0x87b0057		
#define REF_STR_HeadsetText 0x87b005a		
#define REF_STR_JoystickSens 0x87b005e		
#define REF_STR_AilThreeText 0x87b005f		
#define REF_STR_DigiChannelState 0x87b0062		
#define REF_STR_StereoReverseState 0x87b0065		
#define REF_STR_Joystick 0x87b0067		
#define REF_STR_JoystickType 0x87b0068		
#define REF_STR_JoystickTypes 0x87b0069		
#define REF_STR_KeyEquivs6 0x87b006d		
#define REF_STR_MoreHeadset 0x87b006e		
#define RES_credits 0x87c		// (2172)
#define RES_keyhelp 0x87d		// (2173)
#define RES_miscellaneous 0x87e		// (2174)
#define REF_STR_ResurrectBase 0x87e0000		
#define REF_STR_StartHeartString 0x87e0001		
#define REF_STR_StartBrainString 0x87e0003		
#define REF_STR_AutomapButtons 0x87e0005		
#define REF_STR_AutomapSpew 0x87e000d		
#define REF_STR_NoMessage 0x87e000e		
#define REF_STR_WirePuzzHelp 0x87e000f		
#define REF_STR_GridPuzzHelp 0x87e0011		
#define REF_STR_GridPuzzSideMay 0x87e0012		
#define REF_STR_GridPuzzSide0 0x87e0013		
#define REF_STR_PanelSolved 0x87e0018		
#define REF_STR_DefaultPlayName 0x87e0019		
#define REF_STR_fakewinStrings 0x87e001a		
#define REF_STR_ObjSysBad 0x87e001f		
#define REF_STR_Sleeping 0x87e0020		
#define REF_STR_Drugged 0x87e0021		
#define REF_STR_Stunned 0x87e0022		
#define REF_STR_PhrasePickUp 0x87e0023		
#define REF_STR_PhraseUse 0x87e0024		
#define REF_STR_GroveWord 0x87e0025		
#define REF_STR_ReactorWord 0x87e0026		
#define REF_STR_VersionPrefix 0x87e0027		
#define REF_STR_DirectionAbbrev 0x87e0028		
#define REF_STR_Level 0x87e0030		
#define REF_STR_AutomapMFDButtons 0x87e0031		
#define RES_olh_strings 0x87f		// (2175)
#define REF_STR_helpTake 0x87f0000		
#define REF_STR_helpUse 0x87f0001		
#define REF_STR_helpSearch 0x87f0002		
#define REF_STR_helpDoor 0x87f0003		
#define REF_STR_helpSwitch 0x87f0004		
#define REF_STR_helpPanel 0x87f0005		
#define REF_STR_helpElevator 0x87f0006		
#define REF_STR_helpGump 0x87f0007		
#define REF_STR_helpKeypad 0x87f0008		
#define REF_STR_helpCursor 0x87f0009		
#define REF_STR_helpGrenade 0x87f000a		
#define REF_STR_helpCompound 0x87f000b		
#define REF_STR_helpOn 0x87f000c		
#define REF_STR_helpOff 0x87f000d		
#define REF_STR_helpAttackGun 0x87f000e		
#define REF_STR_helpAttackAuto 0x87f000f		
#define REF_STR_helpAttackHTH 0x87f0010		
#define REF_STR_helpSecurity 0x87f0011		
#define RES_diffscreenText 0x880		// (2176)
#define REF_STR_journeyOpts 0x8800000		
#define REF_STR_diffStart 0x8800004		
#define REF_STR_diffName 0x8800005		
#define REF_STR_diffCategories 0x8800006		
#define REF_STR_diffStrings 0x880000a		
#define REF_STR_BadVersion 0x880001a		
#define RES_endgameStat 0x881		// (2177)
#define RES_itemspew 0x882		// (2178)
#define REF_STR_drugSpew0 0x8820000		
#define REF_STR_wareSpew0 0x8820007		
#define REF_STR_wareSpew1 0x882000c		
#define REF_STR_gearSpew0 0x88200a7		
#define REF_STR_plotSpew0 0x88200ad		
#define RES_games 0x883		// (2179)
#define REF_STR_GamesMenu 0x8830000		
#define REF_STR_DontPlay 0x8830001		
#define REF_STR_NotInstalled 0x8830002		
#define REF_STR_YouHave 0x8830004		
#define REF_STR_ComputerHas 0x8830005		
#define REF_STR_Won 0x8830006		
#define REF_STR_Lost 0x8830007		
#define REF_STR_Scored 0x8830008		
#define REF_STR_You 0x8830009		
#define REF_STR_Computer 0x883000a		
#define REF_STR_ClickToPlay 0x883000b		
#define REF_STR_LevelNum 0x883000c		
#define REF_STR_ShodanHiScore 0x883000d		
#define REF_STR_DiegoHiScore 0x883000e		
#define REF_STR_DepthChargeBonus 0x883000f		
#define REF_STR_GuyBonus 0x8830010		
#define REF_STR_Thinking 0x8830011		
#define REF_STR_YourMove 0x8830012		
#define REF_STR_GameName0 0x8830013		
#define REF_STR_GameDescrip0 0x883001c		
#define REF_STR_WingBriefing 0x8830025		
#define REF_STR_WingDebriefing 0x8830033		
#define REF_STR_WingSighted 0x8830040		
#define REF_STR_WingDies 0x883004d		
#define REF_STR_WingAttack 0x883005a		
#define REF_STR_WingForm 0x8830067		
#define REF_STR_NoWing 0x8830074		
#define REF_STR_WingYouDied 0x8830075		
#define RES_objlongnames 0x24		// (36)
#define REF_STR_MINIPISTOL_LNG 0x240000		
#define REF_STR_DARTPISTOL_LNG 0x240001		
#define REF_STR_MAGNUM_LNG 0x240002		
#define REF_STR_ASSAULTRFL_LNG 0x240003		
#define REF_STR_RIOTGUN_LNG 0x240004		
#define REF_STR_FLECHETTE_LNG 0x240005		
#define REF_STR_SKORPION_LNG 0x240006		
#define REF_STR_MAGPULSE_LNG 0x240007		
#define REF_STR_RAILGUN_LNG 0x240008		
#define REF_STR_BATON_LNG 0x240009		
#define REF_STR_LASERAPIER_LNG 0x24000a		
#define REF_STR_PHASER_LNG 0x24000b		
#define REF_STR_BLASTER_LNG 0x24000c		
#define REF_STR_IONBEAM_LNG 0x24000d		
#define REF_STR_STUNGUN_LNG 0x24000e		
#define REF_STR_PLASMABEAM_LNG 0x24000f		
#define REF_STR_SPAMMO_LNG 0x240010		
#define REF_STR_TEFAMMO_LNG 0x240011		
#define REF_STR_NNAMMO_LNG 0x240012		
#define REF_STR_TNAMMO_LNG 0x240013		
#define REF_STR_HTAMMO_LNG 0x240014		
#define REF_STR_HSAMMO_LNG 0x240015		
#define REF_STR_RBAMMO_LNG 0x240016		
#define REF_STR_MRAMMO_LNG 0x240017		
#define REF_STR_PRAMMO_LNG 0x240018		
#define REF_STR_HNAMMO_LNG 0x240019		
#define REF_STR_SPLAMMO_LNG 0x24001a		
#define REF_STR_SLGAMMO_LNG 0x24001b		
#define REF_STR_BGAMMO_LNG 0x24001c		
#define REF_STR_MAGAMMO_LNG 0x24001d		
#define REF_STR_RGAMMO_LNG 0x24001e		
#define REF_STR_BULLTRACE_LNG 0x24001f		
#define REF_STR_ENERTRACE_LNG 0x240020		
#define REF_STR_AUTOTRACE_LNG 0x240021		
#define REF_STR_NEEDTRACE_LNG 0x240022		
#define REF_STR_GRENTRACE_LNG 0x240023		
#define REF_STR_RUBBTRACE_LNG 0x240024		
#define REF_STR_VIRUSSLOW_LNG 0x240025		
#define REF_STR_ELITESLOW_LNG 0x240026		
#define REF_STR_ASSASINSLOW_LNG 0x240027		
#define REF_STR_MUTANTSLOW_LNG 0x240028		
#define REF_STR_ZEROSLOW_LNG 0x240029		
#define REF_STR_MAGBURST_LNG 0x24002a		
#define REF_STR_RAILSLOW_LNG 0x24002b		
#define REF_STR_STUNSLOW_LNG 0x24002c		
#define REF_STR_PLASMABLST_LNG 0x24002d		
#define REF_STR_CYBERBOLT_LNG 0x24002e		
#define REF_STR_CYBERSLOW_LNG 0x24002f		
#define REF_STR_DRILLSLOW_LNG 0x240030		
#define REF_STR_DISCSLOW_LNG 0x240031		
#define REF_STR_SPEWSLOW_LNG 0x240032		
#define REF_STR_PLANTSLOW_LNG 0x240033		
#define REF_STR_INVISOSLOW_LNG 0x240034		
#define REF_STR_DRONECAM_LNG 0x240035		
#define REF_STR_EXPLCAM_LNG 0x240036		
#define REF_STR_FRAG_G_LNG 0x240037		
#define REF_STR_EMP_G_LNG 0x240038		
#define REF_STR_GAS_G_LNG 0x240039		
#define REF_STR_CONC_G_LNG 0x24003a		
#define REF_STR_L_MINE_LNG 0x24003b		
#define REF_STR_NITRO_G_LNG 0x24003c		
#define REF_STR_EARTH_G_LNG 0x24003d		
#define REF_STR_OBJ_G_LNG 0x24003e		
#define REF_STR_STAMINA_DRUG_LNG 0x24003f		
#define REF_STR_SIGHT_DRUG_LNG 0x240040		
#define REF_STR_LSD_DRUG_LNG 0x240041		
#define REF_STR_MEDI_DRUG_LNG 0x240042		
#define REF_STR_NINJA_DRUG_LNG 0x240043		
#define REF_STR_GENIUS_DRUG_LNG 0x240044		
#define REF_STR_DETOX_DRUG_LNG 0x240045		
#define REF_STR_INFRA_GOG_LNG 0x240046		
#define REF_STR_TARG_GOG_LNG 0x240047		
#define REF_STR_SENS_HARD_LNG 0x240048		
#define REF_STR_AIM_GOG_LNG 0x240049		
#define REF_STR_HUD_GOG_LNG 0x24004a		
#define REF_STR_BIOSCAN_HARD_LNG 0x24004b		
#define REF_STR_NAV_HARD_LNG 0x24004c		
#define REF_STR_SHIELD_HARD_LNG 0x24004d		
#define REF_STR_VIDTEX_HARD_LNG 0x24004e		
#define REF_STR_LANTERN_HARD_LNG 0x24004f		
#define REF_STR_FULLSCR_HARD_LNG 0x240050		
#define REF_STR_ENV_HARD_LNG 0x240051		
#define REF_STR_MOTION_HARD_LNG 0x240052		
#define REF_STR_JET_HARD_LNG 0x240053		
#define REF_STR_STATUS_HARD_LNG 0x240054		
#define REF_STR_DRILL_LNG 0x240055		
#define REF_STR_SPEW_LNG 0x240056		
#define REF_STR_MINE_LNG 0x240057		
#define REF_STR_DISC_LNG 0x240058		
#define REF_STR_PULSER_LNG 0x240059		
#define REF_STR_SCRAMBLER_LNG 0x24005a		
#define REF_STR_VIRUS_LNG 0x24005b		
#define REF_STR_SHIELD_LNG 0x24005c		
#define REF_STR_OLD_FAKEID_LNG 0x24005d		
#define REF_STR_ICE_LNG 0x24005e		
#define REF_STR_TURBO_LNG 0x24005f		
#define REF_STR_FAKEID_LNG 0x240060		
#define REF_STR_DECOY_LNG 0x240061		
#define REF_STR_RECALL_LNG 0x240062		
#define REF_STR_GAMES_LNG 0x240063		
#define REF_STR_MONITOR1_LNG 0x240064		
#define REF_STR_IDENTIFY_LNG 0x240065		
#define REF_STR_TRACE_LNG 0x240066		
#define REF_STR_TOGGLE_LNG 0x240067		
#define REF_STR_TEXT1_LNG 0x240068		
#define REF_STR_EMAIL1_LNG 0x240069		
#define REF_STR_MAP1_LNG 0x24006a		
#define REF_STR_PHONE_LNG 0x24006b		
#define REF_STR_VCR_LNG 0x24006c		
#define REF_STR_MICROWAVE_OVN_LNG 0x24006d		
#define REF_STR_STEREO_LNG 0x24006e		
#define REF_STR_KEYBOARD_LNG 0x24006f		
#define REF_STR_SMALL_CPU_LNG 0x240070		
#define REF_STR_TV_LNG 0x240071		
#define REF_STR_MONITOR2_LNG 0x240072		
#define REF_STR_LARGCPU_LNG 0x240073		
#define REF_STR_LDESK_LNG 0x240074		
#define REF_STR_FDESK_LNG 0x240075		
#define REF_STR_CABINET_LNG 0x240076		
#define REF_STR_SHELF_LNG 0x240077		
#define REF_STR_HIDEAWAY_LNG 0x240078		
#define REF_STR_CHAIR_LNG 0x240079		
#define REF_STR_ENDTABLE_LNG 0x24007a		
#define REF_STR_COUCH_LNG 0x24007b		
#define REF_STR_EXECCHR_LNG 0x24007c		
#define REF_STR_COATTREE_LNG 0x24007d		
#define REF_STR_SIGN_LNG 0x24007e		
#define REF_STR_ICON_LNG 0x24007f		
#define REF_STR_GRAF_LNG 0x240080		
#define REF_STR_WORDS_LNG 0x240081		
#define REF_STR_PAINTING_LNG 0x240082		
#define REF_STR_POSTER_LNG 0x240083		
#define REF_STR_SCREEN_LNG 0x240084		
#define REF_STR_TMAP_LNG 0x240085		
#define REF_STR_SUPERSCREEN_LNG 0x240086		
#define REF_STR_BIGSCREEN_LNG 0x240087		
#define REF_STR_REPULSWALL_LNG 0x240088		
#define REF_STR_DESKLAMP_LNG 0x240089		
#define REF_STR_FLOORLAMP_LNG 0x24008a		
#define REF_STR_GLOWBULB_LNG 0x24008b		
#define REF_STR_CHAND_LNG 0x24008c		
#define REF_STR_GENE_SPLICER_LNG 0x24008d		
#define REF_STR_TUBING_LNG 0x24008e		
#define REF_STR_MED_CART_LNG 0x24008f		
#define REF_STR_SURG_MACH_LNG 0x240090		
#define REF_STR_TTUBE_RACK_LNG 0x240091		
#define REF_STR_RSRCH_CHAIR_LNG 0x240092		
#define REF_STR_HOSP_BED_LNG 0x240093		
#define REF_STR_BROKLAB1_LNG 0x240094		
#define REF_STR_BROKLAB2_LNG 0x240095		
#define REF_STR_MICROSCOPE_LNG 0x240096		
#define REF_STR_SCOPE_LNG 0x240097		
#define REF_STR_LAB_PROBE_LNG 0x240098		
#define REF_STR_XRAY_MACHINE_LNG 0x240099		
#define REF_STR_CAMERA_LNG 0x24009a		
#define REF_STR_CONTPAN_LNG 0x24009b		
#define REF_STR_CONTPED_LNG 0x24009c		
#define REF_STR_ENERGY_MINE_LNG 0x24009d		
#define REF_STR_STATUE1_LNG 0x24009e		
#define REF_STR_SHRUB1_LNG 0x24009f		
#define REF_STR_GRASS_LNG 0x2400a0		
#define REF_STR_PLANT1_LNG 0x2400a1		
#define REF_STR_FUNG1_LNG 0x2400a2		
#define REF_STR_FUNG2_LNG 0x2400a3		
#define REF_STR_PLANT2_LNG 0x2400a4		
#define REF_STR_VINE1_LNG 0x2400a5		
#define REF_STR_VINE2_LNG 0x2400a6		
#define REF_STR_PLANT3_LNG 0x2400a7		
#define REF_STR_PLANT4_LNG 0x2400a8		
#define REF_STR_LBOULDER_LNG 0x2400a9		
#define REF_STR_BBOULDER_LNG 0x2400aa		
#define REF_STR_SHRUB2_LNG 0x2400ab		
#define REF_STR_VSHRUB1_LNG 0x2400ac		
#define REF_STR_VSHRUB2_LNG 0x2400ad		
#define REF_STR_BRIDGE_LNG 0x2400ae		
#define REF_STR_CATWALK_LNG 0x2400af		
#define REF_STR_WALL_LNG 0x2400b0		
#define REF_STR_FPILLAR_LNG 0x2400b1		
#define REF_STR_RAILING1_LNG 0x2400b2		
#define REF_STR_RAILING2_LNG 0x2400b3		
#define REF_STR_PILLAR_LNG 0x2400b4		
#define REF_STR_FORCE_BRIJ_LNG 0x2400b5		
#define REF_STR_NON_BRIDGE_LNG 0x2400b6		
#define REF_STR_FORCE_BRIJ2_LNG 0x2400b7		
#define REF_STR_BEV_CONT_LNG 0x2400b8		
#define REF_STR_WRAPPER_LNG 0x2400b9		
#define REF_STR_PAPERS_LNG 0x2400ba		
#define REF_STR_WARECASING_LNG 0x2400bb		
#define REF_STR_EXTING_LNG 0x2400bc		
#define REF_STR_HELMET_LNG 0x2400bd		
#define REF_STR_CLOTHES_LNG 0x2400be		
#define REF_STR_BRIEFCASE_LNG 0x2400bf		
#define REF_STR_BROKEN_GUN_LNG 0x2400c0		
#define REF_STR_MCHUNK1_LNG 0x2400c1		
#define REF_STR_MCHUNK2_LNG 0x2400c2		
#define REF_STR_MCHUNK3_LNG 0x2400c3		
#define REF_STR_CRATE_FRAG_LNG 0x2400c4		
#define REF_STR_BROKEN_PAN_LNG 0x2400c5		
#define REF_STR_BROKEN_CLK_LNG 0x2400c6		
#define REF_STR_MSCRAP_LNG 0x2400c7		
#define REF_STR_BROKEN_LEV1_LNG 0x2400c8		
#define REF_STR_BROKEN_LEV2_LNG 0x2400c9		
#define REF_STR_CORPSE1_LNG 0x2400ca		
#define REF_STR_CORPSE2_LNG 0x2400cb		
#define REF_STR_CORPSE3_LNG 0x2400cc		
#define REF_STR_CORPSE4_LNG 0x2400cd		
#define REF_STR_CORPSE5_LNG 0x2400ce		
#define REF_STR_CORPSE6_LNG 0x2400cf		
#define REF_STR_CORPSE7_LNG 0x2400d0		
#define REF_STR_CORPSE8_LNG 0x2400d1		
#define REF_STR_SKEL_RAGS_LNG 0x2400d2		
#define REF_STR_BONES1_LNG 0x2400d3		
#define REF_STR_BONES2_LNG 0x2400d4		
#define REF_STR_SKULL_LNG 0x2400d5		
#define REF_STR_LIMB_LNG 0x2400d6		
#define REF_STR_HEAD_LNG 0x2400d7		
#define REF_STR_HEAD2_LNG 0x2400d8		
#define REF_STR_EPICK_LNG 0x2400d9		
#define REF_STR_BATTERY2_LNG 0x2400da		
#define REF_STR_ROD_LNG 0x2400db		
#define REF_STR_AIDKIT_LNG 0x2400dc		
#define REF_STR_TRACBEAM_LNG 0x2400dd		
#define REF_STR_BATTERY_LNG 0x2400de		
#define REF_STR_GENCARDS_LNG 0x2400df		
#define REF_STR_STDCARD_LNG 0x2400e0		
#define REF_STR_SCICARD_LNG 0x2400e1		
#define REF_STR_STORECARD_LNG 0x2400e2		
#define REF_STR_ENGCARD_LNG 0x2400e3		
#define REF_STR_MEDCARD_LNG 0x2400e4		
#define REF_STR_MAINTCARD_LNG 0x2400e5		
#define REF_STR_ADMINCARD_LNG 0x2400e6		
#define REF_STR_SECCARD_LNG 0x2400e7		
#define REF_STR_COMCARD_LNG 0x2400e8		
#define REF_STR_GROUPCARD_LNG 0x2400e9		
#define REF_STR_PERSCARD_LNG 0x2400ea		
#define REF_STR_MULTIPLEXR_LNG 0x2400eb		
#define REF_STR_CYBERHEAL_LNG 0x2400ec		
#define REF_STR_CYBERMINE_LNG 0x2400ed		
#define REF_STR_CYBERCARD_LNG 0x2400ee		
#define REF_STR_SHODO_SHRINE_LNG 0x2400ef		
#define REF_STR_ICEWALL_LNG 0x2400f0		
#define REF_STR_INFONODE_LNG 0x2400f1		
#define REF_STR_CSPACE_EXIT_LNG 0x2400f2		
#define REF_STR_DATALET_LNG 0x2400f3		
#define REF_STR_BARRICADE_LNG 0x2400f4		
#define REF_STR_TARGET_LNG 0x2400f5		
#define REF_STR_ARROW_LNG 0x2400f6		
#define REF_STR_BEAMBLST_LNG 0x2400f7		
#define REF_STR_ACIDCORR_LNG 0x2400f8		
#define REF_STR_BULLETHOLE_LNG 0x2400f9		
#define REF_STR_EXBLAST_LNG 0x2400fa		
#define REF_STR_BURNRES_LNG 0x2400fb		
#define REF_STR_BLOODSTN_LNG 0x2400fc		
#define REF_STR_CHEMSPLAT_LNG 0x2400fd		
#define REF_STR_OILPUDDLE_LNG 0x2400fe		
#define REF_STR_WASTESPILL_LNG 0x2400ff		
#define REF_STR_ISOTOPE_X_LNG 0x240100		
#define REF_STR_CIRCBOARD1_LNG 0x240101		
#define REF_STR_PLASTIQUE_LNG 0x240102		
#define REF_STR_FAUX_X_LNG 0x240103		
#define REF_STR_CIRCBOARD4_LNG 0x240104		
#define REF_STR_CIRCBOARD5_LNG 0x240105		
#define REF_STR_CIRCBOARD6_LNG 0x240106		
#define REF_STR_CIRCBOARD7_LNG 0x240107		
#define REF_STR_SWITCH1_LNG 0x240108		
#define REF_STR_SWITCH2_LNG 0x240109		
#define REF_STR_BUTTON1_LNG 0x24010a		
#define REF_STR_BUTTON2_LNG 0x24010b		
#define REF_STR_LEVER1_LNG 0x24010c		
#define REF_STR_LEVER2_LNG 0x24010d		
#define REF_STR_BIGRED_LNG 0x24010e		
#define REF_STR_BIGLEVER_LNG 0x24010f		
#define REF_STR_DIAL_LNG 0x240110		
#define REF_STR_ACCESS_SLOT_LNG 0x240111		
#define REF_STR_CRCT_BD_SLOT_LNG 0x240112		
#define REF_STR_CHEM_RECEPT_LNG 0x240113		
#define REF_STR_ANTENNA_PAN_LNG 0x240114		
#define REF_STR_PLAS_ANTENNA_LNG 0x240115		
#define REF_STR_DEST_ANTENNA_LNG 0x240116		
#define REF_STR_RETSCANNER_LNG 0x240117		
#define REF_STR_CYB_TERM_LNG 0x240118		
#define REF_STR_ENRG_CHARGE_LNG 0x240119		
#define REF_STR_FIXUP_STATION_LNG 0x24011a		
#define REF_STR_ACCPANEL1_LNG 0x24011b		
#define REF_STR_ACCPANEL2_LNG 0x24011c		
#define REF_STR_ACCPANEL3_LNG 0x24011d		
#define REF_STR_ACCPANEL4_LNG 0x24011e		
#define REF_STR_ELEPANEL1_LNG 0x24011f		
#define REF_STR_ELEPANEL2_LNG 0x240120		
#define REF_STR_ELEPANEL3_LNG 0x240121		
#define REF_STR_KEYPAD1_LNG 0x240122		
#define REF_STR_KEYPAD2_LNG 0x240123		
#define REF_STR_ACCPANEL5_LNG 0x240124		
#define REF_STR_ACCPANEL6_LNG 0x240125		
#define REF_STR_AMMOVEND_LNG 0x240126		
#define REF_STR_HEALVEND_LNG 0x240127		
#define REF_STR_CYBERTOG1_LNG 0x240128		
#define REF_STR_CYBERTOG2_LNG 0x240129		
#define REF_STR_CYBERTOG3_LNG 0x24012a		
#define REF_STR_BLAST_DOOR_LNG 0x24012b		
#define REF_STR_ACCESS_DOOR_LNG 0x24012c		
#define REF_STR_RESID_DOOR_LNG 0x24012d		
#define REF_STR_MAINT_DOOR_LNG 0x24012e		
#define REF_STR_HOSP_DOOR_LNG 0x24012f		
#define REF_STR_LAB_DOOR_LNG 0x240130		
#define REF_STR_STOR_DOOR_LNG 0x240131		
#define REF_STR_REACTR_DOOR_LNG 0x240132		
#define REF_STR_EXEC_DOOR_LNG 0x240133		
#define REF_STR_NO_DOOR_LNG 0x240134		
#define REF_STR_LAB_DOORWAY_LNG 0x240135		
#define REF_STR_RES_DOORWAY_LNG 0x240136		
#define REF_STR_BRJ_DOORWAY_LNG 0x240137		
#define REF_STR_RCT_DOORWAY_LNG 0x240138		
#define REF_STR_GRATING1_LNG 0x240139		
#define REF_STR_GRATING2_LNG 0x24013a		
#define REF_STR_GRATING3_LNG 0x24013b		
#define REF_STR_GRATING4_LNG 0x24013c		
#define REF_STR_NO_DOOR2_LNG 0x24013d		
#define REF_STR_LABFORCE_LNG 0x24013e		
#define REF_STR_BROKLABFORCE_LNG 0x24013f		
#define REF_STR_RESFORCE_LNG 0x240140		
#define REF_STR_BROKRESFORCE_LNG 0x240141		
#define REF_STR_GENFORCE_LNG 0x240142		
#define REF_STR_CYBGENFORCE_LNG 0x240143		
#define REF_STR_NO_DOOR3_LNG 0x240144		
#define REF_STR_EXEC_ELEV_LNG 0x240145		
#define REF_STR_REG_ELEV1_LNG 0x240146		
#define REF_STR_REG_ELEV2_LNG 0x240147		
#define REF_STR_FREIGHT_ELEV_LNG 0x240148		
#define REF_STR_NO_DOOR4_LNG 0x240149		
#define REF_STR_DOUB_LEFTDOOR_LNG 0x24014a		
#define REF_STR_DOUB_RITEDOOR_LNG 0x24014b		
#define REF_STR_IRIS_LNG 0x24014c		
#define REF_STR_VERT_OPEN_LNG 0x24014d		
#define REF_STR_VERT_SPLIT_LNG 0x24014e		
#define REF_STR_NO_DOOR5_LNG 0x24014f		
#define REF_STR_SECRET_DOOR1_LNG 0x240150		
#define REF_STR_SECRET_DOOR2_LNG 0x240151		
#define REF_STR_SECRET_DOOR3_LNG 0x240152		
#define REF_STR_INVISO_DOOR_LNG 0x240153		
#define REF_STR_ALERT_PANEL_OFF_LNG 0x240154		
#define REF_STR_ALERT_PANEL_ON_LNG 0x240155		
#define REF_STR_HORZ_KLAXOFF_LNG 0x240156		
#define REF_STR_HORZ_KLAXON_LNG 0x240157		
#define REF_STR_SPARK_CABLE_LNG 0x240158		
#define REF_STR_TWITCH_MUT2_LNG 0x240159		
#define REF_STR_MACHINE_LNG 0x24015a		
#define REF_STR_HOLOG_ANIM_LNG 0x24015b		
#define REF_STR_TWITCH_MUT_LNG 0x24015c		
#define REF_STR_BLOOD1_LNG 0x24015d		
#define REF_STR_CAMEXPL_LNG 0x24015e		
#define REF_STR_TVEXPL_LNG 0x24015f		
#define REF_STR_SIMPLSMOKE_LNG 0x240160		
#define REF_STR_PLANTEXPL_LNG 0x240161		
#define REF_STR_BULLETWALLHIT_LNG 0x240162		
#define REF_STR_BEAMWALLHIT_LNG 0x240163		
#define REF_STR_IMPACT_ANIM_LNG 0x240164		
#define REF_STR_BULL_ROBOT_LNG 0x240165		
#define REF_STR_BEAM_ROBOT1_LNG 0x240166		
#define REF_STR_BEAM_ROBOT2_LNG 0x240167		
#define REF_STR_EXPLOSION1_LNG 0x240168		
#define REF_STR_EXPLOSION2_LNG 0x240169		
#define REF_STR_EXPLOSION3_LNG 0x24016a		
#define REF_STR_LG_EXPLOSION_LNG 0x24016b		
#define REF_STR_MAGPULSEHIT_LNG 0x24016c		
#define REF_STR_STUNHIT_LNG 0x24016d		
#define REF_STR_PLASMAHIT_LNG 0x24016e		
#define REF_STR_SMOKEEXPL_LNG 0x24016f		
#define REF_STR_CRATEEXPL_LNG 0x240170		
#define REF_STR_MNTR2EXPL_LNG 0x240171		
#define REF_STR_GASEXPL_LNG 0x240172		
#define REF_STR_EMPEXPL_LNG 0x240173		
#define REF_STR_CORP_HUM_EXPL_LNG 0x240174		
#define REF_STR_CORP_ROB_EXPL_LNG 0x240175		
#define REF_STR_ENTRY_TRIG_LNG 0x240176		
#define REF_STR_NULL_TRIG_LNG 0x240177		
#define REF_STR_FLOOR_TRIG_LNG 0x240178		
#define REF_STR_PLRDETH_TRIG_LNG 0x240179		
#define REF_STR_DETHWATCH_TRIG_LNG 0x24017a		
#define REF_STR_AOE_ENT_TRIG_LNG 0x24017b		
#define REF_STR_AOE_CON_TRIG_LNG 0x24017c		
#define REF_STR_AI_HINT_LNG 0x24017d		
#define REF_STR_LEVEL_TRIG_LNG 0x24017e		
#define REF_STR_CONTIN_TRIG_LNG 0x24017f		
#define REF_STR_REPULSOR_LNG 0x240180		
#define REF_STR_ECOLOGY_TRIG_LNG 0x240181		
#define REF_STR_SHODO_TRIG_LNG 0x240182		
#define REF_STR_TRIPBEAM_LNG 0x240183		
#define REF_STR_BIOHAZARD_LNG 0x240184		
#define REF_STR_RADHAZARD_LNG 0x240185		
#define REF_STR_CHEMHAZARD_LNG 0x240186		
#define REF_STR_MAPNOTE_LNG 0x240187		
#define REF_STR_MUSIC_MARK_LNG 0x240188		
#define REF_STR_SML_CRT_LNG 0x240189		
#define REF_STR_LG_CRT_LNG 0x24018a		
#define REF_STR_SECURE_CONTR_LNG 0x24018b		
#define REF_STR_RAD_BARREL_LNG 0x24018c		
#define REF_STR_TOXIC_BARREL_LNG 0x24018d		
#define REF_STR_CHEM_TANK_LNG 0x24018e		
#define REF_STR_THERMOS_LNG 0x24018f		
#define REF_STR_VIAL_CONT_LNG 0x240190		
#define REF_STR_FLASK_CONT_LNG 0x240191		
#define REF_STR_BEAKER_CONT_LNG 0x240192		
#define REF_STR_MUT_CORPSE1_LNG 0x240193		
#define REF_STR_MUT_CORPSE2_LNG 0x240194		
#define REF_STR_MUT_CORPSE3_LNG 0x240195		
#define REF_STR_MUT_CORPSE4_LNG 0x240196		
#define REF_STR_MUT_CORPSE5_LNG 0x240197		
#define REF_STR_MUT_CORPSE6_LNG 0x240198		
#define REF_STR_MUT_CORPSE7_LNG 0x240199		
#define REF_STR_MUT_CORPSE8_LNG 0x24019a		
#define REF_STR_ROB_CORPSE1_LNG 0x24019b		
#define REF_STR_ROB_CORPSE2_LNG 0x24019c		
#define REF_STR_ROB_CORPSE3_LNG 0x24019d		
#define REF_STR_ROB_CORPSE4_LNG 0x24019e		
#define REF_STR_ROB_CORPSE5_LNG 0x24019f		
#define REF_STR_ROB_CORPSE6_LNG 0x2401a0		
#define REF_STR_ROB_CORPSE7_LNG 0x2401a1		
#define REF_STR_ROB_CORPSE8_LNG 0x2401a2		
#define REF_STR_ROB_CORPSE9_LNG 0x2401a3		
#define REF_STR_ROB_CORPSE10_LNG 0x2401a4		
#define REF_STR_ROB_CORPSE11_LNG 0x2401a5		
#define REF_STR_ROB_CORPSE12_LNG 0x2401a6		
#define REF_STR_ROB_CORPSE13_LNG 0x2401a7		
#define REF_STR_CYB_CORPSE1_LNG 0x2401a8		
#define REF_STR_CYB_CORPSE2_LNG 0x2401a9		
#define REF_STR_CYB_CORPSE3_LNG 0x2401aa		
#define REF_STR_CYB_CORPSE4_LNG 0x2401ab		
#define REF_STR_CYB_CORPSE5_LNG 0x2401ac		
#define REF_STR_CYB_CORPSE6_LNG 0x2401ad		
#define REF_STR_CYB_CORPSE7_LNG 0x2401ae		
#define REF_STR_OTH_CORPSE1_LNG 0x2401af		
#define REF_STR_OTH_CORPSE2_LNG 0x2401b0		
#define REF_STR_OTH_CORPSE3_LNG 0x2401b1		
#define REF_STR_OTH_CORPSE4_LNG 0x2401b2		
#define REF_STR_OTH_CORPSE5_LNG 0x2401b3		
#define REF_STR_OTH_CORPSE6_LNG 0x2401b4		
#define REF_STR_OTH_CORPSE7_LNG 0x2401b5		
#define REF_STR_OTH_CORPSE8_LNG 0x2401b6		
#define REF_STR_HUMAN_CRIT_LNG 0x2401b7		
#define REF_STR_GOR_TIGER_LNG 0x2401b8		
#define REF_STR_INSECT_CRIT_LNG 0x2401b9		
#define REF_STR_AVIAN_CRIT_LNG 0x2401ba		
#define REF_STR_PLANT_CRIT_LNG 0x2401bb		
#define REF_STR_ZERO_CRIT_LNG 0x2401bc		
#define REF_STR_PLAYER_CRIT_LNG 0x2401bd		
#define REF_STR_INVISO_CRIT_LNG 0x2401be		
#define REF_STR_VIRUS_CRIT_LNG 0x2401bf		
#define REF_STR_LIFT_BOT_LNG 0x2401c0		
#define REF_STR_REPAIRBOT_LNG 0x2401c1		
#define REF_STR_SERVBOT_LNG 0x2401c2		
#define REF_STR_EXECBOT_LNG 0x2401c3		
#define REF_STR_LGTURRET_LNG 0x2401c4		
#define REF_STR_HOPPER_LNG 0x2401c5		
#define REF_STR_SECURITY_BOT1_LNG 0x2401c6		
#define REF_STR_SECURITY_BOT2_LNG 0x2401c7		
#define REF_STR_AUTOBOMB_LNG 0x2401c8		
#define REF_STR_REPAIRBOT2_LNG 0x2401c9		
#define REF_STR_FLIER_LNG 0x2401ca		
#define REF_STR_SECURITY_BOT3_LNG 0x2401cb		
#define REF_STR_CYBORG_DRONE_LNG 0x2401cc		
#define REF_STR_WARRIOR_LNG 0x2401cd		
#define REF_STR_ASSASSIN_LNG 0x2401ce		
#define REF_STR_CYBERBABE_LNG 0x2401cf		
#define REF_STR_ELITE_GUARD_LNG 0x2401d0		
#define REF_STR_CORTEX_REAVER_LNG 0x2401d1		
#define REF_STR_MUTANT_BORG_LNG 0x2401d2		
#define REF_STR_CYBERDOG_LNG 0x2401d3		
#define REF_STR_CYBERGUARD_LNG 0x2401d4		
#define REF_STR_CYBER_CORTEX_LNG 0x2401d5		
#define REF_STR_CYBER_DYN_ICE_LNG 0x2401d6		
#define REF_STR_CYBER_HNT_KIL_LNG 0x2401d7		
#define REF_STR_CYBER_SHODAN_LNG 0x2401d8		
#define REF_STR_CYBERGUARD2_LNG 0x2401d9		
#define REF_STR_ROBOBABE_LNG 0x2401da		
#define REF_STR_DIEGO_LNG 0x2401db		
#define RES_paper0 0x3c		// (60)
#define RES_paper1 0x3d		// (61)
#define RES_paper2 0x3e		// (62)
#define RES_paper3 0x3f		// (63)
#define RES_paper4 0x40		// (64)
#define RES_paper5 0x41		// (65)
#define RES_paper6 0x42		// (66)
#define RES_paper7 0x43		// (67)
#define RES_paper8 0x44		// (68)
#define RES_paper9 0x45		// (69)
#define RES_paper10 0x46		// (70)
#define RES_email0 0x989		// (2441)
#define RES_email1 0x98a		// (2442)
#define RES_email2 0x98b		// (2443)
#define RES_email3 0x98c		// (2444)
#define RES_email4 0x98d		// (2445)
#define RES_email5 0x98e		// (2446)
#define RES_email6 0x98f		// (2447)
#define RES_email7 0x990		// (2448)
#define RES_email8 0x991		// (2449)
#define RES_email9 0x992		// (2450)
#define RES_email10 0x993		// (2451)
#define RES_email11 0x994		// (2452)
#define RES_email12 0x995		// (2453)
#define RES_email13 0x996		// (2454)
#define RES_email14 0x997		// (2455)
#define RES_email15 0x998		// (2456)
#define RES_email16 0x999		// (2457)
#define RES_email17 0x99a		// (2458)
#define RES_email18 0x99b		// (2459)
#define RES_email19 0x99c		// (2460)
#define RES_email20 0x99d		// (2461)
#define RES_email21 0x99e		// (2462)
#define RES_email22 0x99f		// (2463)
#define RES_email23 0x9a0		// (2464)
#define RES_email24 0x9a1		// (2465)
#define RES_email25 0x9a2		// (2466)
#define RES_email26 0x9a3		// (2467)
#define RES_email27 0x9a4		// (2468)
#define RES_email28 0x9a5		// (2469)
#define RES_email29 0x9a6		// (2470)
#define RES_email30 0x9a7		// (2471)
#define RES_email31 0x9a8		// (2472)
#define RES_email32 0x9a9		// (2473)
#define RES_email33 0x9aa		// (2474)
#define RES_email34 0x9ab		// (2475)
#define RES_email35 0x9ac		// (2476)
#define RES_email36 0x9ad		// (2477)
#define RES_email37 0x9ae		// (2478)
#define RES_email38 0x9af		// (2479)
#define RES_email39 0x9b0		// (2480)
#define RES_email40 0x9b1		// (2481)
#define RES_email41 0x9b2		// (2482)
#define RES_email42 0x9b3		// (2483)
#define RES_email43 0x9b4		// (2484)
#define RES_email44 0x9b5		// (2485)
#define RES_email45 0x9b6		// (2486)
#define RES_log00 0x9b8		// (2488)
#define RES_log01 0x9b9		// (2489)
#define RES_log02 0x9ba		// (2490)
#define RES_log03 0x9bb		// (2491)
#define RES_log04 0x9bc		// (2492)
#define RES_log05 0x9bd		// (2493)
#define RES_log10 0x9c8		// (2504)
#define RES_log11 0x9c9		// (2505)
#define RES_log12 0x9ca		// (2506)
#define RES_log13 0x9cb		// (2507)
#define RES_log14 0x9cc		// (2508)
#define RES_log15 0x9cd		// (2509)
#define RES_log16 0x9ce		// (2510)
#define RES_log17 0x9cf		// (2511)
#define RES_log18 0x9d0		// (2512)
#define RES_log19 0x9d1		// (2513)
#define RES_log110 0x9d2		// (2514)
#define RES_log111 0x9d3		// (2515)
#define RES_log112 0x9d4		// (2516)
#define RES_log113 0x9d5		// (2517)
#define RES_log114 0x9d6		// (2518)
#define RES_log115 0x9d7		// (2519)
#define RES_log20 0x9d8		// (2520)
#define RES_log21 0x9d9		// (2521)
#define RES_log22 0x9da		// (2522)
#define RES_log23 0x9db		// (2523)
#define RES_log24 0x9dc		// (2524)
#define RES_log25 0x9dd		// (2525)
#define RES_log26 0x9de		// (2526)
#define RES_log27 0x9df		// (2527)
#define RES_log28 0x9e0		// (2528)
#define RES_log29 0x9e1		// (2529)
#define RES_log210 0x9e2		// (2530)
#define RES_log211 0x9e3		// (2531)
#define RES_log212 0x9e4		// (2532)
#define RES_log30 0x9e8		// (2536)
#define RES_log31 0x9e9		// (2537)
#define RES_log32 0x9ea		// (2538)
#define RES_log33 0x9eb		// (2539)
#define RES_log34 0x9ec		// (2540)
#define RES_log35 0x9ed		// (2541)
#define RES_log36 0x9ee		// (2542)
#define RES_log37 0x9ef		// (2543)
#define RES_log38 0x9f0		// (2544)
#define RES_log40 0x9f8		// (2552)
#define RES_log41 0x9f9		// (2553)
#define RES_log42 0x9fa		// (2554)
#define RES_log43 0x9fb		// (2555)
#define RES_log44 0x9fc		// (2556)
#define RES_log45 0x9fd		// (2557)
#define RES_log46 0x9fe		// (2558)
#define RES_log50 0xa08		// (2568)
#define RES_log51 0xa09		// (2569)
#define RES_log52 0xa0a		// (2570)
#define RES_log53 0xa0b		// (2571)
#define RES_log54 0xa0c		// (2572)
#define RES_log55 0xa0d		// (2573)
#define RES_log56 0xa0e		// (2574)
#define RES_log57 0xa0f		// (2575)
#define RES_log58 0xa10		// (2576)
#define RES_log59 0xa11		// (2577)
#define RES_log510 0xa12		// (2578)
#define RES_log511 0xa13		// (2579)
#define RES_log60 0xa18		// (2584)
#define RES_log61 0xa19		// (2585)
#define RES_log62 0xa1a		// (2586)
#define RES_log63 0xa1b		// (2587)
#define RES_log64 0xa1c		// (2588)
#define RES_log65 0xa1d		// (2589)
#define RES_log66 0xa1e		// (2590)
#define RES_log67 0xa1f		// (2591)
#define RES_log68 0xa20		// (2592)
#define RES_log69 0xa21		// (2593)
#define RES_log70 0xa28		// (2600)
#define RES_log71 0xa29		// (2601)
#define RES_log72 0xa2a		// (2602)
#define RES_log73 0xa2b		// (2603)
#define RES_log74 0xa2c		// (2604)
#define RES_log75 0xa2d		// (2605)
#define RES_log76 0xa2e		// (2606)
#define RES_log77 0xa2f		// (2607)
#define RES_log78 0xa30		// (2608)
#define RES_log80 0xa38		// (2616)
#define RES_log81 0xa39		// (2617)
#define RES_log82 0xa3a		// (2618)
#define RES_log83 0xa3b		// (2619)
#define RES_log84 0xa3c		// (2620)
#define RES_log85 0xa3d		// (2621)
#define RES_data0 0xa98		// (2712)
#define RES_data1 0xa99		// (2713)
#define RES_data2 0xa9a		// (2714)
#define RES_data3 0xa9b		// (2715)
#define RES_data4 0xa9c		// (2716)
#define RES_data5 0xa9d		// (2717)
#define RES_data6 0xa9e		// (2718)
#define RES_data7 0xa9f		// (2719)
#define RES_data8 0xaa0		// (2720)
#define RES_data9 0xaa1		// (2721)
#define RES_data10 0xaa2		// (2722)
#define RES_data11 0xaa3		// (2723)
#define RES_data12 0xaa4		// (2724)

#endif
