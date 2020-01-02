#include "spells.h"
#include <AI/ai_common.h>

namespace AI {

CUnit* findBestParasiteTarget(const CUnit *caster, bool isUnderAttack) {
  int bounds;
  if (isUnderAttack)
    bounds = 32 * 9;
  else
    bounds = 32 * 64;

  auto parasiteTargetFinder = [&caster] (const CUnit *target) -> bool {
    if (!isTargetWorthHitting(target, caster))
      return false;

    if (isUmsMode(caster->playerId) && target->parasiteFlags)
      return false;

    if (target->parasiteFlags & (1 << caster->playerId))
      return false;

    if (!scbw::canWeaponTargetUnit(WeaponId::Parasite, target, caster))
      return false;

    if (target->canDetect())
      return true;

    if (!(target->status & UnitStatus::IsHallucination)
        && (target->id != UnitId::overlord || scbw::getUpgradeLevel(target->playerId, UpgradeId::VentralSacs))
        && units_dat::SpaceProvided[target->id] > 0)
      return true;

    if (target->isValidCaster())
      return true;

    if (units_dat::BaseProperty[target->id] & UnitProperty::Worker)
      return true;

    if (getCurrentLifeInGame(target) >= 300)
      return true;

    return false;
  };

  return scbw::UnitFinder::getNearestTarget(
    caster->getX() - bounds, caster->getY() - bounds,
    caster->getX() + bounds, caster->getY() + bounds,
    caster, parasiteTargetFinder);
}

CUnit* findBestTankBusterTarget(const CUnit *caster, bool isUnderAttack) {
  int bounds;
  if (isUnderAttack)
    bounds = 32 * 14
	;
  else if (isUmsMode(caster->playerId))
    bounds = 32 * 64;
  else
    bounds = 32 * 32;

  auto tankBusterTargetFinder = [&caster, &isUnderAttack] (const CUnit *target) -> bool {
    if (!isTargetWorthHitting(target, caster))
      return false;

    if (target->id == UnitId::egg)
      return false;

	if (target->status & (UnitStatus::GroundedBuilding | UnitStatus::InAir | UnitStatus::Invincible))
	  return false;

    const int targetLife = getCurrentLifeInGame(target);
    if (200 <= targetLife && targetLife <= 450
        && !(units_dat::BaseProperty[target->id] & UnitProperty::Hero))
      return true;

	if (target->id == UnitId::siege_tank_s && 100 <= targetLife)
	  return true;

    return false;
  };

  return scbw::UnitFinder::getNearestTarget(
    caster->getX() - bounds, caster->getY() - bounds,
    caster->getX() + bounds, caster->getY() + bounds,
    caster, tankBusterTargetFinder);
}

} //AI
