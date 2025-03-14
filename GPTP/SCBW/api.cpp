#include "api.h"
#include <SCBW/UnitFinder.h>
#include <algorithm>
#include <cassert>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace scbw {

//-------- Output functions --------//
const u32 Func_ChangeUnitType = 0x0049FED0;
void changeUnitType(CUnit *unit, u16 newUnitId) {
  u32 newUnitId_ = newUnitId;
  __asm {
    PUSHAD
    PUSH newUnitId_
    MOV EAX, unit
    CALL Func_ChangeUnitType
    POPAD
  }
}

const u32 Func_UnitConstructor = 0x004A06C0;
CUnit* unitConstructor(u16 unitType, u32 x, u32 y, u8 playerID) {
  CUnit* unit;
  u32 playerID_ = (u32)playerID;
  u32 unitType_ = (u32)unitType;
  u32 random	= NULL;

  __asm {
    PUSHAD
	MOV	ESI, x
	MOV	EDX, playerID_
	PUSH EDX
	MOV	EDX, y
	MOV EDI, unitType_
	PUSH random
    CALL Func_UnitConstructor
	MOV	unit, EAX
    POPAD
  }

  return unit;
}

const u32 Func_getImageAttackFrame = 0x004A06C0;
const u32 Func_setSpriteMainImgOffset = 0x004A06C0;
void setSub(CUnit* base, CUnit* turret){

	u32 xy[2];

	__asm{
		PUSHAD
		MOV	EBX, base
		MOV	ESI, turret
		MOV ECX, [EBX+0Ch]
		PUSH 2
		XOR	EDI, EDI
		LEA	EAX, xy
		CALL Func_getImageAttackFrame
		LEA	EAX, xy
		MOV	ECX, [EAX]
		MOV	EAX, [EAX+4]
		PUSH EAX
		MOV EAX, [ESI+0Ch]
		PUSH ECX
		CALL Func_setSpriteMainImgOffset
		POPAD
	}

}

const u32 Func_ReplaceSpriteImages = 0x00499BB0;
void replaceSpriteImages(CSprite *sprite, u16 imageId, u8 imageDirection) {
  u32 imageId_ = imageId, imageDirection_ = imageDirection;
  __asm {
    PUSHAD
    PUSH imageDirection_
    PUSH imageId_
    MOV EAX, sprite
    CALL Func_ReplaceSpriteImages
    POPAD
  }
}

const u32 Func_PlaySound = 0x0048ED50;
void playSound(u32 sfxId, const CUnit *sourceUnit) {
  __asm {
    PUSHAD
    PUSH 0
    PUSH 1
    MOV ESI, sourceUnit
    MOV EBX, sfxId
    CALL Func_PlaySound
    POPAD
  }
}

const u32 Func_PrintText = 0x0048CD30;
void printText(const char* text, u32 color) {
  if (!text) return;
  DWORD gtc = GetTickCount() + 7000;

  __asm   {
    PUSHAD
    PUSH 0  ;//unknown
    MOV eax, text
    PUSH gtc
    PUSH color
    CALL Func_PrintText
    POPAD
  }
}

const u32 Func_ShowErrorMessageWithSfx = 0x0048EE30;
void showErrorMessageWithSfx(u32 playerId, u32 statTxtId, u32 sfxId) {
  __asm {
    PUSHAD
    MOV ESI, sfxId
    MOV EDI, statTxtId
    MOV EBX, playerId
    CALL Func_ShowErrorMessageWithSfx
    POPAD
  }
}

//-------- Unit checks --------//

const u32 Func_CanBeEnteredBy = 0x004E6E00; //AKA CanEnterTransport()
bool canBeEnteredBy(const CUnit* transport, const CUnit* unit) {
  u32 result;

  __asm {
    PUSHAD
    MOV EAX, transport
    PUSH unit
    CALL Func_CanBeEnteredBy
    MOV result, EAX
    POPAD
  }

  return result != 0;
}

//Identical to function @ 0x00475CE0
bool canWeaponTargetUnit(u8 weaponId, const CUnit *target, const CUnit *attacker) {
  if (weaponId >= WEAPON_TYPE_COUNT)
    return false;

  if (!target)
    return weapons_dat::TargetFlags[weaponId].terrain;

  if (target->status & UnitStatus::Invincible)
    return false;

  const TargetFlag tf = weapons_dat::TargetFlags[weaponId];
  const u32 targetProps = units_dat::BaseProperty[target->id];

  if ((target->status & UnitStatus::InAir) ? !tf.air : !tf.ground)
    return false;

  if (tf.mechanical && !(targetProps & UnitProperty::Mechanical))
    return false;

  if (tf.organic && !(targetProps & UnitProperty::Organic))
    return false;

  if (tf.nonBuilding && (targetProps & UnitProperty::Building))
    return false;

  if (tf.nonRobotic && (targetProps & UnitProperty::RoboticUnit))
    return false;

  if (tf.orgOrMech && !(targetProps & (UnitProperty::Organic | UnitProperty::Mechanical)))
    return false;

  if (tf.playerOwned && target->playerId != attacker->playerId)
    return false;

  return true;
}

bool isUnderDarkSwarm(const CUnit *unit) {
  static UnitFinder darkSwarmFinder;

  darkSwarmFinder.search(unit->getLeft(), unit->getTop(), unit->getRight(), unit->getBottom());
  const CUnit *darkSwarm = darkSwarmFinder.getFirst([] (const CUnit *unit) {
    return unit->id == UnitId::Spell_DarkSwarm;
  });
  return darkSwarm != nullptr;
}

//-------- Graphics and geometry --------//

//Improved code from BWAPI's include/BWAPI/Position.h: getApproxDistance()
//Logically same as function @ 0x0040C360
u32 getDistanceFast(s32 x1, s32 y1, s32 x2, s32 y2) {
  int dMax = abs(x1 - x2), dMin = abs(y1 - y2);
  if (dMax < dMin)
    std::swap(dMax, dMin);

  if (dMin <= (dMax >> 2))
    return dMax;

  return (dMin * 3 >> 3) + (dMin * 3 >> 8) + dMax - (dMax >> 4) - (dMax >> 6);
}

//Identical to function @ 0x00494C10
int arctangent(int slope) {
  const unsigned int tangentTable[] = {
       7,   13,   19,   26,   32,   38,   45,   51,
      58,   65,   71,   78,   85,   92,   99,  107, 
     114,  122,  129,  137,  146,  154,  163,  172,
     181,  190,  200,  211,  221,  233,  244,  256,
     269,  283,  297,  312,  329,  346,  364,  384,
     405,  428,  452,  479,  509,  542,  578,  619,
     664,  716,  775,  844,  926, 1023, 1141, 1287,
    1476, 1726, 2076, 2600, 3471, 5211, 10429, -1
  };

  bool isNegative = false;
  if (slope < 0) {
    isNegative = true;
    slope = -slope;
  }

  int min = 0, max = 64, angle = 32;

  do {
    if ((unsigned int) slope <= tangentTable[angle])
      max = angle;
    else
      min = angle + 1;
    angle = (min + max) / 2;
  } while (min != max);

  return (isNegative ? -angle : angle);
}

//Identical to function @ 0x00495300
s32 getAngle(s32 xHead, s32 yHead, s32 xTail, s32 yTail) {
  s32 dx = xHead - xTail, dy = yHead - yTail;
  
  if (dx == 0)
    return dy > 0 ? 128 : 0;

  s32 angle = arctangent((dy << 8) / dx);
  if (dx < 0) {
    angle += 192;
    return angle == 256 ? 0 : angle;
  }
  else
    return angle + 64;
}

s32 getPolarX(s32 distance, u8 angle) {
  return distance * angleDistance[angle].x >> 8;
}

s32 getPolarY(s32 distance, u8 angle) {
  return distance * angleDistance[angle].y >> 8;
}

//-------- Player information --------//

s32 getSupplyRemaining(u8 playerId, u8 raceId) {
  assert(raceId <= 2);
  assert(playerId < 12);

  s32 supplyProvided;
  if (isCheatEnabled(CheatFlags::FoodForThought))
    supplyProvided = raceSupply[raceId].max[playerId];
  else
    supplyProvided = std::min(raceSupply[raceId].max[playerId], raceSupply[raceId].provided[playerId]);
  return supplyProvided - raceSupply[raceId].used[playerId];
}

bool hasTechResearched(u8 playerId, u16 techId) {
  assert(playerId < PLAYER_COUNT);
  assert(techId < TechId::None);

  if (techId < TechId::Restoration)
    return TechSc->isResearched[playerId][techId] != 0;
  else
    return TechBw->isResearched[playerId][techId - TechId::Restoration] != 0;
}

void setTechResearchState(u8 playerId, u16 techId, bool isResearched) {
  assert(playerId < PLAYER_COUNT);
  assert(techId < TechId::None);

  if (techId < TechId::Restoration)
    TechSc->isResearched[playerId][techId] = isResearched;
  else
    TechBw->isResearched[playerId][techId - TechId::Restoration] = isResearched;
}

u8 getUpgradeLevel(u8 playerId, u8 upgradeId) {
  assert(playerId < PLAYER_COUNT);
  assert(upgradeId < UpgradeId::None);

  if (upgradeId < UpgradeId::UnusedUpgrade46)
    return UpgradesSc->currentLevel[playerId][upgradeId];
  else
    return UpgradesBw->currentLevel[playerId][upgradeId - UpgradeId::UnusedUpgrade46];
}

void setUpgradeLevel(u8 playerId, u8 upgradeId, u8 level) {
  assert(playerId < PLAYER_COUNT);
  assert(upgradeId < UpgradeId::None);

  if (upgradeId < UpgradeId::UnusedUpgrade46)
    UpgradesSc->currentLevel[playerId][upgradeId] = level;
  else
    UpgradesBw->currentLevel[playerId][upgradeId - UpgradeId::UnusedUpgrade46] = level;
}

//-------- Map information --------//

const u32 Func_GetGroundHeightAtPos = 0x004BD0F0;
u32 getGroundHeightAtPos(s32 x, s32 y) {
  u32 height;

  __asm {
    PUSHAD
    MOV EAX, y
    MOV ECX, x
    CALL Func_GetGroundHeightAtPos
    MOV height, EAX
    POPAD
  }

  return height;
}

//-------- Utility functions --------//

extern const u32 Func_CreateUnit = 0x004A09D0;
CUnit* createUnit(u16 unitType, u16 playerId, u32 x, u32 y) {
  if (unitType >= UNIT_TYPE_COUNT) return nullptr;
  CUnit* unit;

  __asm {
    PUSHAD
    MOVZX ECX, playerId
    PUSH ECX
    PUSH y
    MOVZX ECX, unitType
    MOV EAX, x
    CALL Func_CreateUnit
    MOV unit, EAX
    POPAD
  }

  return unit;
}

const u32 Func_CreateUnitAtPos = 0x004CD360; //AKA createUnitXY()
CUnit* createUnitAtPos(u16 unitType, u16 playerId, u32 x, u32 y) {
  if (unitType >= UNIT_TYPE_COUNT) return nullptr;
  CUnit* unit;

  __asm {
    PUSHAD
    MOV CX, unitType
    MOV AX, playerId
    PUSH y
    PUSH x
    CALL Func_CreateUnitAtPos
    MOV unit, EAX
    POPAD
  }

  return unit;
}

const u32 Func_CreateThingy = 0x00488210;
const u32 Func_SetThingyVisibilityFlags = 0x004878F0;
CSprite* createSprite(u16 spriteType, u8 playerId, int x, int y, u8 elevation) {
  if (spriteType >= SPRITE_TYPE_COUNT) return nullptr;
  CSprite* sprite;
  u32 _playerId = playerId;
  u32 _spriteType = spriteType;

  __asm {
    PUSHAD
    MOV EDI, y
	PUSH _playerId
    PUSH x
    PUSH _spriteType
    CALL Func_CreateThingy
    MOV ESI, EAX
    TEST ESI, ESI
	JZ short _end
    MOV EDX, [EAX+0Ch]
	MOV	AL, elevation
    MOV [EDX+0Dh], AL
	MOV	sprite, EDX
    CALL Func_SetThingyVisibilityFlags
_end:
    POPAD
  }

  return sprite;
}

u32 getUnitOverlayAdjustment(const CUnit* const unit) {
  if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
    return 1;
  else if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
    return 2;
  else
    return 0;
}

void refreshScreen(int left, int top, int right, int bottom) {
  left  >>= 4; right  = (right  + 15) >> 4;
  top   >>= 4; bottom = (bottom + 15) >> 4;

  if (left > right) std::swap(left, right);
  if (top > bottom) std::swap(top, bottom);

  //Rect out of bounds
  if (left >= 40 || right < 0 || top >= 30 || bottom < 0) return;

  left  = std::max(left,  0); right   = std::min(right,   40 - 1);
  top   = std::max(top,   0); bottom  = std::min(bottom,  30 - 1);

  for (int y = top; y <= bottom; ++y)
    memset(&refreshRegions[40 * y + left], 1, right - left + 1);
}

void refreshScreen() {
  memset(refreshRegions, 1, 1200);
}

u32 randBetween(u32 min, u32 max) {
  assert(min <= max);
  return min + ((max - min + 1) * random() >> 15);
}

u16 random() {
  if (*IS_IN_GAME_LOOP) {
    *lastRandomNumber = 22695477 * (*lastRandomNumber) + 1;
    return (*lastRandomNumber >> 16) % 32768;  //Make a number between 0 and 32767
  }
  else
    return 0;
}

//-------- Needs research --------//

//Logically equivalent to function @ 0x004C36C0
void refreshConsole() {
  *bCanUpdateCurrentButtonSet = 1;
  *bCanUpdateSelectedUnitPortrait = 1;
  *bCanUpdateStatDataDialog = 1;
  *someDialogUnknown = 0;
  *unknown2 = 0;
}

} //scbw
