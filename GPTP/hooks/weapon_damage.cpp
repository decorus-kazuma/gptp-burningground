#include "weapon_damage.h"
#include "interface\BG.h"
#include "../SCBW/UnitFinder.h"
#include "../SCBW/scbwdata.h"
#include "../SCBW/enumerations.h"
#include "../SCBW/api.h"
#include <algorithm>

namespace {
//Helper functions
void createShieldOverlay(CUnit *unit, u32 attackDirection);
u16 getUnitStrength(const CUnit *unit, bool useGroundStrength);

/// Definition of damage factors (explosive, concussive, etc.)
struct {
  s32 damageType;
  s32 unitSizeFactor[4];   //Order: {independent, small, medium, large}
} const damageFactor[5] = {
  {0, 0, 0, 0, 0},        //Independent
  {1, 0, 128, 192, 256},  //Explosive
  {2, 0, 256, 128, 64},   //Concussive
  {3, 0, 256, 256, 256},  //Normal
  {4, 0, 256, 256, 256}   //IgnoreArmor
};

} //unnamed namespace

namespace hooks {

/// Hooks into the CUnit::damageWith() function.
void weaponDamageHook(u32     damage,
                      CUnit*  target,
                      u8      weaponId,
                      CUnit*  attacker,
                      u8      attackingPlayer,
                      u8      direction,
                      u8      dmgDivisor) {
  //Default StarCraft behavior
  using scbw::isCheatEnabled;
  using CheatFlags::PowerOverwhelming;

  //Don't bother if the unit is already dead or invincible
  if (target->hitPoints == 0 || (target->status & UnitStatus::Invincible))
    return;

  if(attacker != nullptr){
	  switch(attacker->id){
	  case UnitId::spider_mine:
		  if (target->id == UnitId::spider_mine)
			  return;
		  break;
	  case UnitId::dark_templar:
		  attacker->unusedTimer = 24;
		  if (attacker->secondaryOrderId == OrderId::Cloak 
			  && (attacker->status & UnitStatus::Cloaked) && !(attacker->status & UnitStatus::CloakingForFree)){
			  damage <<= 1;
			  target->sprite->createOverlay(133);
			  attacker->setSecondaryOrder(OrderId::Decloak);
			  attacker->updateSpeed();
		  }
		  break;
	  }
  }
  
  if (isCheatEnabled(PowerOverwhelming)                           //If Power Overwhelming is enabled
	  && playerTable[attackingPlayer].type != PlayerType::Human	  //and the attacker is not a human player
	  && playerTable[target->playerId].type == PlayerType::Human){  
	  damage = 0;
	  target->damageHp(damage, attacker, attackingPlayer, weaponId != WeaponId::Irradiate);
	  const u8 damageType = weapons_dat::DamageType[weaponId];
	  if (damageType != DamageType::Independent && units_dat::ShieldsEnabled[target->id] && target->shields >= 256)
		  createShieldOverlay(target, direction);
	  target->airStrength = getUnitStrength(target, false);
	  target->groundStrength = getUnitStrength(target, true);
	  return;
  }

  if (target->id == UnitId::aldaris){
	  const u8 damageAngle = target->currentDirection1 - (direction-128);
	  if((damageAngle < 43 || damageAngle > 213)
		  && weaponId != WeaponId::Irradiate && weaponId != WeaponId::NuclearMissile && weaponId != WeaponId::Spines 
		  && weaponId != WeaponId::GaussRifle4_Unused)
			  damage >>= 1;
  }

  if (target->status & UnitStatus::IsHallucination)
    damage <<= 1;

  damage = damage / dmgDivisor + (target->acidSporeCount << 8);
  
  if(!(target->status & UnitStatus::InAir)
	  && !(units_dat::BaseProperty[target->id] & UnitProperty::Building)
	  && weaponId != WeaponId::Irradiate){
	const u16 Hwidth = 112;
	const u16 Hheight = 73;
	const u16 targetX = target->getX();
	const u16 targetY = target->getY();
	const s16 focus = 86;
	bool inShield = false;

	scbw::UnitFinder sUnit(targetX-Hwidth, targetY-12-Hheight, targetX+Hwidth, targetY-12+Hheight);
	CUnit *getUnit;
	for (int i = 0; i < sUnit.getUnitCount(); ++i){
		getUnit = sUnit.getUnit(i);
		
		if(getUnit->id == UnitId::high_templar && getUnit->playerId == target->playerId && getUnit->building.silo.isReady){
			const u32 maxDistance = scbw::getDistanceFast(getUnit->getX()-focus, getUnit->getY(), getUnit->getX(), getUnit->getY()-Hheight)<<1;
			if(scbw::getDistanceFast(getUnit->getX()-focus, getUnit->getY()+12, targetX, targetY) 
				+ scbw::getDistanceFast(getUnit->getX()+focus, getUnit->getY()+12, targetX, targetY) <= maxDistance){
					inShield = true;
					break;
			}
		}
	}

	if(inShield)
		damage = damage > 640 ? damage-512 : 128;
  }

  if (damage < 128)
    damage = 128;


  //Reduce Defensive Matrix
  if (target->defensiveMatrixHp) {
    const u32 d_matrix_reduceAmount = std::min<u32>(damage, target->defensiveMatrixHp);
    damage -= d_matrix_reduceAmount;
    target->reduceDefensiveMatrixHp(d_matrix_reduceAmount);
  }

  const u8 damageType = weapons_dat::DamageType[weaponId];

  //Reduce Plasma Shields...but not just yet
  s32 shieldReduceAmount = 0;
  if (units_dat::ShieldsEnabled[target->id] && target->shields >= 256) {
    if (damageType != DamageType::IgnoreArmor) {
      s32 plasmaShieldUpg = scbw::getUpgradeLevel(target->playerId, UpgradeId::ProtossPlasmaShields) << 8;\
      if (damage > (u32)plasmaShieldUpg) //Weird logic, Blizzard dev must have been sleepy
        damage -= plasmaShieldUpg;
      else
        damage = 128;
    }
    shieldReduceAmount = std::min<u32>(damage, target->shields);
    damage -= shieldReduceAmount;
  }

  //Apply armor
  if (damageType != DamageType::IgnoreArmor) {
    const u32 armorTotal = target->getArmor() << 8;
    damage -= std::min(damage, armorTotal);
  }

  //Apply damage type/unit size factor
  damage = (damage * damageFactor[damageType].unitSizeFactor[units_dat::SizeType[target->id]]) >> 8;
  if (shieldReduceAmount == 0 && !(target->defensiveMatrixHp) && damage < 128)
    damage = 128;

  //ÆÄºª º¡Ä¿¹ö½ºÆ®
  if (attacker != nullptr){
	  if (attacker->id == UnitId::firebat && target->id == UnitId::bunker){
		  for(int k = 0; k < units_dat::SpaceProvided[target->id]; ++k){
			  CUnit *loaded = target->getLoadedUnit(k);
			  if(loaded)
				  loaded->damageHp(damage, attacker, attackingPlayer, weaponId != WeaponId::Irradiate);
		  }
	  }
  }

  //Deal damage to target HP, killing it if possible
  target->damageHp(damage, attacker, attackingPlayer,
                   weaponId != WeaponId::Irradiate);    //Prevent Science Vessels from being continuously revealed to the irradiated target

  //Reduce shields (finally)
  if (shieldReduceAmount != 0) {
	target->shields -= shieldReduceAmount;
    if (damageType != DamageType::Independent && !(damage))
      createShieldOverlay(target, direction);
  }

  //Update unit strength data (?)
  target->airStrength = getUnitStrength(target, false);
  target->groundStrength = getUnitStrength(target, true);

}

} //hooks

namespace {

/**** Definitions of helper functions. Do NOT modify anything below! ****/

//Creates the Plasma Shield flickering effect.
//Identical to function @ 0x004E6140
void createShieldOverlay(CUnit *unit, u32 attackDirection) {
  const LO_Header* shield_lo = iImagesShieldOverlayGraphic[unit->sprite->mainGraphic->id];
  u32 frameAngle = ((attackDirection - 124) >> 3) % 32;
  Point8 offset = shield_lo->getOffset(unit->sprite->mainGraphic->direction, frameAngle);
  unit->sprite->createOverlay(ImageId::ShieldOverlay, offset.x, offset.y, frameAngle);
}

//Somehow related to AI stuff; details unknown.
const u32 Helper_GetUnitStrength      = 0x00431800;
u16 getUnitStrength(const CUnit *unit, bool useGroundStrength) {
  u16 strength;
  u32 useGroundStrength_ = (useGroundStrength ? 1 : 0);

  __asm {
    PUSHAD
    PUSH useGroundStrength_
    MOV EAX, unit
    CALL Helper_GetUnitStrength
    MOV strength, AX
    POPAD
  }

  return strength;
}

} //unnamed namespace
