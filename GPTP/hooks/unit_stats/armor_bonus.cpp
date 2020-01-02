#include "armor_bonus.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
#include <algorithm>

namespace hooks {

/// Returns the bonus armor for this unit.
u8 getArmorBonusHook(const CUnit *unit) {
  //Default StarCraft behavior
  using scbw::getUpgradeLevel;

  u8 armorUpg = 0;
  if (scbw::isBroodWarMode()) {
    if (unit->id == UnitId::firebat) {
      if ((units_dat::BaseProperty[unit->id] & UnitProperty::Hero)
          || getUpgradeLevel(unit->playerId, UpgradeId::ChitinousPlating)) {
        armorUpg = 2;
      }
    }
	else if (units_dat::ArmorUpgrade[unit->id] == UpgradeId::UnusedUpgrade45
			&& getUpgradeLevel(unit->playerId, UpgradeId::UnusedUpgrade45)) {
		armorUpg = 1;
	}
  }
  
  return std::min(armorUpg + getUpgradeLevel(unit->playerId, units_dat::ArmorUpgrade[unit->id]), 255);
}

}