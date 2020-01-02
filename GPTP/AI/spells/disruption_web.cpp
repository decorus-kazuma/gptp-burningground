#include "spells.h"
#include <AI/ai_common.h>

namespace AI {

CUnit* findBestDisruptionWebTarget(const CUnit *caster, bool isUnderAttack) {
  int bounds;
  if (isUnderAttack)
    bounds = 32 * 9;
  else if (isUmsMode(caster->playerId))
    bounds = 32 * 128;
  else
    bounds = 32 * 64;

  auto disruptionWebTargetFinder = [&caster] (const CUnit *target) -> bool {
    if (!isTargetWorthHitting(target, caster))
      return false;

    CUnit *targetOfTarget;
    if (target->id == UnitId::bunker && target->hasLoadedUnit()) {
      CUnit *firstLoadedUnit;
      for (int i = 0; i < 8; ++i)
        if (firstLoadedUnit = target->getLoadedUnit(i))
          break;

      targetOfTarget = firstLoadedUnit->orderTarget.unit;
    }
    else
      targetOfTarget = target->orderTarget.unit;

    if (!targetOfTarget
        || targetOfTarget->playerId >= 8
        || !(targetOfTarget->status & UnitStatus::InAir)
        || !scbw::isAlliedTo(caster->playerId, targetOfTarget->getLastOwnerId()))
      return false;

    if (target->status & UnitStatus::CanNotAttack)
      return false;

    if (!(target->status & UnitStatus::InAir)
        && caster->getDistanceToTarget(target) <= 32 * 8)
      return true;

    if (target->id == UnitId::missile_turret
        || target->id == UnitId::bunker
        || target->id == UnitId::spore_colony
        || target->id == UnitId::photon_cannon)
      return true;

    return false;
  };

  return scbw::UnitFinder::getNearestTarget(
    caster->getX() - bounds, caster->getY() - bounds,
    caster->getX() + bounds, caster->getY() + bounds,
    caster, disruptionWebTargetFinder);
}

CUnit* findBestPorcupineMissileTarget(const CUnit *caster, bool isUnderAttack) {
  int bounds;
  if (isUnderAttack)
    bounds = 32 * 12;
  else
    bounds = 32 * 32;

  auto porcupineMissileTargetFinder = [&caster, &isUnderAttack] (const CUnit *target) -> bool {
    if (!isTargetWorthHitting(target, caster))
      return false;

    if (target->id == UnitId::egg)
      return false;

	if (target->status & UnitStatus::Invincible)
	  return false;
	
	if (target->id == UnitId::photon_cannon)
      return true;

    if (target->id == UnitId::bunker && target->hasLoadedUnit())
      return true;
	
	if (units_dat::BaseProperty[target->id] & UnitProperty::Spellcaster)
      return true;

    const int targetLife = getCurrentLifeInGame(target);
    if ((isUnderAttack && 100 <= targetLife && targetLife <= 250)
        && !(units_dat::BaseProperty[target->id] & UnitProperty::Hero))
      return true;

	if (target->id == UnitId::siege_tank_s && 40 < targetLife)
	  return true;

    return false;
  };

  return scbw::UnitFinder::getNearestTarget(
    caster->getX() - bounds, caster->getY() - bounds,
    caster->getX() + bounds, caster->getY() + bounds,
    caster, porcupineMissileTargetFinder);
}


CUnit* findBestForceFieldTarget(const CUnit *caster, bool isUnderAttack) {
  int bounds = 32 * 9;

  auto forceFieldTargetFinder = [&caster] (const CUnit *target) -> bool {
    if (!isTargetWorthHitting(target, caster))
      return false;

    if (isNoDebuffUnit(target)
		|| target->id == UnitId::kukulza_guardian
		|| target->id == UnitId::arcturus_mengsk
		|| target->id == UnitId::dark_archon
		|| target->id == UnitId::infested_duran)
      return false;
	
	if (target->status & (UnitStatus::InAir | UnitStatus::GroundedBuilding | UnitStatus::CanNotAttack | UnitStatus::IgnoreTileCollision))
      return false;
	
	if (isTargetAttackingAlly(target, caster) && target->getMaxWeaponRange(target->getGroundWeapon()) <= 96)
      return true;

    return false;
  };

  return scbw::UnitFinder::getNearestTarget(
    caster->getX() - bounds, caster->getY() - bounds,
    caster->getX() + bounds, caster->getY() + bounds,
    caster, forceFieldTargetFinder);
}

} //AI
