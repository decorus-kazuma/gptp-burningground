//Source file for the Apply Upgrade Flags hook module.
//This file is directly responsible for applying movement speed / attack speed
//upgrades to units.
#include "apply_upgrade_flags.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>

namespace hooks {

//This hook function is called when creating a new unit.
void applyUpgradeFlagsToNewUnitHook(CUnit *unit) {
  //Default StarCraft behavior
  using scbw::getUpgradeLevel;

  u8 speedUpgradeLevel = 0, cooldownUpgradeLevel = 0;

  switch (unit->id) {
    case UnitId::jim_raynor_marine:
      speedUpgradeLevel = getUpgradeLevel(unit->playerId, UpgradeId::IonThrusters);
      break;
	case UnitId::yggdrasill:
      speedUpgradeLevel = getUpgradeLevel(unit->playerId, UpgradeId::PneumatizedCarapace);
      break;
    case UnitId::zergling:
      speedUpgradeLevel = getUpgradeLevel(unit->playerId, UpgradeId::MetabolicBoost);
      cooldownUpgradeLevel = getUpgradeLevel(unit->playerId, UpgradeId::AdrenalGlands);
      break;
    case UnitId::hydralisk:
      speedUpgradeLevel = getUpgradeLevel(unit->playerId, UpgradeId::MuscularAugments);
      break;
	case UnitId::firebat:
      speedUpgradeLevel = getUpgradeLevel(unit->playerId, UpgradeId::AnabolicSynthesis);
      break;
    case UnitId::zealot:
      speedUpgradeLevel = getUpgradeLevel(unit->playerId, UpgradeId::LegEnhancements);
      break;
    case UnitId::scout:
      speedUpgradeLevel = getUpgradeLevel(unit->playerId, UpgradeId::GraviticThrusters);
      break;
    case UnitId::shuttle:
      speedUpgradeLevel = getUpgradeLevel(unit->playerId, UpgradeId::GraviticDrive);
      break;
    case UnitId::observer:
      speedUpgradeLevel = getUpgradeLevel(unit->playerId, UpgradeId::GraviticBoosters);
      break;
	case UnitId::infested_terran:
	  speedUpgradeLevel = getUpgradeLevel(unit->playerId, UpgradeId::UnusedUpgrade48);
      break;
    case UnitId::Hero_DevouringOne:
      cooldownUpgradeLevel = 1;
      speedUpgradeLevel = 1;
      break;
    case UnitId::Hero_HunterKiller:
    case UnitId::Hero_FenixZealot:
    case UnitId::Hero_Mojo:
    case UnitId::lurker:
      speedUpgradeLevel = 1;
      break;
    case UnitId::Hero_Torrasque:
      if (scbw::isBroodWarMode())
        speedUpgradeLevel = 1;
      break;
  }

  if (speedUpgradeLevel) {
    unit->status |= UnitStatus::SpeedUpgrade;
    unit->updateSpeed();
	if(unit->id == UnitId::zealot)
		unit->setButtonSet(UnitId::fenix_zealot);
  }
  if (cooldownUpgradeLevel) {
    unit->status |= UnitStatus::CooldownUpgrade;
  }
}

//This function is called when an upgrade is finished, or when transferring a
//unit's ownership from one player to another (via triggers or Mind Control).
void applyUpgradeFlagsToExistingUnitsHook(u8 playerId, u8 upgradeId) {
  //Default StarCraft logic
  bool isSpeedUpgrade = true, isCooldownUpgrade = false;
  u16 validUnitId1 = UnitId::None, validUnitId2 = UnitId::None;

  switch (upgradeId) {
    case UpgradeId::IonThrusters:
	  validUnitId1 = UnitId::jim_raynor_marine;
      break;
    case UpgradeId::PneumatizedCarapace:
	  validUnitId1 = UnitId::yggdrasill;
      break;
    case UpgradeId::MetabolicBoost:
      validUnitId1 = UnitId::zergling;
      break;
    case UpgradeId::MuscularAugments:
      validUnitId1 = UnitId::hydralisk;
      break;
    case UpgradeId::LegEnhancements:
      validUnitId1 = UnitId::zealot;
      break;
    case UpgradeId::GraviticThrusters:
      validUnitId1 = UnitId::scout;
      break;
    case UpgradeId::GraviticDrive:
      validUnitId1 = UnitId::shuttle;
      break;
    case UpgradeId::GraviticBoosters:
      validUnitId1 = UnitId::observer;
      break;
    case UpgradeId::AnabolicSynthesis:
	  validUnitId1 = UnitId::firebat;
      break;
	case UpgradeId::UnusedUpgrade48:
	  validUnitId1 = UnitId::infested_terran;
      break;
    case UpgradeId::AdrenalGlands:
      validUnitId1 = UnitId::zergling;
      isSpeedUpgrade = false;
      isCooldownUpgrade = true;
      break;
    default:
      return;
  }

  for (CUnit *unit = firstPlayerUnit->unit[playerId]; unit; unit = unit->player_link.next) {
    if (unit->id == validUnitId1 || unit->id == validUnitId2) {
      if (isSpeedUpgrade) {
        unit->status |= UnitStatus::SpeedUpgrade;
        unit->updateSpeed();
		if(unit->id == UnitId::zealot)
			unit->setButtonSet(UnitId::fenix_zealot);
      }
      if (isCooldownUpgrade)
        unit->status |= UnitStatus::CooldownUpgrade;
    }
  }
}

} //hooks
