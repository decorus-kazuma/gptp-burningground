#include "kill_count.h"
#include <SCBW/scbwdata.h>

namespace hooks {

void incrementUnitKillCount(CUnit *unit) {

	if(unit->killCount < 255){
		++unit->killCount;

		if(!units_dat::GroupFlags[unit->id].isZerg && !units_dat::Rank[unit->id]
		&& !(units_dat::BaseProperty[unit->id] & (UnitProperty::Building | UnitProperty::RoboticUnit | UnitProperty::Hero))){
			switch(unit->killCount){
			case 5:
				unit->rankIncrease = 4;
				break;
			case 10:
				unit->rankIncrease = 6;
				break;
			case 15:
				unit->rankIncrease = 10;
				break;
			case 20:
				unit->rankIncrease = 15;
				break;
			}
		}
	}

	if(unit->id == UnitId::interceptor && unit->interceptor.parent)
		incrementUnitKillCount(unit->interceptor.parent);

	return;
}

}