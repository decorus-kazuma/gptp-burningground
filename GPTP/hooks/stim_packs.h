#pragma once
#include "../SCBW/structures/CUnit.h"

namespace hooks {

bool aiUseStimPack(u16 unitId);
void useStimPacksHook(CUnit *unit);
bool canUseStimPacksHook(const CUnit *unit);

void injectStimPacksHooks();

} //hooks
