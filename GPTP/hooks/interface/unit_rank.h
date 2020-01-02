#pragma once
#include <SCBW/structures/CUnit.h>

namespace hooks {

const char* getUnitRankString(CUnit *unit);

void injectGetUnitRankStringHook();

} //hooks