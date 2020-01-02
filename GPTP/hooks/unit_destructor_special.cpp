//All functions in this file are heavily involved with memory management.
//Do NOT modify anything unless you really know what you are doing.

#include "unit_destructor_special.h"
#include "../SCBW/api.h"
#include "psi_field.h"
#include "../hook_tools.h"
#include <algorithm>

void killAllHangarUnits(CUnit *unit) {
  while (unit->carrier.inHangarCount--) {
    CUnit *childInside = unit->carrier.inHangarChild;
    unit->carrier.inHangarChild = childInside->interceptor.hangar_link.prev;
    childInside->interceptor.parent = nullptr;
    childInside->remove();
  }

  while (unit->carrier.outHangarCount--) {
    CUnit *childOutside = unit->carrier.outHangarChild;
    unit->carrier.outHangarChild = childOutside->interceptor.hangar_link.prev;
    childOutside->interceptor.parent = nullptr;
    childOutside->interceptor.hangar_link.prev = nullptr;
    childOutside->interceptor.hangar_link.next = nullptr;

    //Kill interceptors only (Scarabs will defuse anyway)
    if (childOutside->id != UnitId::scarab) {
      const u16 deathTimer = scbw::randBetween(15, 45);
      if (childOutside->removeTimer == 0
          || childOutside->removeTimer > deathTimer)
        childOutside->removeTimer = deathTimer;
    }
  }

  unit->carrier.outHangarChild = nullptr;
}

void freeResourceContainer(CUnit *resource) {
  resource->building.resource.gatherQueueCount = 0;

  CUnit *worker = resource->building.resource.nextGatherer;
  while (worker) {
    if (worker->worker.harvest_link.prev)
      worker->worker.harvest_link.prev->worker.harvest_link.next = worker->worker.harvest_link.next;
    else
      resource->building.resource.nextGatherer = worker->worker.harvest_link.next;

    if (worker->worker.harvest_link.next)
      worker->worker.harvest_link.next->worker.harvest_link.prev = worker->worker.harvest_link.prev;

    CUnit *nextWorker = worker->worker.harvest_link.next;
    worker->worker.harvestTarget = nullptr;
    worker->worker.harvest_link.prev = nullptr;
    worker->worker.harvest_link.next = nullptr;
    worker = nextWorker;
  }
}

//Defined in psi_field_util.cpp.
void removePsiField(CUnit *unit);

void unitDestructorSpecialHook(CUnit *unit) {

	const u16 unitId = unit->id;

	switch(unitId){
  //Destroy interceptors and scarabs
	case UnitId::carrier:
	case UnitId::gantrithor:
	case UnitId::reaver:
	case UnitId::warbringer:
		killAllHangarUnits(unit);
		return;
  //Destroy nuclear missiles mid-launch
	case UnitId::ghost:
	case UnitId::sarah_kerrigan:
	case UnitId::Hero_AlexeiStukov:
	case UnitId::Hero_SamirDuran:
	case UnitId::Hero_InfestedDuran:
		if (unit->building.ghostNukeMissile) {
		unit->building.ghostNukeMissile->playIscriptAnim(IscriptAnimation::Death);
		unit->building.ghostNukeMissile = nullptr;
		}
		return;
  //Is a scarab or interceptor
	case UnitId::scarab:
	case UnitId::interceptor:
		if (unit->status & UnitStatus::Completed) {
			if (unit->interceptor.parent) {
			if (unit->interceptor.hangar_link.next)
				unit->interceptor.hangar_link.next->interceptor.hangar_link.prev = unit->interceptor.hangar_link.prev;
			else {
				if (unit->interceptor.isOutsideHangar)
				unit->interceptor.parent->carrier.outHangarChild = unit->interceptor.hangar_link.prev;
				else
				unit->interceptor.parent->carrier.inHangarChild = unit->interceptor.hangar_link.prev;
			}
        
			if (unit->interceptor.isOutsideHangar)
				unit->interceptor.parent->carrier.outHangarCount--;
			else
				unit->interceptor.parent->carrier.inHangarCount--;

			if (unit->interceptor.hangar_link.prev)
				unit->interceptor.hangar_link.prev->interceptor.hangar_link.next = unit->interceptor.hangar_link.next;
			}
			else {
			unit->interceptor.hangar_link.prev = nullptr;
			unit->interceptor.hangar_link.next = nullptr;
			}
		}
		return;
  //Is a Nuclear Silo
	case UnitId::nuclear_silo:
		if (unit->building.silo.nuke) {
			unit->building.silo.nuke->remove();
			unit->building.silo.nuke = nullptr;
		}
		return;
  //Is a Nuclear Missile
	case UnitId::nuclear_missile:
		if (unit->connectedUnit && unit->connectedUnit->id == UnitId::nuclear_silo) {
			unit->connectedUnit->building.silo.nuke = nullptr;
			unit->connectedUnit->building.silo.isReady = false;
		}
		return;
	case UnitId::nydus_canal:{
		CUnit *nydusExit = unit->building.nydusExit;
		if (nydusExit) {
		  unit->building.nydusExit = nullptr;
		  nydusExit->building.nydusExit = nullptr;
		  nydusExit->remove();
		}
	}
		return;
  //Is a harvestable mineral patch or gas building
	case UnitId::mineral_field_1:
	case UnitId::mineral_field_2:
	case UnitId::mineral_field_3:
	case UnitId::UnusedCave:
	case UnitId::UnusedCaveIn:
	case UnitId::UnusedCantina:
	case UnitId::extractor:
	case UnitId::refinery:
	case UnitId::assimilator:
		if(unit->status & UnitStatus::Completed){
			freeResourceContainer(unit);
		}
		return;
	case UnitId::zergling:
	case UnitId::hydralisk:
	case UnitId::ultralisk:
	case UnitId::scourge:
	case UnitId::fenix_zealot:
	case UnitId::fenix_dragoon:
		if(units_dat::MineralCost[unit->id] || units_dat::GasCost[unit->id]){
			CUnit* Gold = scbw::createUnitAtPos(UnitId::Powerup_MineralClusterType2, 11, unit->getX(), unit->getY());
			if(Gold){
				Gold->status |= UnitStatus::Invincible;
				Gold->secondaryOrderPos.x = units_dat::MineralCost[unit->id];
				Gold->secondaryOrderPos.y = units_dat::GasCost[unit->id];
			}
		}
		
		return;
	}

  if (hooks::canMakePsiField(unit->id)) {
    removePsiField(unit);
    return;
  }
  
}

//-------- Actual hooking --------//

void __declspec(naked) unitDestructorSpecialWrapper() {
  CUnit *unit;
  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, EAX
  }

  unitDestructorSpecialHook(unit);

  __asm {
    POPAD
    RETN
  }
}

namespace hooks {

void injectUnitDestructorSpecial() {
  callPatch(unitDestructorSpecialWrapper, 0x004A075F);
}

} //hooks