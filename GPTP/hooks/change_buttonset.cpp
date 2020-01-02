#include "change_buttonset.h"
#include <SCBW/structures/CUnit.h>
#include <SCBW/enumerations.h>
#include <SCBW/scbwdata.h>

namespace hooks {

void changeUnitButtonSet(CUnit *unit, u16 buttonSet) {

	if(unit->status & UnitStatus::DoodadStatesThing
		|| unit->lockdownTimer 
		|| unit->stasisTimer 
		|| unit->maelstromTimer){
			if(!(units_dat::BaseProperty[unit->id] & UnitProperty::Building)
				&& buttonSet != UnitId::None)
				return;
	}

	if(unit->id == UnitId::zealot && (unit->status & UnitStatus::SpeedUpgrade))
		unit->currentButtonSet = UnitId::fenix_zealot;
	else
		unit->currentButtonSet = buttonSet;

	return;
}

}