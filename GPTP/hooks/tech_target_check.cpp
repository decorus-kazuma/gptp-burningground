//Source file for the Tech Target Check hook module.
#include "tech_target_check.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>

//-------- Helper function declarations. Do NOT modify! --------//
namespace {
bool isInfestableCC(const CUnit *unit);
} //Unnamed namespace

bool isNoDebuffUnit(const CUnit *unit) {
	return unit->id == UnitId::tassadar_zeratul
		|| unit->id == UnitId::norad_ii
		|| unit->id == UnitId::alan_schezar_turret
		|| unit->id == UnitId::mojo
		|| unit->id == UnitId::artanis
		|| unit->id == UnitId::cargo_ship
		|| unit->id == UnitId::mercenary_gunship
		|| unit->id == UnitId::torrasque
		|| unit->id == UnitId::infested_duran
		|| unit->id == UnitId::zeratul
		|| unit->id == UnitId::UnusedRuins
		|| unit->id == UnitId::unclean_one
		|| unit->id == UnitId::kukulza_mutalisk;
}

//-------- Actual hook functions --------//

namespace hooks {

/// Test whether @p castingPlayer can use @p techId on the @p target.
/// If successful, returns zero. If unsuccessful, returns the index of the
/// appropriate error message string in stat_txt.tbl.
u16 getTechUseErrorMessageHook(const CUnit *target, u8 castingPlayer, u16 techId) {
  //Default StarCraft behavior

  if (target->stasisTimer)
    return 900;         //Units in stasis can't be targeted.<0>

  switch (techId) {
    case TechId::Feedback:
      if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
        return 877;     //Unable to target structure.<0>
	  if (!target->isValidCaster() || target->energy <= 256)
        return 1330;    //Must target units with energy.<0>
	  if (isNoDebuffUnit(target))
        return 876;     //Invalid target.<0>
      break;

    case TechId::MindControl:
      if (target->playerId == castingPlayer)
        return 1327;    //Must target enemy units<0>
      if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
        return 877;     //Unable to target structure.<0>
      if (target->id == UnitId::spider_mine
          || target->id == UnitId::larva
          || target->id == UnitId::egg
          || target->id == UnitId::cocoon
          || target->id == UnitId::interceptor
          || target->id == UnitId::scarab
          || target->id == UnitId::lurker_egg
		  || isNoDebuffUnit(target))
        return 876;     //Invalid target.<0>
      break;

    case TechId::Hallucination:
      if (target->id == UnitId::interceptor
		  || isNoDebuffUnit(target))
        return 876;     //Invalid target.<0>
      if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
        return 877;     //Unable to target structure.<0>
      break;
	  
    case TechId::Irradiate:
    case TechId::OpticalFlare:
		if (isNoDebuffUnit(target))
        return 876;     //Invalid target.<0>
    case TechId::DefensiveMatrix:
    case TechId::Restoration:
    case TechId::Consume:
      if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
        return 877;     //Unable to target structure.<0>
      break;

	case TechId::Lockdown:
	  if (!(units_dat::BaseProperty[target->id] & UnitProperty::Mechanical))
        return 881;     //Must target mechanical units.<0>
      if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
        return 877;     //Unable to target structure.<0>
	  if (isNoDebuffUnit(target))
        return 876;     //Invalid target.<0>
      break;


    case TechId::Infestation:
      if (!isInfestableCC(target))
        return 870;     //Must target severely damaged Terran Command Center.<0>
      break;
  }

  return 0;             //No error
}

} //hooks


//-------- Helper function definitions. Do NOT modify! --------//
namespace {

const u32 Func_IsInfestable = 0x00402210;
bool isInfestableCC(const CUnit *unit) {
  static Bool32 result;
  
  __asm {
    PUSHAD
    MOV EDX, unit
    CALL Func_IsInfestable
    MOV result, EAX
    POPAD
  }

  return result != 0;
}

} //Unnamed namespace
