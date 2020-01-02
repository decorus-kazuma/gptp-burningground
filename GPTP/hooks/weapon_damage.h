#pragma once
#include "../SCBW/structures/CUnit.h"

namespace hooks {

void weaponDamageHook(u32     damage,
                      CUnit*  target,
                      u8      weaponId,
                      CUnit*  attacker,
                      u8      attackingPlayer,
                      u8      direction,
                      u8      dmgDivisor);

void injectWeaponDamageHook();

} //hooks
