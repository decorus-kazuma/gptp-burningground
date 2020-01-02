#include "update_unit_state.h"
#include <hook_tools.h>

namespace {

//Inject with jmpPatch()
const u32 Hook_UpdateUnitState = 0x004EC290;
void __declspec(naked) updateUnitStateWrapper() {
  CUnit *unit;

  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, EAX
  }

  hooks::updateUnitStateHook(unit);

  __asm {
    POPAD
    RETN
  }
}

//Inject with jmpPatch()
const u32 Hook_DecrementRemainingBuildTime = 0x00466940;
void __declspec(naked) decrementRemainingBuildTimeWrapper() {
  CUnit *unit;
  Bool32 result;

  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, ECX
  }

  result = hooks::decrementRemainingBuildTimeHook(unit);

  __asm {
    POPAD
	MOV	EAX, result
    RETN
  }
}

} //unnamed namespace

namespace hooks {

void injectUpdateUnitState() {
  jmpPatch(updateUnitStateWrapper, Hook_UpdateUnitState);
  jmpPatch(decrementRemainingBuildTimeWrapper, Hook_DecrementRemainingBuildTime);
}

} //hooks
