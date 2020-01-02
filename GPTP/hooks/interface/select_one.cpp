#include <SCBW/scbwdata.h>

namespace hooks {

bool setSelectOneHook(const CUnit *unit){
	using units_dat::BaseProperty;

	if(BaseProperty[unit->id] & (UnitProperty::Building | UnitProperty::NeutralAccessories)) 
		return false;

	if(unit->isFrozen())
		return false;

	if (unit->id >= 203 && unit->id <= 213) // 203 = Floor Gun Trap, 213 = Right Wall Flame Trap 
		return false;

	switch(unit->id){
	case UnitId::spider_mine:
	case UnitId::egg:
	/*case UnitId::Critter_Rhynadon:
	case UnitId::Critter_Bengalaas:
	case UnitId::Critter_Scantid:
	case UnitId::Critter_Kakaru:
	case UnitId::Critter_Ragnasaur:
	case UnitId::Critter_Ursadon:*/
	case UnitId::Spell_DarkSwarm:
	case UnitId::Spell_DisruptionWeb:
	case UnitId::broodling:
	case UnitId::infested_duran:
	case UnitId::artanis:
		return false;
	case UnitId::mojo:
		if(unit->_padding_0x132)
			return false;
	}

	return true;
}

}