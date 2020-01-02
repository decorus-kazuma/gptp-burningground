#include "spells.h"
#include <AI/ai_common.h>

namespace AI {

CUnit* findBestSpawnBroodlingsTarget(const CUnit *caster, bool isUnderAttack) {
  int bounds;
  if (isUnderAttack)
    bounds = 32 * 12;
  else
    bounds = 32 * 64;

  auto spawnBroodlingsTargetFinder = [&caster, &isUnderAttack] (const CUnit *target) -> bool {
    if (!scbw::isAlliedTo(caster->playerId, target->getLastOwnerId()))
      return false;

	if (units_dat::BaseProperty[target->id] & (UnitProperty::Hero | UnitProperty::Invincible))
      return false;

	if (target->status & (UnitStatus::Invincible | UnitStatus::InAir | UnitStatus::RequiresDetection))
      return false;

	if (!(units_dat::BaseProperty[target->id] & UnitProperty::Organic))
      return false;

	if (target->isBeingHealed)
      return false;

	if(target->hitPoints >= units_dat::MaxHitPoints[target->id])
	  return false;
	
	if(target->hitPoints <= units_dat::MaxHitPoints[target->id]>>1)
	  return true;

	if(target->mainOrderId == units_dat::AttackUnitOrder[target->id] && target->orderTarget.unit 
		&& (target->orderTarget.unit->canAttackTarget(target) || target->orderTarget.unit->orderTarget.unit == target))
	  return true;


    return false;
  };

  return scbw::UnitFinder::getNearestTarget(
    caster->getX() - bounds, caster->getY() - bounds,
    caster->getX() + bounds, caster->getY() + bounds,
    caster, spawnBroodlingsTargetFinder);
}

} //AI
