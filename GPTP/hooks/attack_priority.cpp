//Note: Units with smaller attack priority ranks will be attacked first!

#include "attack_priority.h"
#include <SCBW/scbwdata.h>
#include <SCBW/api.h>
#include <cassert>
#include <algorithm>
#include <SCBW/UnitFinder.h>


const int ATTACK_PRIORITY_GROUP_SIZE  = 16;
const int ATTACK_PRIORITY_LEVELS      = 6;

class AttackPriorityData {
  public:
    const CUnit* findBestTarget(const CUnit* attacker) const;
    const CUnit* findRandomTarget() const;

    void addTarget(const CUnit* target, u32 attackPriority);
    void reset();

  private:
    const CUnit* targets[ATTACK_PRIORITY_LEVELS][ATTACK_PRIORITY_GROUP_SIZE];
    unsigned int targetCounts[ATTACK_PRIORITY_LEVELS];
};

//Global variables (bad practice, but it's faster)
AttackPriorityData attackPriorityData;
scbw::UnitFinder attackTargetFinder;

//Helper function declarations
namespace {
bool isTargetPosWithinAttackAngle(s32 x, s32 y, CUnit *unit, u8 weaponId);
bool cannotChaseTarget(const CUnit* unit, const CUnit* target);
//Returns the minimum air/ground weapon range of the @p unit, whichever is smaller.
u32 getMinimumRange(const CUnit* unit);

//Checks whether the @p target should be added to the attack priority group.
//Based on function @ 0x00442DA0
bool checkAttackableTarget(CUnit* unit, const CUnit* target, u32 seekRange, u32 minRange = 0) {
  //Default StarCraft behavior

  //Don't attack if the target is the unit itself.
  if (unit == target)
    return false;

  //Don't attack if the target is covered by fog of war.
  if (!target->sprite->isVisibleTo(unit->playerId))
    return false;

  //Check if the target can be attacked.
  if (!unit->canAttackTarget(target))
    return false;

  //Check if the target is an enemy
  if (!unit->isTargetEnemy(target))
    return false;

  //Check if the target is within the minimum weapon range
  if (minRange != 0) {
    if (unit->getDistanceToTarget(target) <= minRange)
      return false;
  }

  //Check if the target is within the seek range
  if (!(unit->getDistanceToTarget(target) <= seekRange))
    return false;

  CUnit *actualUnit = unit->subunit->isSubunit() ? unit->subunit : unit;
  
  //If the unit can't turn around, check if the target is within the attack angle.
  if (!(actualUnit->status & UnitStatus::CanTurnAroundToAttack)) {
    if (!isTargetPosWithinAttackAngle(target->getX(), target->getY(),
		actualUnit, actualUnit->getGroundWeapon()))
    {
      return false;
    }
  }

  //If the target cannot be chased (i.e. ground vs air, is out of range),
  //don't bother
  if (cannotChaseTarget(unit, target))
    return false;

  return true;
}

} //unnamed namespace

namespace hooks {
  
//Calculates the attack priority of the @p target for the @p attacker.
u32 getAttackPriorityHook(const CUnit* target, const CUnit* attacker) {
  //Default StarCraft behavior

  const CUnit *actualTarget = target;
  
  switch(target->id){
  case UnitId::bunker:{
	  CUnit *firstLoadedUnit = target->getFirstLoadedUnit();
	  if (firstLoadedUnit)
		actualTarget = firstLoadedUnit;
	  break;
  }
  case UnitId::larva:
  case UnitId::egg:
  case UnitId::cocoon:
  case UnitId::lurker_egg:
    return 5;
  case UnitId::aldaris:
	//const u8 damageAngle = target->currentDirection1 - attacker->currentDirection1-128;
	//if(damageAngle < 43 || damageAngle > 213)
		return 0;
  }

  //Normal units
  u32 attackPriority = 4;

  //Workers
  if (units_dat::BaseProperty[actualTarget->id] & UnitProperty::Worker)
    attackPriority = 2;

  //Units that can fight back
  else if (actualTarget->canAttackTarget(attacker))
    attackPriority = 0;

  //Active combat units
  else if (actualTarget->hasWeapon())
    attackPriority = 2;

  else if (actualTarget->mainOrderId == OrderId::MedicHeal1)
    attackPriority = 2;

  else if (actualTarget->status & UnitStatus::IsBuilding)
    attackPriority = 3;

  if (!(actualTarget->status & UnitStatus::Completed) || actualTarget != target)
    attackPriority += 1;
    
  //Units under Disruption Web are less important
  if (attackPriority == 0 && actualTarget->status & UnitStatus::CanNotAttack)
    attackPriority = 1;

  return attackPriority;
}

//Searches for the best attack target nearby for the @p unit.
const CUnit* findBestAttackTargetHook(CUnit* unit) {
  //Default StarCraft behavior

  attackPriorityData.reset();

  int seekRange = unit->getSeekRange() * 32;

  //Bunker bonus
  if (unit->status & UnitStatus::InBuilding)
    seekRange += 64;
  //AI units have use default sight range if it is bigger
  else if (unit->pAI)
    seekRange = std::max(seekRange, units_dat::SightRange[unit->id] * 32);

  int searchRange = seekRange + 64;
  u32 minRange = getMinimumRange(unit);

  attackTargetFinder.search(
    unit->getX() - searchRange, unit->getY() - searchRange,
    unit->getX() + searchRange, unit->getY() + searchRange);

  attackTargetFinder.forEach([unit, seekRange, minRange] (const CUnit* target) {
    if (checkAttackableTarget(unit, target, seekRange, minRange))
      attackPriorityData.addTarget(target, getAttackPriorityHook(target, unit));
  });

  return attackPriorityData.findBestTarget(unit);
}

//Searches for a random attack target nearby for the @p unit.
const CUnit* findRandomAttackTargetHook(CUnit* unit) {
  //Default StarCraft behavior

  attackPriorityData.reset();

  int seekRange = unit->getSeekRange() * 32;

  //Bunker bonus
  if (unit->status & UnitStatus::InBuilding)
    seekRange += 64;

  int searchRange = seekRange + 64;
  
  attackTargetFinder.search(
    unit->getX() - searchRange, unit->getY() - searchRange,
    unit->getX() + searchRange, unit->getY() + searchRange);

  attackTargetFinder.forEach([unit, seekRange] (const CUnit* target) {
    if (checkAttackableTarget(unit, target, seekRange))
      attackPriorityData.addTarget(target, getAttackPriorityHook(target, unit));
  });

  return attackPriorityData.findRandomTarget();
}

} //hooks


//Returns a random target selected from the attack target groups.
//Based on 0x004401B0
const CUnit* AttackPriorityData::findRandomTarget() const {
  //Default StarCraft behavior

  int priority = 0;
  while (targetCounts[priority] == 0) {
    ++priority;
    if (priority >= countof(targetCounts))
      return nullptr;
  }

  if (targetCounts[priority] == 1)
    return targets[priority][0];
  else
    return targets[priority][scbw::random() % targetCounts[priority]];
}

//Returns the best target selected from the attack target groups.
//Based on 0x004405E0
const CUnit* AttackPriorityData::findBestTarget(const CUnit* attacker) const {
  //Default StarCraft behavior

  int priority = 0;
  while (targetCounts[priority] == 0) {
    ++priority;
    if (priority >= countof(targetCounts))
      return nullptr;
  }

  if (targetCounts[priority] == 1)
    return targets[priority][0];

  //Prepare for search
  const CUnit *bestTarget = targets[priority][0];

  if (attacker->pAI && attacker->id == UnitId::scourge) {
    //AI-controlled Scourges auto-target units with the most HP + shields
    u32 bestTargetLife = bestTarget->getCurrentLifeInGame();

    for (unsigned int i = 1; i < targetCounts[priority]; ++i) {
      const CUnit *currentTarget = targets[priority][i];
      const u32 currentTargetLife = currentTarget->getCurrentLifeInGame();

      if (currentTargetLife > bestTargetLife) {
        bestTarget = currentTarget;
        bestTargetLife = currentTargetLife;
      }
    }
  }
  else {
    //Find the nearest target (don't use unit box sizes)
    u32 bestTargetDistance = scbw::getDistanceFast(
      attacker->getX(),   attacker->getY(),
      bestTarget->getX(), bestTarget->getY());

    for (unsigned int i = 1; i < targetCounts[priority]; ++i) {
      const CUnit *currentTarget = targets[priority][i];
      const u32 currentTargetDistance = scbw::getDistanceFast(
        attacker->getX(),       attacker->getY(),
        currentTarget->getX(),  currentTarget->getY());

      if (currentTargetDistance < bestTargetDistance) {
        bestTarget = currentTarget;
        bestTargetDistance = currentTargetDistance;
      }
    }
  }

  return bestTarget;
}


//-------- The following function definitions should not be touched. --------//

//Identical to function @ 0x00440160
void AttackPriorityData::addTarget(const CUnit* target, u32 attackPriority) {
  assert(target);
  assert(attackPriority < countof(targetCounts));

  if (targetCounts[attackPriority] < ATTACK_PRIORITY_GROUP_SIZE) {
    targets[attackPriority][targetCounts[attackPriority]] = target;
    targetCounts[attackPriority] += 1;
  }
}

void AttackPriorityData::reset() {
  for (int i = 0; i < countof(targetCounts); ++i)
    targetCounts[i] = 0;
}

namespace {

//Identical to function @ 0x00475BE0
const u32 Func_IsTargetPosWithinAttackAngle = 0x00475BE0;
bool isTargetPosWithinAttackAngle(s32 x, s32 y, CUnit *unit, u8 weaponId) {
	static u32 result;

  __asm{
	  PUSHAD
	  MOV	ESI, unit
	  MOVZX	EDX, weaponId
	  PUSH	EDX
	  MOV	EDX, x
	  MOV	EAX, y
	  CALL	Func_IsTargetPosWithinAttackAngle
	  MOV	result, EAX
	  POPAD
  }

  return result != 0;

  /*
  assert(unit);

  s32 angle = scbw::getAngle(unit->getX(), unit->getY(), x, y);

  if (unit->id == UnitId::lurker) {
    unit->currentDirection1 = angle;
    return true;
  }

  s32 angleDiff = unit->currentDirection1 - angle;
  if (angleDiff < 0)
    angleDiff += 128;
  if (angleDiff > 128)
    angleDiff = 256 - angleDiff;
  return angleDiff <= weapons_dat::AttackAngle[weaponId];
  */
}

const u32 Func_CannotChaseTarget = 0x004A1140;
bool cannotChaseTarget(const CUnit* unit, const CUnit* target) {
  assert(unit);
  assert(target);
  static u32 result;

  __asm {
    PUSHAD
    MOV ESI, target
    MOV ECX, unit
    CALL Func_CannotChaseTarget
    MOV result, EAX
    POPAD
  }

  return result != 0;
}

//Identical to function @ 0x00440520
u32 getMinimumRange(const CUnit* unit) {
  u8 groundWeapon = unit->getGroundWeapon();
  if (groundWeapon == WeaponId::None && unit->subunit)
    groundWeapon = unit->subunit->getGroundWeapon();

  u8 airWeapon = unit->getAirWeapon();
  if (airWeapon == WeaponId::None && unit->subunit)
    airWeapon = unit->subunit->getAirWeapon();

  if (groundWeapon == WeaponId::None) {
    if (airWeapon == WeaponId::None)
      return 0;
    else
      return weapons_dat::MinRange[airWeapon];
  }
  else {
    if (airWeapon == WeaponId::None)
      return weapons_dat::MinRange[groundWeapon];
    else
      return std::min(weapons_dat::MinRange[groundWeapon], weapons_dat::MinRange[airWeapon]);
  }
}

} //unnamed namespace