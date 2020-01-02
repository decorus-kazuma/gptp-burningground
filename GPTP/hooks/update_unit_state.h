#pragma once
#include <SCBW/structures/CUnit.h>

namespace hooks {

void updateUnitStateHook(CUnit* unit);

Bool32 decrementRemainingBuildTimeHook(CUnit* unit);

void injectUpdateUnitState();

} //hooks
