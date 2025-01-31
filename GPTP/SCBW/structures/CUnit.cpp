#include <cassert>
#include "CUnit.h"
#include "../api.h"
#include "../enumerations.h"
#include <algorithm>

//-------- Unit stats and properties --------//

extern const u32 Func_CanDetect = 0x00403430;
bool CUnit::canDetect() const {
  assert(this);
  static u32 result;

  __asm {
    PUSHAD
    MOV EAX, this
    CALL Func_CanDetect
    MOV result, EAX
    POPAD
  }

  return result != 0;
}

u32 CUnit::getCurrentHpInGame() const {
  assert(this);
  return (this->hitPoints + 255) >> 8;
}

u32 CUnit::getCurrentShieldsInGame() const {
  assert(this);
  return this->shields >> 8;
}

//Identical to function @ 0x004026D0
u32 CUnit::getCurrentLifeInGame() const {
  assert(this);
  if (units_dat::ShieldsEnabled[this->id])
    return getCurrentHpInGame() + getCurrentShieldsInGame();
  else
    return getCurrentHpInGame();
};

//Identical to function @ 0x00475AD0
u8 CUnit::getGroundWeapon() const {
  assert(this);
  if (this->id == UnitId::lurker && !(this->status & UnitStatus::Burrowed))
    return WeaponId::None;
  else
    return units_dat::GroundWeapon[this->id];
}

u8 CUnit::getAirWeapon() const {
  assert(this);
  return units_dat::AirWeapon[this->id];
}

u8 CUnit::getArmor() const {
  assert(this);
  return units_dat::ArmorAmount[this->id] + this->getArmorBonus();
}

extern const u32 Func_GetArmorBonus = 0x00453FC0;
u8 CUnit::getArmorBonus() const {
  assert(this);

  static u8 armorBonus;
  __asm {
    PUSHAD
    MOV EAX, this
    CALL Func_GetArmorBonus
    MOV armorBonus, AL
    POPAD
  }

  return armorBonus;
}

extern const u32 Func_GetMaxEnergy = 0x00491870;
u16 CUnit::getMaxEnergy() const {
  assert(this);
  
  static u16 result;
  __asm {
    PUSHAD
    MOV EAX, this
    CALL Func_GetMaxEnergy
    MOV result, AX
    POPAD
  }

  return result;
}

extern const u32 Func_GetMaxWeaponRange = 0x00475870;
u32 CUnit::getMaxWeaponRange(u8 weaponId) const {
  assert(this);
  assert(weaponId < WEAPON_TYPE_COUNT);

  static u32 maxWeaponRange;
  __asm {
    PUSHAD
    MOV BL, weaponId
    MOV EAX, this
    CALL Func_GetMaxWeaponRange
    MOV maxWeaponRange, EAX
    POPAD
  }

  return maxWeaponRange;
}

const u32 Func_IsTargetWithinMinMovementRange = 0x004763D0;
bool CUnit::isTargetWithinMinMovementRange(CUnit* target, u32 range) const {
  assert(this);
  assert(target);

  static u32 result;
  __asm {
    PUSHAD
    MOV EAX, target
    MOV ECX, range
    MOV EDI, this
    CALL Func_IsTargetWithinMinMovementRange
    MOV result, EAX
    POPAD
  }

  return result != 0;
}

const char* CUnit::getName() const {
  assert(this);
  return getName(this->id);
}

//Based on function @ 0x0047B090
const char* CUnit::getName(u16 unitId) {
  assert(unitId < UNIT_TYPE_COUNT);

  if (units_dat::MapStringId[unitId])
    return (*mapStringTbl)->getString(units_dat::MapStringId[unitId]);
  else
    return (*statTxtTbl)->getString(unitId + 1);
}

RaceId::Enum CUnit::getRace() const {
  assert(this);
  return getRace(this->id);
}

//Based on function at 0x0047B150
RaceId::Enum CUnit::getRace(u16 unitId) {
  assert(unitId < UNIT_TYPE_COUNT);

  if (units_dat::GroupFlags[unitId].isZerg)
    return RaceId::Zerg;
  else if (units_dat::GroupFlags[unitId].isProtoss)
    return RaceId::Protoss;
  else if (units_dat::GroupFlags[unitId].isTerran)
    return RaceId::Terran;
  else
    return RaceId::Neutral;
}

extern const u32 Func_GetSeekRange = 0x00476000;
u8 CUnit::getSeekRange() const {
  assert(this);

  static u8 seekRange;
  __asm {
    PUSHAD
    MOV EDX, this
    CALL Func_GetSeekRange
    MOV seekRange, AL
    POPAD
  }

  return seekRange;
}

extern const u32 Func_GetSightRange = 0x004E5B40;
u32 CUnit::getSightRange(bool isForSpellCasting) const {
  assert(this);

  static u32 sightRange;
  Bool32 ignoreStatusEffects = (isForSpellCasting ? 0 : 1);
  __asm {
    PUSHAD
    PUSH ignoreStatusEffects
    MOV EDX, this
    CALL Func_GetSightRange
    MOV sightRange, EAX
    POPAD
  }

  return sightRange;
}

//Identical to function @ 0x00476180
bool CUnit::hasWeapon() const {
  assert(this);

  if (this->getGroundWeapon() != WeaponId::None
      || this->getAirWeapon() != WeaponId::None)
    return true;

  if (this->id == UnitId::carrier || this->id == UnitId::gantrithor) {
    if ((this->carrier.inHangarCount + this->carrier.outHangarCount) > 0)
      return true;
  }

  if (this->id == UnitId::reaver || this->id == UnitId::warbringer) {
    if (this->carrier.inHangarCount > 0)
      return true;
  }

  if (this->subunit) {
    if (this->subunit->getGroundWeapon() != WeaponId::None
        || this->subunit->getAirWeapon() != WeaponId::None)
      return true;
  }

  return false;
}

//Identical to function @ 0x004020B0
bool CUnit::isFrozen() const {
  assert(this);
  return this->status & UnitStatus::DoodadStatesThing
    || this->lockdownTimer || this->stasisTimer || this->maelstromTimer;
}

extern const u32 Func_IsRemorphingBuilding = 0x0045CD00;
bool CUnit::isRemorphingBuilding() const {
  static Bool32 result;
  assert(this);

  __asm {
    PUSHAD
    MOV EAX, this
    CALL Func_IsRemorphingBuilding
    MOV result, EAX
    POPAD
  }

  return result != 0;
}

extern const u32 Func_IsConstructingAddon = 0x004E66B0;
bool CUnit::isConstructingAddon() const {
  static Bool32 result;
  assert(this);

  __asm {
    PUSHAD
    MOV EAX, this
    CALL Func_IsConstructingAddon
    MOV result, EAX
    POPAD
  }

  return result != 0;
}

extern const u32 Func_IsActiveTransport = 0x004E6BA0;
bool CUnit::isActiveTransport() const {
  static Bool32 result;
  assert(this);

  __asm {
    PUSHAD
    MOV ECX, this
    CALL Func_IsActiveTransport
    MOV result, EAX
    POPAD
  }

  return result != 0;
}

//Identical to function @ 0x00401D40
bool CUnit::isSubunit() const {
  return (this && units_dat::BaseProperty[this->id] & UnitProperty::Subunit);
}

//Identical to function @ 0x00401210
bool CUnit::isValidCaster() const {
  assert(this);
  return units_dat::BaseProperty[this->id] & UnitProperty::Spellcaster
    && !(this->status & UnitStatus::IsHallucination);
}

//-------- Positions and dimensions --------//

u16 CUnit::getX() const {
  assert(this);
  assert(this->sprite);
  return this->sprite->position.x;
};

u16 CUnit::getY() const {
  assert(this);
  assert(this->sprite);
  return this->sprite->position.y;
};

u16 CUnit::getTileX() const {
  assert(this);
  return this->getX() / 32;
};

u16 CUnit::getTileY() const {
  assert(this);
  return this->getY() / 32;
};

s16 CUnit::getLeft() const {
  assert(this);
  return this->getX() - units_dat::UnitBounds[this->id].left;
}

s16 CUnit::getRight() const {
  assert(this);
  return this->getX() + units_dat::UnitBounds[this->id].right;
}

s16 CUnit::getTop() const {
  assert(this);
  return this->getY() - units_dat::UnitBounds[this->id].top;
}

s16 CUnit::getBottom() const {
  assert(this);
  return this->getY() + units_dat::UnitBounds[this->id].bottom;
}

s16 CUnit::getLeftPlacement() const {
  assert(this);
  return this->getX() - units_dat::PlacementBox[this->id].x/2;
}

s16 CUnit::getRightPlacement() const {
  assert(this);
  return this->getX() + units_dat::PlacementBox[this->id].x/2;
}

s16 CUnit::getTopPlacement() const {
  assert(this);
  return this->getY() - units_dat::PlacementBox[this->id].y/2;
}

s16 CUnit::getBottomPlacement() const {
  assert(this);
  return this->getY() + units_dat::PlacementBox[this->id].y/2;
}

//-------- Modifying unit state --------//

const u32 Func_SetUnitHp = 0x00467340;
void CUnit::setHp(s32 hitPoints) {
  assert(this);

  __asm {
    PUSHAD
    MOV EAX, this
    MOV ECX, hitPoints
    CALL Func_SetUnitHp
    POPAD
  }
}

extern const u32 Func_ChangeUnitButtonSet = 0x004E5D60;
void CUnit::setButtonSet(u16 buttonSet) {
	assert(this);
	
	u16 buttonSet_ = buttonSet == -1 ? this->id : buttonSet;

	__asm {
	  PUSHAD
      MOV EAX, this
      MOV CX, buttonSet_
	  CALL Func_ChangeUnitButtonSet
	  POPAD
	}
}

extern const u32 Func_DoWeaponDamage = 0x00479930; //Note: Also used by weaponDamageHook()
void CUnit::damageWith(u32 damage, u8 weaponId, CUnit *attacker,
                       u8 attackingPlayer, s8 direction, u8 damageDivisor) {
  assert(this);
  assert(weaponId < WEAPON_TYPE_COUNT);
  assert(damageDivisor != 0);

  u32 weaponId_         = weaponId;
  s32 attackingPlayer_  = attackingPlayer;
  s32 direction_        = direction;
  s32 damageDivisor_    = damageDivisor;
  __asm {
    PUSHAD
    MOV EDI, this
    MOV EAX, damage
    PUSH attackingPlayer_
    PUSH attacker
    PUSH direction_
    PUSH damageDivisor_
    PUSH weaponId_
    CALL Func_DoWeaponDamage
    POPAD    
  }
}

const u32 Func_DamageUnitHp = 0x004797B0;
void CUnit::damageHp(u32 damage, CUnit *attacker, s32 attackingPlayer, bool notify) {
  assert(this);

  u32 notify_ = notify ? 1 : 0;
  __asm {
    PUSHAD
    PUSH notify_
    PUSH attackingPlayer
    PUSH attacker
    MOV EAX, damage
    MOV ECX, this
    CALL Func_DamageUnitHp
    POPAD
  }
}

//Logically equivalent to function @ 0x00454ED0
void CUnit::reduceDefensiveMatrixHp(u32 amount) {
  assert(this);

  if (this->defensiveMatrixHp > amount) {
    this->defensiveMatrixHp -= amount;
  }
  else {
    this->defensiveMatrixHp = 0;
    this->defensiveMatrixTimer = 0;

	CImage* defensiveOverlay = this->sprite->getOverlay(ImageId::DefensiveMatrixFront_Small + scbw::getUnitOverlayAdjustment(this));
	if(defensiveOverlay)
		defensiveOverlay->playIscriptAnim(IscriptAnimation::Death);
	

	/*
    this->removeOverlay(ImageId::DefensiveMatrixFront_Small,
                        ImageId::DefensiveMatrixFront_Large);
    this->removeOverlay(ImageId::DefensiveMatrixBack_Small,
                        ImageId::DefensiveMatrixBack_Large);*/
  }
  //���׿� ����ú� �� ����Ʈ ��� ����. 
  /*if (this->defensiveMatrixTimer && !(this->status & UnitStatus::Burrowed)) {
    CUnit *unit = this->subunit ? this->subunit : this;
    unit->sprite->createTopOverlay(scbw::getUnitOverlayAdjustment(unit) + ImageId::DefensiveMatrixHit_Small);
  }*/
}

const u32 Func_RemoveUnit = 0x00475710; //AKA orders_SelfDestructing()
void CUnit::remove() {
  assert(this);

  __asm {
    PUSHAD
    MOV EAX, this
    CALL Func_RemoveUnit
    POPAD
  }
}

const u32 Func_RemoveLockdown = 0x00454D90;
void CUnit::removeLockdown() {
  assert(this);
  __asm {
    PUSHAD
    MOV ESI, this
    CALL Func_RemoveLockdown
    POPAD
  }
}

const u32 Func_RemoveMaelstrom = 0x00454D20;
void CUnit::removeMaelstrom() {
  assert(this);
  __asm {
    PUSHAD
    MOV ESI, this
    CALL Func_RemoveMaelstrom
    POPAD
  }
}

const u32 Func_RemoveStasisField = 0x004F62D0;
void CUnit::removeStasisField() {
  assert(this);
  __asm {
    PUSHAD
    MOV ESI, this
    CALL Func_RemoveStasisField
    POPAD
  }
}

const u32 Func_RestoreAllUnitStats = 0x00492F70;
void CUnit::restoreAllUnitStats(){
  assert(this);

  __asm{
	  PUSHAD
	  MOV EAX, this
	  CALL Func_RestoreAllUnitStats
	  POPAD
  }
}

void CUnit::removeDebuffEffect(){
	assert(this);

	if(this->ensnareTimer){
		this->ensnareTimer = 0;
		this->removeOverlay(ImageId::EnsnareOverlay_Small, ImageId::EnsnareOverlay_Large);
		this->updateSpeed();
	}
	if(this->plagueTimer){
		this->plagueTimer = 0;
		this->removeOverlay(ImageId::PlagueOverlay_Small, ImageId::PlagueOverlay_Large);
	}
	if(this->irradiateTimer){
		this->irradiateTimer = 0;
		this->irradiatedBy = nullptr;
		this->irradiatePlayerId = 8;
		this->removeOverlay(ImageId::Irradiate_Small, ImageId::Irradiate_Large);
    }
	if(this->maelstromTimer){
		this->maelstromTimer = 0;
		this->removeMaelstrom();
	}
	if(this->lockdownTimer){
		this->lockdownTimer = 0;
		this->removeLockdown();
	}
	if(this->stasisTimer){
		this->stasisTimer = 0;
		this->removeStasisField();
	}
	if(this->acidSporeCount){
		for (int i = 0; i <= 8; ++i)
			this->acidSporeTime[i] = 0;
		this->acidSporeCount = 0;
		this->removeOverlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);
	}

}

//-------- Unit orders --------//

//Logically equivalent to function @ 0x004752B0
void CUnit::orderTo(u8 orderId, const CUnit *target) {
  assert(this);

  this->userActionFlags |= 0x1;
  if (target)
    this->order(orderId, target->getX(), target->getY(), target, UnitId::None, true);
  else
    this->order(orderId, 0, 0, target, UnitId::None, true);
  
  prepareForNextOrder(this);
}

//Logically equivalent to function @ 0x00475260
void CUnit::orderTo(u8 orderId, u16 x, u16 y) {
  assert(this);

  this->userActionFlags |= 0x1;
  this->order(orderId, x, y, nullptr, UnitId::None, true);
  prepareForNextOrder(this);
}

//Logically equivalent to function @ 0x004753A0
void CUnit::orderToIdle() {
  assert(this);

  if (this->orderQueueHead) {
    this->userActionFlags |= 1;
    prepareForNextOrder(this);
  }
  else {
    if (this->pAI)
      this->orderTo(OrderId::ComputerAI);
    else
      this->orderTo(units_dat::ReturnToIdleOrder[this->id]);
  }
}

const u32 Func_Order = 0x00474810;
void CUnit::order(u8 orderId, u16 x, u16 y, const CUnit *target, u16 targetUnitId, bool stopPreviousOrders) {
  assert(this);
  static Point16 pos;
  static u32 targetUnitId_;
  pos.x = x, pos.y = y;
  targetUnitId_ = targetUnitId;

  __asm {
    PUSHAD
    PUSH targetUnitId_
    PUSH target
    PUSH pos
    MOVZX EAX, stopPreviousOrders
    MOV CL, orderId
    MOV EDX, this
    CALL Func_Order
    POPAD
  }
}

//Identical to @ 0x004743D0
void CUnit::setSecondaryOrder(u8 orderId) {
  assert(this);
  if (this->secondaryOrderId == orderId)
    return;
  this->secondaryOrderId = orderId;
  this->secondaryOrderPos.x = 0;
  this->secondaryOrderPos.y = 0;
  this->currentBuildUnit = nullptr;
  this->secondaryOrderState = 0;
}

const u32 Func_SetRetreatPoint = 0x004011A0;
void CUnit::setRetreatPoint(u8 direct){
	assert(this);
	u8 _direct = direct;

  __asm {
    PUSHAD
    MOV EDX, this
	MOV AL, _direct
    CALL Func_SetRetreatPoint
    POPAD
  }
}

const u32 Func_ResetUnitPowerup = 0x004F3B70;
void CUnit::resetUnitPowerup(){
	assert(this);

  __asm {
    PUSHAD
    PUSH this
    CALL Func_ResetUnitPowerup
    POPAD
  }
}
const u32 Func_DestroyPowerupImageOverlay = 0x004F3900;
void CUnit::destroyPowerupImageOverlay(){
	assert(this);

  __asm {
    PUSHAD
	MOV EAX, this
    CALL Func_DestroyPowerupImageOverlay
    POPAD
  }
}

//-------- Graphics and animations --------//

//Similar to function @ 0x004E5C90
CImage* CUnit::getOverlay(u16 imageId) const {
  assert(this);

  CImage *image = this->sprite->getOverlay(imageId);
  if (!image && this->subunit)
    image = this->subunit->sprite->getOverlay(imageId);

  return image;
}

void CUnit::playIscriptAnim(IscriptAnimation::Enum animation) {
  assert(this);
  this->sprite->playIscriptAnim(animation);
}

void CUnit::removeOverlay(u32 imageIdStart, u32 imageIdEnd) {
  assert(this);

  this->sprite->removeOverlay(imageIdStart, imageIdEnd);
  if (this->subunit)
    this->subunit->sprite->removeOverlay(imageIdStart, imageIdEnd);
}

void CUnit::removeOverlay(u32 imageId) {
  assert(this);

  this->sprite->removeOverlay(imageId);
  if (this->subunit)
    this->subunit->sprite->removeOverlay(imageId);
}

graphics::ColorId CUnit::getColor() {
	assert(this);

	//store a playerId until the end where it will properly store a ColorId
	graphics::ColorId return_value;

	if(this->sprite != NULL)
		// note: using the sprite playerId to preserve recruitable units color
		return_value = this->sprite->playerId;
	else
		// note: using player color for spriteless units (if that exist)
		return_value = this->playerId;

	//for normal players, apply colors switching
	if(this->playerId >= 0 && this->playerId <= 7) {

		// get the proper playerId in multiplayer game (don't break campaigns/UMS)
		return_value = colorReordering[return_value];

		//fix BROWN to GREEN color swap in Desert map
		if(*CURRENT_TILESET == TilesetType::Desert && return_value == 5)
			return_value = 8;
		
		//fix WHITE to GREEN color swap in Ice map
		if(*CURRENT_TILESET == TilesetType::Ice && return_value == 6)
			return_value = 8;

	}

	return_value = graphics::playersColors[return_value];

	return return_value;

}

const u32 Func_ApplyUnitEffects = 0x00492DA0;
void CUnit::applyUnitEffects(){
  assert(this);

  __asm{
	  PUSHAD
	  MOV EAX, this
	  CALL Func_ApplyUnitEffects
	  POPAD
  }
}

//-------- Distances and terrain --------//

u32 CUnit::getDistanceToTarget(const CUnit *target) const {
  using units_dat::BaseProperty;
  assert(this);
  assert(target);

  const CUnit *unit = this;
  if (BaseProperty[this->id] & UnitProperty::Subunit)
    unit = this->subunit; // Current unit is a turret, so use it's base instead

  s32 dx = unit->getLeft() - target->getRight() - 1;
  if (dx < 0) {
    dx = target->getLeft() - unit->getRight() - 1;
    if (dx < 0)
      dx = 0;
  }

  s32 dy = unit->getTop() - target->getBottom() - 1;
  if (dy < 0) {
    dy = target->getTop() - unit->getBottom() - 1;
    if (dy < 0)
      dy = 0;
  }

  return scbw::getDistanceFast(0, 0, dx, dy);
}


const u32 Func_isDistanceGreaterThanHaltDistance = 0x00401240;
Bool32 CUnit::isDistanceGreaterThanHaltDistance(s32 x, s32 y, u32 distance) const {
  assert(this);
  static Bool32 result;

  __asm {
    PUSHAD
    PUSH x
    MOV EAX, y
    PUSH distance
    MOV ECX, this
    CALL Func_isDistanceGreaterThanHaltDistance 
    MOV result, EAX
    POPAD
  }

  return result;
}

const u32 Func_HasPathToPos = 0x0049CB60; //AKA unitHasPathToDest()
bool CUnit::hasPathToPos(u32 x, u32 y) const {
  assert(this);
  u32 result;

  __asm {
    PUSHAD
    PUSH y
    PUSH x
    MOV EAX, this
    CALL Func_HasPathToPos
    MOV result, EAX
    POPAD
  }

  return result != 0;
}

const u32 Func_HasPathToTarget = 0x0049CBB0; //AKA unitHasPathToUnit()
bool CUnit::hasPathToUnit(const CUnit *target) const {
  assert(this);
  assert(target);
  u32 result;

  __asm {
    PUSHAD
    MOV EAX, this
    MOV ECX, target
    CALL Func_HasPathToTarget
    MOV result, EAX
    POPAD
  }

  return result != 0;
}

//-------- DAT Requirements --------//

const u32 Func_CanMakeUnit = 0x0046E1C0;
int CUnit::canMakeUnit(u16 unitId, u8 playerId) const {
  assert(this);
  s32 playerId_ = playerId;
  static s32 result;

  __asm {
    PUSHAD
    PUSH playerId_
    MOV AX, unitId
    MOV ESI, this
    CALL Func_CanMakeUnit
    MOV result, EAX
    POPAD
  }

  return result;
}

const u32 Func_CanUseTech = 0x0046DD80;
int CUnit::canUseTech(u8 techId, u8 playerId) const {
  assert(this);
  s32 playerId_ = playerId;
  static s32 result;

  __asm {
    PUSHAD
    PUSH playerId_
    MOVZX DI, techId
    MOV EAX, this
    CALL Func_CanUseTech
    MOV result, EAX
    POPAD
  }

  return result;
}

//-------- Utility methods --------//

const u32 Func_CanUnitAttackTarget = 0x00476730;
bool CUnit::canAttackTarget(const CUnit* target, bool checkVisibility) const {
  assert(this);
  assert(target);

  u32 _checkVisibility = checkVisibility ? 1 : 0;
  static u32 result;

  __asm {
    PUSHAD
    PUSH _checkVisibility
    MOV EBX, target
    MOV ESI, this
    CALL Func_CanUnitAttackTarget
    MOV result, EAX
    POPAD
  }

  return result != 0;
}

extern const u32 Func_FireUnitWeapon = 0x00479C90;
void CUnit::fireWeapon(u8 weaponId) const {
  assert(this);
  static u32 weaponId_ = weaponId;
  __asm {
    PUSHAD
    PUSH weaponId_
    MOV ESI, this
    CALL Func_FireUnitWeapon
    POPAD
  }
}

CUnit* CUnit::getFromIndex(u16 index) {
  if (1 <= index && index <= UNIT_ARRAY_LENGTH)
    return &unitTable[index - 1];
  return nullptr;
}

u16 CUnit::getIndex() const {
  return this - unitTable + 1;
}

u8 CUnit::getLastOwnerId() const {
  assert(this);
  assert(this->sprite);
  if (this->playerId == 11) //The owner has left the game
    return this->sprite->playerId;
  else
    return this->playerId;
}

CUnit* CUnit::getLoadedUnit(int index) const {
  assert(0 <= index && index < 8);

  CUnit *loadedUnit = CUnit::getFromIndex(this->loadedUnit[index].index);
  if (loadedUnit && loadedUnit->sprite
      && !(loadedUnit->mainOrderId == OrderId::Die && loadedUnit->mainOrderState == 1)
      && loadedUnit->targetOrderSpecial == this->loadedUnit[index].unitId)
    return loadedUnit;

  return nullptr;
}

//Logically equivalent to function @ 0x004E6C90
CUnit* CUnit::getFirstLoadedUnit() const {
  assert(this);

  for (int i = 0; i < units_dat::SpaceProvided[this->id]; ++i) {
    CUnit *firstLoadedUnit = getLoadedUnit(i);
    if (firstLoadedUnit)
      return firstLoadedUnit;
  }

  return nullptr;
}

//Logically equivalent to function @ 0x004E7110
bool CUnit::hasLoadedUnit() const {
  assert(this);

  for (int i = 0; i < 8; ++i)
    if (!getLoadedUnit(i))
      return true;

  return false;
}

//Identical to function @ 0x00475A50
bool CUnit::isDead() const {
  assert(this);
  return (!this->sprite) || (this->mainOrderId == OrderId::Die);
}

bool CUnit::isTargetEnemy(const CUnit* target) const {
  assert(this);
  return scbw::isUnitEnemy(this->playerId, target);
}

bool CUnit::isVisibleTo(u8 playerId) const {
  assert(this);
  return (this->visibilityStatus & (1 << playerId)) != 0;
}

const u32 Func_PlayMorphingCompleteSound = 0x0048F440;
void CUnit::playCompleteEvent() {
  assert(this);

  __asm {
    PUSHAD
    MOV EAX, this
    CALL Func_PlayMorphingCompleteSound
    POPAD
  }
}

const u32 Func_UpdateSpeed = 0x00454310;
void CUnit::updateSpeed() {
  assert(this);

  __asm {
    PUSHAD
    MOV EAX, this
    CALL Func_UpdateSpeed
    POPAD
  }
}

const u32 Func_UpdateUnitDamageOverlay = 0x004E6090;
void CUnit::updateUnitDamageOverlay() {
  assert(this);

  if(iImagesDamageOverlay[this->sprite->mainGraphic->id]){
	  __asm {
		PUSHAD
		MOV ESI, this
		CALL Func_UpdateUnitDamageOverlay
		POPAD
	  }
  }
}

const u32 Func_MoveUnit = 0x004EB9F0;
void CUnit::MoveUnit(u16 x, u16 y) {
	assert(this);

	__asm {
		PUSHAD
			MOV EDX, this
			MOV ax, x
			MOV cx, y
			CALL Func_MoveUnit
			POPAD
	}
}

const u32 Func_RefreshUnitVision = 0x004E5F30;
void CUnit::refreshUnitVision() {
	assert(this);

	__asm {
		PUSHAD
			PUSH this
			CALL Func_RefreshUnitVision
			POPAD
	}
}

const u32 Func_RefreshUnit = 0x00494160;
void CUnit::refreshUnit() {
	assert(this);

	__asm {
		PUSHAD
			MOV EAX, this
			CALL Func_RefreshUnit
			POPAD
	}
}