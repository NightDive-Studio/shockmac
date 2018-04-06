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
/*
** $Header: n:/project/cit/src/RCS/objapp.c 1.17 1994/05/14 03:30:03 xemu Exp $
 * 
*/

#include "objects.h"
#include "objsim.h"
#include "objwpn.h"
#include "objwarez.h"
#include "objstuff.h"
#include "objgame.h"
#include "objcrit.h"
#include "objprop.h"
#include "map.h"

////////////////////////////// APPLICATION-SPECIFIC DATA
//
// Here we define the arrays for all the application-specific
// classes defined in objapp.h.  Follow this example, and there
// won't be any trouble.
//
// ## INSERT NEW CLASS HERE

/*const*/ ObjSpecHeader objSpecHeaders[NUM_CLASSES] =
{
   {NUM_OBJECTS_GUN, sizeof(ObjGun), (char *)&objGuns},
   {NUM_OBJECTS_AMMO, sizeof(ObjAmmo), (char *)&objAmmos},
   {NUM_OBJECTS_PHYSICS, sizeof(ObjPhysics), (char *)&objPhysicss},
   {NUM_OBJECTS_GRENADE, sizeof(ObjGrenade), (char *)&objGrenades},
   {NUM_OBJECTS_DRUG, sizeof(ObjDrug), (char *)&objDrugs},
   {NUM_OBJECTS_HARDWARE, sizeof(ObjHardware), (char *)&objHardwares},
   {NUM_OBJECTS_SOFTWARE, sizeof(ObjSoftware), (char *)&objSoftwares},
   {NUM_OBJECTS_BIGSTUFF, sizeof(ObjBigstuff), (char *)&objBigstuffs},
   {NUM_OBJECTS_SMALLSTUFF, sizeof(ObjSmallstuff), (char *)&objSmallstuffs},
   {NUM_OBJECTS_FIXTURE, sizeof(ObjFixture), (char *)&objFixtures},
   {NUM_OBJECTS_DOOR, sizeof(ObjDoor), (char *)&objDoors},
   {NUM_OBJECTS_ANIMATING, sizeof(ObjAnimating), (char *)&objAnimatings},
   {NUM_OBJECTS_TRAP, sizeof(ObjTrap), (char *)&objTraps},
   {NUM_OBJECTS_CONTAINER, sizeof(ObjContainer), (char *)&objContainers},
   {NUM_OBJECTS_CRITTER, sizeof(ObjCritter), (char *)&objCritters},
};

//const ObjSpecHeader ObjPropHeader = {NUM_OBJECT, sizeof(ObjProp), &ObjProps}

const ObjSpecHeader ClassPropHeaders[NUM_CLASSES] =
{
   {NUM_GUN, sizeof(GunProp), (char *)&GunProps},
   {NUM_AMMO, sizeof(AmmoProp), (char *)&AmmoProps},
   {NUM_PHYSICS, sizeof(PhysicsProp), (char *)&PhysicsProps},
   {NUM_GRENADE, sizeof(GrenadeProp), (char *)&GrenadeProps},
   {NUM_DRUG, sizeof(DrugProp), (char *)&DrugProps},
   {NUM_HARDWARE, sizeof(HardwareProp), (char *)&HardwareProps},
   {NUM_SOFTWARE, sizeof(SoftwareProp), (char *)&SoftwareProps},
   {NUM_BIGSTUFF, sizeof(BigstuffProp), (char *)&BigstuffProps},
   {NUM_SMALLSTUFF, sizeof(SmallstuffProp), (char *)&SmallstuffProps},
   {NUM_FIXTURE, sizeof(FixtureProp), (char *)&FixtureProps},
   {NUM_DOOR, sizeof(DoorProp), (char *)&DoorProps},
   {NUM_CONTAINER, sizeof(ContainerProp), (char *)&ContainerProps},
   {NUM_CRITTER, sizeof(CritterProp), (char *)&CritterProps},
};

const ObjSpecHeader SubclassPropHeaders[NUM_SUBCLASSES] =
{
   {NUM_PISTOL_GUN, sizeof(PistolGunProp), (char *)&PistolGunProps},
   {NUM_AUTO_GUN, sizeof(AutoGunProp), (char *)&AutoGunProps},
   {NUM_SPECIAL_GUN	      , sizeof(SpecialGunProp), (char *)&SpecialGunProps},
   {NUM_HANDTOHAND_GUN    , sizeof(HandtohandGunProp), (char *)&HandtohandGunProps},
   {NUM_BEAM_GUN	         , sizeof(BeamGunProp), (char *)&BeamGunProps},
   {NUM_BEAMPROJ_GUN	   , sizeof(BeamprojGunProp), (char *)&BeamprojGunProps},
   {NUM_PISTOL_AMMO	      , sizeof(PistolAmmoProp), (char *)&PistolAmmoProps},
   {NUM_NEEDLE_AMMO	      , sizeof(NeedleAmmoProp), (char *)&NeedleAmmoProps},
   {NUM_MAGNUM_AMMO	      , sizeof(MagnumAmmoProp), (char *)&MagnumAmmoProps},
   {NUM_RIFLE_AMMO	      , sizeof(RifleAmmoProp), (char *)&RifleAmmoProps},
   {NUM_FLECHETTE_AMMO	   , sizeof(FlechetteAmmoProp), (char *)&FlechetteAmmoProps},
   {NUM_AUTO_AMMO	      , sizeof(AutoAmmoProp), (char *)&AutoAmmoProps},
   {NUM_PROJ_AMMO	      , sizeof(ProjAmmoProp), (char *)&ProjAmmoProps},
   {NUM_TRACER_PHYSICS    , sizeof(TracerPhysicsProp), (char *)&TracerPhysicsProps},
   {NUM_SLOW_PHYSICS      , sizeof(SlowPhysicsProp), (char *)&SlowPhysicsProps},
   {NUM_CAMERA_PHYSICS    , sizeof(CameraPhysicsProp), (char *)&CameraPhysicsProps},
   {NUM_DIRECT_GRENADE    , sizeof(DirectGrenadeProp), (char *)&DirectGrenadeProps},
   {NUM_TIMED_GRENADE     , sizeof(TimedGrenadeProp), (char *)&TimedGrenadeProps},
   {NUM_STATS_DRUG	        , sizeof(StatsDrugProp), (char *)&StatsDrugProps},
   {NUM_GOGGLE_HARDWARE	, sizeof(GoggleHardwareProp), (char *)&GoggleHardwareProps},
   {NUM_HARDWARE_HARDWARE	, sizeof(HardwareHardwareProp), (char *)&HardwareHardwareProps},
   {NUM_OFFENSE_SOFTWARE	, sizeof(OffenseSoftwareProp), (char *)&OffenseSoftwareProps},
   {NUM_DEFENSE_SOFTWARE	, sizeof(DefenseSoftwareProp), (char *)&DefenseSoftwareProps},
   {NUM_ONESHOT_SOFTWARE	, sizeof(OneshotSoftwareProp), (char *)&OneshotSoftwareProps},
   {NUM_MISC_SOFTWARE	   , sizeof(MiscSoftwareProp), (char *)&MiscSoftwareProps},
   {NUM_DATA_SOFTWARE     , sizeof(DataSoftwareProp), (char *)&DataSoftwareProps},
   {NUM_ELECTRONIC_BIGSTUFF , sizeof(ElectronicBigstuffProp), (char *)&ElectronicBigstuffProps},
   {NUM_FURNISHING_BIGSTUFF , sizeof(FurnishingBigstuffProp), (char *)&FurnishingBigstuffProps},
   {NUM_ONTHEWALL_BIGSTUFF , sizeof(OnthewallBigstuffProp), (char *)&OnthewallBigstuffProps},
   {NUM_LIGHT_BIGSTUFF , sizeof(LightBigstuffProp), (char *)&LightBigstuffProps},
   {NUM_LABGEAR_BIGSTUFF , sizeof(LabgearBigstuffProp), (char *)&LabgearBigstuffProps},
   {NUM_TECHNO_BIGSTUFF , sizeof(TechnoBigstuffProp), (char *)&TechnoBigstuffProps},
   {NUM_DECOR_BIGSTUFF , sizeof(DecorBigstuffProp), (char *)&DecorBigstuffProps},
   {NUM_TERRAIN_BIGSTUFF , sizeof(TerrainBigstuffProp), (char *)&TerrainBigstuffProps},
   {NUM_USELESS_SMALLSTUFF , sizeof(UselessSmallstuffProp), (char *)&UselessSmallstuffProps},
   {NUM_BROKEN_SMALLSTUFF , sizeof(BrokenSmallstuffProp), (char *)&BrokenSmallstuffProps},
   {NUM_CORPSELIKE_SMALLSTUFF , sizeof(CorpselikeSmallstuffProp), (char *)&CorpselikeSmallstuffProps},
   {NUM_GEAR_SMALLSTUFF , sizeof(GearSmallstuffProp), (char *)&GearSmallstuffProps},
   {NUM_CARDS_SMALLSTUFF , sizeof(CardsSmallstuffProp), (char *)&CardsSmallstuffProps},
   {NUM_CYBER_SMALLSTUFF , sizeof(CyberSmallstuffProp), (char *)&CyberSmallstuffProps},
   {NUM_ONTHEWALL_SMALLSTUFF , sizeof(OnthewallSmallstuffProp), (char *)&OnthewallSmallstuffProps},
   {NUM_PLOT_SMALLSTUFF , sizeof(PlotSmallstuffProp), (char *)&PlotSmallstuffProps},
   {NUM_CONTROL_FIXTURE , sizeof(ControlFixtureProp), (char *)&ControlFixtureProps},
   {NUM_RECEPTACLE_FIXTURE , sizeof(ReceptacleFixtureProp), (char *)&ReceptacleFixtureProps},
   {NUM_TERMINAL_FIXTURE , sizeof(TerminalFixtureProp), (char *)&TerminalFixtureProps},
   {NUM_PANEL_FIXTURE , sizeof(PanelFixtureProp), (char *)&PanelFixtureProps},
   {NUM_VENDING_FIXTURE , sizeof(VendingFixtureProp), (char *)&VendingFixtureProps},
   {NUM_CYBER_FIXTURE , sizeof(CyberFixtureProp), (char *)&CyberFixtureProps},
   {NUM_NORMAL_DOOR , sizeof(NormalDoorProp), (char *)&NormalDoorProps},
   {NUM_DOORWAYS_DOOR , sizeof(DoorwaysDoorProp), (char *)&DoorwaysDoorProps},
   {NUM_FORCE_DOOR , sizeof(ForceDoorProp), (char *)&ForceDoorProps},
   {NUM_ELEVATOR_DOOR , sizeof(ElevatorDoorProp), (char *)&ElevatorDoorProps},
   {NUM_SPECIAL_DOOR , sizeof(SpecialDoorProp), (char *)&SpecialDoorProps},
   {NUM_OBJECT_ANIMATING , sizeof(ObjectsAnimatingProp), (char *)&ObjectsAnimatingProps},
   {NUM_TRANSITORY_ANIMATING , sizeof(TransitoryAnimatingProp), (char *)&TransitoryAnimatingProps},
   {NUM_EXPLOSION_ANIMATING , sizeof(ExplosionAnimatingProp), (char *)&ExplosionAnimatingProps},
   {NUM_TRIGGER_TRAP , sizeof(TriggerTrapProp), (char *)&TriggerTrapProps},
   {NUM_FEEDBACKS_TRAP , sizeof(FeedbacksTrapProp), (char *)&FeedbacksTrapProps},
   {NUM_SECRET_TRAP , sizeof(SecretTrapProp), (char *)&SecretTrapProps},
   {NUM_ACTUAL_CONTAINER , sizeof(ActualContainerProp), (char *)&ActualContainerProps},
   {NUM_WASTE_CONTAINER , sizeof(WasteContainerProp), (char *)&WasteContainerProps},
   {NUM_LIQUID_CONTAINER , sizeof(LiquidContainerProp), (char *)&LiquidContainerProps},
   {NUM_MUTANT_CORPSE_CONTAINER , sizeof(MutantCorpseContainerProp), (char *)&MutantCorpseContainerProps},
   {NUM_ROBOT_CORPSE_CONTAINER , sizeof(RobotCorpseContainerProp), (char *)&RobotCorpseContainerProps},
   {NUM_CYBORG_CORPSE_CONTAINER , sizeof(CyborgCorpseContainerProp), (char *)&CyborgCorpseContainerProps},
   {NUM_OTHER_CORPSE_CONTAINER , sizeof(OtherCorpseContainerProp), (char *)&OtherCorpseContainerProps},
   {NUM_MUTANT_CRITTER         , sizeof(MutantCritterProp), (char *)&MutantCritterProps},
   {NUM_ROBOT_CRITTER          , sizeof(RobotCritterProp), (char *)&RobotCritterProps},
   {NUM_CYBORG_CRITTER         , sizeof(CyborgCritterProp), (char *)&CyborgCritterProps},
   {NUM_CYBER_CRITTER          , sizeof(CyberCritterProp), (char *)&CyberCritterProps},
   {NUM_ROBOBABE_CRITTER       , sizeof(RobobabeCritterProp), (char *)&RobobabeCritterProps},
};
                                                                                          	   		

////////////////////////////// APPLICATION-SPECIFIC FUNCTIONS

static int map_x, map_y;

void ObjInfoInit (ObjInfo *info)
{
   info->type = 0;
	info->ph = -1;
}

void ObjRefStateBinIteratorInit (void)
{
	map_x = map_y = 0;
}

bool ObjRefStateBinIterator (ObjRefStateBin *bin)
{
	if (map_y == MAP_YSIZE) return FALSE;
	bin->sq.x = map_x; bin->sq.y = map_y;
	if (++map_x == MAP_XSIZE) { map_x = 0; map_y++; }
	return TRUE;
}
