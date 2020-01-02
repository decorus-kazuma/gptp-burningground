#include "weapon_begin.h"
#include "../hook_tools.h"

namespace {

void __declspec(naked) weaponBeginWrapper() {
	static CUnit* unit;
	static u8 iscriptAnimation;

	__asm{
		PUSHAD
		MOV		unit, EAX
		MOV		iscriptAnimation, DL
	}

	hooks::weaponBeginHook(unit, iscriptAnimation);

	__asm{
		POPAD
		RETN
	}

}

}

namespace hooks {

void injectWeaponBeginHook() {
  jmpPatch(weaponBeginWrapper, 0x00476ED0);
}

}