#pragma once
#include "../SCBW/structures/CUnit.h"

extern const u32 Hook_DoWeaponIscript;

namespace hooks {
	
Bool32 doWeaponIscript(CUnit* unit, u8 weaponId, u32 GndOrAir, u8 animation);

void weaponBeginHook(CUnit* unit, u8 iscriptAnimation);

void injectWeaponBeginHook();

//Bool32 doWeaponIscriptHook(CUnit* unit, u8 weaponId, u32 GndOrAir, u8 iscriptAnimation);

} //hooks