#include "change_buttonset.h"
#include <hook_tools.h>

extern const u32 Func_ChangeUnitButtonSet;

namespace {

//Inject with jmpPatch()
void __declspec(naked) changeUnitButtonSetWrapper() {
  static CUnit *unit;
  static u16 buttonset;

  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, EAX
	MOV buttonset, CX
  }

  hooks::changeUnitButtonSet(unit, buttonset);

  __asm {
    POPAD
    RETN
  }
}

//Inject with jmpPatch()
const u32 Func_UpdateUnitStatsFinishBuilding = 0x004A0222;
const u32 Func_UpdateUnitStatsFinishBuildingBack = 0x004A0263;
void __declspec(naked) updateUnitStatsFinishBuildingWrapper() {
  static CUnit *unit;
  static u16 buttonset;

  __asm {
	MOV	ECX, [EDI+0DCh]
	MOV	AX, [EDI+64h]
    PUSHAD
    MOV unit, EDI
	MOV buttonset, AX
  }

  hooks::changeUnitButtonSet(unit, buttonset);

  __asm {
    POPAD
	JMP	Func_UpdateUnitStatsFinishBuildingBack
  }
}

//Inject with jmpPatch()
const u32 Func_ConvertUnitStats= 0x0049F30D;
const u32 Func_ConvertUnitStatsBack = 0x0049F34C;
void __declspec(naked) convertUnitStatsWrapper() {
  static CUnit *unit;
  static u16 buttonset;

  __asm {
    PUSHAD
    MOV		unit, ESI
	MOV		buttonset, CX
  }

  hooks::changeUnitButtonSet(unit, buttonset);

  __asm {
    POPAD
	JMP	Func_ConvertUnitStatsBack
  }
}

} //unnamed namespace

namespace hooks {

void injectChangeUnitButtonSetHook() {
  jmpPatch(changeUnitButtonSetWrapper, Func_ChangeUnitButtonSet);
  jmpPatch(updateUnitStatsFinishBuildingWrapper, Func_UpdateUnitStatsFinishBuilding);
  jmpPatch(convertUnitStatsWrapper, Func_ConvertUnitStats);
}

} //hooks