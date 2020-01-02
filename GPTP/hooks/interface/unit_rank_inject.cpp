#include "unit_rank.h"
#include <hook_tools.h>

namespace {

//Inject with jmpPatch()
const u32 Hook_SetUnitStatusStrText = 0x00425D9B;
const u32 Hook_SetUnitStatusStrTextBack = 0x00425DA0;
void __declspec(naked) setUnitStatusStrTextWrapper() {
  CUnit *unit;
  static char *str;

  __asm {
    PUSHAD
    MOV unit, ESI
  }

  str = (char*)hooks::getUnitRankString(unit);

  __asm {
    POPAD
	MOV	EAX, str
	JMP	Hook_SetUnitStatusStrTextBack
  }
}

} //unnamed namespace

namespace hooks {

void injectGetUnitRankStringHook() {
  jmpPatch(setUnitStatusStrTextWrapper, Hook_SetUnitStatusStrText);
}

} //hooks