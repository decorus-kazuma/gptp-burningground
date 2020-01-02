#pragma once
#include <SCBW/structures/CUnit.h>

namespace hooks {

void changeUnitButtonSet(CUnit *unit, u16 buttonSet);

void injectChangeUnitButtonSetHook();

} //hooks