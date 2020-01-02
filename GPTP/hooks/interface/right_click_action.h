#pragma once
#include <SCBW/structures/CUnit.h>

namespace hooks {

u8 getRightClickActionHook(const CUnit *unit);
bool setRightClickActionHook(const CUnit *unit);

bool getSCVBuild(const CUnit *unit);

void injectRightClickActionHooks();

} //hooks