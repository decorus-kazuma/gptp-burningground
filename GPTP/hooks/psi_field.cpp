#include "psi_field.h"
#include "../SCBW/scbwdata.h"

namespace hooks {

//Check if the given unit id can generate psi fields
bool canMakePsiField(u16 unitId) {
  //Default StarCraft behavior
	switch(unitId){
	case UnitId::dark_templar_hero:
	case UnitId::pylon:
		return true;
	}

  return false;
}

//Actual state check whether a unit can generate a psi field
bool isReadyToMakePsiField(CUnit *unit) {
  //Default StarCraft behavior
	switch(unit->id){
	case UnitId::dark_templar_hero:
	case UnitId::pylon:
		if(unit->status & UnitStatus::Completed)
			return true;
	}

  return false;
}

} //hooks
