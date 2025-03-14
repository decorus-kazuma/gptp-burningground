#include "spells.h"
#include <AI/ai_common.h>

namespace AI {

CUnit* findBestDarkSwarmTarget(const CUnit *caster, bool isUnderAttack) {
  int bounds;
  if (isUnderAttack)
    bounds = 32 * 9;
  else if (isUmsMode(caster->playerId))
    bounds = 32 * 64;
  else
    bounds = 32 * 32;

  auto darkSwarmTargetFinder = [&caster] (const CUnit *target) -> bool {
    if (!isTargetWorthHitting(target, caster))
      return false;

    CUnit *targetOfTarget = target->orderTarget.unit;
    if (!targetOfTarget || targetOfTarget->playerId >= 8)
      return false;

    if (!scbw::isAlliedTo(caster->playerId, targetOfTarget->getLastOwnerId()))
      return false;

    if (targetOfTarget->subunit)
      return false;
      
    if (targetOfTarget->status & UnitStatus::InAir)
      return false;

    u8 totGroundWeapon = targetOfTarget->getGroundWeapon();
    if (totGroundWeapon == WeaponId::None)
      return false;

    if (weapons_dat::Behavior[totGroundWeapon] != WeaponBehavior::AppearOnAttacker)
      return false;

    if (target->subunit && (units_dat::BaseProperty[target->subunit->id] & UnitProperty::Subunit))
      target = target->subunit;

    u8 targetGroundWeapon = target->getGroundWeapon();

    if (targetGroundWeapon == WeaponId::None)
      return false;

    switch (weapons_dat::Behavior[targetGroundWeapon]) {
      case WeaponBehavior::Fly_DoNotFollowTarget:
      case WeaponBehavior::Fly_FollowTarget:
      case WeaponBehavior::AppearOnTargetUnit:
      case WeaponBehavior::AppearOnTargetSite:
      case WeaponBehavior::Bounce:
        if (!(targetOfTarget->status & UnitStatus::InAir)
            && !(units_dat::BaseProperty[targetOfTarget->id] & UnitProperty::Building)
            && scbw::isUnderDarkSwarm(targetOfTarget))
          return true;

        if (scbw::getActiveTileAt(targetOfTarget->getX(), targetOfTarget->getY()).hasDoodadCover)
          return true;

        if (!(targetOfTarget->status & UnitStatus::InAir || target->status & UnitStatus::InAir))
        {
          u32 targGndHeight = scbw::getGroundHeightAtPos(target->getX(), target->getY());
          u32 totGndHeight = scbw::getGroundHeightAtPos(targetOfTarget->getX(), targetOfTarget->getY());
          if (targGndHeight < totGndHeight)
            return true;
        }
      default:
        return false;
    }
  };

  CUnit *result = scbw::UnitFinder::getNearestTarget(
    caster->getX() - bounds, caster->getY() - bounds,
    caster->getX() + bounds, caster->getY() + bounds,
    caster, darkSwarmTargetFinder);

  if (result && (result->status & UnitStatus::InAir))
    result = result->orderTarget.unit;

  return result;
}

CUnit* findBest330mmStrikeCannonsTarget(const CUnit *caster, bool isUnderAttack) {
  int bounds;
  if (isUnderAttack)
    bounds = 32 * 9;
  else if (isUmsMode(caster->playerId))
    bounds = 32 * 64;
  else
    bounds = 32 * 32;

  auto _330mmStrikeCannonsTargetFinder = [&caster, &isUnderAttack] (const CUnit *target) -> bool {
    if (!isTargetWorthHitting(target, caster))
      return false;

    if (target->id == UnitId::egg)
      return false;

	if (target->status & (UnitStatus::InAir | UnitStatus::Invincible))
	  return false;

    if (target->id == UnitId::bunker && target->hasLoadedUnit())
      return true;

	const int totalEnemyLife = getTotalEnemyLifeInArea(target->getX(), target->getY(), 96, caster, WeaponId::GaussRifle11_Unused);
    if (500 <= totalEnemyLife)
      return true;

    return false;
  };

  return scbw::UnitFinder::getNearestTarget(
	caster->getX() - bounds, caster->getY() - bounds,
	caster->getX() + bounds, caster->getY() + bounds,
	caster, _330mmStrikeCannonsTargetFinder);
}

} //AI
