//All-in-one header file for various enumerations used by GPTP.

#pragma once
#pragma warning( disable: 4482 )

#include "enumerations/UnitId.h"
#include "enumerations/WeaponId.h"
#include "enumerations/UpgradeId.h"
#include "enumerations/TechId.h"
#include "enumerations/OrderId.h"
#include "enumerations/ImageId.h"

namespace RaceId {
enum Enum : u8 {
  Zerg    = 0,
  Terran  = 1,
  Protoss = 2,
  Other   = 3,
  Neutral = 4,
  Select  = 5,
  Random  = 6,
  None    = 7,
};
}

namespace PlayerType {
enum Enum {
  NotUsed          = 0,
  Computer         = 1,
  Human            = 2,
  Rescuable        = 3,
  Unknown0         = 4,
  ComputerSlot     = 5,
  OpenSlot         = 6,
  Neutral          = 7,
  ClosedSlot       = 8,
  Unknown1         = 9,
  HumanDefeated    = 10, /**< Left */
  ComputerDefeated = 11,  /**< Left */
};
}

//Based on BWAPI; for use with units_dat::BaseProperty[]
namespace UnitProperty {
enum Enum {
  Building            = 0x00000001,
  Addon               = 0x00000002,
  Flyer               = 0x00000004,
  Worker              = 0x00000008,
  Subunit             = 0x00000010,
  FlyingBuilding      = 0x00000020,
  Hero                = 0x00000040,
  RegeneratesHP       = 0x00000080,
  AnimatedIdle        = 0x00000100,
  Cloakable           = 0x00000200,
  TwoUnitsIn1Egg      = 0x00000400,
  NeutralAccessories  = 0x00000800, //AKA "Single entity" (prevents multi-select, set on all pickup items)
  ResourceDepot       = 0x00001000,
  ResourceContainer   = 0x00002000,
  RoboticUnit         = 0x00004000,
  Detector            = 0x00008000,
  Organic             = 0x00010000,
  CreepBuilding       = 0x00020000,
  Unused              = 0x00040000,
  RequiredPsi         = 0x00080000,
  Burrowable          = 0x00100000,
  Spellcaster         = 0x00200000,
  PermanentCloak      = 0x00400000,
  NPCOrAccessories    = 0x00800000, //AKA "Pickup item" (data disc, crystals, mineral chunks, gas tanks, etc.)
  MorphFromOtherUnit  = 0x01000000,
  MediumOverlay       = 0x02000000, //Used to determine overlay for various spells and effects
  LargeOverlay        = 0x04000000, //Used to determine overlay for various spells and effects
  AutoAttackAndMove   = 0x08000000,
  Attack              = 0x10000000, /**< Can attack */
  Invincible          = 0x20000000,
  Mechanical          = 0x40000000,
  ProducesUnits       = 0x80000000, /**< It can produce units directly (making buildings doesn't count) */
};
}

//Based on BWAPI; For use with the CUnit::status member
namespace UnitStatus {
enum Enum {
  Completed             = 0x00000001,
  GroundedBuilding      = 0x00000002, // a building that is on the ground
  InAir                 = 0x00000004,
  Disabled              = 0x00000008,  /**< Protoss Unpowered */
  Burrowed              = 0x00000010,
  InBuilding            = 0x00000020,
  InTransport           = 0x00000040,
  UNKNOWN1              = 0x00000080,  /**< @todo Unknown */
  RequiresDetection     = 0x00000100,
  Cloaked               = 0x00000200,
  DoodadStatesThing     = 0x00000400,  ///< Unknown. ++protoss unpowered buildings have this flag set
  CloakingForFree       = 0x00000800,  /**< Requires no energy to cloak */
  CanNotReceiveOrders   = 0x00001000,
  NoBrkCodeStart        = 0x00002000,  /**< Unbreakable code section in iscript */
  UNKNOWN2              = 0x00004000,  /**< @todo Unknown */
  CanNotAttack          = 0x00008000,  /**< @todo Unknown */
  CanTurnAroundToAttack = 0x00010000,  // canAttack? /**< @todo Unknown */
  IsBuilding            = 0x00020000,
  IgnoreTileCollision   = 0x00040000,
  Unmovable             = 0x00080000,
  IsNormal              = 0x00100000,  /**< 1 for "normal" units, 0 for hallucinated units */
  NoCollide             = 0x00200000, // ++if set, other units wont collide with the unit (like burrowed units)
  UNKNOWN5              = 0x00400000,
  IsGathering           = 0x00800000, // ++if set, the unit wont collide with other units (like workers gathering)
  UNKNOWN6              = 0x01000000,
  UNKNOWN7              = 0x02000000, // Turret related
  Invincible            = 0x04000000,
  HoldingPosition       = 0x08000000, // Set if the unit is currently holding position
  SpeedUpgrade          = 0x10000000,
  CooldownUpgrade       = 0x20000000,
  IsHallucination       = 0x40000000,  /**< 1 for hallucinated units, 0 for "normal" units */
  IsSelfDestructing     = 0x80000000  // Set for when the unit is self-destructing (scarab, scourge, infested terran)
};
}

//---- Taken from buttons.cpp ----//
namespace BUTTON_STATE {
enum Enum {
  Invisible = -1,
  Disabled  = 0,
  Enabled   = 1
};
}

// For use with scbw::printText()
namespace GameTextColor {
enum Enum {
  Grey          = 0x01,
  Grey2         = 0x02,
  Yellow        = 0x03,
  White         = 0x04,
  DarkGrey      = 0x05,
  Red           = 0x06,
  Green         = 0x07,
  Invisible     = 0x08,
  Tab           = 0x09,
  Newline       = 0x0A,
  Unknown       = 0x0B,
  Newline2      = 0x0C,
  Invisible2    = 0x0D,
  AlignRight    = 0x12,
  AlignCenter   = 0x13,
  TextGone      = 0x14,
  RiverBlue     = 0x0E,
  Teal          = 0x0F,
  Purple        = 0x10,
  Orange        = 0x11,
  Brown         = 0x15,
  LightGrey     = 0x16,
  Yellow2       = 0x17,
  DarkGreen     = 0x18,
  Yellow3       = 0x19,
  Yellow4       = 0x1A,
  Navy          = 0x1C,
  ArmyGreen     = 0x1D,
  Grey3         = 0x1E,
  Teal2         = 0x1F,
  AlignCenter2  = 0x13,
  TextGone2     = 0x14,
};
}

namespace LobbyTextColor {
enum Enum {
  Purple1     = 0x01,
  Purple2     = 0x02,
  YellowGreen = 0x04,
  Grey        = 0x05,
  White       = 0x06,
  Red         = 0x07,
  Tab         = 0x09,
  Black       = 0x10, // Could be wrong. Needs check!
  AlignRight  = 0x18,
  AlignCenter = 0x19,
};
};

// For use with scbw::playIscriptAnim()
namespace IscriptAnimation {
enum Enum {
Init          = 0x00,
Death         = 0x01,
GndAttkInit   = 0x02,
AirAttkInit   = 0x03,
Unused1       = 0x04,
GndAttkRpt    = 0x05,
AirAttkRpt    = 0x06,
CastSpell     = 0x07,
GndAttkToIdle = 0x08,
AirAttkToIdle = 0x09,
Unused2       = 0x0A,
Walking       = 0x0B,
WalkingToIdle = 0x0C,
SpecialState1 = 0x0D,
SpecialState2 = 0x0E,
AlmostBuilt   = 0x0F,
Built         = 0x10,
Landing       = 0x11,
LiftOff       = 0x12,
IsWorking     = 0x13,
WorkingToIdle = 0x14,
WarpIn        = 0x15,
Unused3       = 0x16,
StarEditInit  = 0x17,
Disable       = 0x18,
Burrow        = 0x19,
UnBurrow      = 0x1A,
Enable        = 0x1B
};
}

namespace CheatFlags {
enum Enum {
  None							= 0,
  BlackSheepWall				= 1 <<  0,
  OperationCwal					= 1 <<  1,
  PowerOverwhelming				= 1 <<  2,
  SomethingForNothing			= 1 <<  3,
  ShowMeTheMoney				= 1 <<  4,
  TheBestOverseer				= 1 <<  5,
  GameOverMan					= 1 <<  6,
  ThereIsNoCowLevel				= 1 <<  7,
  StayingAlive					= 1 <<  8,
  Ophelia						= 1 <<  9,
  IAmTheCrazyBastardAroundHere	= 1 << 10,
  TheGathering					= 1 << 11,
  MedievalMan					= 1 << 12,
  ModifyThePhaseVariance		= 1 << 13,
  WarAintWhatItUsedToBe			= 1 << 14,
  FoodForThought				= 1 << 17,
  WhatsMineIsMine				= 1 << 18,
  BreatheDeep					= 1 << 19,
  NoGlues						= 1 << 29
};
}

namespace DamageType {
enum Enum {
  Independent = 0,
  Explosive   = 1,
  Concussive  = 2,
  Normal      = 3,
  IgnoreArmor = 4,
};
}

//Use with weapons_dat::ExplosionType[]
namespace WeaponEffect {
enum Enum {
  None            = 0,
  NormalHit       = 1,
  SplashRadial    = 2,
  SplashEnemy     = 3,
  Lockdown        = 4,
  NuclearMissile  = 5,
  Parasite        = 6,
  Broodlings      = 7,
  EmpShockwave    = 8,
  Irradiate       = 9,
  Ensnare         = 10,
  Plague          = 11,
  StasisField     = 12,
  DarkSwarm       = 13,
  Consume         = 14,
  YamatoGun       = 15,
  Restoration     = 16,
  DisruptionWeb   = 17,
  CorrosiveAcid   = 18,
  MindControl     = 19,
  Feedback        = 20,
  OpticalFlare    = 21,
  Maelstrom       = 22,
  Unknown_Crash   = 23,
  SplashAir       = 24
};
}

//Use with weapons_dat::Behavior[]
namespace WeaponBehavior {
enum Enum {
  Fly_DoNotFollowTarget = 0,
  Fly_FollowTarget      = 1,
  AppearOnTargetUnit    = 2,
  PersistOnTargetSite   = 3,  //Psionic Storm
  AppearOnTargetSite    = 4,
  AppearOnAttacker      = 5,
  AttackAndSelfDestruct = 6,
  Bounce                = 7,  //Mutalisk Glave Wurms
  AttackNearbyArea      = 8,  //Valkyrie Halo Rockets
  GoToMaxRange          = 9   //Lurker Subterranean Spines
};
}

//Use with colorShift
namespace ColorRemapping {
enum Enum {
  None        = 0,
  OFire       = 1,
  GFire       = 2,
  BFire       = 3,
  BExpl       = 4,
  Trans50     = 5,  //Special - OwnCloak
  Red_Crash   = 6,
  Green_Crash = 7,
  DFire		  = 8
};
}

//Compare with (*GAME_TYPE)
namespace GameType {
enum Enum {
  Melee           = 2,
  FreeForAll      = 3,
  OneOnOne        = 4,
  CaptureTheFlag  = 5,
  Greed           = 6,
  Slaughter       = 7,
  SuddenDeath     = 8,
  Ladder          = 9,
  UseMapSettings  = 10,
  TeamMelee       = 11,
  TeamFreeForAll  = 12,
  TeamCTF         = 13,
  TopVsBottom     = 15,
};
}

/// Compare with (*CURRENT_TILESET)
namespace TilesetType {
enum Enum {
	Badlands = 0,
	Space_Platform = 1,
	Installation = 2,
	Ash_World = 3,
	Jungle_World = 4,
	Desert = 5,
	Ice = 6,
	Twilight = 7
};
}
