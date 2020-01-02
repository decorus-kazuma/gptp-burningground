#include "spells.h"
#include <AI/ai_common.h>

namespace AI {

CUnit* findBestOpticalFlareTarget(const CUnit *caster, bool isUnderAttack) {
  int bounds;
  if (isUnderAttack)
    bounds = 32 * 9;
  else
    bounds = 32 * 32;

  auto opticalFlareTargetFinder = [&caster] (const CUnit *target) -> bool {
    if (!isTargetWorthHitting(target, caster))
      return false;

    if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
      return false;

    if (target->isBlind)
      return false;

    if (target->canDetect())
      return true;

    if (getCurrentLifeInGame(target) > 80)
      return true;

    return false;
  };

  return scbw::UnitFinder::getNearestTarget(
    caster->getX() - bounds, caster->getY() - bounds,
    caster->getX() + bounds, caster->getY() + bounds,
    caster, opticalFlareTargetFinder);
}

CUnit* findBestStrikeCannonsTarget(const CUnit *caster, bool isUnderAttack) {
  int bounds;
  if (isUnderAttack)
    bounds = 32 * 14;
  else if (isUmsMode(caster->playerId))
    bounds = 32 * 64;
  else
    bounds = 32 * 32;

  auto strikeCannonsTargetFinder = [&caster, &isUnderAttack] (const CUnit *target) -> bool {
    if (!isTargetWorthHitting(target, caster))
      return false;

    if ((target->status & UnitStatus::GroundedBuilding)
        && unitCanAttack(target))
      return true;

    if (target->id == UnitId::bunker && target->hasLoadedUnit())
      return true;

    if (target->id == UnitId::egg)
      return false;

	if (target->status & (UnitStatus::InAir | UnitStatus::Invincible))
	  return false;

    const int targetLife = getCurrentLifeInGame(target);
    if (250 <= targetLife)
      return true;

    return false;
  };

  return scbw::UnitFinder::getNearestTarget(
    caster->getX() - bounds, caster->getY() - bounds,
    caster->getX() + bounds, caster->getY() + bounds,
    caster, strikeCannonsTargetFinder);
}

} //AI
