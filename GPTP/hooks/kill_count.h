#pragma once
#include <SCBW/structures/CUnit.h>

namespace hooks {

void incrementUnitKillCount(CUnit *unit);

void injectIncrementUnitKillCountHook();

} //hooks