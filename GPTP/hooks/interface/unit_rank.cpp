#include "unit_rank.h"
#include <SCBW/scbwdata.h>

namespace hooks {

const char* getUnitRankString(CUnit *unit) {

	const u16 unitId = unit->id;

	if(units_dat::BaseProperty[unitId] & (UnitProperty::NeutralAccessories | UnitProperty::RoboticUnit))
		return NULL;

	if(units_dat::GroupFlags[unit->id].isZerg){
		if(!(units_dat::BaseProperty[unitId] & UnitProperty::Hero))
			return NULL;
	}
	else if(!units_dat::GroupFlags[unit->id].isTerran && !units_dat::GroupFlags[unit->id].isProtoss)
		return NULL;

	switch(unitId){
	case UnitId::spider_mine:
	case UnitId::civilian:
		return NULL;
	}

	return (*statTxtTbl)->getString(units_dat::Rank[unitId] + unit->rankIncrease + 1302);
}

}