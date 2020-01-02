#include "max_energy.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>

namespace hooks {

/// Replaces the CUnit::getMaxEnergy() function.
/// Return the amount of maximum energy that a unit can have.
/// Note: 1 energy displayed in-game equals 256 energy.
u16 getUnitMaxEnergyHook(const CUnit* const unit) {
  //Default StarCraft behavior
  using scbw::isCheatEnabled;
  using scbw::getUpgradeLevel;
  using CheatFlags::MedievalMan;

  switch (unit->id) {
    case UnitId::zealot:
	  if (getUpgradeLevel(unit->playerId, UpgradeId::LegEnhancements))
		  return 2560;
	  else
		  return 0;
	  break;
	case UnitId::dragoon:
	  if (scbw::hasTechResearched(unit->playerId, TechId::Consume) || isCheatEnabled(MedievalMan))
		  return techdata_dat::EnergyCost[TechId::Consume]<<8;
	  else
		  return 0;
	  break;
	case UnitId::gateway:
		  return unit->_unknown_0x066;
	  break;
    case UnitId::ghost:
	case UnitId::sarah_kerrigan:
      if (getUpgradeLevel(unit->playerId, UpgradeId::MoebiusReactor))
        return 65535; //255
      break;
	case UnitId::wraith:
	case UnitId::tom_kazansky:
      if (getUpgradeLevel(unit->playerId, UpgradeId::ApolloReactor))
        return 65535; //255
      break;
    case UnitId::science_vessel:
	case UnitId::magellan:
      if (getUpgradeLevel(unit->playerId, UpgradeId::TitanReactor))
        return 65535; //255
      break;
    case UnitId::battlecruiser:
      if (getUpgradeLevel(unit->playerId, UpgradeId::ColossusReactor))
        return 65535; //255
      break;
	case UnitId::medic:
	case UnitId::alexei_stukov:
      if (getUpgradeLevel(unit->playerId, UpgradeId::CaduceusReactor))
        return 65535; //255
      break;
    case UnitId::arcturus_mengsk:
      if (getUpgradeLevel(unit->playerId, UpgradeId::GameteMeiosis))
        return 65535; //255
      break;
    case UnitId::kukulza_guardian:
      if (getUpgradeLevel(unit->playerId, UpgradeId::MetasynapticNode))
        return 65535; //255
      break;
	case UnitId::corsair:
      if (getUpgradeLevel(unit->playerId, UpgradeId::ArgusJewel))
        return 65535; //255
      break;
	case UnitId::hyperion:
	  if (getUpgradeLevel(unit->playerId, UpgradeId::UnusedUpgrade46))
        return 65535; //255
      break;
	case UnitId::dark_archon:
      if (getUpgradeLevel(unit->playerId, UpgradeId::ArgusTalisman))
        return 65535; //255
      break;
    case UnitId::high_templar:
      if (getUpgradeLevel(unit->playerId, UpgradeId::KhaydarinAmulet))
        return 65535; //255
      break;
    case UnitId::arbiter:
      if (getUpgradeLevel(unit->playerId, UpgradeId::KhaydarinCore))
        return 65535; //255
      break;
  }
  
  if (units_dat::BaseProperty[unit->id] & (UnitProperty::Hero | UnitProperty::Building))
    return 65535; //255

  return 51200; //200
}

} //hooks
