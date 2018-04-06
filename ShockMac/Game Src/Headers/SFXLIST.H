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
#ifndef __SFXLIST_H
#define __SFXLIST_H

#ifdef DEMO
#define SFX_NONE              -1

// Doors
#define SFX_DOOR_METAL        3
#define SFX_DOOR_NORMAL       5
#define SFX_DOOR_IRIS         6
#define SFX_DOOR_BULKHEAD     67
#define SFX_DOOR_GRATING      90

// Ambient     
#define SFX_BRIDGE_1          -1
#define SFX_BRIDGE_2          -1
#define SFX_BRIDGE_3          -1
#define SFX_BRIDGE_4          -1
#define SFX_MAINT_1           -1
#define SFX_MAINT_2           -1
#define SFX_MAINT_3           -1
#define SFX_MAINT_4           -1
#define SFX_GROVE_1           -1

// Wacky Objects
#define SFX_REPULSOR          -1
#define SFX_FORCE_BRIDGE      72
#define SFX_TERRAIN_ELEV_LOOP       15  
#define SFX_SPARKING_CABLE          -1  // not done?
#define SFX_SURGERY_MACHINE         102

// Combat
#define SFX_GUN_MINIPISTOL    39
#define SFX_GUN_DARTPISTOL    86
#define SFX_GUN_MAGNUM        40
#define SFX_GUN_ASSAULT       -1
#define SFX_GUN_RIOT          -1
#define SFX_GUN_FLECHETTE     -1
#define SFX_GUN_SKORPION      -1
#define SFX_GUN_MAGPULSE      45
#define SFX_GUN_RAILGUN       -1
#define SFX_GUN_PIPE_HIT_MEAT  4 
#define SFX_GUN_PIPE_HIT_METAL 21 
#define SFX_GUN_PIPE_MISS      24 
#define SFX_GUN_LASEREPEE_HIT  -1
#define SFX_GUN_LASEREPEE_MISS -1
#define SFX_GUN_PHASER        18
#define SFX_GUN_BLASTER       -1
#define SFX_GUN_IONBEAM       -1
#define SFX_GUN_STUNGUN       19
#define SFX_GUN_PLASMA        -1

#define SFX_PLAYER_HURT       64  
#define SFX_SHIELD_1          -1
#define SFX_SHIELD_2          -1
#define SFX_SHIELD_UP         -1
#define SFX_SHIELD_DOWN       -1
#define SFX_METAL_SPANG       -1 // robot hit?   not in
#define SFX_RADIATION         SFX_STATIC        

#define SFX_RELOAD_1          22 
#define SFX_RELOAD_2          23
#define SFX_GRENADE_ARM       7     
#define SFX_BATTERY_USE       28

#define SFX_EXPLOSION_1       44
#define SFX_RUMBLE            -1
#define SFX_TELEPORT          -1

#define SFX_MONITOR_EXPLODE   57
#define SFX_CAMERA_EXPLODE    8
#define SFX_CPU_EXPLODE       10
#define SFX_DESTROY_CRATE     37
#define SFX_DESTROY_BARREL    109

// Cspace
#define SFX_PULSER            -1
#define SFX_DRILL             -1
#define SFX_DISC              -1
#define SFX_DATASTORM         -1

#define SFX_RECALL            -1
#define SFX_TURBO             -1
#define SFX_FAKEID            -1
#define SFX_DECOY             -1

#define SFX_ENTER_CSPACE      27
#define SFX_OTTO_SHODAN       -1
#define SFX_CYBER_DAMAGE      -1
#define SFX_CYBERHEAL         -1
#define SFX_CYBERTOGGLE       -1
#define SFX_ICE_DEFENSE       -1

#define SFX_CYBER_ATTACK_1    -1
#define SFX_CYBER_ATTACK_2    -1
#define SFX_CYBER_ATTACK_3    -1

// MFD and UI Wackiness
#define SFX_VIDEO_DOWN        33
#define SFX_MFD_BUTTON        35
#define SFX_INVENT_BUTTON     79
#define SFX_INVENT_SELECT     80
#define SFX_INVENT_ADD        81
#define SFX_INVENT_WARE       82
#define SFX_PATCH_USE         83
#define SFX_ZOOM_BOX          84
#define SFX_MAP_ZOOM          85
#define SFX_MFD_KEYPAD        76
#define SFX_MFD_BUZZ          77
#define SFX_MFD_SUCCESS       78
#define SFX_GOGGLE            0
#define SFX_HUDFROB           1
#define SFX_STATIC            2
#define SFX_EMAIL             107

// SHODAN
#define SFX_SHODAN_BARK       -1
#define SFX_SHODAN_WEAK       -1
#define SFX_SHODAN_STRONG     -1

// Other
#define SFX_PANEL_SUCCESS     78
#define SFX_POWER_OUT         26
#define SFX_ENERGY_DRAIN      -1
#define SFX_ENERGY_RECHARGE   98
#define SFX_SURGE             -1
#define SFX_VMAIL             -1
#define SFX_DROP_ITEM         -1

// 3d World
#define SFX_BUTTON            71
#define SFX_MECH_BUTTON       36
#define SFX_BIGBUTTON         61
#define SFX_NORMAL_LEVER      62
#define SFX_BIGLEVER          62
#define SFX_KLAXON            -1  // not in

// Plot
#define SFX_GROVE_JETT        -1

#else

#define SFX_NONE              -1

// Doors
#define SFX_DOOR_METAL        3
#define SFX_DOOR_NORMAL       5
#define SFX_DOOR_IRIS         6
#define SFX_DOOR_BULKHEAD     67
#define SFX_DOOR_GRATING      90

// Ambient     
#define SFX_BRIDGE_1          -1
#define SFX_BRIDGE_2          -1
#define SFX_BRIDGE_3          -1
#define SFX_BRIDGE_4          -1
#define SFX_MAINT_1           9
#define SFX_MAINT_2           -1
#define SFX_MAINT_3           -1
#define SFX_MAINT_4           -1
#define SFX_GROVE_1           43

// Critters
#define SFX_DEATH_1           11 // robot 
#define SFX_DEATH_2           49 // bird
#define SFX_DEATH_3           50 // gort
#define SFX_DEATH_4           51 // mutant
#define SFX_DEATH_5           53 // big robot
#define SFX_DEATH_6           54 // small robot
#define SFX_DEATH_7           68 // cyb1
#define SFX_DEATH_8           69 // cyb2
#define SFX_DEATH_9           88 // 0 grav
#define SFX_DEATH_10          93 // plant 
#define SFX_DEATH_11          101 // virus mutant
#define SFX_ATTACK_1          12
#define SFX_ATTACK_4          46
#define SFX_ATTACK_5          48
#define SFX_ATTACK_6          52
#define SFX_ATTACK_7          55
#define SFX_ATTACK_8          63
#define SFX_ATTACK_9          16 // cyborg drone
#define SFX_NOTICE_1          58   
#define SFX_NOTICE_2          59 // replace with new cyborg sound
#define SFX_NOTICE_3          74   
#define SFX_NOTICE_4          75   
#define SFX_NOTICE_5          100   // virus mutant
#define SFX_NEAR_1            73
#define SFX_NEAR_2            56
#define SFX_NEAR_3            47 // gort
#define SFX_NEAR_4            25 // bigcyb near

// Wacky Objects
#define SFX_REPULSOR          -1
#define SFX_FORCE_BRIDGE      72
#define SFX_TERRAIN_ELEV_LOOP       15  
#define SFX_SPARKING_CABLE          87  // not done?
#define SFX_SURGERY_MACHINE         102

// Combat
#define SFX_GUN_MINIPISTOL    39
#define SFX_GUN_DARTPISTOL    86
#define SFX_GUN_MAGNUM        40
#define SFX_GUN_ASSAULT       17
#define SFX_GUN_RIOT          41
#define SFX_GUN_FLECHETTE     38
#define SFX_GUN_SKORPION      65
#define SFX_GUN_MAGPULSE      45
#define SFX_GUN_RAILGUN       29
#define SFX_GUN_PIPE_HIT_MEAT  4 
#define SFX_GUN_PIPE_HIT_METAL 21 
#define SFX_GUN_PIPE_MISS      24 
#define SFX_GUN_LASEREPEE_HIT  31
#define SFX_GUN_LASEREPEE_MISS 34
#define SFX_GUN_PHASER        18
#define SFX_GUN_BLASTER       94
#define SFX_GUN_IONBEAM       95
#define SFX_GUN_STUNGUN       19
#define SFX_GUN_PLASMA        97

#define SFX_PLAYER_HURT       64  
#define SFX_SHIELD_1          32
#define SFX_SHIELD_2          20
#define SFX_SHIELD_UP         96
#define SFX_SHIELD_DOWN       42
#define SFX_METAL_SPANG       89 // robot hit?   not in
#define SFX_RADIATION         SFX_STATIC        

#define SFX_RELOAD_1          22 
#define SFX_RELOAD_2          23
#define SFX_GRENADE_ARM       7     
#define SFX_BATTERY_USE       28

#define SFX_EXPLOSION_1       44
#define SFX_RUMBLE            106
#define SFX_TELEPORT          103

#define SFX_MONITOR_EXPLODE   57
#define SFX_CAMERA_EXPLODE    8
#define SFX_CPU_EXPLODE       10
#define SFX_DESTROY_CRATE     37
#define SFX_DESTROY_BARREL    109

// Cspace
#define SFX_PULSER            -1
#define SFX_DRILL             -1
#define SFX_DISC              -1
#define SFX_DATASTORM         -1

#define SFX_RECALL            -1
#define SFX_TURBO             -1
#define SFX_FAKEID            -1
#define SFX_DECOY             -1

#define SFX_ENTER_CSPACE      27
#define SFX_OTTO_SHODAN       30
#define SFX_CYBER_DAMAGE      -1
#define SFX_CYBERHEAL         -1
#define SFX_CYBERTOGGLE       -1
#define SFX_ICE_DEFENSE       -1

#define SFX_CYBER_ATTACK_1    -1
#define SFX_CYBER_ATTACK_2    -1
#define SFX_CYBER_ATTACK_3    -1

// MFD and UI Wackiness
#define SFX_VIDEO_DOWN        33
#define SFX_MFD_BUTTON        35
#define SFX_INVENT_BUTTON     79
#define SFX_INVENT_SELECT     80
#define SFX_INVENT_ADD        81
#define SFX_INVENT_WARE       82
#define SFX_PATCH_USE         83
#define SFX_ZOOM_BOX          84
#define SFX_MAP_ZOOM          85
#define SFX_MFD_KEYPAD        76
#define SFX_MFD_BUZZ          77
#define SFX_MFD_SUCCESS       78
#define SFX_GOGGLE            0
#define SFX_HUDFROB           1
#define SFX_STATIC            2
#define SFX_EMAIL             107

// SHODAN
#define SFX_SHODAN_BARK       30
#define SFX_SHODAN_WEAK       30
#define SFX_SHODAN_STRONG     30

// Other
#define SFX_PANEL_SUCCESS     78
#define SFX_POWER_OUT         26
#define SFX_ENERGY_DRAIN      14
#define SFX_ENERGY_RECHARGE   98
#define SFX_SURGE             60
#define SFX_VMAIL             92
#define SFX_DROP_ITEM         99

// 3d World
#define SFX_BUTTON            71
#define SFX_MECH_BUTTON       36
#define SFX_BIGBUTTON         61
#define SFX_NORMAL_LEVER      62
#define SFX_BIGLEVER          62
#define SFX_KLAXON            70  // not in

// Plot
#define SFX_GROVE_JETT        66

#endif
#endif
