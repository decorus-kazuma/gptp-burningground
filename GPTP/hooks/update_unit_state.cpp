#include "update_unit_state.h"
#include "update_status_effects.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
#include <algorithm>

namespace {
//Helper function: Returns true if the unit's HP <= 33%.
static bool unitHpIsInRedZone(const CUnit *unit);
} //unnamed namespace

namespace hooks {
	
  using scbw::isCheatEnabled;
  using CheatFlags::TheGathering;

/// This function regenerates energy for spellcasters and drains energy for
/// cloaking units.
void updateUnitEnergy(CUnit *unit) {
  //Default StarCraft behavior
  using scbw::isCheatEnabled;

  //If the unit is not a spellcaster, don't regenerate energy
  if (!unit->isValidCaster())
    return;
  
  //If the unit is not fully constructed, don't regenerate energy
  if (!(unit->status & UnitStatus::Completed))
    return;

  //Spend energy for cloaked units
  if (unit->status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection)  //If the unit is cloaked
      && !(unit->status & UnitStatus::CloakingForFree)                      //...and must consume energy to stay cloaked (i.e. not under an Arbiter)
      && !isCheatEnabled(CheatFlags::TheGathering))                         //...and the energy cheat is not available
  {
    u16 cloakingEnergyCost = 0;
    if (unit->id == UnitId::TerranGhost
        || unit->id == UnitId::Hero_SarahKerrigan
        || unit->id == UnitId::Hero_InfestedKerrigan
        || unit->id == UnitId::Hero_SamirDuran
        || unit->id == UnitId::Hero_InfestedDuran
        || unit->id == UnitId::Hero_AlexeiStukov)
      cloakingEnergyCost = 10;
    else if (unit->id == UnitId::TerranWraith
           || unit->id == UnitId::Hero_TomKazansky)
      cloakingEnergyCost = 13;

    if (unit->energy < cloakingEnergyCost) {
      if (unit->secondaryOrderId == OrderId::Cloak)
        unit->setSecondaryOrder(OrderId::Nothing2); //Supposedly, immediately decloaks the unit.
      return;
    }
    unit->energy -= cloakingEnergyCost;
  }
  else {
    int maxEnergy;
    if (unit->id == UnitId::dark_archon
        && unit->mainOrderId == OrderId::CompletingArchonSummon
        && !(unit->mainOrderState))
      maxEnergy = 12800;  //50 * 256; Identical to energy amount on spawn
    else
      maxEnergy = unit->getMaxEnergy();

    if (unit->energy != maxEnergy){
		u8 plus = 8;

		if(units_dat::BaseProperty[unit->id] & UnitProperty::Hero)
			plus = 10;
		else if(units_dat::BaseProperty[unit->id] & UnitProperty::Building && unit->status & (UnitStatus::Disabled | UnitStatus::DoodadStatesThing))
			plus = 0;
		else {
			switch(unit->id){
			case UnitId::gateway://게이트웨이는 11씩 참
				plus = 11;
				break;
			case UnitId::dragoon://드라군은 10씩 참
				plus = 10;
				break;
			case UnitId::zealot://질럿이 돌진하고 있는 중에는 마나 안참.
				if(unit->sprite->mainGraphic->animationEx == IscriptAnimation::Unused1 && !isCheatEnabled(TheGathering))
					plus = 0;
				else
					plus = 10;
				break;
			case UnitId::high_templar://수호방패 켰을 때는 안참
				if(unit->building.silo.isReady)
					plus = 0;
				break;
			}
		}

		unit->energy = std::min(unit->energy + plus, maxEnergy);
	}
  }

  //If the unit is currently selected, redraw its graphics
  if (unit->sprite->flags & 8) {
    for (CImage *i = unit->sprite->images.head; i; i = i->link.next)
      if (i->paletteType == 11)
        i->flags |= 1;
  }
}

/// Updates unit timers, regenerates hp and shields, and burns down Terran buildings.
/// Logically equivalent to function @ 0x004EC290
void updateUnitStateHook(CUnit* unit) {
  //Default StarCraft logic

  //Timers
  if (unit->mainOrderTimer)
    unit->mainOrderTimer--;
  if (unit->groundWeaponCooldown)
    unit->groundWeaponCooldown--;
  if (unit->airWeaponCooldown)
    unit->airWeaponCooldown--;
  if (unit->spellCooldown)
    unit->spellCooldown--;

  //Shield regeneration
  if (units_dat::ShieldsEnabled[unit->id]) {
    s32 maxShields = (s32)(units_dat::MaxShieldPoints[unit->id]) << 8;
    if (unit->shields != maxShields) {
      unit->shields = (s32)std::min(unit->shields + 6, maxShields);
      if (unit->sprite->flags & 8) {  //If the unit is currently selected, redraw its graphics
        for (CImage *i = unit->sprite->images.head; i; i = i->link.next)
          if (i->paletteType == 11)
            i->flags |= 1;
      }
    }
  }

  //Supposedly, allows Zerglings to attack after unburrowing without delay.
  if (unit->id == UnitId::zergling || unit->id == UnitId::Hero_DevouringOne)
    if (unit->groundWeaponCooldown == 0)
      unit->orderQueueTimer = 0;

  //Clear the healing flag every frame
  unit->isBeingHealed = 0;

  //Update unit status effects (stim, maelstrom, plague, etc.)
  if (unit->status & UnitStatus::Completed || !(unit->sprite->flags & 0x20)) {
    unit->cycleCounter++;
    if (unit->cycleCounter >= 8) {
      unit->cycleCounter = 0;
      //Call the native function in StarCraft.exe; Do NOT directly call the hook in update_status_effects.cpp
      updateStatusEffects(unit);
    }
  }

  //Only for fully-constructed units and buildings
  if (unit->status & UnitStatus::Completed) {
    //HP regeneration
    if (units_dat::BaseProperty[unit->id] & UnitProperty::RegeneratesHP
        && unit->hitPoints > 0
        && unit->hitPoints != units_dat::MaxHitPoints[unit->id])
    {
      unit->setHp(unit->hitPoints + 4);
    }

    //Update unit energy (energy regen/drain)
    updateUnitEnergy(unit);

    //Recent order timer
    if (unit->recentOrderTimer)
      unit->recentOrderTimer--;

    //Self-destruct timer
    if (unit->removeTimer) {
		unit->removeTimer--;
		if (!unit->removeTimer){
			if(unit->id == UnitId::bunker){
				u8 bonus = (units_dat::MineralCost[unit->id] & 3);
				if(bonus == 3)
					--bonus;
				resources->minerals[unit->playerId] += ((units_dat::MineralCost[unit->id]*3)>>2)+bonus;//0.75(반올림 적용)
			}
			unit->remove();
		}
	}

    //Terran building burn-down
    if (unit->getRace() == RaceId::Terran) {
      //Check if the unit is a grounded or lifted building
      if (unit->status & UnitStatus::GroundedBuilding
          || units_dat::BaseProperty[unit->id] & UnitProperty::FlyingBuilding)
      {
        //...whose current HP is less or equal to 33% of max HP
        if (unitHpIsInRedZone(unit)) {
          unit->damageHp(20, NULL, unit->lastAttackingPlayer);
        }
      }
    }
  }
}

/// Logically equivalent to function @ 0x00466940
Bool32 decrementRemainingBuildTimeHook(CUnit* unit) {

	if(!unit->remainingBuildTime)
		return true;

	if(scbw::isCheatEnabled(CheatFlags::OperationCwal))
		unit->remainingBuildTime = unit->remainingBuildTime >= 16 ? unit->remainingBuildTime-16 : 0;
	else
		--unit->remainingBuildTime;

	return false;
}

} //hooks

namespace {

/**** Helper function definitions. Do not modify anything below this! ****/
const u32 Helper_UnitHpIsInRedZone  = 0x004022C0;
bool unitHpIsInRedZone(const CUnit *unit) {
  static Bool32 result;
  __asm {
    PUSHAD
    MOV ECX, unit
    CALL Helper_UnitHpIsInRedZone
    MOV result, EAX
    POPAD
  }
  return result != 0;
}

}
