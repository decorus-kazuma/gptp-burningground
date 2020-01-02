#include "stim_packs.h"
#include "../SCBW/api.h"

namespace hooks {

bool aiUseStimPack(u16 unitId) {

	switch(unitId){
	case UnitId::marine:
	case UnitId::gui_montag:
		return true;
	}

	return false;
}

void useStimPacksHook(CUnit *unit) {
  //Default StarCraft behavior
  if (unit->hitPoints > 3840) {
	unit->sprite->createOverlay(975);
    scbw::playSound(scbw::randBetween(278, 279), unit);
    unit->damageHp(3840);
    if (unit->stimTimer < 40) {
      unit->stimTimer = 40;
      unit->updateSpeed();
    }
  }
}

bool canUseStimPacksHook(const CUnit *unit) {
  //Default StarCraft behavior
  return unit->hitPoints > 3840;
}

} //hooks
