#include "kill_count.h"
#include <hook_tools.h>

namespace {

//Inject with jmpPatch()
const u32 Hook_IncrementUnitKillCount = 0x004759C0;
void __declspec(naked) incrementUnitKillCountWrapper() {
  CUnit *unit;

  __asm {
    PUSHAD
    MOV unit, ECX
  }

  hooks::incrementUnitKillCount(unit);

  __asm {
    POPAD
	RETN
  }
}

} //unnamed namespace

namespace hooks {

void injectIncrementUnitKillCountHook() {
  jmpPatch(incrementUnitKillCountWrapper, Hook_IncrementUnitKillCount);
}

} //hooks
