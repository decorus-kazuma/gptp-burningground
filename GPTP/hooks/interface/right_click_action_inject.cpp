#include "right_click_action.h"
#include <hook_tools.h>

namespace {
//Inject with jmpPatch()
const u32 Func_GetRightClickAction0 = 0x004E5EA0;
const u32 Func_GetRightClickAction0Back = 0x004E5ED7;
void __declspec(naked) getRightClickAction0() {
  static CUnit *unit;
  static u8 rightClkAct;

  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, EAX
  }

  rightClkAct = hooks::getRightClickActionHook(unit);

  __asm {
	POPAD
    MOV CL, rightClkAct
	JMP Func_GetRightClickAction0Back
  }
}

const u32 Func_GetRightClickAction1 = 0x00455660;
const u32 Func_GetRightClickAction1Back = 0x00455698;
void __declspec(naked) getRightClickAction1() {
  static CUnit *unit;
  static u8 rightClkAct;

  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, EAX
  }

  rightClkAct = hooks::getRightClickActionHook(unit);

  __asm {
	POPAD
    MOV CL, rightClkAct
	JMP Func_GetRightClickAction1Back
  }
}

const u32 Func_SetRightClickAction = 0x004556D6;
const u32 Func_SetRightClickActionBack0 = 0x00455745;
const u32 Func_SetRightClickActionBack1 = 0x00455751;
void __declspec(naked) setRightClickAction() {
  static CUnit *unit;
  static bool noRightClkAct;

  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, EAX
  }

  noRightClkAct = hooks::setRightClickActionHook(unit);

  if(noRightClkAct) {
	  __asm {
		POPAD
		JMP Func_SetRightClickActionBack1
	  }
  }
  else {
	  __asm {
		POPAD
		JMP Func_SetRightClickActionBack0
	  }
  }

}

const u32 Func_BTNSCOND_Rally = 0x004295B7;
const u32 Func_BTNSCOND_Rallyback0 = 0x004295C7;
const u32 Func_BTNSCOND_Rallyback1 = 0x004295DA;
void __declspec(naked) setBTNSCOND_Rally() {
  static CUnit *unit;
  static bool build;

  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, ESI
  }

  build = hooks::getSCVBuild(unit);

  if(build) {
	  __asm {
		POPAD
		JMP Func_BTNSCOND_Rallyback0
	  }
  }
  else {
	  __asm {
		POPAD
		JMP Func_BTNSCOND_Rallyback1
	  }
  }

}

const u32 Func_BTNSCOND_Nothing = 0x004C1110;
const u32 Func_BTNSCOND_Nothingback0 = 0x004C111C;
const u32 Func_BTNSCOND_Nothingback1 = 0x004C112B;
void __declspec(naked) setBTNSCOND_Nothing() {
  static CUnit *unit;
  static bool build;

  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, ESI
  }

  build = hooks::getSCVBuild(unit);

  if(build) {
	  __asm {
		POPAD
		JMP Func_BTNSCOND_Nothingback0
	  }
  }
  else {
	  __asm {
		POPAD
		JMP Func_BTNSCOND_Nothingback1
	  }
  }

}

const u32 Func_BTNSCOND_ReaverStop = 0x004C1300;
const u32 Func_BTNSCOND_ReaverStopback0 = 0x004C130C;
const u32 Func_BTNSCOND_ReaverStopback1 = 0x004C131B;
void __declspec(naked) setBTNSCOND_ReaverStop() {
  static CUnit *unit;
  static bool build;

  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, ESI
  }

  build = hooks::getSCVBuild(unit);

  if(build) {
	  __asm {
		POPAD
		JMP Func_BTNSCOND_ReaverStopback0
	  }
  }
  else {
	  __asm {
		POPAD
		JMP Func_BTNSCOND_ReaverStopback1
	  }
  }

}

const u32 Func_BTNSCOND_CarrierStop = 0x004C14F0;
const u32 Func_BTNSCOND_CarrierStopback0 = 0x004C14FC;
const u32 Func_BTNSCOND_CarrierStopback1 = 0x004C150B;
void __declspec(naked) setBTNSCOND_CarrierStop() {
  static CUnit *unit;
  static bool build;

  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, ESI
  }

  build = hooks::getSCVBuild(unit);

  if(build) {
	  __asm {
		POPAD
		JMP Func_BTNSCOND_CarrierStopback0
	  }
  }
  else {
	  __asm {
		POPAD
		JMP Func_BTNSCOND_CarrierStopback1
	  }
  }

}

const u32 Func_BTNSCOND_Liftoff = 0x004C1735;
const u32 Func_BTNSCOND_Liftoffback0 = 0x004C1741;
const u32 Func_BTNSCOND_Liftoffback1 = 0x004C1752;
void __declspec(naked) setBTNSCOND_Liftoff() {
  static CUnit *unit;
  static bool build;

  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, ESI
  }

  build = hooks::getSCVBuild(unit);

  if(build) {
	  __asm {
		POPAD
		JMP Func_BTNSCOND_Liftoffback0
	  }
  }
  else {
	  __asm {
		POPAD
		JMP Func_BTNSCOND_Liftoffback1
	  }
  }

}

const u32 Func_BTNSCOND_UnloadAll = 0x004C1D7A;
const u32 Func_BTNSCOND_UnloadAllback0 = 0x004C1D86;
const u32 Func_BTNSCOND_UnloadAllback1 = 0x004C1D95;
void __declspec(naked) setBTNSCOND_UnloadAll() {
  static CUnit *unit;
  static bool build;

  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, ESI
  }

  build = hooks::getSCVBuild(unit);

  if(build) {
	  __asm {
		POPAD
		JMP Func_BTNSCOND_UnloadAllback0
	  }
  }
  else {
	  __asm {
		POPAD
		JMP Func_BTNSCOND_UnloadAllback1
	  }
  }

}

const u32 Func_CHK_UNIT_ApplyOtherFlags = 0x004CC52F;
const u32 Func_CHK_UNIT_ApplyOtherFlagsback0 = 0x004CC53B;
const u32 Func_CHK_UNIT_ApplyOtherFlagsback1 = 0x004CC54C;
void __declspec(naked) setCHK_UNIT_ApplyOtherFlags() {
  static CUnit *unit;
  static bool build;

  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, ESI
  }

  build = hooks::getSCVBuild(unit);

  if(build) {
	  __asm {
		POPAD
		JMP Func_CHK_UNIT_ApplyOtherFlagsback0
	  }
  }
  else {
	  __asm {
		POPAD
		JMP Func_CHK_UNIT_ApplyOtherFlagsback1
	  }
  }

}

const u32 Func_OrderAllowed = 0x0046DCFD;
const u32 Func_OrderAllowedback0 = 0x0046DD09;
const u32 Func_OrderAllowedback1 = 0x0046DD21;
void __declspec(naked) setOrderAllowed() {
  static CUnit *unit;
  static bool build;

  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, ESI
  }

  build = hooks::getSCVBuild(unit);

  if(build) {
	  __asm {
		POPAD
		JMP Func_OrderAllowedback0
	  }
  }
  else {
	  __asm {
		POPAD
		JMP Func_OrderAllowedback1
	  }
  }

}

} //unnamed namespace

namespace hooks {

void injectRightClickActionHooks() {
  jmpPatch(getRightClickAction0, Func_GetRightClickAction0);
  jmpPatch(getRightClickAction1, Func_GetRightClickAction1);
  jmpPatch(setRightClickAction, Func_SetRightClickAction);
  jmpPatch(setBTNSCOND_Rally, Func_BTNSCOND_Rally);
  jmpPatch(setBTNSCOND_Nothing, Func_BTNSCOND_Nothing);
  jmpPatch(setBTNSCOND_ReaverStop, Func_BTNSCOND_ReaverStop);
  jmpPatch(setBTNSCOND_CarrierStop, Func_BTNSCOND_CarrierStop);
  jmpPatch(setBTNSCOND_Liftoff, Func_BTNSCOND_Liftoff);
  jmpPatch(setBTNSCOND_UnloadAll, Func_BTNSCOND_UnloadAll);
  jmpPatch(setCHK_UNIT_ApplyOtherFlags, Func_CHK_UNIT_ApplyOtherFlags);
  jmpPatch(setOrderAllowed, Func_OrderAllowed);
}

} //hooks
