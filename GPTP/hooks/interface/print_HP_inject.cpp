#include "print_HP.h"
#include <hook_tools.h>

const u32 Hook_setTextStr00 = 0x0042640D;
const u32 Hook_setTextStr01 = 0x00426418;
const u32 Hook_setTextStrBack = 0x00426428;

namespace hooks {

void injectSetTextStr() {
  jmpPatch((void*)Hook_setTextStrBack, Hook_setTextStr00);
  jmpPatch((void*)Hook_setTextStrBack, Hook_setTextStr01);
}

} //hooks