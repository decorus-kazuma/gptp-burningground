#include "BG.h"
#include "../tech_target_check.h"
#include "../psi_field.h"
#include <SCBW\api.h>
#include <corona/corona.h>
#include <hooks/game_hooks.h>
#include <hook_tools.h>
#include <shellapi.h>

static const u32 CImgsiz = sizeof(CImage);

extern ColorShiftData colorShiftData[] = {
	{0, nullptr, ""},
	{1, nullptr, "ofire"},
	{2, nullptr, "gfire"},
	{3, nullptr, "bfire"},
	{4, nullptr, "bexpl"},
	{5, nullptr, "trans50"},
	{6, nullptr, "red"},
	{7, nullptr, "green"},
	{8, nullptr, "dfire"}//,
//	{9, nullptr, "sfire"}
};

const char* BG_Screenshot = "BG_ScreenShot_%02d%02d%02d_%02d%02d%02d_%02d.pcx";//나중에 플긴으로 png로 저장됨.
const u32 Hook_TakeScreenshot00 = 0x004D1B55;
const u32 Hook_TakeScreenshot00Back = 0x004D1B5D;
void __declspec(naked) takeScreenshot0() {

	__asm{
		MOVZX	EAX, WORD PTR [EBP-2]
		CDQ
		MOV     ECX, 100
		IDIV    ECX
		PUSH	EDX
		MOVZX	ECX, WORD PTR [EBP-4]
		MOVZX	EDX, WORD PTR [EBP-6]
		JMP		Hook_TakeScreenshot00Back
	}
}

const u32 Hook_TakeScreenshot01 = 0x004D1B9F;
const u32 Hook_TakeScreenshot01Back = 0x004D1BA7;
void __declspec(naked) takeScreenshot1() {
	static char path[260];
	static char* fileName;
	static corona::Image* img;

	__asm{
		PUSHAD
		LEA		EAX, [EBP-114h]
		MOV		fileName, EAX
	}

	GetCurrentDirectory(sizeof(path), path);
	sprintf_s(path, sizeof(path), "%s\\%s", path, fileName);
	img = corona::OpenImage(path, corona::FileFormat::FF_PCX);
	strncpy_s(&path[strlen(path)-2], 3, "ng", 2);
	corona::SaveImage(path, corona::FileFormat::FF_PNG, img);
	strncpy_s(&path[strlen(path)-2], 3, "cx", 2);
	remove(path);
	strncpy_s(&fileName[strlen(fileName)-2], 3, "ng", 2);

	__asm{
		POPAD
		CMP		DWORD PTR DS:[596904h], 3
		JMP		Hook_TakeScreenshot01Back
	}

}

const u32 Hook_DoWeaponIscript = 0x00476CEF;
const u32 Hook_DoWeaponIscriptBack = 0x00476D05;
void __declspec(naked) doWeaponIscript() {

	__asm{
		CMP	[ESI+64h], 0BBh
		JNZ	short _jmp0
		CMP	EDI, 1
		JZ	_jmp2
		CMP	EDI, 2
		JZ	short _jmp1
_jmp0:
		MOV	[ESI+55h], DL
_jmp1:
		MOV	[ESI+56h], DL
		JMP	Hook_DoWeaponIscriptBack

_jmp2:
		MOV	[ESI+55h], DL
		JMP	Hook_DoWeaponIscriptBack
	}
}

void removePsiField(CUnit *unit);
const u32 Hook_MoveUnit0 = 0x004EBACB;
const u32 Hook_MoveUnit0Back = 0x004EBAD0;
void __declspec(naked) moveUnit0() {
	static CUnit *unit;

	__asm{
		PUSHAD
		MOV		unit, ESI
	}

	if(hooks::canMakePsiField(unit->id)){
		removePsiField(unit);
		unit->orderTo(OrderId::InitPsiProvider);
	}

	__asm{
		POPAD
		POP		EDI
		POP		ESI
		POP		EBX
		MOV		ESP, EBP
		JMP		Hook_MoveUnit0Back
	}
}

const u32 Hook_MoveUnit1 = 0x004EBBBB;
const u32 Hook_MoveUnit1Back = 0x004EBBC0;
void __declspec(naked) moveUnit1() {
	static CUnit *unit;

	__asm{
		PUSHAD
		MOV		unit, ESI
	}

	if(hooks::canMakePsiField(unit->id)){
		removePsiField(unit);
		unit->orderTo(OrderId::InitPsiProvider);
	}

	__asm{
		POPAD
		POP		EDI
		POP		ESI
		POP		EBX
		MOV		ESP, EBP
		JMP		Hook_MoveUnit1Back
	}
}

const u32 Hook_AttackApplyCooldown0 = 0x00478BA9;
const u32 Hook_AttackApplyCooldownBack00 = 0x00478BB0;
const u32 Hook_AttackApplyCooldownBack01 = 0x00478BB2;
const u32 Hook_AttackApplyCooldownBack02 = 0x00478BC2;
const u32 Hook_AttackApplyCooldownBack03 = 0x00478BCF;
const u32 Func_getWeaponHitPos = 0x004762C0;
void __declspec(naked) attackApplyCooldown0() {
	static CUnit *unit;
	static CUnit *target;
	static s32 x, y;

	__asm{
		PUSHAD
		MOV		unit, ESI
		MOV		target, EDX
	}

	if(unit->id == UnitId::battlecruiser || unit->id == UnitId::norad_ii|| unit->id == UnitId::mutalisk){
		__asm{
		  LEA ECX, y
		  PUSH ECX
		  LEA EDX, x
		  PUSH EDX
		  PUSH unit
		  CALL Func_getWeaponHitPos //weapon_fire.cpp에 있는 __stdcall 참고해서 다르게 짤 수도 있음.
		}

		if(unit->isDistanceGreaterThanHaltDistance(x, y, 160)){ //거리값은 스크립트 값과 동일
			//계산방식은 0x004D7F64 참고
			__asm{
			  POPAD
			  JMP Hook_AttackApplyCooldownBack02
			}
		}
		else{
			__asm{
			  POPAD
			  JMP Hook_AttackApplyCooldownBack01
			}
		}
	}
	//target 검사는 기본 스타코드에 있어서 조건문에 안넣어도 됨
	else if(unit->id == UnitId::jim_raynor_marine && target->status & UnitStatus::GroundedBuilding && target->id != UnitId::UnusedRuins){

		__asm{
			POPAD
			MOV		EBX, 81h
			MOV     [ebp-1], 1
			JMP		Hook_AttackApplyCooldownBack03
		}
	}
	else if(unit->id == UnitId::jim_raynor_vulture && target->status & UnitStatus::GroundedBuilding && target->id != UnitId::UnusedRuins){

		__asm{
			POPAD
			MOV		EBX, 80h
			MOV     [ebp-1], 1
			JMP		Hook_AttackApplyCooldownBack03
		}
	}
	else {
		__asm{
			POPAD
			TEST    byte ptr [edx+0DCh], 4
			JMP		Hook_AttackApplyCooldownBack00
		}
	}
}

const u32 Hook_AttackApplyCooldown1 = 0x00478CE4;
const u32 Hook_AttackApplyCooldownBack10 = 0x00478CF3;
const u32 Hook_AttackApplyCooldownBack11 = 0x00478D02;
void __declspec(naked) attackApplyCooldown1() {
	static CUnit *unit;
	static CUnit *target;

	__asm{
		MOV		unit, ESI
		MOV     ESI, [ECX+1Ch]
		TEST    ESI, ESI
		JNZ     short jump
		JMP		Hook_AttackApplyCooldownBack11
jump:
		XOR     EBX, EBX
		PUSHAD
	}
	
	//target 검사는 기본 스타코드에 있어서 조건문에 안넣어도 됨
	target = unit->orderTarget.unit;

	if(unit->id == UnitId::jim_raynor_marine && target->status & UnitStatus::GroundedBuilding && target->id != UnitId::UnusedRuins){

		__asm{
			POPAD
			MOV		EBX, 0Ah
			JMP		Hook_AttackApplyCooldownBack10
		}
	}
	else if(unit->id == UnitId::jim_raynor_vulture && target->status & UnitStatus::GroundedBuilding && target->id != UnitId::UnusedRuins){

		__asm{
			POPAD
			MOV		EBX, 7
			JMP		Hook_AttackApplyCooldownBack10
		}
	}

	else {
		__asm{
			POPAD
			MOV     BL, [EBP-1]
			ADD     BL, 4
			JMP		Hook_AttackApplyCooldownBack10
		}
	}
}


//따로 훅 만들기 귀찮에서 인젝트 하나에 때려박음. 나중에 건질거 있으면 따로 훅으로 만들면 되고
const u32 Hook_OrdersEntries_ReaverStop = 0x004EC7EC;
const u32 Orders_ReaverStop = 0x004654B0;
const u32 Hook_OrdersEntries_Retn = 0x004EC4FF;
void __declspec(naked) ordersEntries_ReaverStop() {
	
	__asm{
		MOV		AX, WORD PTR [ESI+64h]
		CMP		AX, 53h
		JZ		short _reaverStop
		CMP		AX, 51h
		JNZ		short _Retn
_reaverStop:
		PUSH	ESI
		CALL	Orders_ReaverStop
_Retn:
		JMP		Hook_OrdersEntries_Retn
	}

}

const u32 Hook_CMDRECV_PlaceBuildingAllowed = 0x0048DBD0;
const u32 Hook_CMDRECV_PlaceBuildingAllowed_Back = 0x0048DBD6;
void __declspec(naked) CMDRECV_PlaceBuildingAllowed() {

	__asm{
		CMP		WORD PTR [ECX+64h], 68h
		JZ		short _Retn
		PUSH	EBX
		PUSH	ESI
		MOV		ESI, ECX
		MOV		bl, dl
		JMP		Hook_CMDRECV_PlaceBuildingAllowed_Back
_Retn:
		MOV		EAX, 1
		RETN
	}
}

const u32 Hook_CreateBuildingFlames = 0x00499623;
const u32 Hook_CreateBuildingFlamesBack = 0x0049962B;
const u32 Hook_CreateDamageOverlay0 = 0x0049943C;
const u32 Hook_CreateDamageOverlay0Back = 0x00499444;
const u32 Hook_CreateDamageOverlay1 = 0x00499505;
const u32 Hook_CreateDamageOverlay1Back = 0x0049950F;
void __declspec(naked) createDamageOverlay1() {

	__asm{
		MOVSX		ECX, BYTE PTR [EAX]
		JMP			Hook_CreateDamageOverlay1Back
	}
}

const u32 Hook_SetImageDirection = 0x004D5F83;
const u32 Hook_SetImageDirectionBack0 = 0x004D5F88;
const u32 Hook_SetImageDirectionBack1 = 0x004D6002;
void __declspec(naked) setImageDirection() {

	__asm{
		CMP		WORD PTR [ESI+8], 1A8h
		JZ		SHORT _Back1
		MOV     AL, [ESI+0Ch]
		TEST    AL, AL
		JMP		Hook_SetImageDirectionBack0
_Back1:
		JMP		Hook_SetImageDirectionBack1
	}
}

const u32 Hook_StasisFieldHit = 0x004F68EF;
const u32 Hook_StasisFieldHitBack0 = 0x004F68F4;
const u32 Hook_StasisFieldHitBack1 = 0x004F6901;
void __declspec(naked) StasisFieldHit() {
	static CUnit *unit;

	__asm{
		PUSHAD
		MOV		unit, ECX
	}

	if((unit->status & UnitStatus::InAir) || isNoDebuffUnit(unit)){
		__asm{
			POPAD
			jmp		Hook_StasisFieldHitBack1
		}
	}
	else{
		__asm{
			POPAD
			PUSH	EDI
			jmp		Hook_StasisFieldHitBack0
		}
	}
}

const u32 Hook_EMPShockwaveHit0 = 0x00492C40;
const u32 Hook_EMPShockwaveHit0Back0 = 0x00492C46;
const u32 Hook_EMPShockwaveHit0Back1 = 0x00492C48;
void __declspec(naked) EMPShockwaveHit0() {

	__asm{
		CMP		WORD PTR [EAX+64h], 9Eh
		JZ		short _jmp
		TEST    [eax+0DCh], EDI
		JMP		Hook_EMPShockwaveHit0Back0
_jmp:
		JMP		Hook_EMPShockwaveHit0Back1
	}
}

const u32 Hook_EMPShockwaveHit1 = 0x00492C57;
const u32 Hook_EMPShockwaveHit1Back0 = 0x00492C73;
const u32 Hook_EMPShockwaveHit1Back1 = 0x00492C61;
void __declspec(naked) EMPShockwaveHit1() {
	static CUnit *unit;

	__asm{
		PUSHAD
		MOV		EBP, ESP
		MOV		unit, EAX
	}

	if((unit->status & UnitStatus::Invincible) || isNoDebuffUnit(unit)){
		__asm{
			POPAD
			jmp		Hook_EMPShockwaveHit1Back0
		}
	}
	else if(unit->id == UnitId::broodling 
		|| unit->id == UnitId::zealot
		|| unit->id == UnitId::dragoon
		|| unit->id == UnitId::gateway){
			
		if(unit->shields < 64000)
			unit->shields = 0;
		else
			unit->shields -= 64000;

		__asm{
			POPAD
			jmp		Hook_EMPShockwaveHit1Back1
		}
	}
	else{
		
		if(unit->energy < 25600)
			unit->energy = 0;
		else
			unit->energy -= 25600;

		if(unit->shields < 64000)
			unit->shields = 0;
		else
			unit->shields -= 64000;

		__asm{
			POPAD
			jmp		Hook_EMPShockwaveHit1Back1
		}
	}
}

const u32 Hook_EnsnareHit = 0x004F46DF;
const u32 Hook_EnsnareHitBack0 = 0x004F46E4;
const u32 Hook_EnsnareHitBack1 = 0x004F46EC;
void __declspec(naked) EnsnareHit() {
	static CUnit *unit;

	__asm{
		PUSHAD
		MOV		unit, ECX
	}

	if(isNoDebuffUnit(unit)){
		__asm{
			POPAD
			jmp		Hook_EnsnareHitBack1
		}
	}
	else{
		__asm{
			POPAD
			PUSH	EDI
			jmp		Hook_EnsnareHitBack0
		}
	}
}

const u32 Hook_BroodlingHit0 = 0x0048B9C8;
const u32 Hook_BroodlingHitBack0 = 0x0048B9FC;
void __declspec(naked) BroodlingHit0() {

	__asm{
		MOV EAX,DWORD PTR SS:[EBP+8]
		JMP	Hook_BroodlingHitBack0
	}

}

const u32 Hook_BroodlingHit1 = 0x004F494B;
const u32 Hook_BroodlingHitBack1 = 0x004F495B;

const u32 Hook_BroodlingHit2 = 0x004F4966;
const u32 Hook_BroodlingHitBack2 = 0x004F496C;

const u32 Hook_BroodlingHit3 = 0x004F4980;
const u32 Hook_BroodlingHitBack3 = 0x004F49E2;

const u32 Hook_BroodlingHit4 = 0x004F4A9B;
const u32 Hook_BroodlingHitBack4 = 0x004F4ADA;
void __declspec(naked) BroodlingHit4() {

	__asm{
		POP	EBX
		JMP	Hook_BroodlingHitBack4
	}

}

const u32 Hook_PlagueHit = 0x004F4B07;
const u32 Hook_PlagueHitBack0 = 0x004F4B10;
const u32 Hook_PlagueHitBack1 = 0x004F4B20;
const u32 CreatePlagueOverlay = 0x004F4550;
void __declspec(naked) PlagueHit() {
	static CUnit *unit;

	__asm{
		PUSHAD
		MOV		unit, EDI
	}

	if(isNoDebuffUnit(unit)){
		__asm{
			POPAD
			jmp		Hook_PlagueHitBack1
		}
	}
	else{
		__asm{
			POPAD
			CALL	CreatePlagueOverlay
			jmp		Hook_PlagueHitBack0
		}
	}
}

const u32 Hook_CorrosiveAcidHit = 0x004F4B07;
const u32 Hook_CorrosiveAcidHitBack0 = 0x004F468F;
const u32 Hook_CorrosiveAcidHitBack1 = 0x004F46B5;
void __declspec(naked) CorrosiveAcidHit() {
	static CUnit *unit;

	__asm{
		PUSHAD
		MOV		unit, EBX
	}

	if(isNoDebuffUnit(unit)){
		__asm{
			POPAD
			jmp		Hook_CorrosiveAcidHitBack1
		}
	}
	else{
		__asm{
			POPAD
			mov     eax, [ebx+0DCh]
			jmp		Hook_CorrosiveAcidHitBack0
		}
	}
}

const u32 Hook_RecallUnitsCB = 0x0049444A;
const u32 Hook_RecallUnitsCBBack0 = 0x00494452;
const u32 Hook_RecallUnitsCBBack1 = 0x0049445E;
void __declspec(naked) recallUnitsCB() {
	static CUnit *unit;
	//EDX는 시전유닛(아비터 같은)

	__asm{
		PUSHAD
		MOV		unit, ECX
	}

	if((unit->status & UnitStatus::IsHallucination) || isNoDebuffUnit(unit)){
		__asm{
			POPAD
			jmp		Hook_RecallUnitsCBBack1
		}
	}
	else{
		__asm{
			POPAD
			jmp		Hook_RecallUnitsCBBack0
		}
	}
}

const u32 Hook_SiegeTank_SelfDestructProc = 0x00463F92;
const u32 Hook_SiegeTank_SelfDestructProcBack0 = 0x00463FA9;
const u32 Hook_SiegeTank_SelfDestructProcBack1 = 0x00463FB4;
void __declspec(naked) siegeTank_SelfDestructProc() {
	//EAX 시즈탱크 유닛
	//ECX 검색된 유닛

	__asm{
		CMP		ECX, EAX
		JZ      short _jmp
		CMP		WORD PTR [ECX+64h], 9Eh
		JZ      short _jmp
		MOV	    ECX, [ECX+0DCh]
		TEST    CL, 2
		JZ      short _jmp
		TEST    ECX, 200000h
		JNZ     short _jmp
		JMP		Hook_SiegeTank_SelfDestructProcBack0
_jmp:
		JMP		Hook_SiegeTank_SelfDestructProcBack1
	}
}


const u32 Hook_KillTargetUnitCheck2 = 0x00479863;
const u32 Orders_SapUnit = 0x004788E0;
const u32 Hook_KillTargetUnitCheck2Back0 = 0x0047986A;
const u32 Hook_KillTargetUnitCheck2Back1 = 0x0047986F;
void __declspec(naked) killTargetUnitCheck2() {
	static CUnit *unit;

	__asm{
		MOV		DWORD PTR [EBX+8], 0
		PUSHAD
		MOV		unit, EBX
	}

	if(unit->id == UnitId::infested_terran){
		__asm{
			POPAD
			MOV		[EBX+5Ch], EBX
			MOV		BYTE PTR [EBX+4Eh], 0
			PUSH	EAX
			MOV		EAX, currentScriptSubunit
			MOV		[EAX], EBX
			MOV		EAX, currentIscriptThingy
			MOV		[EAX], EBX
			POP		EAX
			CALL	Orders_SapUnit
			JMP		Hook_KillTargetUnitCheck2Back1
		}
	}
	else{
		__asm{
			POPAD
			JMP		Hook_KillTargetUnitCheck2Back0
		}
	}

}

const u32 Hook_Orders_SapUnit0 = 0x004789F4;
const u32 Hook_Orders_SapUnitBack0 = 0x004789FA;
void __declspec(naked) orders_SapUnit0() {

	__asm{
		MOV		EDX, [ESI+0Ch]
		JMP		Hook_Orders_SapUnitBack0
	}

}

const u32 Hook_Orders_SapUnit1 = 0x00478928;
const u32 Hook_Orders_SapUnitBack10 = 0x00478933;
const u32 Hook_Orders_SapUnitBack11 = 0x0047894D;
void __declspec(naked) orders_SapUnit1() {
	static CUnit *unit;
	static CUnit *target;

	__asm{
		PUSHAD
		MOV	unit, ESI
		MOV	target, EBX
	}

	if(unit == target || unit->canAttackTarget(target, true)){
		__asm{
			POPAD
			JMP	Hook_Orders_SapUnitBack11
		}
	}
	else{
		__asm{
			POPAD
			JMP	Hook_Orders_SapUnitBack10
		}
	}

}

const u32 Hook_GetPlayerDefaultRefineryUnitType = 0x004320DA;
void __declspec(naked) getPlayerDefaultRefineryUnitType() {

	__asm{
		CMP		AL, 2
		MOV		EAX, 9Dh
		JZ		short _jmp
		MOV		EAX, 6Eh
_jmp:
		RETN
	}
}

const u32 Hook_AI_GetGeyserState = 0x00432718;
const u32 Hook_AI_GetGeyserStateBack = 0x00432734;
void __declspec(naked) AI_GetGeyserState() {

	__asm{
		CMP		CL, 2
		MOV		ECX, 9Dh
		JZ		short _jmp
		MOV		ECX, 6Eh
_jmp:
		JMP		Hook_AI_GetGeyserStateBack
	}
}

const u32 Hook_AI_BuildGasBuildings0 = 0x004336EF;
const u32 Hook_AI_BuildGasBuildings0Back0 = 0x004336F6;
const u32 Hook_AI_BuildGasBuildings0Back1 = 0x00433714;
void __declspec(naked) AI_BuildGasBuildings() {

	__asm{
		CMP		AL, 2
		MOV     EAX, [EBP-8]
		JZ		short _jmp
		JMP		Hook_AI_BuildGasBuildings0Back0
_jmp:
		MOV		ESI, 9Dh
		JMP		Hook_AI_BuildGasBuildings0Back1
	}
}

const u32 Hook_AI_BuildGasBuildings1 = 0x00433717;
const u32 Hook_AI_BuildGasBuildings1Back = 0x00433720;

const u32 Hook_AI_BuildUnitAtTown = 0x00448434;
const u32 Hook_AI_BuildUnitAtTownBack = 0x0044844F;
void __declspec(naked) AI_BuildUnitAtTown() {

	__asm{
		CMP		AL, 2
		MOV		EAX, 9Dh
		JZ		short _jmp
		MOV		EAX, 6Eh
_jmp:
		JMP		Hook_AI_BuildUnitAtTownBack
	}
}

const u32 Hook_AI_BuildSupplyUnits = 0x00433753;
const u32 Hook_AI_BuildSupplyUnitsBack = 0x00433770;
void __declspec(naked) AI_BuildSupplyUnits() {

	__asm{
		MOV		[EBP-0Ch], ESI 
		CMP		AL, 2
		MOV		[EBP-4], 9Ch
		JZ		short _jmp
		MOV		[EBP-4], 6Dh
_jmp:
		JMP		Hook_AI_BuildSupplyUnitsBack
	}
}

const u32 Hook_OrderAllMoveToRechargeShieldsProc = 0x00493915;
const u32 Hook_OrderAllMoveToRechargeShieldsProcBack = 0x0049391A;
const u32 orderToUnit = 0x004752B0;
void __declspec(naked) orderAllMoveToRechargeShieldsProc() {

	__asm{
		CMP		BYTE PTR [ESI+0A6h], 44h
		JNZ		short _jmp
		MOV		BYTE PTR [ESI+0A6h], 17h
_jmp:
		CALL	orderToUnit
		JMP		Hook_OrderAllMoveToRechargeShieldsProcBack
	}
}


const u32 Hook_OrdersEntries_Building = 0x0047C85F;
const u32 Hook_OrdersEntries_BuildingBack0 = 0x0047C882;
const u32 Hook_OrdersEntries_BuildingBack1 = 0x0047C89B;
const u32 Hook_OrdersEntries_BuildingBack2 = 0x0047C86B;
void __declspec(naked) ordersEntries_Building() {

	__asm{
		CMP		EAX, 86h
		JZ		short _jmp0
		CMP		EAX, 47h
		JZ		short _jmp1
		CMP		EAX, 56h
		JZ		short _jmp1
		CMP		EAX, 0ACh
		JZ		short _jmp1
		JMP		Hook_OrdersEntries_BuildingBack1
_jmp0:
		JMP		Hook_OrdersEntries_BuildingBack0
_jmp1:
		JMP		Hook_OrdersEntries_BuildingBack2
	}
}

const u32 Hook_OpcodeCases_Attack = 0x004D80C6;
const u32 Hook_OpcodeCases_AttackBack0 = 0x004D8108;
const u32 Hook_OpcodeCases_AttackBack1 = 0x004D80CF;
const u32 Hook_OpcodeCases_AttackBack2 = 0x004D7500;
void __declspec(naked) opcodeCases_Attack() {
	static CUnit *unit, *target;

	__asm{
		PUSHAD
		MOV		unit, EAX
		MOV		target, ECX
	}

	if(target->status & UnitStatus::InAir){
		if(unit->getAirWeapon() != WeaponId::None){
			__asm{
				POPAD
				JMP		Hook_OpcodeCases_AttackBack1
			}
		}
		else{
			__asm{
				POPAD
				JMP		Hook_OpcodeCases_AttackBack2
			}
		}
	}
	else{
		if(unit->getGroundWeapon() != WeaponId::None){
			__asm{
				POPAD
				JMP		Hook_OpcodeCases_AttackBack0
			}
		}
		else{
			__asm{
				POPAD
				JMP		Hook_OpcodeCases_AttackBack2
			}
		}
	}
}


const u32 Hook_InitializeUnitState = 0x0049ED96;
const u32 Hook_InitializeUnitStateBack = 0x0049EDF5;
void __declspec(naked) initializeUnitState() {

	__asm{
		mov     edx, [ebp-4]
		mov     byte ptr [edi+90h], 8
		mov     eax, currentScriptSubunit
		mov     [eax], edx
		jmp		Hook_InitializeUnitStateBack
	}
}

const u32 Hook_InitializeUnitMembers = 0x004A03BC;
const u32 Hook_InitializeUnitMembersBack = 0x004A03C1;
const u32 InitializeUnitState = 0x0049ECF0;
void __declspec(naked) initializeUnitMembers() {
	static CUnit *unit;

	__asm{
		CALL	InitializeUnitState
		PUSHAD
		MOV		unit, ESI
	}

	if(!(unit->status & UnitStatus::GroundedBuilding))
		unit->shields = units_dat::MaxShieldPoints[unit->id]<<8;

	//원래 스타코딩에서는 쉴드배터리는 마나 50 고정
	if(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster)
		unit->energy = unit->getMaxEnergy()>>2;

	__asm{
		POPAD
		JMP		Hook_InitializeUnitMembersBack
	}
}

const u32 Hook_ConvertUnitStats = 0x0049F20B;
const u32 Hook_ConvertUnitStatsBack = 0x0049F211;
void __declspec(naked) convertUnitStats() {
	static CUnit *unit;

	__asm{
		MOV     EDI, [ESI+0DCh]
		PUSHAD
		MOV		unit, ESI
	}

	if(units_dat::ShieldsEnabled[unit->id]){
		if(units_dat::MaxShieldPoints[unit->displayedUnitId])//0으로 나누지 말라고 따로 조건문 넣음
			unit->shields = ((unit->shields>>8) * units_dat::MaxShieldPoints[unit->id] / units_dat::MaxShieldPoints[unit->displayedUnitId])<<8;
		else
			unit->shields = NULL;
	}

	__asm{
		POPAD
		JMP Hook_ConvertUnitStatsBack
	}
}

const u32 Hook_ConvertUnit = 0x004A004E;
const u32 Hook_ConvertUnitBack0 = 0x004A0055;
const u32 Hook_ConvertUnitBack1 = 0x004A0069;
const u32 UnitConstructor = 0x004A06C0;
const u32 UnitDestructor = 0x004A0990;
const u32 GetImageAttackFrame = 0x00401DF0;
const u32 SetSpriteMainImgOffset = 0x00401E40;
const u32 toggleUnitPath = 0x004E42A0;
void __declspec(naked) convertUnit() {

	__asm{
		MOV		EDI, [ESI+70h]
		TEST	EDI, EDI
		JZ		short _jmp
		JMP		Hook_ConvertUnitBack0
_jmp:
		PUSHAD
		MOV		EBX, ESI
		MOVZX	ESI, WORD PTR [EBX+28h]
		MOVZX	EDI, WORD PTR [EBX+64h]
		CMP		EDI, 69h
		JA		short _Back1
		MOV		AX, WORD PTR [EDI*2+6607C0h]
		CMP		AX, 0E4h
		JZ		short _Back1
		MOV		ECX, 0
		MOVZX	EDX, BYTE PTR [EBX+4Ch]
		PUSH	ECX
		PUSH	EDX
		MOVZX	EDX, WORD PTR [EBX+2Ah]
		MOVZX	EDI, AX
		CALL	UnitConstructor
		MOV		ESI, EAX
		TEST	ESI, ESI
		JNZ		short _convertUnit2
		MOV		ECX, EBX
		CALL	UnitDestructor
_Back1:
		POPAD
		JMP		Hook_ConvertUnitBack1

_convertUnit2:
		MOV		[EBX+70h], ESI
		MOV		[ESI+70h], EBX
		PUSH    EBP
		MOV     EBP, ESP
		SUB     ESP, 8
		MOV		ECX, [EBX+0Ch]
		PUSH	2
		XOR		EDI, EDI
		LEA		EAX, [EBP-8]
		CALL	GetImageAttackFrame
		MOV		EAX, [EBP-4]
		MOV		ECX, [EBP-8]
		PUSH	EAX
		MOV     EAX, [ESI+0Ch]
		PUSH    ECX
		CALL    SetSpriteMainImgOffset
		MOV		ESP, EBP
		POP		EBP
		MOV		EAX, [ESI+0Ch]
		AND		BYTE PTR [EAX+0Eh], 0DFh
		CALL	toggleUnitPath
		MOV		EAX, [ESI+0Ch]
		MOV     BL, [ESI+105h]
		MOV     BYTE PTR [ESI+97h], 0
		CMP     BYTE PTR [EAX+0Dh], 0Ch
		SETB    CL
		XOR     CL, BL
		AND     CL, 1
		MOV     AL, BL
		XOR     AL, CL
		MOV     [ESI+105h], AL
		POPAD
		JMP		Hook_ConvertUnitBack1 
	}

}

const u32 Hook_Sub_496030_0 = 0x004960F5;
const u32 PlayImageIscript = 0x004D8470;
const u32 Hook_Sub_496030_0Back = 0x004960FE;
void __declspec(naked) sub_496030_0() {
	static CUnit *unit;

	__asm{
		PUSHAD
		MOV		EAX, [EBP+8]
		MOV		unit, EAX
	}

	if(unit->id == UnitId::dark_archon && unit->unusedTimer){
		__asm{
			POPAD
			PUSH	0Eh
		}
	}else{
		__asm{
			POPAD
			PUSH	0Ch
		}
	}
	
	__asm{
		MOV		ECX, EDI
		CALL	PlayImageIscript
		JMP		Hook_Sub_496030_0Back
	}
}

const u32 Hook_Sub_496030_1 = 0x00496124;
const u32 Hook_Sub_496030_1Back = 0x0049612D;
void __declspec(naked) sub_496030_1() {
	static CUnit *unit;

	__asm{
		PUSHAD
		MOV		EAX, [EBP+8]
		MOV		unit, EAX
	}

	if(unit->id == UnitId::dark_archon && unit->unusedTimer){
		__asm{
			POPAD
			PUSH	0Ah
		}
	}else{
		__asm{
			POPAD
			PUSH	0Bh
		}
	}
	
	__asm{
		MOV		ECX, ESI
		CALL	PlayImageIscript
		JMP		Hook_Sub_496030_1Back
	}
}

const u32 Hook_PsiField00 = 0x00493600;
const u32 Hook_PsiField00Back0 = 0x00493611;
const u32 Hook_PsiField00Back1 = 0x00493631;
void __declspec(naked) psiField0() {
	static CUnit *unit;

	__asm{
		PUSHAD
		MOV		unit, ESI
	}

	if(hooks::isReadyToMakePsiField(unit)){
		__asm{
			POPAD
			JMP		Hook_PsiField00Back0
		}
	}
	else{
		__asm{
			POPAD
			JMP		Hook_PsiField00Back1
		}
	}

}

const u32 Hook_PsiField01 = 0x00494050;
const u32 Hook_PsiField01Back0 = 0x00494068;
const u32 Hook_PsiField01Back1 = 0x004940DF;
void __declspec(naked) psiField1() {
	static CUnit *unit;

	__asm{
		PUSHAD
		MOV		unit, ESI
	}

	if(hooks::isReadyToMakePsiField(unit)){
		__asm{
			POPAD
			JMP		Hook_PsiField01Back0
		}
	}
	else{
		__asm{
			POPAD
			JMP		Hook_PsiField01Back1
		}
	}

}

const u32 Hook_PsiField02 = 0x004E3935;
const u32 Hook_PsiField02Back0 = 0x004E393C;
const u32 Hook_PsiField02Back1 = 0x004E3961;
void __declspec(naked) psiField2() {
	static u16 unitId;

	__asm{
		PUSHAD
		MOV		unitId, CX
	}

	if(hooks::canMakePsiField(unitId)){
		__asm{
			POPAD
			JMP		Hook_PsiField02Back0
		}
	}
	else{
		__asm{
			POPAD
			JMP		Hook_PsiField02Back1
		}
	}

}

const u32 Hook_PsiField03 = 0x004E3BCD;
const u32 Hook_PsiField03Back0 = 0x004E3BD9;
const u32 Hook_PsiField03Back1 = 0x004E3C63;
void __declspec(naked) psiField3() {
	static u16 unitId;

	__asm{
		PUSHAD
		MOV		AX, [ESI+64h]
		MOV		unitId, AX
	}

	if(hooks::canMakePsiField(unitId)){
		__asm{
			POPAD
			JMP		Hook_PsiField03Back0
		}
	}
	else{
		__asm{
			POPAD
			JMP		Hook_PsiField03Back1
		}
	}

}

const u32 Hook_PsiField04 = 0x004E6239;
const u32 Hook_PsiField04Back0 = 0x004E6249;
const u32 Hook_PsiField04Back1 = 0x004E624E;
void __declspec(naked) psiField4() {
	static CUnit *unit;

	__asm{
		PUSHAD
		MOV		unit, EDI
	}

	if(hooks::isReadyToMakePsiField(unit)){
		__asm{
			POPAD
			JMP		Hook_PsiField04Back0
		}
	}
	else{
		__asm{
			POPAD
			JMP		Hook_PsiField04Back1
		}
	}

}

const u32 Hook_PsiField05 = 0x004E62D4;
const u32 Hook_PsiField05Back0 = 0x004E62E5;
const u32 Hook_PsiField05Back1 = 0x004E62B3;
void __declspec(naked) psiField5() {
	static CUnit *unit;

	__asm{
		PUSHAD
		MOV		unit, ESI
	}

	if(hooks::isReadyToMakePsiField(unit)){
		__asm{
			POPAD
			JMP		Hook_PsiField05Back0
		}
	}
	else{
		__asm{
			POPAD
			JMP		Hook_PsiField05Back1
		}
	}

}

const u32 Hook_PsiField06 = 0x004E3182;
const u32 Hook_PsiField06Back0 = 0x004E3189;
const u32 Hook_PsiField06Back1 = 0x004E31A3;
void __declspec(naked) psiField6() {
	static u16 unitId;

	__asm{
		PUSHAD
		MOV		unitId, CX
	}

	if(hooks::canMakePsiField(unitId)){
		__asm{
			POPAD
			JMP		Hook_PsiField06Back0
		}
	}
	else{
		__asm{
			POPAD
			JMP		Hook_PsiField06Back1
		}
	}

}

const u32 Hook_PsiField07 = 0x004E32F7;
const u32 Hook_PsiField07Back0 = 0x004E32FE;
const u32 Hook_PsiField07Back1 = 0x004E3348;
void __declspec(naked) psiField7() {
	static u16 unitId;

	__asm{
		PUSHAD
		MOV		unitId, CX
	}

	if(hooks::canMakePsiField(unitId)){
		__asm{
			POPAD
			JMP		Hook_PsiField07Back0
		}
	}
	else{
		__asm{
			POPAD
			JMP		Hook_PsiField07Back1
		}
	}

}

const u32 Hook_OrdersEntries_CarrierStop = 0x004EC7D7;
const u32 Orders_CarrierStop = 0x00465910;
void __declspec(naked) ordersEntries_CarrierStop() {
	
	__asm{
		MOV		EAX, [ESI+64h]
		CMP		EAX, 48h
		JZ		short _carrierStop
		CMP		EAX, 52h
		JNZ		short _Retn
_carrierStop:
		POP     EDI
		MOV     EAX, ESI
		POP     ESI
		POP     EBX
		JMP		Orders_CarrierStop
_Retn:
		JMP		Hook_OrdersEntries_Retn
	}

}

const u32 Hook_BTNSACT_CarrierMove = 0x0042427C;
const u32 Hook_BTNSACT_CarrierMoveBack = 0x00424283;
void __declspec(naked) btnsAct_CarrierMove() {
	
	__asm{
		MOV		EDX, DS:[0x00597248]
		MOV		EDX, [EDX+64h]
		CMP		EDX, 2Eh
		JZ		SHORT _jmp0
		CMP		EDX, 33h
		JZ		SHORT _jmp0
		CMP		EDX, 34h
		JZ		SHORT _jmp0
		CMP		EDX, 37h
		JNZ		SHORT _jmp1
_jmp0:
		MOV		BYTE PTR DS:[0x00641692], 33h
		JMP		Hook_BTNSACT_CarrierMoveBack
_jmp1:
		MOV		BYTE PTR DS:[0x00641692], 31h
		JMP		Hook_BTNSACT_CarrierMoveBack
	}

}

const u32 Hook_CancelUnit = 0x004682FD;
const u32 Hook_CancelUnitBack = 0x00468310;

const u32 Hook_UnitStatAct_Building0 = 0x004279DF;
const u32 Hook_UnitStatAct_Building0Back = 0x00427A01;
const u32 Hook_UnitStatAct_Building1 = 0x00427A57;
const u32 Hook_UnitStatAct_Building1Back = 0x00427A5C;
const u32 Hook_426C60 = 0x00426C60;
const u32 SetKillsStrText = 0x00425DD0;
const u32 SetUnitStatusStrText = 0x00425B50;

void __declspec(naked) setUnitStatAct_Building1() {
static CUnit *unit;
	
	__asm{
		CALL SetUnitStatusStrText
	}

	unit = *activePortraitUnit;

	if(unit->status & UnitStatus::Completed
		&& unit->buildQueue[unit->buildQueueSlot] == UnitId::None
		&& !unit->isConstructingAddon()
		&& (unit->playerId == *LOCAL_HUMAN_ID || *IS_IN_REPLAY)){
		__asm{
			MOV EBX, EDI
			CALL SetKillsStrText
		}

		using units_dat::SupplyProvided;
		using units_dat::SpaceProvided;
		if(SupplyProvided[unit->id] 
		|| (unit->isActiveTransport() && !(unit->getLoadedUnit(0)))
		|| unit->id == UnitId::refinery){
			__asm{
				PUSH EDI
				CALL Hook_426C60
			}
		}
	}

	__asm{
		JMP Hook_UnitStatAct_Building1Back
	}
}

const u32 Hook_PlayBuildingCompleteSound = 0x0048F39D;
const u32 Hook_PlayBuildingCompleteSoundBack = 0x0048F3A4;
void __declspec(naked) setPlayBuildingCompleteSound() {
static u16 unitId;

	__asm {
		MOV unitId, CX
	}

	if(unitId == 7 || unitId == 41){
		__asm{
			MOV BL, 0
		}
	}
	else{
		__asm{
			MOV BL, 1
		}
	}

	__asm {
		JMP Hook_PlayBuildingCompleteSoundBack
	}
}

const u32 Hook_GetRemainingBuildTimePercentage = 0x00466A1B;
const u32 Hook_GetRemainingBuildTimePercentageBack = 0x00466A22;
void __declspec(naked) getRemainingBuildTimePercentage() {
static u32	time;

	__asm {
		MOV	time, ECX
	}

	if(!time){
		__asm {
			XOR EDX, EDX
			MOV	EAX, 100
			RETN
		}
	}else{
		__asm {
			MOVZX   EDX, word ptr [ESI+0ACh]
			JMP	Hook_GetRemainingBuildTimePercentageBack
		}
	}

}

const u32 Hook_KillTargetUnitCheck = 0x004797E4;
const u32 Hook_KillTargetUnitCheckBack = 0x004797FF;

const u32 Hook_SetSpellSpecialBtnGraphic = 0x00424C7A;
const u32 Hook_SetSpellSpecialBtnGraphicBack0 = 0x00424C81;
const u32 Hook_SetSpellSpecialBtnGraphicBack1 = 0x00424DB2;
const u32 Hook_SetSpellSpecialBtnGraphicBack2 = 0x00424D00;
void __declspec(naked) setSpellSpecialBtnGraphic() {
	static u32	unitId;

	__asm {
		MOVZX	EDX, WORD PTR [ECX+64h]
		MOV		AL, [EBP-1]
		PUSHAD
		MOV		unitId, EDX
	}

	if(units_dat::SpaceRequired[unitId] == 8){
		__asm {
			POPAD
			MOV     EAX, EDI
			MOV		DWORD PTR [EBP-8], 14h
			JMP		Hook_SetSpellSpecialBtnGraphicBack2
		}
	}
	else{
		__asm {
			POPAD
			JMP		Hook_SetSpellSpecialBtnGraphicBack0
		}
		
	}
}

const u32 loc_4D7500 = 0x004D7500;
const u32 ISCRIPT_CreateImage = 0x004D6D90;
void __declspec(naked) __3e() {

	__asm {
		mov     ecx, [ebp+0Ch]
		test    ecx, ecx
		jnz     short _4D7500
		xor     ecx, ecx
		xor     edx, edx
		mov     cl, [esi+0Fh]
		mov     dl, [esi+0Eh]
		movzx   eax, word ptr [esi+8]
		inc     eax
		inc     eax
		push    1
		push    ecx
		push    edx
		push    eax
		push    esi
		call    ISCRIPT_CreateImage
_4D7500:
		jmp     loc_4D7500
	}
}

void __declspec(naked) __43() {
	static CImage* image;
	static unsigned char strength;

	__asm{
		INC	EDI
		MOV EAX, [EBP+0Ch]
		TEST EAX, EAX
		JNZ short _4D7500
		MOV	AL, [EDI-1]
		MOV	strength, AL
		PUSHAD
		MOV	image, ESI
	}

	hooks::setShock(strength, image->parentSprite->position.x, image->parentSprite->position.y);

	__asm{
		POPAD
_4D7500:
		JMP     loc_4D7500
	}

}

const u32 ISCRIPT_PlayFrame = 0x004D5E70;
void __declspec(naked) __45() {

	__asm{
		MOV EAX, [EBP+0Ch]
		ADD EDI, 2
		TEST EAX, EAX
		JNZ short _4D7500

		MOVZX EDX, WORD PTR [ESI+18h]
		ADD DX, WORD PTR [EDI-2]
		MOV EAX, ESI
		CALL ISCRIPT_PlayFrame
_4D7500:
		JMP     loc_4D7500
	}

}

void __declspec(naked) __46() {

	__asm{
		MOV EAX, [EBP+0Ch]
		MOVZX EDX, WORD PTR [EDI]
		ADD EDI, 6
		TEST EAX, EAX
		JNZ short _4D7500

		CMP DX, WORD PTR [ESI+18h]
		JNZ short _4D7500
		MOV EDX, [EDI-4]
		ADD EDX, DWORD PTR DS:[0x006D1200]
		MOV DWORD PTR [EBP-8], EDX
		MOV EDI, EDX
_4D7500:
		JMP     loc_4D7500
	}

}

void __declspec(naked) __47() {

	__asm{
		MOV EAX, [EBP+0Ch]
		MOVZX EDX, WORD PTR [EDI]
		ADD EDI, 8
		TEST EAX, EAX
		JNZ short _4D7500

		CMP WORD PTR [ESI+18h], DX
		JB short _4D7500
		MOVZX EDX, WORD PTR [EDI-6]
		CMP WORD PTR [ESI+18h], DX
		JA short _4D7500
		MOV EDX, [EDI-4]
		ADD EDX, DWORD PTR DS:[0x006D1200]
		MOV DWORD PTR [EBP-8], EDX
		MOV EDI, EDX
_4D7500:
		JMP     loc_4D7500
	}

}

const u32 iscript_OpcodeCasesOffset[] = {
	0x004D7513, 0x004D7541, 0x004D7575, 0x004D7594, 
	0x004D75D0, 0x004D8320,	0x004D75EE, 0x004D7637, 
	0x004D764A,	0x004D7683, 0x004D76D1, 0x004D771E,
	0x004D7500, 0x004D776B, 0x004D77B1,	0x004D7870, 
	0x004D79DE, 0x004D7A12,	0x004D7915, 0x004D7A2B, 
	0x004D7A8F,	0x004D7967, 0x004D7BBF, 0x004D7BE9,	
	0x004D7C04, 0x004D7C33, 0x004D7C88,	0x004D7E20, 
	0x004D7C22, 0x004D7CCC,	0x004D7F2C, 0x004D7E57, 
	0x004D7E75,	0x004D7E9B, 0x004D7ECA, 0x004D7F0D,
	0x004D7DE1, 0x004D808E, 0x004D80AF,	0x004D8114,
	0x004D815B, 0x004D81CF,	0x004D81AE, 0x004D7D30, 
	0x004D7D53,	0x004D7D9B, 0x004D823C, 0x004D8262,	
	0x004D8216, 0x004D817D, 0x004D7DC9,	0x004D7DB2, 
	0x004D82A4, 0x004D82C6,	0x004D82EA, 0x004D8301, 
	0x004D7B13,	0x004D7B94, 0x004D7F64, 0x004D7FCD,	
	0x004D8052, 0x004D769C, (u32)&__3e,	0x004D8284, 
	0x004D752A, 0x004D7DFD,	0x004D77F7, (u32)&__43, 
	0x004D7E3B, (u32)&__45, (u32)&__46, (u32)&__47
};

const u32 Hook_ImageRenderFxn11 = 0x0047A9BA;
const u32 Hook_ImageRenderFxn11Back0 = 0x0047A9D7;
const u32 Hook_ImageRenderFxn11Back1 = 0x0047A9C9;
void __declspec(naked) imageRenderFxn11() {
	static CUnit* unit;

	__asm{
		PUSHAD
		MOV unit, esi
	}

	if((units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster) && unit->getMaxEnergy()){
		__asm{
			POPAD
			JMP Hook_ImageRenderFxn11Back1	
		}
	}
	else{
		__asm{
			POPAD
			JMP	Hook_ImageRenderFxn11Back0	
		}
	}
}

const u32 Hook_PrintEnergyStr = 0x00425A50;
const u32 Hook_PrintEnergyStrBack0 = 0x00425AAA;
const u32 Hook_PrintEnergyStrBack1 = 0x00425A5E;
void __declspec(naked) printEnergyStr() {
	static CUnit* unit;

	__asm{
		PUSHAD
		MOV unit, ESI
	}

	if((units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster) 
		&& unit->getMaxEnergy() 
		&& !(unit->status & UnitStatus::IsHallucination)){
		__asm{
			POPAD
			JMP Hook_PrintEnergyStrBack1	
		}
	}
	else{
		__asm{
			POPAD
			JMP	Hook_PrintEnergyStrBack0	
		}
	}
}

const u32 Hook_CreateInitialMeleeUnits = 0x0049DBEF;
const u32 Hook_CreateInitialMeleeUnitsBack = 0x0049DC06;
const u32 Hook_CreateInitialMeleeOverload = 0x0049DAB4;
const u32 Hook_CreateInitialMeleeOverloadBack = 0x0049DAB9;
const u32 Hook_CreateUnit = 0x004A09D0;
const u32 Hook_updateUnitStatsFinishBuilding = 0x004A01F0;
const u32 Hook_positionUnitPrep = 0x0049EC30;
const u32 Hook_updateUnitStrengthAndApplyDefaultOrders = 0x0049FA40;
void __declspec(naked) extendCreateInitialMeleeUnits() {
	__asm{
		jz      short Jmp00
		call    Hook_updateUnitStatsFinishBuilding
		call    Hook_positionUnitPrep
		test    eax, eax
		jz      short Jmp00
		mov     eax, esi
		call    Hook_updateUnitStrengthAndApplyDefaultOrders
Jmp00:
		mov     ecx, [ebp-4]
		movzx   eax, byte ptr [ecx]
		sub     eax, 0
		jz      short Jmp03
		dec     eax
		jz      short Jmp02
		dec     eax
		jz      short Jmp01
		mov     cl, 0E4h
		jmp     short Jmp04
Jmp01:
		mov     cl, 40h
		jmp     short Jmp04
Jmp02:
		mov     cl, 7
		jmp     short Jmp04
Jmp03:
		mov     cl, 29h
Jmp04:
		movsx   edx, word ptr [ebx]
		mov     eax, [ebp-8]
		movsx   eax, word ptr [eax]
		push    edi 
		movzx   ecx, cl
		push    edx   
		call    Hook_CreateUnit
		mov     esi, eax
		test    esi, esi 
		jz      short Hook_CreateInitialMeleeUnitsBack
		call    Hook_updateUnitStatsFinishBuilding
		call    Hook_positionUnitPrep
		test    eax, eax
		jz      short Hook_CreateInitialMeleeUnitsBack
		mov     eax, esi
		call    Hook_updateUnitStrengthAndApplyDefaultOrders
		jmp		Hook_CreateInitialMeleeUnitsBack
/*		jz      short Jmp05
		call    Hook_updateUnitStatsFinishBuilding
		call    Hook_positionUnitPrep
		test    eax, eax
		jz      short Jmp05
		mov     eax, esi
		call    Hook_updateUnitStrengthAndApplyDefaultOrders
Jmp05:
		mov     ecx, [ebp-4]
		movzx   eax, byte ptr [ecx]
		sub     eax, 0
		jz      short Jmp08
		dec     eax
		jz      short Jmp07
		dec     eax
		jz      short Jmp06
		mov     cl, 0E4h
		jmp     short Jmp09
Jmp06:
		mov     cl, 40h
		jmp     short Jmp09
Jmp07:
		mov     cl, 7
		jmp     short Jmp09
Jmp08:
		mov     cl, 29h
Jmp09:
		movsx   edx, word ptr [ebx]
		mov     eax, [ebp-8]
		movsx   eax, word ptr [eax]
		push    edi 
		movzx   ecx, cl
		push    edx   
		call    Hook_CreateUnit
		mov     esi, eax
		test    esi, esi 
		jz      short Hook_CreateInitialMeleeUnitsBack
		call    Hook_updateUnitStatsFinishBuilding
		call    Hook_positionUnitPrep
		test    eax, eax
		jz      short Hook_CreateInitialMeleeUnitsBack
		mov     eax, esi
		call    Hook_updateUnitStrengthAndApplyDefaultOrders
		jmp		Hook_CreateInitialMeleeUnitsBack*/
	}
}

const u32 Hook_verifyCheatCodeEnd = 0x004B1DAD;
void __declspec(naked) extendCheatCode() {
	static u32 i, *k;
	static u8 j;
	static bool check;

	__asm{
		mov     ecx, 2
		mov     esi, eax
		mov		k, eax
		PUSHAD
	}
	
	i = 0;
	while(i < maxExtendCheat && i < 15){
		check = true;
		
		j = 0;
		while(j < 2){
			if(*k != extendCheatHashTable[i][j]){
				check = false;
				break;
			}
			++k;
			++j;
		}

		if(check){
			switch(i){
			case 0:
				i = 5;
				break;
			case 1:
				i = 10;
				break;
			case 2:
				i = 15;
				break;
			case 3:
				i = 16;
				break;
			default:
				i += 16;
				if(i >= 29)
					++i;
				break;
			}

			i = (1<<i);

			__asm{
				POPAD
				mov     eax, [edx]
				pop     edi
				xor     eax, i
				pop     esi
				mov     [edx], eax
				mov     eax, 1
				pop     ebx
				retn 
			}
			break;
		}

		++i;
	}

	__asm{
		POPAD
		jmp Hook_verifyCheatCodeEnd
	}
}

const u32 Hook_verifyCheatCode = 0x004B1D95;
const u32 Hook_verifyCheatCodeBack = 0x004B1D9B;
void __declspec(naked) verifyCheatCode() {

	__asm{
		xor     ebx, ebx
		repe cmpsd
		jnz		short ExtendCheatCode
		jmp		Hook_verifyCheatCodeBack
ExtendCheatCode:
		jmp		extendCheatCode
	}

}

const u32 Hook_CheckUnitVisibility = 0x00443398;
const u32 Hook_CheckUnitVisibilityBack = 0x0044339E;
void __declspec(naked) checkUnitVisibility() {

	__asm{
		MOV		ECX, DWORD PTR DS:[6D5A6Ch]
		TEST	ECX, 20h
		JZ		short _jmp
		PUSH	EBX
		MOV		ECX, DWORD PTR DS:[512684h]
		MOV		EBX, 1
		SHL		EBX, CL
		OR		EAX, EBX
		POP		EBX
_jmp:
		MOV		ECX, [ESI+0E4h]
		JMP		Hook_CheckUnitVisibilityBack
	}

}

/*
const u32 loc_4BB740 = 0x004BB740;
void __declspec(naked) soundCheck() {

	__asm {
		MOV		EAX, 1
		RETN	4
	}
}
*/

const u32 Sound00 = 0x004BB792;
const u32 loc_4BB885 = 0x004BB885;
const u32 Sound00back = 0x004BB79B;
void __declspec(naked) Sound0() {
static u32 value = (u32)maxSound;
static u32 check;

	__asm {
		PUSHAD
		MOV		check, EBX
	}

	if(check < value){
		__asm {
			POPAD
			JMP     loc_4BB885
		}
	}
	else{
		__asm {
			POPAD
			JMP     Sound00back
		}
	}
}

const u32 Sound01 = 0x004BB82F;
const u32 loc_4BB7A8 = 0x004BB7A8;
const u32 Sound01back = 0x004BB838;
void __declspec(naked) Sound1() {
static u32 value = (u32)maxSound;
static u32 check;

	__asm {
		PUSHAD
		MOV		check, EDX
	}

	if(check < value){
		__asm {
			POPAD
			JMP     loc_4BB7A8
		}
	}
	else{
		__asm {
			POPAD
			JMP     Sound01back
		}
	}
}

const u32 Sound02 = 0x004BB838;
const u32 loc_4BB849 = 0x004BB849;
const u32 Sound02back = 0x004BB83D;
void __declspec(naked) Sound2() {
static u32 value = (u32)maxSound;
static u32 check;

	__asm {
		PUSHAD
		MOV		check, EBX
	}

	if(check < value){
		__asm {
			POPAD
			JMP     loc_4BB849
		}
	}
	else{
		__asm {
			POPAD
			JMP     Sound02back
		}
	}
}

void setSoundArray(s32 EAX, s32 EDI);

const u32 Sound03 = 0x004BB8FA;
const u32 Sound03back = 0x004BB96A;
void __declspec(naked) Sound3() {
static u32 _EAX, _EDI;

	__asm {
		PUSHAD
		MOV _EAX, EAX
		MOV _EDI, EDI
	}

	setSoundArray(_EAX, _EDI);

	__asm {
		POPAD
		JMP	Sound03back
	}

}

const u32 bullet00 = 0x0048C26E;
const u32 bullet00back = 0x0048C27C;

const u32 bullet01 = 0x0048AF7C;
const u32 bullet01back = 0x0048AF82;
const u32 loc_48AF50 = 0x0048AF50;
void __declspec(naked) bullet1() {

	__asm {
		cmp     dx, maxCBullet_16
		jb      short _48AF50
		jmp     bullet01back
_48AF50:
		jmp     loc_48AF50
	}
}

const u32 bullet02 = 0x0048A771;
const u32 bullet02back = 0x0048A776;
const u32 loc_48A784 = 0x0048A784;
void __declspec(naked) bullet2() {

	__asm {
		cmp     eax, maxCBullet
		jnz     short _48A784
		jmp     bullet02back
_48A784:
		jmp     loc_48A784
	}
}

const u32 bullet03 = 0x0048A790;
const u32 bullet03back = 0x0048A799;
void __declspec(naked) bullet3() {

	__asm {
		cmp     eax, maxCBullet
		push	esi
		mov		esi, firstUnuseBullet
		mov		[esi], ecx
		pop		esi
		jmp     bullet03back
	}
}

const u32 bullet04 = 0x0048A831;
const u32 bullet04back = 0x0048A836;
const u32 loc_48A84A = 0x0048A84A;
void __declspec(naked) bullet4() {

	__asm {
		cmp     eax, maxCBullet
		jnz     short _48A84A
		jmp     bullet04back
_48A84A:
		jmp     loc_48A84A
	}
}

const u32 bullet05 = 0x0048A855;
const u32 bullet05back = 0x0048A85A;
void __declspec(naked) bullet5() {

	__asm {
		push    esi
		inc     eax
		cmp     eax, maxCBullet
		jmp     bullet05back
	}
}

/*
const u32 sprite00 = 0x00498837;
const u32 sprite00back = 0x0049883E;
void __declspec(naked) sprite0() {
static u32 value = (u32)maxCSprite;

	__asm {
		cmp     eax, value
		mov     [ebp-8], esi
		jmp     sprite00back
	}
}
*/
/*
const u32 regionAccessibilityFlags00 = 0x0042B9F4;
const u32 regionAccessibilityFlags00back = 0x0042B9FE;
void __declspec(naked) regionAccessibilityFlags0() {
static u32 value = (u32)maxCSprite;

	__asm {
		add     word ptr [ebx+18h], 65536
		mov     ax, [ebx+18h]
		jmp     regionAccessibilityFlags00back
	}
}

const u32 regionAccessibilityFlags01 = 0x0042BA63;
const u32 regionAccessibilityFlags01back = 0x0042BA6D;
void __declspec(naked) regionAccessibilityFlags1() {
static u32 value = (u32)maxCSprite;

	__asm {
		add     word ptr [ebx+1Ah], 65536
		mov     ax, [ebx+1Ah]
		jmp     regionAccessibilityFlags01back
	}
}

const u32 regionAccessibilityFlags02 = 0x0042BADA;
const u32 regionAccessibilityFlags02back = 0x0042BAE3;
void __declspec(naked) regionAccessibilityFlags2() {
static u32 value = (u32)maxCSprite;

	__asm {
		add     word ptr [ebx+1Ch], 65536
		mov     [ebx+4], edx
		jmp     regionAccessibilityFlags02back
	}
}

const u32 regionAccessibilityFlags03 = 0x0042BB4C;
const u32 regionAccessibilityFlags03back = 0x0042BB56;
void __declspec(naked) regionAccessibilityFlags3() {
static u32 value = (u32)maxCSprite;

	__asm {
		add     word ptr [ebx+1Eh], 65536
		mov     ax, [ebx+1Eh]
		jmp     regionAccessibilityFlags03back
	}
}

const u32 regionAccessibilityFlags04 = 0x0042E61F;
const u32 regionAccessibilityFlags04back = 0x0042E626;
void __declspec(naked) regionAccessibilityFlags4() {
static u32 value = (u32)maxCSprite;

	__asm {
		cmp     eax, value
		mov     [ebp-40h], ecx
		jmp     regionAccessibilityFlags04back
	}
}

const u32 regionAccessibilityFlags05 = 0x0042E673;
const u32 regionAccessibilityFlags05back = 0x0042E67C;
void __declspec(naked) regionAccessibilityFlags5() {
static u32 value = (u32)maxCSprite;

	__asm {
		cmp     word ptr [ebp+10h], 65536
		mov     [ebp-44h], eax
		jmp     regionAccessibilityFlags05back
	}
}*/


const u32 image00 = 0x004D65BE;
const u32 image00back = 0x004D65C5;
void __declspec(naked) image0() {
static u32 value = (u32)maxCImage;

	__asm {
		cmp     eax, value
		mov     [ebp-8], esi
		jmp     image00back
	}
}
Sound* SoundArray = new Sound[maxSound];

void setSoundArray(s32 EAX, s32 EDI){

	for(int i=0; i < maxSound; ++i){
		if(SoundArray[i].ddUnknown0 == EAX)
			SoundArray[i].ddUnknown0 = EDI;
	}
}

CBullet* makeBulletArray(int num){

	CBullet* CBulletArray = new CBullet[num];
	memset(CBulletArray, NULL, num*sizeof(CBullet));

	return CBulletArray;
}

CBullet* CBulletArray = makeBulletArray(maxCBullet);
/*
CSprite* Sprites = new CSprite[maxCSprite];
CSprite** spriteListPointers = new CSprite*[maxCSprite];

CSprite** spriteHeads = new CSprite*[maxSprtieHeadTail];
CSprite** spriteTails = new CSprite*[maxSprtieHeadTail];

char* regionAccessibilityFlags = new char[8*maxCSprite];

CThingy* SpriteThingyArray = new CThingy[maxCThingy];
CThingy* THG2_Array = new CThingy[maxCThingy];
*/
CImage* Images = new CImage[maxCImage];
CImage** imageListPointers = new CImage*[maxCImage+2];

CImage* circleMarkers3 = new CImage[64];
CImage* buildingPlacementImages = new CImage[64];
CImage* someImageArray80 = new CImage[80];
CImage* playerCursorCircles = new CImage[SELECT_UNIT_COUNT+1];
CImage* somePlayerImages = &playerCursorCircles[1];

const u32 image01 = 0x004D6C33;
const u32 image01back = 0x004D6C53;
const u32 dword_5240B4 = 0x005240B4;
void __declspec(naked) image1() {

	__asm {
		mov     [eax+7Ah], esi
		jnz     short loc_4D6C41
		lea     ecx, [eax+94h]
		push	eax
		mov		eax, [dword_5240B4]
		mov     [eax], ecx
		pop		eax

loc_4D6C41:
		lea     ecx, [eax+94h]
		push	eax
		mov		eax, circleMarkers3
		mov     dword ptr [ecx], eax
		add		eax, 4
		mov     esi, [eax]
		pop		eax
		mov     [eax+98h], esi
		jmp		image01back
	}
}

const u32 image02 = 0x0049738C;
const u32 image02back = 0x00497394;
void __declspec(naked) image2() {
	static u32 value;

	__asm{
		PUSHAD
		mov		value, eax
	}

	value /= sizeof(CImage);

	__asm{
		POPAD
		mov		eax, value
		inc		eax
		jmp     image02back
	}
}

const u32 image03 = 0x004973A3;
const u32 image03back = 0x004973AB;
void __declspec(naked) image3() {
	static u32 value;

	__asm{
		PUSHAD
		mov		value, eax
	}

	value /= sizeof(CImage);

	__asm{
		POPAD
		mov		eax, value
		inc		eax
		jmp     image03back
	}
}


const u32 image04 = 0x004973BA;
const u32 image04back = 0x004973C2;
void __declspec(naked) image4() {
	static u32 value;

	__asm{
		PUSHAD
		mov		value, eax
	}

	value /= sizeof(CImage);

	__asm{
		POPAD
		mov		eax, value
		inc		eax
		jmp     image04back
	}
}

const u32 image05 = 0x004D4C0D;
const u32 image05back = 0x004D4C12;
void __declspec(naked) image5() {
	static u32 value;

	__asm{
		mov     edx, eax
		PUSHAD
		mov		value, edx
	}

	value *= sizeof(CImage);

	__asm{
		POPAD
		mov		edx, value
		jmp     image05back
	}
}

const u32 image06 = 0x004D4C2E;
const u32 image06back = 0x004D4C36;
void __declspec(naked) image6() {
	static u32 value;

	__asm{
		PUSHAD
		mov		value, eax
	}

	value *= sizeof(CImage);

	__asm{
		POPAD
		mov		eax, value
		push	ebx
		mov		ebx, Images
		add     eax, ebx
		pop		ebx
		jmp     image06back
	}
}

const u32 image07 = 0x004D62D9;
const u32 image07back = 0x004D62DE;
void __declspec(naked) image7() {
	static u32 value;

	__asm{
		PUSHAD
		mov		value, edi
	}

	value *= sizeof(CImage);

	__asm{
		POPAD
		mov		edi, value
		mov     esi, ebx
		jmp     image07back
	}
}

const u32 image08 = 0x004D63F7;
const u32 image08back = 0x004D63FF;
void __declspec(naked) image8() {
	static u32 value;

	__asm{
		PUSHAD
		mov		value, eax
	}

	value /= sizeof(CImage);

	__asm{
		POPAD
		mov		eax, value
		inc		eax
		jmp     image08back
	}
}

const u32 image09 = 0x004D640D;
void __declspec(naked) image9() {
	static u32 value;

	__asm{
		PUSHAD
		mov		value, eax
	}

	value /= sizeof(CImage);

	__asm{
		POPAD
		mov		eax, value
		inc     eax
		mov     [esi+4], eax
		retn
	}
}

const u32 image10 = 0x004D5AC7;
const u32 image10back = 0x004D5ACC;
void __declspec(naked) image_10() {

	__asm{
		mov     [ebx], edx
		mov     [ebx+4], edx
		mov     [ebx+8], edx
		mov     [ebx+0Ch], dx
		jmp		image10back
	}
}

const u32 image11 = 0x004D685F;
const u32 image11back = 0x004D6864;
void __declspec(naked) image_11() {

	__asm{
		mov     [ecx], eax
		mov     [ecx+4], eax
		mov     [ecx+8], eax
		mov     [ecx+0Ch], ax
		jmp		image11back
	}
}

const u32 image12 = 0x004D56C6;
const u32 image12back = 0x004D56D3;
void __declspec(naked) image_12() {
	static u32 value;

	__asm{
		PUSHAD
		mov		value, ecx
	}

	value *= sizeof(CImage);

	__asm{
		POPAD
		mov     ecx, value
		push	eax
		mov		eax, Images
		sub		eax, CImgsiz
		lea     ecx, [eax+ecx]
		pop		eax
		jmp		image12back
	}
}

const u32 image13 = 0x004D56DC;
const u32 image13back = 0x004D56E5;
void __declspec(naked) image_13() {
	static u32 value;

	__asm{
		PUSHAD
		mov		value, ecx
	}

	value *= sizeof(CImage);

	__asm{
		POPAD
		mov     ecx, value
		push	eax
		mov		eax, Images
		sub		eax, CImgsiz
		lea     ecx, [eax+ecx]
		pop		eax
		jmp		image13back
	}
}

const u32 image14 = 0x00497317;
const u32 image14back = 0x00497324;
void __declspec(naked) image_14() {
	static u32 value;

	__asm{
		PUSHAD
		mov		value, ecx
	}

	value *= sizeof(CImage);

	__asm{
		POPAD
		mov     ecx, value
		push	eax
		mov		eax, Images
		sub		eax, CImgsiz
		lea     ecx, [eax+ecx]
		pop		eax
		jmp		image14back
	}
}

const u32 image15 = 0x0049732E;
const u32 image15back = 0x0049733B;
void __declspec(naked) image_15() {
	static u32 value;

	__asm{
		PUSHAD
		mov		value, ecx
	}

	value *= sizeof(CImage);

	__asm{
		POPAD
		mov     ecx, value
		push	eax
		mov		eax, Images
		sub		eax, CImgsiz
		lea     ecx, [eax+ecx]
		pop		eax
		jmp		image15back
	}
}

const u32 image16 = 0x00497345;
const u32 image16back = 0x00497352;
void __declspec(naked) image_16() {
	static u32 value;

	__asm{
		PUSHAD
		mov		value, ecx
	}

	value *= sizeof(CImage);

	__asm{
		POPAD
		mov     ecx, value
		push	eax
		mov		eax, Images
		sub		eax, CImgsiz
		lea     ecx, [eax+ecx]
		pop		eax
		jmp		image16back
	}
}

const u32 Hook_IsValidScript = 0x004D664A;
const u32 FatalError = 0x004215D0;
void __declspec(naked) isValidScript() {

	__asm {
		mov     ax, [esi]
		movzx   ecx, ax
		cmp     ecx, edi
		jz      short loc_4D6680

loc_4D6654:
		cmp     ax, 0FFFFh
		jnz     short loc_4D6668
		push    edi
		push    501F08h
		call    FatalError
		add     esp, 8

loc_4D6668:
		mov     ax, [esi+6]
		add     esi, 6       
		movzx	edx, ax
		cmp     edx, edi
		jnz     short loc_4D6654
		mov     eax, [esi+2]
		mov     [ebx+40h], eax
		pop     esi
		retn

loc_4D6680:
		mov     ecx, [esi+2]
		mov     [ebx+40h], ecx
		pop     esi
		retn
	}
}

IscriptEx *someIscriptStruct = new IscriptEx;

const u32 Hook_UpdateUnitSpeed_iscriptControlled = 0x0042D8D6;
const u32 Hook_UpdateUnitSpeed_iscriptControlledBack = 0x0042D8EA;
void __declspec(naked) updateUnitSpeed_iscriptControlled() {

	__asm{
		push	ebx
		mov		ebx, someIscriptStruct
		mov     ecx, [eax+40h]
		mov     [ebx], ecx
		mov     ecx, [eax+44h]
		mov     [ebx+4], ecx
		mov     edx, [eax+48h]
		mov     [ebx+8], edx
		mov     dx, [eax+4Ch]
		mov     [ebx+0Ch], dx
		pop		ebx
		mov     al, 0Bh
		jmp		Hook_UpdateUnitSpeed_iscriptControlledBack
	}
}

const u32 Hook_42DB50 = 0x0042DC11;
const u32 Hook_42DB50Back = 0x0042DC28;
void __declspec(naked) loc_42DB50() {

	__asm{
		push	ebx
		mov		ebx, someIscriptStruct
		mov     edx, [ecx+40h]
		mov     [ebx], edx
		mov     edx, [ecx+44h]
		mov     [ebx+4], edx
		mov     eax, [ecx+48h]
		mov     [ebx+8], eax
		mov     ax, [ecx+4Ch]
		mov     [ebx+0Ch], ax
		mov     ecx, [ebx+8]
		pop		ebx
		jmp		Hook_42DB50Back
	}
}

const u32 Hook_42D600 = 0x0042D600;
void __declspec(naked) loc_42D600() {

	__asm{
		push	ebx
		mov		ebx, someIscriptStruct
		mov     ecx, [ebx]
		mov     edx, dword ptr DS:[0x006D1200]
		lea     ecx, [ecx+edx+4]
		movzx   dx, al         
		mov     [ebx+0Ch], al
		cmp     dx, [ecx]
		ja      short loc_42D635 
		movzx   eax, al
		mov     ecx, [ecx+eax*4+4]
		xor     eax, eax 
		cmp     ecx, eax        
		mov     [ebx+4], ecx 
		jnz     short loc_42D638

loc_42D635: 
		xor     eax, eax
		POP		ebx
		retn  

loc_42D638:                    
		mov     [ebx+0Dh], al
		mov     [ebx+8], eax 
		mov     eax, 1
		POP		ebx
		retn
	}
}

const u32 Hook_PlayImageIscript = 0x004D847D;
const u32 Hook_PlayImageIscriptBack = 0x004D84F3;
const u32 loc_4D84FA = 0x04D84FA;
void __declspec(naked) playImageIscript() {

	__asm {
		test    edx, edx
		jz      short loc_4D8487
		cmp     byte ptr [ecx+4Ch], 1
		jz      short _4D84FA

loc_4D8487:
		test    byte ptr [ecx+0Ch], 10h
		jnz     short loc_4D8495
		test    al, al 
		jz      short loc_4D8495
		test    edx, edx
		jz      short _4D84FA

loc_4D8495:
		mov     dl, [ecx+4Ch]
		cmp     al, dl        
		jnz     short loc_4D84A4
		cmp     al, 0Bh        
		jz      short _4D84FA
		cmp     al, 13h        
		jz      short _4D84FA

loc_4D84A4:
		cmp     al, 5
		jnz     short loc_4D84B5
		cmp     dl, al
		jz      short loc_4D84C4 
		cmp     dl, 2          
		jz      short loc_4D84C4
		mov     al, 2
		jmp     short loc_4D84C4

loc_4D84B5:
		cmp     al, 6 
		jnz     short loc_4D84C4
		cmp     dl, al
		jz      short loc_4D84C4
		cmp     dl, 3
		jz      short loc_4D84C4
		mov     al, 3

loc_4D84C4:
		push    esi
		mov		esi, [ecx+40h]
		lea     edx, [ecx+40h]
		push    edi
		mov     edi, dword ptr DS:[0x006D1200]
		mov     [ecx+4Ch], al
		movzx   eax, al
		push    0
		add     esi, edi
		mov     eax, [esi+eax*4+8]
		push    0 
		push    edx 
		mov     [ecx+44h], eax
		mov     byte ptr [ecx+4Dh], 0
		mov     dword ptr [ecx+48h], 0
		jmp		Hook_PlayImageIscriptBack
_4D84FA:
		jmp		loc_4D84FA
	}
}

const u32 Hook_PlayWarpInOverlay = 0x004D850F;
const u32 Hook_PlayWarpInOverlayBack = 0x004D8536;
void __declspec(naked) playWarpInOverlay() {

	__asm {
		mov		ecx, [ebx+40h]
		mov     edx, dword ptr DS:[0x006D1200]
		xor     eax, eax    
		lea     esi, [ebx+40h]
		push    eax
		mov     [ebx+4Ch], al
		mov     ecx, [ecx+edx+8]
		push    eax
		mov     [ebx+44h], ecx
		push    esi
		mov     ecx, ebx
		mov     [ebx+4Dh], al
		mov     [ebx+48h], eax
		jmp		Hook_PlayWarpInOverlayBack
	}

}

const u32 Hook_IscriptSomething_Death = 0x004D85C0;
const u32 Hook_IscriptSomething_DeathBack0 = 0x004D85EB;
const u32 Hook_IscriptSomething_DeathBack1 = 0x004D85F1;
void __declspec(naked) iscriptSomething_Death() {
	
	__asm {
		cmp     [esi+4Ch], bl
		jz      short _4D85F1
		mov		edx, [esi+40h]
		push    edi
		mov     edi, dword ptr DS:[0x006D1200]
		lea     eax, [esi+40h]
		push    ecx           
		mov     [esi+4Ch], bl
		mov     edx, [edx+edi+0Ch]
		push    ecx          
		mov     [esi+4Dh], cl
		mov     [esi+48h], ecx
		push    eax     
		mov     ecx, esi  
		mov     [esi+44h], edx
		jmp		Hook_IscriptSomething_DeathBack0

_4D85F1:
		jmp		Hook_IscriptSomething_DeathBack1
	}

}

const u32 Hook_Orders_BuildSelf2_0 = 0x004E4F7C;
const u32 Hook_Orders_BuildSelf2_0Back = 0x004E4FA2;
void __declspec(naked) orders_BuildSelf2_0() {

	__asm {
		mov		ecx, [edi+40h]
		mov     edx, dword ptr DS:[0x006D1200]
		lea     eax, [edi+40h]
		push    ebx        
		mov     byte ptr [edi+4Ch], 0Dh
		mov     ecx, [ecx+edx+3Ch]
		push    ebx  
		mov     [edi+44h], ecx
		push    eax     
		mov     ecx, edi  
		mov     [edi+4Dh], bl
		mov     [edi+48h], ebx
		JMP		Hook_Orders_BuildSelf2_0Back
	}
}

const u32 Hook_Orders_BuildSelf2_1 = 0x004E5039;
const u32 Hook_Orders_BuildSelf2_1Back = 0x004E505F;
void __declspec(naked) orders_BuildSelf2_1() {
	
	__asm {
		mov		ecx, [edi+40h]
		mov     edx, dword ptr DS:[0x006D1200]
		lea     eax, [edi+40h]
		push    ebx
		mov     byte ptr [edi+46h], 15h
		mov     ecx, [ecx+edx+5Ch]
		push    ebx
		mov     [edi+44h], ecx
		push    eax 
		mov     ecx, edi
		mov     [edi+4Dh], bl
		mov     [edi+48h], ebx
		JMP		Hook_Orders_BuildSelf2_1Back
	}
}

const u32 Hook_Orders_BuildSelf2_2 = 0x004E50B8;
const u32 Hook_Orders_BuildSelf2_2Back = 0x004E50E2;
void __declspec(naked) orders_BuildSelf2_2() {
	
	__asm {
		mov		ecx, [edi+40h]
		mov     edx, dword ptr DS:[0x006D1200]
		lea     eax, [edi+40h]
		push    0
		mov     [edi+4Ch], bl
		mov     ecx, [ecx+edx+68h]
		push    0
		mov     [edi+44h], ecx
		push    eax 
		mov     ecx, edi
		mov     [edi+4Dh], 0
		mov     [edi+48h], 0
		JMP		Hook_Orders_BuildSelf2_2Back
	}
}

const u32 Hook_GroundAttackInit = 0x00488696;
const u32 Hook_GroundAttackInitBack = 0x004886BC;
void __declspec(naked) groundAttackInit() {
	
	__asm {
		mov		ecx, [esi+40h]
		mov     edx, dword ptr DS:[0x006D1200]
		lea     eax, [esi+40h]
		push    ebx
		mov     byte ptr [esi+4Ch], 2
		mov     ecx, [ecx+edx+10h]
		push    ebx
		mov     [esi+44h], ecx
		push    eax
		mov     ecx, esi
		mov     [esi+4Dh], bl
		mov     [esi+48h], ebx
		jmp		Hook_GroundAttackInitBack
	}
}

const u32 Hook_Iscript_OpcodeCases = 0x004D74E6;
const u32 Hook_Iscript_OpcodeCasesback = 0x004D74F7;
void __declspec(naked) iscript_OpcodeCases() {

	__asm {
		mov		edi, [ecx+4]
		add     edi, eax
		jmp     Hook_Iscript_OpcodeCasesback
	}
}

const u32 Hook_OpcodeCases00 = 0x004D7627;
const u32 Hook_OpcodeCases00Back = 0x004D7630;
void __declspec(naked) OpcodeCases00() {
	
	__asm {
		mov     [eax+4], edi
		pop     edi
		pop     esi
		mov     [eax+0Dh], dl
		jmp		Hook_OpcodeCases00Back
	}

}

const u32 Hook_OpcodeCases01 = 0x004D7BDA;
const u32 Hook_OpcodeCases01Back = 0x004D7BE0;
void __declspec(naked) OpcodeCases01() {
	
	__asm {
		sub     edi, eax
		mov     [edx+4], edi
		jmp		Hook_OpcodeCases01Back
	}

}

const u32 Hook_OpcodeCases02 = 0x004D82C6;
const u32 Hook_OpcodeCases02Back = 0x004D82E0;
void __declspec(naked) OpcodeCases02() {
	
	__asm {
		mov     eax, [edi]
		mov     ecx, dword ptr ds:[0x006D1200]
		mov     edx, [ebp+8]
		add     edi, 4
		sub     edi, ecx
		add     eax, ecx
		mov     [edx+8], edi
		jmp		Hook_OpcodeCases02Back
	}

}

const u32 Hook_OpcodeCases03 = 0x004D82EA;
const u32 Hook_OpcodeCases03Back = 0x004D82F1;
void __declspec(naked) OpcodeCases03() {
	
	__asm {
		mov     ecx, [ebp+8]
		mov     eax, [ecx+8]
		jmp		Hook_OpcodeCases03Back
	}

}

const u32 Hook_OpcodeCases04 = 0x004D8328;
const u32 Hook_OpcodeCases04Back = 0x004D8335;
void __declspec(naked) OpcodeCases04() {
	
	__asm {
		mov     [eax+0Dh], cl
		sub     edi, dword ptr ds:[0x006D1200]
		mov     [eax+4], edi
		jmp		Hook_OpcodeCases04Back
	}

}

const u32 Hook_OpcodeCases05 = 0x004D834A;
const u32 Hook_OpcodeCases05Back = 0x004D8354;
void __declspec(naked) OpcodeCases05() {
	
	__asm {
		mov     [eax+4], edi
		pop     edi
		pop     esi
		mov     byte ptr [eax+0Dh], 0Ah
		jmp		Hook_OpcodeCases05Back
	}

}

const u32 Hook_OpcodeCases06 = 0x004D7F2F;
const u32 Hook_OpcodeCases06Back = 0x004D7F58;
const u32 RandomizeShort = 0x004DC4A0;
void __declspec(naked) OpcodeCases06() {
	
	__asm {
		mov     ebx, [edi]
		add     edi, 4
		mov     eax, 7
		call    RandomizeShort
		movzx   ecx, dl
		and     eax, 0FFh
		cmp     eax, ecx
		ja      short _4D7500 
		
		mov     ecx, dword ptr ds:[0x006D1200]
		mov		eax, ebx
		jmp		Hook_OpcodeCases06Back
_4D7500:
		jmp		loc_4D7500
	}

}

const u32 Hook_OpcodeCases07 = 0x004D7637;
const u32 Hook_OpcodeCases07Back = 0x004D7640;
void __declspec(naked) OpcodeCases07() {
	
	__asm {
		mov		eax, [edi]
		add     eax, dword ptr ds:[0x006D1200]
		jmp		Hook_OpcodeCases07Back
	}

}

const u32 Hook_OpcodeCases08 = 0x004D7BAB;
const u32 Hook_OpcodeCases08Back = 0x004D7BB5;
void __declspec(naked) OpcodeCases08() {
	
	__asm {
		mov		eax, [edi-4]
		add     eax, dword ptr ds:[0x006D1200]
		jmp		Hook_OpcodeCases08Back
	}

}

const u32 Hook_OpcodeCases09 = 0x004D7F6D;
const u32 Hook_OpcodeCases09Back = 0x004D7F73;
void __declspec(naked) OpcodeCases09() {
	
	__asm {
		mov     ebx, [edi]
		add     edi, 4
		jmp		Hook_OpcodeCases09Back
	}

}

const u32 Hook_OpcodeCases10 = 0x004D7FC0;
const u32 Hook_OpcodeCases10Back = 0x004D7FC5;
void __declspec(naked) OpcodeCases10() {
	
	__asm {
		mov		eax, ebx
		add     eax, ecx
		jmp		Hook_OpcodeCases10Back
	}

}

const u32 Hook_OpcodeCases11 = 0x004D803E;
const u32 Hook_OpcodeCases11Back = 0x004D8048;
void __declspec(naked) OpcodeCases11() {
	
	__asm {
		mov		edx, [edi-4]
		add     edx, dword ptr ds:[0x006D1200]
		jmp		Hook_OpcodeCases11Back
	}

}

/*

DWORD setMaxUnitBit(int num){
	unsigned int i=1;
	
	while(i<=num)
		i <<= 1;

	return i-1;
}

CUnit* units = new CUnit[maxCUnit+1];
const CUnit* units = (CUnit*)0x0059CCA8;
CUnit** airSplashList = new CUnit*[maxCUnit];
CUnit** getAllUnitsInBoundsList = new CUnit*[maxCUnit];
DWORD* getAllUnitsInBoundsIndex = new DWORD[maxCUnit];
DWORD maxUnitBit = setMaxUnitBit(maxCUnit);
*/

const u32 Hook_Unit_IsResource = 0x004688F0;
const u32 Hook_Unit_IsResourceBack00 = 0x004688F6;
const u32 Hook_Unit_IsResourceBack01 = 0x004688FF;
void __declspec(naked) unit_IsResource() {
 
 
__asm{
CMP AX, 0B2h
JZ SHORT _Retn
CMP AX, 0B3h
JZ SHORT _Retn
CMP AX, 0B4h
JZ SHORT _Retn
CMP AX, 0B5h
JZ SHORT _Retn
JMP Hook_Unit_IsResourceBack00
_Retn:
JMP Hook_Unit_IsResourceBack01
 
 
}
}


const u32 Hook_Sub_4746D0 = 0x00474723;
const u32 Hook_Sub_4746D0_Back00 = 0x00474728;
const u32 Hook_Sub_4746D0_Back01 = 0x00474738; 
void __declspec(naked) sub_4746D0() {


__asm{
  CMP CX, 0B2h
  JZ SHORT _Retn
  CMP CX, 0B3h
  JZ SHORT _Retn
  CMP CX, 0B4h
  JZ SHORT _Retn
  CMP CX, 0B5h
  JMP Hook_Sub_4746D0_Back00
_Retn:
  JMP Hook_Sub_4746D0_Back01
}
}


const u32 Hook_UnitCanPlaySFX = 0x0048EB63;
const u32 Hook_UnitCanPlaySFXBack00 = 0x0048EB68;
const u32 Hook_UnitCanPlaySFXBack01 = 0x0048EB7E; 
void __declspec(naked) unitCanPlaySFX() {


__asm{
  CMP CX, 0B2h
  JZ SHORT _Retn
  CMP CX, 0B3h
  JZ SHORT _Retn
  CMP CX, 0B4h
  JZ SHORT _Retn
  CMP CX, 0B5h
  JMP Hook_UnitCanPlaySFXBack00
_Retn:
  JMP Hook_UnitCanPlaySFXBack01


}
}


const u32 Hook_GiveUnits = 0x004C8090;
const u32 Hook_GiveUnitsBack00 = 0x004C8096;
const u32 Hook_GiveUnitsBack01 = 0x004C806C;

void __declspec(naked) giveUnits() {


  __asm{
  CMP AX, 0B2h
  JZ SHORT _Retn
  CMP AX, 0B3h
  JZ SHORT _Retn
  CMP AX, 0B4h
  JZ SHORT _Retn
  CMP AX, 0B5h
  JZ SHORT _Retn
  JMP Hook_GiveUnitsBack00
_Retn:
  JMP Hook_GiveUnitsBack01
  }

}


const u32 Hook_CreateUnitWithProperties = 0x004C8CD6;
const u32 Hook_CreateUnitWithPropertiesBack00 = 0x004C8CDB;
const u32 Hook_CreateUnitWithPropertiesBack01 = 0x004C8CE4; 
void __declspec(naked) createUnitWithProperties() {


__asm{
CMP DI, 0B2h

JZ SHORT _Retn
CMP DI, 0B3h

JZ SHORT _Retn
CMP DI, 0B4h

JZ SHORT _Retn
CMP DI, 0B5h

JMP Hook_CreateUnitWithPropertiesBack00
_Retn:
JMP Hook_CreateUnitWithPropertiesBack01


}
}

const u32 Hook_GetNextNearestResource = 0x00442F8E;
const u32 Hook_GetNextNearestResourceBack = 0x00442F94; 
void __declspec(naked) getNextNearestResource() {

	__asm{
		MOV EAX, DWORD PTR [0x00584DE4+21BCh]
		TEST EAX, EAX
		JNZ SHORT _Back
		MOV EAX, DWORD PTR [0x00584DE4+21ECh]
		TEST EAX, EAX
		JNZ SHORT _Back
		MOV EAX, DWORD PTR [0x00584DE4+221Ch]
		TEST EAX, EAX
		JNZ SHORT _Back
		XOR EAX, EAX
		POP EBP
		RETN 4
_Back:
		JMP Hook_GetNextNearestResourceBack
	}

}

const u32 Hook_HarvestNextNearestResourcesEx = 0x004432F1;
const u32 Hook_HarvestNextNearestResourcesExBack = 0x004432F7; 
void __declspec(naked) harvestNextNearestResourcesEx() {

	__asm{
		MOV ECX, DWORD PTR [0x00584DE4+21BCh]
		TEST ECX, ECX
		JNZ SHORT _Back
		MOV ECX, DWORD PTR [0x00584DE4+21ECh]
		TEST ECX, ECX
		JNZ SHORT _Back
		MOV ECX, DWORD PTR [0x00584DE4+221Ch]
		TEST ECX, ECX
		JNZ SHORT _Back
		XOR EAX, EAX
		POP EBP
		RETN 8
_Back:
		JMP Hook_HarvestNextNearestResourcesExBack
	}

}

static bool isMatrixShield = false;

const u32 Hook_ImageRenderFxn11_0 = 0x0047A841;
const u32 Hook_ImageRenderFxn11_0Back0 = 0x0047A85B;
const u32 Hook_ImageRenderFxn11_0Back1 = 0x0047A8F0;
void __declspec(naked) imageRenderFxn11_0() {
 static CUnit* unit;

 __asm{
  MOV [EBP-8], EDX
  MOV EDX, [EAX+8]
  MOV EAX, [EAX+0Ch]
  MOV [EBP-14h], EDX
  PUSHAD
  MOV unit, ESI
 }

 if((unit->defensiveMatrixHp && unit->defensiveMatrixTimer) || (units_dat::ShieldsEnabled[unit->id] && units_dat::MaxShieldPoints[unit->id])){
  __asm{
   POPAD
   JMP Hook_ImageRenderFxn11_0Back1
  }
 }
 else{
  __asm{
   POPAD
   MOV ECX, [EBP+8]
   CMP BYTE PTR [ECX+3], 0Dh
   JNZ SHORT _Jump
   MOV BYTE PTR [ECX+3], 0Bh //배틀 기준으로 이렇게 넣었는데 다른 마나없는 유닛이 쉴드 가질때 체력바 오류나면 여기 수정
_Jump:
   JMP Hook_ImageRenderFxn11_0Back0
  }
 }
}

const u32 Hook_ImageRenderFxn11_1 = 0x0047A952;
const u32 Hook_ImageRenderFxn11_1Back0 = 0x0047A957;
const u32 Hook_ImageRenderFxn11_1Back1 = 0x0047A960;
void __declspec(naked) imageRenderFxn11_1() {
 static CUnit* unit;

 __asm{
  MOV EBX, EAX
  PUSHAD
  MOV unit, ESI
 }

 if(unit->defensiveMatrixHp && unit->defensiveMatrixTimer){
  __asm{
   MOV isMatrixShield, 1
   POPAD
   MOV EAX, [EBP+8]
   MOV BYTE PTR [EAX+3], 0Dh //배틀 기준으로 이렇게 넣었는데 다른 마나없는 유닛이 쉴드 가질때 체력바 오류나면 여기 먼저 수정
   MOVZX EAX, WORD PTR [ESI+112h]
   PUSH ECX
   MOV ECX, 250 //파그에 설정된 쉴드 부여 최대 수치/8를 넣어야 함.
   JMP Hook_ImageRenderFxn11_1Back1
  }
 }
 else{ //디펜시브 없으면 바로 쉴드 값으로 적용함. 
  __asm{
   MOV isMatrixShield, 0
   POPAD
   MOV EAX, [ESI+60h]
   JMP Hook_ImageRenderFxn11_1Back0
  }
 }
}

const u32 Hook_CompileHealthBar = 0x004D60C2;
const u32 Hook_CompileHealthBarBack00 = 0x004D60C7;
const u32 Hook_CompileHealthBarBack01 = 0x004D60CC;
void __declspec(naked) compileHealthBar () {
  static CUnit* unit;

  __asm{
	MOV EAX, [ESI+2Ch]
	PUSHAD
	MOV unit, EDI
  }

  if(unit->defensiveMatrixHp && unit->defensiveMatrixTimer){
	  __asm{
		POPAD
		MOV CL, [EAX+9]
		JMP Hook_CompileHealthBarBack01
	  }
  }
  else{
	  __asm{
		POPAD
		CMP CL, BL
		JMP Hook_CompileHealthBarBack00
	  }
  }

}

static bool displayShield = false;

const u32 Hook_SetTextStr00 = 0x0042645A;
const u32 Hook_SetTextStr00Back = 0x00426466;
const u32 Func_AddTextToDialog = 0x004258B0;
void __declspec(naked) setTextStr00 () {
  static char shieldField[32];
  static CUnit* unit;

  __asm{
  MOV ECX, ESI
  PUSHAD
  MOV unit, ESI
  }

  if(unit->defensiveMatrixHp && unit->defensiveMatrixTimer){
	  displayShield = true;

	__asm{
	  PUSHAD
	}

	sprintf_s(shieldField, 0x20, "%c%d/%d", GameTextColor::DarkGreen, unit->defensiveMatrixHp >> 8, 250); //파그에 설정된 쉴드 부여 최대 수치/8를 넣어야 함.

	__asm{
	  POPAD
	  PUSH OFFSET shieldField
	  MOV EBX, [EBP+8]
	  MOV EAX, EBX
	  MOV ECX, -6 //BIN 파일에서 추가로 만들 목록의 ID
	  CALL Func_AddTextToDialog
	}
  }
  else if(units_dat::ShieldsEnabled[unit->id] && units_dat::MaxShieldPoints[unit->id]){
	  displayShield = true;

	__asm{
	  PUSHAD
	}

	sprintf_s(shieldField, 0x20, "%c%d/%d", GameTextColor::Teal2, unit->shields >> 8, units_dat::MaxShieldPoints[unit->id]);

	__asm{
	  POPAD
	  //ADD ESP, 0Ch //sprintf_s의 필요에 있는 코드같아서 제외.
	  PUSH OFFSET shieldField
	  MOV EBX, [EBP+8]
	  MOV EAX, EBX
	  MOV ECX, -6 //BIN 파일에서 추가로 만들 목록의 ID
	  CALL Func_AddTextToDialog
	}
  }
  else {
	  displayShield = false;

	  __asm{
	    PUSH 0
	    MOV EBX, [EBP+8]
	    MOV EAX, EBX
	    MOV ECX, -6 //BIN 파일에서 추가로 만들 목록의 ID
	    CALL Func_AddTextToDialog
	  }
  }

 __asm{
  POPAD
  JMP Hook_SetTextStr00Back
  }

}

const u32 Hook_SetTextStr01 = 0x00426493;
const u32 Hook_SetTextStr01Back = 0x004264F2;
void __declspec(naked) setTextStr01 () {

  __asm{
  PUSHAD
  }
  
  if(displayShield == true){
	  __asm{
		POPAD
		PUSH EDI
	    MOV EBX, [EBP+8]
	    MOV EAX, EBX
		MOV ECX, -4
	    CALL Func_AddTextToDialog
	    PUSH 0
	    MOV EBX, [EBP+8]
	    MOV EAX, EBX
	    MOV ECX, -7
	    CALL Func_AddTextToDialog
	  }
  }
  else{
	  __asm{
		POPAD
		PUSH EDI
	    MOV EBX, [EBP+8]
	    MOV EAX, EBX
		MOV ECX, -7
	    CALL Func_AddTextToDialog
	    PUSH 0
	    MOV EBX, [EBP+8]
	    MOV EAX, EBX
	    MOV ECX, -4 //BIN 파일에서 추가로 만들 목록의 ID
	    CALL Func_AddTextToDialog
	  }
  }

  __asm{
  JMP Hook_SetTextStr01Back
  }

}

const u32 Hook_UnitStatCond_Standard = 0x00424990;
const u32 Hook_UnitStatCond_Standard_Back00 = 0x00424999;
const u32 Hook_UnitStatCond_Standard_Back01 = 0x004249CA;
const u32 dword_6CA9EC = 0x006CA9EC;
void __declspec(naked) unitStatCond_Standard() {
  static CUnit* unit;

  __asm{
	  PUSHAD
	  MOV unit, EAX
  }

  if(unit->defensiveMatrixHp && unit->defensiveMatrixTimer){
	  __asm{
		POPAD
		JMP Hook_UnitStatCond_Standard_Back01
	  }
  }
  else{
	  __asm{
		POPAD
		MOV EDX, [EAX+60h]
		MOV ECX, dword_6CA9EC
		JMP Hook_UnitStatCond_Standard_Back00
	  }
  }

}

static u8* wireColorBuffer = new u8[28];

const u32 Hook_Sub_4566B0 = 0x004566B0;
const u32 Hook_Sub_4566B0_Back00 = 0x004566BA;
const u32 Hook_Sub_4566B0_Back01 = 0x00456709;
void __declspec(naked) sub_4566B0() {

  __asm{
  XOR ECX, ECX
  MOVZX EAX, WORD PTR [EDX+114h]
  CMP EAX, ECX
  JZ SHORT _Retn
  MOVZX EAX, WORD PTR [EDX+112h]
  CMP EAX, ECX
  JZ SHORT _Retn
  MOV ECX, 250 //파그에 설정된 쉴드 부여 최대 수치/8을 넣어야 함.
  SAR EAX, 8
  IMUL EAX, 7
  CDQ
  IDIV ECX
  MOVZX EDX, wireShieldColor[EAX*2]
  MOV ECX, wireColorBuffer
  MOV CL, BYTE PTR [ECX+EDX]
  MOVZX EDX, wireShieldColor[EAX*2+1]
  MOV EAX, wireColorBuffer
  MOV AL, BYTE PTR [EAX+EDX]
  JMP Hook_Sub_4566B0_Back01
_Retn:
  MOVZX EAX, WORD PTR [EDX+64h]
  MOVZX ECX, BYTE PTR [0x006647B0+EAX]
  JMP Hook_Sub_4566B0_Back00
  }

}

static u8* hpBarColorBuffer = new u8[22];

const u32 Hook_Sub_47A120 = 0x0047A16C;
const u32 Hook_Sub_47A120_Back00 = 0x0047A172;
const u32 Hook_Sub_47A120_Back01 = 0x0047A179;
void __declspec(naked) sub_47A120() {

  __asm{
  MOVSX EBX, WORD PTR [EDX]
  MOV EDI, [EBP+0Ch]
  CMP isMatrixShield, 1
  JZ SHORT _Jump
  JMP Hook_Sub_47A120_Back00
_Jump:
  MOVZX EAX, hpBarShieldColor[ESI]
  JMP Hook_Sub_47A120_Back01
  }

}

const u32 Hook_GetSelectedUnitsInBox00 = 0x0046FA41;
const u32 Hook_GetSelectedUnitsInBox00_Back = 0x0046FA46;
void __declspec(naked) getSelectedUnitsInBox00() {
static s32 num = SELECT_UNIT_COUNT*8;

  __asm{
  MOV EBP, ESP
  SUB ESP, num
  JMP Hook_GetSelectedUnitsInBox00_Back
  }

}

const u32 Hook_GetSelectedUnitsInBox01 = 0x0046FAAC;
const u32 Hook_GetSelectedUnitsInBox01_Back = 0x0046FAB9;
void __declspec(naked) getSelectedUnitsInBox01() {
static s32 num0 = -(SELECT_UNIT_COUNT*4);
static s32 num1 = -(SELECT_UNIT_COUNT*8);

  __asm{
  MOV EDI, num1
  LEA EDI, [EBP+EDI]
  PUSH EBX
  MOV EAX, num0
  LEA EAX, [EBP+EAX]
  REP MOVSD
  PUSH EAX
  MOV EDI, num1
  LEA EDI, [EBP+EDI]
  JMP Hook_GetSelectedUnitsInBox01_Back
  }

}

const u32 Hook_GetSelectedUnitsAtPoint00 = 0x0046FB41;
const u32 Hook_GetSelectedUnitsAtPoint00_Back = 0x0046FB46;
void __declspec(naked) getSelectedUnitsAtPoint00() {
static s32 num = SELECT_UNIT_COUNT*8+12;

  __asm{
  MOV EBP, ESP
  SUB ESP, num
  JMP Hook_GetSelectedUnitsAtPoint00_Back
  }

}

const u32 Hook_GetSelectedUnitsAtPoint01 = 0x0046FC9A;
const u32 Hook_GetSelectedUnitsAtPoint01_Back = 0x0046FCA0;
void __declspec(naked) getSelectedUnitsAtPoint01() {
static s32 num = -(SELECT_UNIT_COUNT*8+12);

  __asm{
  MOV EDI, num
  LEA EDI, [EBP+EDI]
  LEA EAX, [EBP-0Ch]
  JMP Hook_GetSelectedUnitsAtPoint01_Back
  }

}

const u32 Hook_GetSelectedUnitsAtPoint02 = 0x0046FCD6;
const u32 Hook_GetSelectedUnitsAtPoint02_Back = 0x0046FCDD;
void __declspec(naked) getSelectedUnitsAtPoint02() {
static s32 num0 = -(SELECT_UNIT_COUNT*4+12);
static s32 num1 = -(SELECT_UNIT_COUNT*8+12);

  __asm{
  MOV EAX, num0
  LEA EAX, [EBP+EAX]
  PUSH EAX
  MOV EDI, num1
  LEA EDI, [EBP+EDI]
  JMP Hook_GetSelectedUnitsAtPoint02_Back
  }

}

const u32 Hook_CMDACT_Select00 = 0x004C0861;
const u32 Hook_CMDACT_Select00_Back = 0x004C0866;
void __declspec(naked) CMDACT_Select00() {
static s32 num = (SELECT_UNIT_COUNT*2+4)*3+8;

  __asm{
  MOV EBP, ESP
  SUB ESP, num
  JMP Hook_CMDACT_Select00_Back
  }

}

const u32 Hook_CMDACT_Select01 = 0x004C086F;
const u32 Hook_CMDACT_Select01_Back = 0x004C08C0;
void __declspec(naked) CMDACT_Select01() {
static s32 num0 = -((SELECT_UNIT_COUNT*2+4)*3+8);
static s32 num1 = ((SELECT_UNIT_COUNT*2+4)*3)/4;

  __asm{
  XOR EAX, EAX
  MOV ECX, num1
  MOV EDI, num0
  LEA EDI, [EBP+EDI]
  REP STOSD
  XOR ECX, ECX
  XOR EDX, EDX
  MOV EDI, [EBP+0Ch]
  MOV EBX, [EBP+8]
  TEST EBX, EBX
  JMP Hook_CMDACT_Select01_Back
  }

}

const u32 Hook_CMDACT_Select02 = 0x004C0959;
const u32 Hook_CMDACT_Select02_Back = 0x004C095E;
void __declspec(naked) CMDACT_Select02() {
static s32 num = -((SELECT_UNIT_COUNT*2+4)*3+6);

  __asm{
  MOV ESI, num
  ADD ESI, EBP
  MOV WORD PTR [ECX*2+ESI], AX
  JMP Hook_CMDACT_Select02_Back
  }

}

const u32 Hook_CMDACT_Select03 = 0x004C09E6;
const u32 Hook_CMDACT_Select03_Back = 0x004C09EB;
void __declspec(naked) CMDACT_Select03() {
static s32 num = -((SELECT_UNIT_COUNT*2+4)*2+6);

  __asm{
  MOV ESI, num
  ADD ESI, EBP
  MOV WORD PTR [ECX*2+ESI], AX
  JMP Hook_CMDACT_Select03_Back
  }

}

const u32 Hook_CMDACT_Select04 = 0x004C0A65;
const u32 Hook_CMDACT_Select04_Back = 0x004C0A6C;
void __declspec(naked) CMDACT_Select04() {
static s32 num = -((SELECT_UNIT_COUNT*2+4)+6);

  __asm{
  MOV ESI, num
  ADD ESI, EBP
  MOV WORD PTR [EDI*2+ESI], AX
  TEST EDI, EDI
  JMP Hook_CMDACT_Select04_Back
  }

}

const u32 Hook_CMDACT_Select05 = 0x004C0A71;
const u32 Hook_CMDACT_Select05_Back = 0x004C0A7F;
void __declspec(naked) CMDACT_Select05() {
static s32 num = -((SELECT_UNIT_COUNT*2+4)+8);

  __asm{
  MOV ECX, num
  MOV BYTE PTR [EBP+ECX], 9
  MOV BYTE PTR [EBP+ECX+1], AL
  LEA ECX, [EBP+ECX]
  LEA EDX, [EAX+EAX+2]
  JMP Hook_CMDACT_Select05_Back
  }

}

const u32 Hook_CMDACT_Select06 = 0x004C0A91;
const u32 Hook_CMDACT_Select06_Back = 0x004C0A9F;
void __declspec(naked) CMDACT_Select06() {
static s32 num = -((SELECT_UNIT_COUNT*2+4)*2+8);

  __asm{
  MOV ECX, num
  MOV BYTE PTR [EBP+ECX], 0Bh
  MOV BYTE PTR [EBP+ECX+1], DL
  LEA ECX, [EBP+ECX]
  LEA EDX, [EDX+EDX+2]
  JMP Hook_CMDACT_Select06_Back
  }

}

const u32 Hook_CMDACT_Select07 = 0x004C0AAB;
const u32 Hook_CMDACT_Select07_Back = 0x004C0AB9;
void __declspec(naked) CMDACT_Select07() {
static s32 num = -((SELECT_UNIT_COUNT*2+4)*3+8);

  __asm{
  MOV ECX, num
  MOV BYTE PTR [EBP+ECX], 0Ah
  MOV BYTE PTR [EBP+ECX+1], AL
  LEA ECX, [EBP+ECX]
  LEA EDX, [EAX+EAX+2]
  JMP Hook_CMDACT_Select07_Back
  }

}


const u32 Hook_InitializeBullet = 0x0048BF21;
const u32 Hook_InitializeBullet_Back = 0x0048BF27;
void __declspec(naked) initializeBullet() {
static CBullet* bullet;

  __asm{
  PUSHAD
  MOV bullet, ESI
  }

  if(scbw::isCheatEnabled(CheatFlags::IAmTheCrazyBastardAroundHere) && playerTable[bullet->unknown_0x4C].type == PlayerType::Human){
	  __asm{
	  POPAD
	  MOV [ESI+61h], 255
	  }
  }
  else{
	  __asm{
	  POPAD
	  MOV [ESI+61h], DL
	  }
  }

  __asm{
  MOV DL, BYTE PTR [ESI+20h]
  JMP Hook_InitializeBullet_Back
  }

}

const char* cafe_URL = "http://cafe.naver.com/sdraft";
const char* moddb_URL = "http://www.moddb.com/mods/burningground";

const u32 Hook_GluMain_Dlg_Interact = 0x004DB846;
const u32 Hook_GluMain_Dlg_Interact_Back00 = 0x004DB8E7;
const u32 Hook_GluMain_Dlg_Interact_Back01 = 0x004DB84D;
const u32 Hook_GluMain_Dlg_Interact_Back02 = 0x004DB87B;
void __declspec(naked) gluMain_Dlg_Interact() {
static u32 control;

	__asm{
		DEC EAX
		JNZ SHORT _jmp0
		JMP Hook_GluMain_Dlg_Interact_Back01
_jmp0:
		PUSHAD
		MOV control, EAX
	}

	if(control == 6){
		SetForegroundWindow(GetDesktopWindow());
		ShellExecute(NULL, "open", cafe_URL, NULL, NULL, SW_SHOWDEFAULT);

		__asm{
			POPAD
			JMP Hook_GluMain_Dlg_Interact_Back02
		}
	}
	else if(control == 7){
		SetForegroundWindow(GetDesktopWindow());
		ShellExecute(NULL, "open", moddb_URL, NULL, NULL, SW_SHOWDEFAULT);

		__asm{
			POPAD
			JMP Hook_GluMain_Dlg_Interact_Back02
		}
	}
	else{
		__asm{
			POPAD
			JMP Hook_GluMain_Dlg_Interact_Back00
		}
	}
}

const u32 Hook_SetRepulseAngle = 0x00453325;
const u32 Hook_SetRepulseAngle_Back00 = 0x0045332F;
const u32 Hook_SetRepulseAngle_Back01 = 0x004533C2;
void __declspec(naked) setRepulseAngle() {

  __asm{
  CMP AX, 49h
  JZ SHORT _jmp
  CMP AX, 37h
  JZ SHORT _jmp
  CMP AX, 62h
  JZ SHORT _jmp
  JMP Hook_SetRepulseAngle_Back00
_jmp:
  JMP Hook_SetRepulseAngle_Back01
  }

}

const u32 Hook_RemoveRepulseTile = 0x004533ED;
const u32 Hook_RemoveRepulseTile_Back00 = 0x004533F3;
const u32 Hook_RemoveRepulseTile_Back01 = 0x00453419;
void __declspec(naked) removeRepulseTile() {

  __asm{
  CMP CX, 49h
  JZ SHORT _jmp
  CMP CX, 37h
  JZ SHORT _jmp
  CMP CX, 62h
  JZ SHORT _jmp
  JMP Hook_RemoveRepulseTile_Back00
_jmp:
  JMP Hook_RemoveRepulseTile_Back01
  }

}

const u32 Hook_Sub_453420 = 0x00453447;
const u32 Hook_Sub_453420_Back00 = 0x0045344D;
const u32 Hook_Sub_453420_Back01 = 0x0045347D;
void __declspec(naked) sub_453420() {

  __asm{
  CMP AX, 49h
  JZ SHORT _jmp
  CMP AX, 37h
  JZ SHORT _jmp
  CMP AX, 62h
  JZ SHORT _jmp
  JMP Hook_Sub_453420_Back00
_jmp:
  JMP Hook_Sub_453420_Back01
  }

}

const u32 Hook_Sub_4535A0 = 0x004535CB;
const u32 Hook_Sub_4535A0_Back00 = 0x004535D5;
const u32 Hook_Sub_4535A0_Back01 = 0x00453671;
void __declspec(naked) sub_4535A0() {

  __asm{
  CMP AX, 49h
  JZ SHORT _jmp
  CMP AX, 37h
  JZ SHORT _jmp
  CMP AX, 62h
  JZ SHORT _jmp
  JMP Hook_Sub_4535A0_Back00
_jmp:
  JMP Hook_Sub_4535A0_Back01
  }

}

const u32 Hook_UnitCanAttackTarget = 0x004767CB;
const u32 Hook_UnitCanAttackTarget_Back00 = 0x004767D4;
const u32 Hook_UnitCanAttackTarget_Back01 = 0x004767FC;
const u32 Hook_UnitCanAttackTarget_Back02 = 0x004767F3;
const u32 Func_UnitIsReaver = 0x00401490;
void __declspec(naked) unitCanAttackTarget() {
static CUnit *unit, *target;

  __asm{
  CALL Func_UnitIsReaver
  TEST EAX, EAX
  JZ SHORT _jmp0
  JMP Hook_UnitCanAttackTarget_Back00
_jmp0:
  CMP [ESI+64h], 33h
  JZ SHORT _jmp1
  JMP Hook_UnitCanAttackTarget_Back01
_jmp1:
  PUSHAD
  MOV unit, ESI
  MOV target, EBX
  }

  if(!(target->status & UnitStatus::Invincible) && 
	  scbw::getDistanceFast(unit->secondaryOrderPos.x, unit->secondaryOrderPos.y, target->getX(), target->getY()) <= BRONTES_ATTACKRADIUS){
	  __asm{
		  POPAD
		  JMP Hook_UnitCanAttackTarget_Back01
	  }
  }
  else{
	  __asm{
		  POPAD
		  JMP Hook_UnitCanAttackTarget_Back02
	  }
  }

}


extern GrpHead** iImagesGRPGraphic = new GrpHead*[IMAGE_TYPE_COUNT];
extern ImagesDatExtraOverlayLO_Files* iLO_s = new ImagesDatExtraOverlayLO_Files;
extern LO_Header** iImagesShieldOverlayGraphic = new LO_Header*[IMAGE_TYPE_COUNT];
extern u32* iImagesGRP = new u32[IMAGE_TYPE_COUNT];
extern u8*	iImagesGFXTurns = new u8[IMAGE_TYPE_COUNT];
extern u8*	iImagesClickable = new u8[IMAGE_TYPE_COUNT];
extern u8*	iGubImageUseScript = new u8[IMAGE_TYPE_COUNT];
extern u8*	iImagesDrawifCloaked = new u8[IMAGE_TYPE_COUNT];
extern u8*	iGubImageRLE = new u8[IMAGE_TYPE_COUNT];
extern u8*	iGubImageColorShift = new u8[IMAGE_TYPE_COUNT];
extern u32*	iImagesIscriptID = new u32[IMAGE_TYPE_COUNT];
extern u32*	iImagesShieldOverlay = new u32[IMAGE_TYPE_COUNT];
extern u32*	iImagesAttackOverlay = new u32[IMAGE_TYPE_COUNT];
extern u32*	iImagesDamageOverlay = new u32[IMAGE_TYPE_COUNT];
extern u32*	iImagesSpecialOverlay = new u32[IMAGE_TYPE_COUNT];
extern u32*	iImagesLandingDustOverlay = new u32[IMAGE_TYPE_COUNT];
extern u32*	iImagesLiftoffOverlay = new u32[IMAGE_TYPE_COUNT];
u8*			iImagesSomething = new u8[IMAGE_TYPE_COUNT+1];

extern u16*	sImageID = new u16[SPRITE_TYPE_COUNT];
extern u8*	sHealthBarSize = new u8[SPRITE_TYPE_COUNT - SPRITE_TYPE_START];
extern u8*	sUnknown2 = new u8[SPRITE_TYPE_COUNT];
extern u8*	sIsVisible = new u8[SPRITE_TYPE_COUNT];
extern u8*	sSelCircleImage = new u8[SPRITE_TYPE_COUNT - SPRITE_TYPE_START];
extern u8*	sSelCircleOffset = new u8[SPRITE_TYPE_COUNT - SPRITE_TYPE_START];

extern u16*	fSpriteID = new u16[FLINGY_TYPE_COUNT];
extern u32*	fTopSpeed = new u32[FLINGY_TYPE_COUNT];
extern u16*	fAcceleration = new u16[FLINGY_TYPE_COUNT];
extern u32*	fHaltDistance = new u32[FLINGY_TYPE_COUNT];
extern u8*	fTurnRadius = new u8[FLINGY_TYPE_COUNT];
extern u8*	fUnused = new u8[FLINGY_TYPE_COUNT];
extern u8*	fMoveControl = new u8[FLINGY_TYPE_COUNT];

UNK* makeCleanSFXDAT_Unknown(int count){ 

	UNK* _Unknown = new UNK[count];
	for(int x = 0; x < count; ++x)
		_Unknown[x] = NULL;

	return _Unknown;
}

extern u8*	SFXDAT_Volume = new u8[SOUND_TYPE_COUNT];
extern u8*	SFXDAT_Flags = new u8[SOUND_TYPE_COUNT];
extern u16*	SFXDAT_Race = new u16[SOUND_TYPE_COUNT];
extern u8*	SFXDAT_Type = new u8[SOUND_TYPE_COUNT];
extern u32*	SFXDAT_SoundFile = new u32[SOUND_TYPE_COUNT];
extern UNK*	SFXDAT_Unknown = makeCleanSFXDAT_Unknown(SOUND_TYPE_COUNT*16);

extern u32* PORTDAT_IdleDir = new u32[PORTRAIT_TYPE_COUNT];
extern u32* PORTDAT_TalkingDir = new u32[PORTRAIT_TYPE_COUNT];
extern u8* PORTDAT_IdleSMKChange = new u8[PORTRAIT_TYPE_COUNT];
extern u8* PORTDAT_TalkingSMKChange = new u8[PORTRAIT_TYPE_COUNT];
extern u8* PORTDAT_IdleUnknown1 = new u8[PORTRAIT_TYPE_COUNT];
extern u8* PORTDAT_TalkingUnknown1 = new u8[PORTRAIT_TYPE_COUNT];

extern CUnit** localSelectionGroup = new CUnit*[SELECT_UNIT_COUNT];
CUnit** localSelectionGroup2 = new CUnit*[SELECT_UNIT_COUNT];
CUnit** activeUnitSelection= new CUnit*[SELECT_UNIT_COUNT];
CUnit** CurrentUnitSelection= new CUnit*[SELECT_UNIT_COUNT];
CUnit** AllPlayerSelectionGroups= new CUnit*[8*SELECT_UNIT_COUNT]; //PLAYER_COUNT는 12
CUnit** selections = new CUnit*[144*SELECT_UNIT_COUNT];//8*18 값임
u16* selGroupType= new u16[SELECT_UNIT_COUNT];
u32* selGroupHp = new u32[SELECT_UNIT_COUNT];
u32* dword_63FE40 = new u32[SELECT_UNIT_COUNT*8/3];

static SightStruct* sightData = new SightStruct[MAX_SIGHT_RANGE + 1];

void setSightRange(){
	for (unsigned int x = 0; x <= MAX_SIGHT_RANGE; ++x) {
		sightData[x].tileSightWidth   = x * 2 + 3;
		sightData[x].tileSightHeight  = x * 2 + 3;
		sightData[x].unknown1 = 3;
		sightData[x].unknown2 = 3;
		sightData[x].unknown3 = 0;
		sightData[x].unknown4 = 0;
		sightData[x].unknown5 = 0;  
	}

	return;
}

const u32 orders_Repair10 = 0x00467446;
const u32 orders_Repair11 = 0x0046744E;

const u32 jmpCheckStar0 = 0x004E0B02;
const u32 jmpCheckStar1 = 0x004E0B07;

namespace hooks {

void inject_BG() {
  memoryPatch_Byte(0x004DB7E0, sizeof(mainBinOffset));
  memoryPatch(0x004DB7E3, &mainBinOffset);
  jmpPatch(gluMain_Dlg_Interact, Hook_GluMain_Dlg_Interact);

  memoryPatch_ByteNo(0x006556E4, -1,2);
  memoryPatch(0x0049848A, ((u32)&colorShiftData->data)+0x64);
  memoryPatch(0x004BDAC3, ((u32)&colorShiftData->data)+0x14);
  memoryPatch(0x004BDE6D, ((u32)&colorShiftData->data)+0x14);
  memoryPatch(0x004BDE74, sizeof(colorShiftData)/sizeof(ColorShiftData)-1);
  memoryPatch(0x004D4B21, sizeof(colorShiftData)/sizeof(ColorShiftData));
  memoryPatch(0x004D4B26, &colorShiftData[sizeof(colorShiftData)/sizeof(ColorShiftData)].data);
  memoryPatch(0x004D4B3D, &colorShiftData->data);
  memoryPatch(0x004D545A, ((u32)&colorShiftData->data)+0x14);
  memoryPatch(0x004D548A, ((u32)&colorShiftData->data)+0x14);
  memoryPatch(0x004D54B4, ((u32)&colorShiftData->data)+0x64);
  memoryPatch(0x004D54D4, ((u32)&colorShiftData->data)+0x64);
  memoryPatch(0x004D54F4, ((u32)&colorShiftData->data)+0x64);
  memoryPatch(0x004D5534, ((u32)&colorShiftData->data)+0x64);
  memoryPatch(0x004D56A4, &colorShiftData->data);
  memoryPatch(0x004D5AF6, &colorShiftData->data); 

  memoryPatch_Byte(0x004C3BBD, 0x16);//hp바 색상 늘림
  memoryPatch(0x004C3BBF, hpBarColorBuffer); 
  memoryPatch(0x0047A17C, hpBarColorBuffer); 
  memoryPatch(0x0047A1E5, hpBarColorBuffer); 
  memoryPatch(0x0047A3BC, hpBarColorBuffer); 
  memoryPatch(0x0047A40B, hpBarColorBuffer); 
  memoryPatch(0x0047A54C, hpBarColorBuffer); 
  memoryPatch(0x0047A59B, hpBarColorBuffer); 

  memoryPatch_Byte(0x00456AAD, 0x1C);//와이어프레임 색상 늘림
  memoryPatch(0x00456AAF, wireColorBuffer); 
  memoryPatch(0x004566F7, wireColorBuffer); 
  memoryPatch(0x00456705, wireColorBuffer); 
  memoryPatch(0x00456716, ((u32)wireColorBuffer)+2);
  memoryPatch(0x00456779, wireColorBuffer); 
  memoryPatch(0x0045678D, wireColorBuffer); 
  memoryPatch(0x004567A9, wireColorBuffer); 
  memoryPatch(0x004567AF, wireColorBuffer); 
  memoryPatch(0x004567C7, ((u32)wireColorBuffer)+2);
  memoryPatch(0x00456885, wireColorBuffer); 
  
  memoryPatch_Byte(0x00457D7E, 0x20+SELECT_UNIT_COUNT);
  memoryPatch(0x00425A0C, 0x20+SELECT_UNIT_COUNT);
  memoryPatch(0x004584C3, sizeof(statDataBinOffset));//데이터창 오프셋 메모리 크기
  memoryPatch(0x004584C8, &statDataBinOffset);//데이터창 오프셋 메모리 크기
  
  memoryPatch(0x004234D5, localSelectionGroup);
  memoryPatch(0x004234F0, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00423556, localSelectionGroup);
  memoryPatch(0x004235CD, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00423683, localSelectionGroup);
  memoryPatch(0x004236A9, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00424385, localSelectionGroup);
  memoryPatch(0x004243DD, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00424552, localSelectionGroup);
  memoryPatch(0x0042457C, ((u32)localSelectionGroup)+4);
  memoryPatch(0x004245A5, ((u32)localSelectionGroup)+8);
  memoryPatch(0x004245CF, ((u32)localSelectionGroup)+12);
  memoryPatch(0x004245F9, ((u32)localSelectionGroup)+16);
  memoryPatch(0x00424623, ((u32)localSelectionGroup)+20);
  memoryPatch(0x0042466D, localSelectionGroup);
  memoryPatch(0x00424691, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x004259C3, localSelectionGroup);
  memoryPatch(0x004259C9, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428311, localSelectionGroup);
  memoryPatch(0x00428327, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428373, localSelectionGroup);
  memoryPatch(0x004283A5, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x004283C1, localSelectionGroup);
  memoryPatch(0x004283DA, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428481, localSelectionGroup);
  memoryPatch(0x00428499, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x004284B1, localSelectionGroup);
  memoryPatch(0x004284C9, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428571, localSelectionGroup);
  memoryPatch(0x00428587, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x004285B1, localSelectionGroup);
  memoryPatch(0x004285C7, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x004285E1, localSelectionGroup);
  memoryPatch(0x004285F7, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428611, localSelectionGroup);
  memoryPatch(0x00428627, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428641, localSelectionGroup);
  memoryPatch(0x00428657, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428671, localSelectionGroup);
  memoryPatch(0x00428687, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x004286A1, localSelectionGroup);
  memoryPatch(0x004286CA, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x004286E1, localSelectionGroup);
  memoryPatch(0x0042870E, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428734, localSelectionGroup);
  memoryPatch(0x00428753, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428784, localSelectionGroup);
  memoryPatch(0x004287A3, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428861, localSelectionGroup);
  memoryPatch(0x00428880, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x004289A1, localSelectionGroup);
  memoryPatch(0x004289B7, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428A21, localSelectionGroup);
  memoryPatch(0x00428A37, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428D41, localSelectionGroup);
  memoryPatch(0x00428D81, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428DA1, localSelectionGroup);
  memoryPatch(0x00428DE7, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428E22, localSelectionGroup);
  memoryPatch(0x00428EA2, localSelectionGroup);
  memoryPatch(0x00428EBA, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428ED2, localSelectionGroup);
  memoryPatch(0x00428F13, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428F32, localSelectionGroup);
  memoryPatch(0x00428F7E, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428FA4, localSelectionGroup);
  memoryPatch(0x00428FC3, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00428FF3, localSelectionGroup);
  memoryPatch(0x00429053, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x0042907D, localSelectionGroup);
  memoryPatch(0x004290C3, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00429100, localSelectionGroup);
  memoryPatch(0x00429149, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00429188, localSelectionGroup);
  memoryPatch(0x004291AB, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x004291D8, localSelectionGroup);
  memoryPatch(0x004291FB, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x0042921D, localSelectionGroup);
  memoryPatch(0x00429298, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x004292CD, localSelectionGroup);
  memoryPatch(0x0042934A, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00429380, localSelectionGroup);
  memoryPatch(0x004293BE, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x004293F0, localSelectionGroup);
  memoryPatch(0x00429430, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00429486, localSelectionGroup);
  memoryPatch(0x004294B3, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x00455A07, ((u32)localSelectionGroup)+(SELECT_UNIT_COUNT*4)-4);
  memoryPatch(0x00455BBA, localSelectionGroup);
  memoryPatch(0x004563D3, localSelectionGroup);
  memoryPatch(0x00456403, localSelectionGroup);
  memoryPatch(0x00456543, localSelectionGroup);
  memoryPatch(0x004565C3, localSelectionGroup);
  memoryPatch(0x00458BD7, localSelectionGroup);
  memoryPatch(0x00458C8F, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x0046F657, localSelectionGroup);
  memoryPatch(0x0046F7E8, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x004C389E, localSelectionGroup);
  memoryPatch(0x004C38BE, localSelectionGroup);
  memoryPatch(0x004C38D4, localSelectionGroup);
  memoryPatch(0x004C38F4, &localSelectionGroup[SELECT_UNIT_COUNT]);
  memoryPatch(0x004C3911, ((u32)localSelectionGroup)+4);
  memoryPatch(0x004C3917, localSelectionGroup);
  memoryPatch(0x004C3C3D, localSelectionGroup);
  memoryPatch(0x004E5692, localSelectionGroup);
  memoryPatch(0x004564FD, localSelectionGroup2);
  memoryPatch(0x004BF8C5, localSelectionGroup2);
  memoryPatch(0x004BF8D0, ((u32)localSelectionGroup2)+4);
  memoryPatch(0x004BF8DB, ((u32)localSelectionGroup2)+8);
  memoryPatch(0x004BF8E6, ((u32)localSelectionGroup2)+12);
  memoryPatch(0x004BF8F1, ((u32)localSelectionGroup2)+16);
  memoryPatch(0x004BF8FC, ((u32)localSelectionGroup2)+20);
  memoryPatch(0x004BF933, localSelectionGroup2);
  memoryPatch(0x004BF942, ((u32)localSelectionGroup2)-4);
  memoryPatch(0x004BF94A, localSelectionGroup2);
  memoryPatch(0x004BF951, localSelectionGroup2);
  memoryPatch(0x004C07E1, localSelectionGroup2);
  memoryPatch(0x004C07F4, localSelectionGroup2);
  memoryPatch(0x004C08E3, localSelectionGroup2);
  memoryPatch(0x004C08FA, localSelectionGroup2);
  memoryPatch(0x004C096F, localSelectionGroup2);
  memoryPatch(0x004C0A03, localSelectionGroup2);
  memoryPatch(0x004C0A11, localSelectionGroup2);
  memoryPatch(0x004EED2C, localSelectionGroup2);
  memoryPatch(0x004EED31, ((u32)localSelectionGroup2)+4);
  memoryPatch(0x004EED36, ((u32)localSelectionGroup2)+8);
  memoryPatch(0x004D083E, activeUnitSelection);
  memoryPatch(0x004EEDDD, activeUnitSelection);
  memoryPatch(0x0045D04E, CurrentUnitSelection);
  memoryPatch(0x0046FA98, CurrentUnitSelection);
  memoryPatch(0x0046FAA8, CurrentUnitSelection);
  memoryPatch(0x0046FBAB, CurrentUnitSelection);
  memoryPatch(0x0046FC96, CurrentUnitSelection);
  memoryPatch(0x0046FCF7, CurrentUnitSelection);
  memoryPatch(0x00499A73, CurrentUnitSelection);
  memoryPatch(0x00499A9B, CurrentUnitSelection);
  memoryPatch(0x0049A2C7, CurrentUnitSelection);
  memoryPatch(0x0049A305, &CurrentUnitSelection[SELECT_UNIT_COUNT]);
  memoryPatch(0x0049AE49, CurrentUnitSelection);
  memoryPatch(0x0049AE77, &CurrentUnitSelection[SELECT_UNIT_COUNT]);
  memoryPatch(0x0049AE95, CurrentUnitSelection);
  memoryPatch(0x0049AEB9, CurrentUnitSelection);
  memoryPatch(0x0049AEC9, CurrentUnitSelection);
  memoryPatch(0x0049B6A3, CurrentUnitSelection);
  memoryPatch(0x0049B6FA, CurrentUnitSelection);
  memoryPatch(0x0049F7C3, CurrentUnitSelection);
  memoryPatch(0x0049F7D4, CurrentUnitSelection);
  memoryPatch(0x004C38B9, CurrentUnitSelection);
  memoryPatch(0x004C3B4A, CurrentUnitSelection);
  memoryPatch(0x004C3B64, &CurrentUnitSelection[SELECT_UNIT_COUNT]);
  memoryPatch(0x004D603F, CurrentUnitSelection);
  memoryPatch(0x004EED19, CurrentUnitSelection);
  memoryPatch(0x004966D4, AllPlayerSelectionGroups);
  memoryPatch(0x00496A8C, AllPlayerSelectionGroups);
  memoryPatch(0x0049A228, AllPlayerSelectionGroups);
  memoryPatch(0x0049A244, AllPlayerSelectionGroups);
  memoryPatch(0x0049A75A, AllPlayerSelectionGroups);
  memoryPatch(0x0049A772, AllPlayerSelectionGroups);
  memoryPatch(0x0049A873, AllPlayerSelectionGroups);
  memoryPatch(0x0049AFBE, AllPlayerSelectionGroups);
  memoryPatch(0x0049B729, AllPlayerSelectionGroups);
  memoryPatch(0x004C265C, AllPlayerSelectionGroups);
  memoryPatch(0x004C26B9, AllPlayerSelectionGroups);
  memoryPatch(0x004C27DA, AllPlayerSelectionGroups);
  memoryPatch(0x004C2D23, AllPlayerSelectionGroups);
  memoryPatch(0x004CEDA3, AllPlayerSelectionGroups);
  memoryPatch(0x004CEE04, AllPlayerSelectionGroups);
  memoryPatch(0x004D013F, AllPlayerSelectionGroups);
  memoryPatch(0x004EED25, AllPlayerSelectionGroups);
  memoryPatch(0x004EEDD3, AllPlayerSelectionGroups);
  memoryPatch(0x004EEDEB, AllPlayerSelectionGroups);
  memoryPatch(0x0049A193, ((u32)AllPlayerSelectionGroups)+4);
  memoryPatch(0x0049A22E, ((u32)AllPlayerSelectionGroups)+4);
  memoryPatch(0x004C2584, ((u32)AllPlayerSelectionGroups)+4);
  memoryPatch(0x00499AE3, &AllPlayerSelectionGroups[8][SELECT_UNIT_COUNT]);
  memoryPatch(0x004965FE, selections);
  memoryPatch(0x00496655, ((u32)selections)+4);
  memoryPatch(0x004966A0, selections);
  memoryPatch(0x00496769, selections);
  memoryPatch(0x0049680E, ((u32)selections)+4);
  memoryPatch(0x00496854, selections);
  memoryPatch(0x00496968, ((u32)selections)+4);
  memoryPatch(0x004969F2, selections);
  memoryPatch(0x00496AC7, selections);
  memoryPatch(0x00496ACE, selections);
  memoryPatch(0x00496AD5, selections);
  memoryPatch(0x00496B60, selections);
  memoryPatch(0x00496D51, ((u32)selections)+(72*SELECT_UNIT_COUNT)+4);
  memoryPatch(0x004EEC79, selections);
  memoryPatch(0x00424528, selGroupHp);
  memoryPatch(0x00424565, selGroupHp);
  memoryPatch(0x0042458F, ((u32)selGroupHp)+4);
  memoryPatch(0x004245B8, ((u32)selGroupHp)+8);
  memoryPatch(0x004245E2, ((u32)selGroupHp)+12);
  memoryPatch(0x0042460C, ((u32)selGroupHp)+16);
  memoryPatch(0x00424636, ((u32)selGroupHp)+20);
  memoryPatch(0x00424663, selGroupHp);
  memoryPatch(0x00424987, selGroupHp);
  memoryPatch(0x00424F2E, ((u32)selGroupHp)+4);
  memoryPatch(0x00424FCA, ((u32)selGroupHp)+8);
  memoryPatch(0x0042514E, selGroupHp);
  memoryPatch(0x00425187, selGroupHp);
  memoryPatch(0x00425901, selGroupHp);
  memoryPatch(0x00426F1E, selGroupHp);
  memoryPatch(0x00426F5E, selGroupHp);
  memoryPatch(0x004274CE, selGroupHp);
  memoryPatch(0x0042789D, selGroupHp);
  memoryPatch(0x00424542, ((u32)selGroupType)+2);
  memoryPatch(0x00424653, ((u32)&selGroupType[SELECT_UNIT_COUNT])+2);
  memoryPatch(0x00424668, selGroupType);
  memoryPatch(0x00496568, ((u32)dword_63FE40)+16);
  memoryPatch(0x00496B2E, dword_63FE40);
  memoryPatch(0x00496D83, dword_63FE40);
  memoryPatch(0x00496D8E, dword_63FE40);
  memoryPatch(0x00496DA8, dword_63FE40);
  memoryPatch(0x00496DB3, dword_63FE40);
  memoryPatch(0x00496DCE, dword_63FE40);
  memoryPatch(0x00496DD9, dword_63FE40);
  memoryPatch(0x00496DF4, dword_63FE40);
  memoryPatch(0x00496DFF, dword_63FE40);
  memoryPatch(0x00496E1A, dword_63FE40);
  memoryPatch(0x00496E25, dword_63FE40);
  memoryPatch(0x00496E40, dword_63FE40);
  memoryPatch(0x00496E4B, dword_63FE40);
  memoryPatch(0x004BFC42, dword_63FE40);
  memoryPatch(0x004C270B, ((u32)dword_63FE40)+16);
  memoryPatch(0x004C2742, dword_63FE40);
  memoryPatch(0x004C2858, dword_63FE40);
  memoryPatch(0x004EEC85, dword_63FE40);
  
  memoryPatch_Byte(0x004965FB, 5);
  memoryPatch_Byte(0x0049669D, 5);
  memoryPatch_Byte(0x004966D1, 5);
  memoryPatch_Byte(0x00496761, 0xCF);//*8로 바꿈
  memoryPatch_Byte(0x00496807, 5);
  memoryPatch_Byte(0x00496960, 5);
  memoryPatch_Byte(0x004969EB, 3);
  memoryPatch_Byte(0x00496A88, 0xD7);//*8로 바꿈
  memoryPatch_Byte(0x00496B5D, 5);
  memoryPatch_Byte(0x0049A18A, 5);
  memoryPatch_Byte(0x0049A757, 5);
  memoryPatch_Byte(0x0049A222, 0xCB);
  memoryPatch_Byte(0x0049A240, 0xD0);
  memoryPatch_Byte(0x0049A86F, 0xC1);
  memoryPatch_Byte(0x0049AFBA, 0xD3);
  memoryPatch_Byte(0x004C2580, 5);
  memoryPatch_Byte(0x004C265A, 5);
  memoryPatch_Byte(0x004C26B5, 0xCF);
  memoryPatch_Byte(0x004C27D8, 5);
  
  memoryPatch_Byte(0x00428E46, SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x004563E7, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x004563F2, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x00456417, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x00456422, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x00456557, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x00456563, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x004565D7, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x004565E3, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x0046F208, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x0046F2AF, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x0046F33C, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x00496A73, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x004BF909, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x004C08F2, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x004D0825, SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x004D083A, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x004D0869, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x004D0886, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x004D08CD, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x004D08D5, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x0045D045, SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x0045D054, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x0045D063, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x0045D069, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x0045D076, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x0045D082, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x0045D086, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x0045D08E, -SELECT_UNIT_COUNT*4);
  //1바이트에 128 이상의 값을 넣으면 -가 되므로 따로 함
  //memoryPatch_Byte(0x0046FA45, SELECT_UNIT_COUNT*8);
  jmpPatch(getSelectedUnitsInBox00, Hook_GetSelectedUnitsInBox00);
  memoryPatch_Byte(0x0046FA59, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x0046FA69, -SELECT_UNIT_COUNT*4);
  //memoryPatch_Byte(0x0046FAAE, -SELECT_UNIT_COUNT*8);
  //memoryPatch_Byte(0x0046FAB2, -SELECT_UNIT_COUNT*4);
  //memoryPatch_Byte(0x0046FAB8, -SELECT_UNIT_COUNT*8);
  jmpPatch(getSelectedUnitsInBox01, Hook_GetSelectedUnitsInBox01);
  memoryPatch_Byte(0x0046FAE4, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x0046FAFA, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x0046FB09, -SELECT_UNIT_COUNT*4);
  //memoryPatch_Byte(0x0046FB45, SELECT_UNIT_COUNT*8+12);
  jmpPatch(getSelectedUnitsAtPoint00, Hook_GetSelectedUnitsAtPoint00);
  memoryPatch_Byte(0x0046FB9A, -(SELECT_UNIT_COUNT*4+12));
  //memoryPatch_Byte(0x0046FC9C, -(SELECT_UNIT_COUNT*8+12));
  jmpPatch(getSelectedUnitsAtPoint01, Hook_GetSelectedUnitsAtPoint01);
  memoryPatch_Byte(0x0046FCAA, -(SELECT_UNIT_COUNT*4+12));
  //memoryPatch_Byte(0x0046FCD8, -(SELECT_UNIT_COUNT*4+12));
  //memoryPatch_Byte(0x0046FCDC, -(SELECT_UNIT_COUNT*8+12));
  jmpPatch(getSelectedUnitsAtPoint02, Hook_GetSelectedUnitsAtPoint02);
  memoryPatch_Byte(0x0046FD09, -(SELECT_UNIT_COUNT*4+12));
  memoryPatch_Byte(0x0046FD26, -(SELECT_UNIT_COUNT*4+12));
  memoryPatch_Byte(0x0046FD62, -(SELECT_UNIT_COUNT*4+12));
  memoryPatch_Byte(0x0046FD6A, -(SELECT_UNIT_COUNT*4+12));
  memoryPatch_Byte(0x0046FD87, -(SELECT_UNIT_COUNT*4+8));
  memoryPatch_Byte(0x0046FD8C, -(SELECT_UNIT_COUNT*4+12));
//46FD95, 49F7F5의 0Ch가 무엇을 의미하는지 모름. 아마 상관 없을거라 예상됨.
  memoryPatch_Byte(0x0046FD9E, -(SELECT_UNIT_COUNT*4+12));
  memoryPatch_Byte(0x0046FDAC, -(SELECT_UNIT_COUNT*4+12));
  memoryPatch_Byte(0x0046FDC5, -(SELECT_UNIT_COUNT*4+12));
  memoryPatch_Byte(0x0046FDCD, -(SELECT_UNIT_COUNT*4+12));
  memoryPatch_Byte(0x0046FE3E, -(SELECT_UNIT_COUNT*4+12));
  memoryPatch_Byte(0x0046FE69, -(SELECT_UNIT_COUNT*4+12));
  memoryPatch_Byte(0x0049F7A5, SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x0049F7DA, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x0049F7E9, -(SELECT_UNIT_COUNT*4-4));
  memoryPatch_Byte(0x0049F7EE, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x0049F7FD, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x0049F801, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x004C3B45, SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x004C3B5D, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x004C3B6D, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x004C3B75, -SELECT_UNIT_COUNT*4);
  memoryPatch_Byte(0x0046FD10, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x0046FD1D, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x00499AAB, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x00499B88, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x0049B70A, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x0049B83C, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x0049F7CE, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x00496690, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x00496765, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x0049677B, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x004969A0, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x0049A1F1, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x0049A215, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x0049A7AF, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x0049A859, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x0049AF8B, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x004C256F, SELECT_UNIT_COUNT);//확실치 않음
  memoryPatch_Byte(0x004C25C0, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x004C25E2, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x004C2670, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x004C2687, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x004C275D, SELECT_UNIT_COUNT);//확실치 않음
  memoryPatch_Byte(0x004C27EE, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x00496838, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x004968FA, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x00496B45, SELECT_UNIT_COUNT*4+8);
  memoryPatch_Byte(0x00496B72, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x00496B80, -(SELECT_UNIT_COUNT*4+8));
  memoryPatch_Byte(0x00496BFD, -(SELECT_UNIT_COUNT*4+8));
  memoryPatch_Byte(0x00496CA5, -(SELECT_UNIT_COUNT*4+8));
  memoryPatch_Byte(0x00496CB4, -(SELECT_UNIT_COUNT*4+8));
  memoryPatch_Byte(0x00496CE8, -(SELECT_UNIT_COUNT*4+8));
  memoryPatch_Byte(0x004BFB4E, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x00458225, SELECT_UNIT_COUNT*4+4);
  memoryPatch_Byte(0x0045823F, -(SELECT_UNIT_COUNT*4+4));
  memoryPatch_Byte(0x00458289, -(SELECT_UNIT_COUNT*4+4));
  memoryPatch_Byte(0x004582AF, -(SELECT_UNIT_COUNT*4+4));
  memoryPatch_Byte(0x004582FF, -(SELECT_UNIT_COUNT*4+4));
  memoryPatch_Byte(0x00458347, -(SELECT_UNIT_COUNT*4+4));
  memoryPatch_Byte(0x0045836F, -(SELECT_UNIT_COUNT*4+4));
  memoryPatch_Byte(0x00458377, -(SELECT_UNIT_COUNT*4+4));
  memoryPatch(0x004C3897, SELECT_UNIT_COUNT);
  memoryPatch(0x004C38B4, SELECT_UNIT_COUNT);//CurrentUnitSelection 중복
  memoryPatch(0x004C390A, SELECT_UNIT_COUNT-1);
  memoryPatch(0x004C3C38, SELECT_UNIT_COUNT);
  memoryPatch(0x004C07DC, SELECT_UNIT_COUNT);//그 밑의 코드도 수정이 필요한지 확인 필요
//RETN 인자 값도 0Ch이기 때문에 확인 필요
  memoryPatch(0x004C0977, SELECT_UNIT_COUNT);
  memoryPatch(0x004C09FE, SELECT_UNIT_COUNT);
  memoryPatch(0x004D0832, SELECT_UNIT_COUNT);
  memoryPatch(0x004D0843, SELECT_UNIT_COUNT);
  memoryPatch(0x004EEDD8, SELECT_UNIT_COUNT);
  memoryPatch(0x0045D049, SELECT_UNIT_COUNT);
  memoryPatch(0x0046F2B4, SELECT_UNIT_COUNT);
  memoryPatch(0x0046FA53, SELECT_UNIT_COUNT);
  memoryPatch(0x0046FAA3, SELECT_UNIT_COUNT);
  memoryPatch(0x0046FB8F, SELECT_UNIT_COUNT);
  memoryPatch(0x0046FC91, SELECT_UNIT_COUNT);
  memoryPatch(0x004EED14, SELECT_UNIT_COUNT);
  memoryPatch(0x004966AD, SELECT_UNIT_COUNT);
  memoryPatch(0x0049A18D, SELECT_UNIT_COUNT);
  memoryPatch(0x0049A75F, SELECT_UNIT_COUNT);
  memoryPatch(0x004C2D1E, SELECT_UNIT_COUNT*32);
  memoryPatch(0x004CEDA8, SELECT_UNIT_COUNT*8);
  memoryPatch(0x004CEE09, SELECT_UNIT_COUNT*8);
  memoryPatch(0x004D013A, SELECT_UNIT_COUNT*32);
  memoryPatch(0x004EED20, SELECT_UNIT_COUNT*8);
  memoryPatch(0x004EEDE6, SELECT_UNIT_COUNT*8);
  memoryPatch(0x00496B7A, SELECT_UNIT_COUNT);
  memoryPatch(0x00496D43, SELECT_UNIT_COUNT*72);
  memoryPatch(0x004EEC74, SELECT_UNIT_COUNT*144);
  memoryPatch(0x00458239, SELECT_UNIT_COUNT);
  memoryPatch(0x0045826F, SELECT_UNIT_COUNT);
  memoryPatch(0x004582F9, SELECT_UNIT_COUNT);
  memoryPatch(0x0045832F, SELECT_UNIT_COUNT);
  jmpPatch(CMDACT_Select00, Hook_CMDACT_Select00);
  jmpPatch(CMDACT_Select01, Hook_CMDACT_Select01);
  jmpPatch(CMDACT_Select02, Hook_CMDACT_Select02);
  jmpPatch(CMDACT_Select03, Hook_CMDACT_Select03);
  jmpPatch(CMDACT_Select04, Hook_CMDACT_Select04);
  jmpPatch(CMDACT_Select05, Hook_CMDACT_Select05);
  jmpPatch(CMDACT_Select06, Hook_CMDACT_Select06);
  jmpPatch(CMDACT_Select07, Hook_CMDACT_Select07);
  
  setSightRange();
  memoryPatch(0x00480354, &sightData->unknown5);
  memoryPatch(0x00480359, MAX_SIGHT_RANGE + 1);
  memoryPatch(0x00480709, sightData);
  memoryPatch(0x0048098B, MAX_SIGHT_RANGE + 1);
  memoryPatch(0x00480990, sightData);
  

  jmpPatch((void*)jmpCheckStar1, jmpCheckStar0);//스타 여러개 킬 수 있게 함
  jmpPatch(takeScreenshot0, Hook_TakeScreenshot00);
  jmpPatch(takeScreenshot1, Hook_TakeScreenshot01);//저장시 png 포맷으로 저장됨.
  jmpPatch(doWeaponIscript, Hook_DoWeaponIscript);//스카웃은 지상공격 쿨타임만 지나감.
  jmpPatch(ordersEntries_ReaverStop, Hook_OrdersEntries_ReaverStop);//리버 스탑 관련 튕김 방지
  jmpPatch(ordersEntries_CarrierStop, Hook_OrdersEntries_CarrierStop);//캐리어 스탑 관련 튕김 방지
  jmpPatch(btnsAct_CarrierMove, Hook_BTNSACT_CarrierMove);//캐리어 무브 아라크네, 브론테스는 팔로우 적용 안함
  jmpPatch(unitCanAttackTarget, Hook_UnitCanAttackTarget);//브론테스 공격범위 내에 타겟만 공격가능한 걸로 인식함
  jmpPatch(moveUnit0, Hook_MoveUnit0);
  jmpPatch(moveUnit1, Hook_MoveUnit1);//동력장 있는 유닛 무브시에 동력장 그래픽도 같이 따라옴
  jmpPatch(attackApplyCooldown0, Hook_AttackApplyCooldown0);//배틀 근접공격과 원거리공격 공격시작방향 차별화.
  jmpPatch(attackApplyCooldown1, Hook_AttackApplyCooldown1);//벌쳐와 데스 헤드 홀드시에도 건물 공격 정삭적으로 나오게 함
  jmpPatch(setRepulseAngle, Hook_SetRepulseAngle);//공중유닛 비벼지는 유닛 제외 설정
  jmpPatch(removeRepulseTile, Hook_RemoveRepulseTile);
  jmpPatch(sub_453420, Hook_Sub_453420);
  jmpPatch(sub_4535A0, Hook_Sub_4535A0);
  jmpPatch(CMDRECV_PlaceBuildingAllowed, Hook_CMDRECV_PlaceBuildingAllowed);//MCV 랜드 가능하게 수정
  jmpPatch(sub_496030_0, Hook_Sub_496030_0);//스피어백 달리기 관련 애니메이션 재생
  jmpPatch(sub_496030_1, Hook_Sub_496030_1);
  jmpPatch(killTargetUnitCheck2, Hook_KillTargetUnitCheck2);//인페테 공격받아서 사망시 자폭
  jmpPatch(orders_SapUnit0, Hook_Orders_SapUnit0);//인페테 아군 유닛 강제공격시 데미지 입힘
  jmpPatch(orders_SapUnit1, Hook_Orders_SapUnit1);//인페테가 무적 상태에서 자기 자신을 공격 대상으로 삼았을 때 명령이 취소되지 않게 함
  jmpPatch((void*)Hook_CreateBuildingFlamesBack, Hook_CreateBuildingFlames);//17방향 건물 불 이상하게 나는거 방지
  jmpPatch((void*)Hook_CreateDamageOverlay0Back, Hook_CreateDamageOverlay0);
  jmpPatch(createDamageOverlay1, Hook_CreateDamageOverlay1);
  jmpPatch(setImageDirection, Hook_SetImageDirection);//쉴드 오버레이의 방향은 변하지 않게 함
  jmpPatch((void*)Hook_UnitStatAct_Building0Back, Hook_UnitStatAct_Building0);//일반 테란 건물 방어력 표기
  jmpPatch(setUnitStatAct_Building1, Hook_UnitStatAct_Building1);//스텟 창이 특이한 테란 건물(ex. 서플, 리파이너리, 벙커 등) 방어력 표기. 터렛은 킬수 표기까지
  jmpPatch(setPlayBuildingCompleteSound, Hook_PlayBuildingCompleteSound);//scv 이외에 건물 건설 완료시 애드온 컴플리트 대신 에시비 건설완료음 나게 수정
  jmpPatch(getRemainingBuildTimePercentage, Hook_GetRemainingBuildTimePercentage);//유닛 생산시간이 0이어도 튕기지 않게 함. 
  jmpPatch((void*)Hook_KillTargetUnitCheckBack, Hook_KillTargetUnitCheck);//power overwhelming 치트 무시. weapon_damage 탭에서 겹치기 때문.
  jmpPatch(setSpellSpecialBtnGraphic, Hook_SetSpellSpecialBtnGraphic);//수송공간 8인 유닛도 아이콘 표시
  jmpPatch((void*)Hook_CreateInitialMeleeOverloadBack, Hook_CreateInitialMeleeOverload);//테란, 프로토스도 시작 시 오버로드를 줌
  jmpPatch((void*)Hook_CancelUnitBack, Hook_CancelUnit);//저그 건물 취소시 드론 안나오고 바로 파괴
  jmpPatch(extendCreateInitialMeleeUnits, Hook_CreateInitialMeleeUnits);//시작시 일꾼 늘림
  jmpPatch(imageRenderFxn11, Hook_ImageRenderFxn11);//최대마나 0인 유닛 마나 바 표시안함
  jmpPatch(printEnergyStr, Hook_PrintEnergyStr);//최대마나 0인 유닛 마나 스트링 표시안함
  jmpPatch(initializeUnitState, Hook_InitializeUnitState);
  jmpPatch(initializeUnitMembers, Hook_InitializeUnitMembers);
  jmpPatch(convertUnitStats, Hook_ConvertUnitStats);           //유닛변환시 쉴드도 비례변환함. 마나는 초기화만 안됨
  jmpPatch(convertUnit, Hook_ConvertUnit);
  jmpPatch(imageRenderFxn11_0, Hook_ImageRenderFxn11_0);//최대쉴드 0인 유닛 쉴드 바 표시안함
  jmpPatch(imageRenderFxn11_1, Hook_ImageRenderFxn11_1);//매트릭스 쉴드가 있으면 우선순위로 체력바 설정
  jmpPatch(compileHealthBar, Hook_CompileHealthBar);//마나 있고 쉴드 없는 유닛 디펜시브 쓰면 마나바 사라지는 문제 수정
  jmpPatch(sub_4566B0, Hook_Sub_4566B0);//매트릭스 쉴드 부여된 유닛 와이어프레임 효과 설정
  jmpPatch(setTextStr00, Hook_SetTextStr00);
  jmpPatch(setTextStr01, Hook_SetTextStr01);
  jmpPatch(sub_47A120, Hook_Sub_47A120);//매트릭스 쉴드 hp바는 초록색으로 나오게 함
  jmpPatch(unitStatCond_Standard, Hook_UnitStatCond_Standard);//매트릭스 쉴드 있는 유닛 선택시 데이터창 업데이트 가속
  jmpPatch(opcodeCases_Attack, Hook_OpcodeCases_Attack);	//attack 스크립트 명령어 관련 버그 고침
  jmpPatch(psiField0, Hook_PsiField00);						//psi 공급가능한 유닛 추가적용
  jmpPatch(psiField1, Hook_PsiField01);
  jmpPatch(psiField2, Hook_PsiField02);
  jmpPatch(psiField3, Hook_PsiField03);
  jmpPatch(psiField4, Hook_PsiField04);
  jmpPatch(psiField5, Hook_PsiField05);
  jmpPatch(getPlayerDefaultRefineryUnitType, Hook_GetPlayerDefaultRefineryUnitType);//켈모리안도 리파이너리 짓게 함
  jmpPatch((void*)orders_Repair11, orders_Repair10);//AI가 미완성건물 이어서 짓도록 만듬.
  jmpPatch(AI_GetGeyserState, Hook_AI_GetGeyserState);
  jmpPatch(AI_BuildGasBuildings, Hook_AI_BuildGasBuildings0);
  jmpPatch((void*)Hook_AI_BuildGasBuildings1Back, Hook_AI_BuildGasBuildings1);
  jmpPatch(AI_BuildUnitAtTown, Hook_AI_BuildUnitAtTown);
  jmpPatch(AI_BuildSupplyUnits, Hook_AI_BuildSupplyUnits);//켈모리안 인구는 서플라이로 채움
  jmpPatch(orderAllMoveToRechargeShieldsProc, Hook_OrderAllMoveToRechargeShieldsProc);//아비터랑 대니모쓰도 쉴드 받음
  jmpPatch(ordersEntries_Building, Hook_OrdersEntries_Building);//아비터랑 대니모쓰도 쉴드 채워줌
  jmpPatch(siegeTank_SelfDestructProc, Hook_SiegeTank_SelfDestructProc);//역장 위에선 시즈모드해도 안터짐
  jmpPatch(verifyCheatCode, Hook_verifyCheatCode);//치트 추가
  jmpPatch(StasisFieldHit, Hook_StasisFieldHit);
  jmpPatch(EMPShockwaveHit0, Hook_EMPShockwaveHit0);//역장은 EMP맞으면 사라짐
  jmpPatch(EMPShockwaveHit1, Hook_EMPShockwaveHit1);
  jmpPatch(EnsnareHit, Hook_EnsnareHit);
  jmpPatch(BroodlingHit0, Hook_BroodlingHit0);//맨땅에서도 부르드링이 소환되도록 수정함
  jmpPatch((void*)Hook_BroodlingHitBack1, Hook_BroodlingHit1);
  jmpPatch((void*)Hook_BroodlingHitBack2, Hook_BroodlingHit2);
  jmpPatch((void*)Hook_BroodlingHitBack3, Hook_BroodlingHit3);
  jmpPatch(BroodlingHit4, Hook_BroodlingHit4);
  jmpPatch(PlagueHit, Hook_PlagueHit);
  jmpPatch(CorrosiveAcidHit, Hook_CorrosiveAcidHit);
  jmpPatch(recallUnitsCB, Hook_RecallUnitsCB);
  jmpPatch(checkUnitVisibility, Hook_CheckUnitVisibility);//버로우, 은신 유닛 치트 적용 시 다 보여줌
  jmpPatch(initializeBullet, Hook_InitializeBullet);//사거리 무한 치트 쓰면 리무브 타임 최대치로 설정함
  jmpPatch(recallUnitsCB, Hook_RecallUnitsCB);
  memoryPatch_ByteNo(0x004A4F42, 0x04EB, 2);//미니맵 업데이트 속도 가속
  memoryPatch(0x004A4F44, 0x90909090);//미니맵 업데이트 속도 가속
  memoryPatch(0x004D1B87, BG_Screenshot);//BG 스크린샷 파일이름
  memoryPatch(0x004A52B4, 0xFFF733E8);//미니맵 콘솔 버튼 항시 표기
  memoryPatch_ByteNo(0x005152C0, 0x84, 2);//저그 콘솔 관련 패치
  memoryPatch_ByteNo(0x005152C4, 0x22, 2);//저그 콘솔 관련 패치
  memoryPatch_ByteNo(0x005152C6, 0x43, 2);//저그 콘솔 관련 패치
  memoryPatch_ByteNo(0x005152D0, 0x11E, 2);//저그 콘솔 관련 패치
  memoryPatch_ByteNo(0x005152D2, 0x25, 2);//저그 콘솔 관련 패치
  memoryPatch_ByteNo(0x005152DC, 7, 2);//저그 콘솔 관련 패치
  memoryPatch_ByteNo(0x0051530C, 0x84, 2);//테란 콘솔 관련 패치
  memoryPatch_ByteNo(0x00515310, 0x20, 2);//테란 콘솔 관련 패치
  memoryPatch_ByteNo(0x00515312, 0x43, 2);//테란 콘솔 관련 패치
  memoryPatch_ByteNo(0x0051531C, 0x138, 2);//테란 콘솔 관련 패치
  memoryPatch_ByteNo(0x0051531E, 0x0F, 2);//테란 콘솔 관련 패치
  memoryPatch_ByteNo(0x00515326, 0x16C, 2);//테란 콘솔 관련 패치
  memoryPatch_ByteNo(0x00515328, 9, 2);//테란 콘솔 관련 패치
  memoryPatch_ByteNo(0x00515354, 0x84, 2);//프로토스 콘솔 관련 패치
  memoryPatch_ByteNo(0x0051535A, 0x43, 2);//프로토스 콘솔 관련 패치
  memoryPatch_ByteNo(0x00515364, 0x13A, 2);//프로토스 콘솔 관련 패치
  memoryPatch_ByteNo(0x00515366, 0x1C, 2);//프로토스 콘솔 관련 패치
  memoryPatch_ByteNo(0x005153A0, 0x84, 2);//리플레이 콘솔 관련 패치
  memoryPatch_ByteNo(0x005153A6, 0x43, 2);//리플레이 콘솔 관련 패치
  memoryPatch_ByteNo(0x005153B2, 0x0F, 2);//리플레이 콘솔 관련 패치
  memoryPatch(0x004F68DB, 0x04000010);//스테이시스 필드 제외 대상은 무적 상태와 터렛 유닛
  memoryPatch(0x004F46D9, 0x04000010);//인스네어 제외 대상은 무적 상태와 터렛 유닛
  memoryPatch(0x004F4B01, 0x04000010);//플레이그 제외 대상은 무적 상태와 터렛 유닛
  memoryPatch(0x00402221, 0x662534);//아이언 스트롱홀드 점령할 때 확인할 HP주소값 지정
  memoryPatch_Byte(0x00464D97, 8);//고스트 핵 조준 끝날 시 재생할 애니메이션은 'GndAttkToIdle'
  memoryPatch_Byte(0x004D80C5, 0x49);//attack 명령어 최적화 관련 수정(그래봐야 체감은 없겠지만)
  memoryPatch_Byte(0x00497915, 5);//스킬 사용 후 다른 스크립트로 넘어가지 않게 함
  memoryPatch(0x004D8454, &__3e);//커스텀 스크립트 명령어 인젝트(__3e)
  memoryPatch(0x004D8468, &__43);//커스텀 스크립트 명령어 인젝트(__43)
  memoryPatch_Byte(0x004D7506, sizeof(iscript_OpcodeCasesOffset) / 4);
  memoryPatch(0x004D750F, &iscript_OpcodeCasesOffset);//오프셋 패치
  memoryPatch_Byte(0x00459240, sizeof(btnBinOffset)/sizeof(u32) - 1);//버튼 수
  memoryPatch_Byte(0x004588C6, sizeof(btnBinOffset)/sizeof(u32) - 1);
  memoryPatch(0x00427A9A, sizeof(btnBinOffset)/sizeof(u32));//버튼 수+1
  memoryPatch(0x00459AD3, sizeof(btnBinOffset));//버튼오프셋 메모리 크기
  memoryPatch(0x00459AD8, &btnBinOffset);
  memoryPatch(0x00425AD4, 0x9F6);//할루시네이션 지속시간 255로 수정
  memoryPatch(0x0047AA1A, 0x9F6);
  memoryPatch(0x004C7BDA, 0x9F6);
  memoryPatch(0x004F4A41, 0x9F6);
  memoryPatch(0x004F64D1, 0x9F6);
  memoryPatch(0x004F6BE3, 0x9F6);
  memoryPatch(0x00425AE4, 0x1F4);//부르드링 지속시간 50으로 수정
  memoryPatch(0x0047AA2A, 0x1F4);
  memoryPatch(0x004C7BEB, 0x1F4);
  memoryPatch(0x004CD5DF, 0x1F4);
  memoryPatch(0x004F4A53, 0x1F4);
  memoryPatch(0x004F64E2, 0x1F4);
  memoryPatch(0x004F6BF6, 0x1F4);
  memoryPatch(0x004D9A26, 6);//기본 게임 속도를 Fastest로 설정
  memoryPatch_Byte(0x004484BA, 0xFF);//AI 아칸 생성방식 기본으로 설정
  memoryPatch_Byte(0x00448509, 0xFF);//AI 다크아칸 생성방식 기본으로 설정
  //jmpPatch(soundCheck, loc_4BB740);
  jmpPatch((void*)bullet01back, bullet01);//발키리 무기 제한 뚫기//자원유닛 만들기(Cantina 포함되서 만들어졌기 때문에 켈모 컴셋 유닛 아이디는 옮겨야 됨.)

  //꿀네랄
  memoryPatch_ByteNo(0x00426209, 0xB5 , 2);
  memoryPatch_ByteNo(0x004278DB, 0xB5 , 2);
  memoryPatch_ByteNo(0x00442E92, 0xB5 , 2);
  memoryPatch_ByteNo(0x004444D0, 0xB5 , 2);
  memoryPatch_ByteNo(0x00444613, 0xB5 , 2);
  memoryPatch_ByteNo(0x004456F4, 0xB5 , 2);
  memoryPatch_ByteNo(0x0044598D, 0xB5 , 2);
  memoryPatch_ByteNo(0x00445A4D, 0xB5 , 2);
  memoryPatch_ByteNo(0x00446518, 0xB5 , 2);
  memoryPatch_ByteNo(0x00446555, 0xB5 , 2);
  memoryPatch_ByteNo(0x004557E3, 0xB5 , 2);
  memoryPatch_ByteNo(0x00467F38, 0xB5 , 2);
  memoryPatch_ByteNo(0x0046889C, 0xB5 , 2);
  jmpPatch(unit_IsResource, Hook_Unit_IsResource);
  memoryPatch_ByteNo(0x00468F81, 0xB5 , 2);
  memoryPatch_ByteNo(0x004692E8, 0xB5 , 2);
  memoryPatch_ByteNo(0x00469608, 0xB5 , 2);
  memoryPatch_ByteNo(0x004697E8, 0xB5 , 2);
  jmpPatch(sub_4746D0, Hook_Sub_4746D0);
  jmpPatch(unitCanPlaySFX, Hook_UnitCanPlaySFX);
  memoryPatch_ByteNo(0x0049F50F, 0xB5 , 2);
  memoryPatch_ByteNo(0x004A473D, 0xB5 , 2);
  memoryPatch_ByteNo(0x004C56AE, 0xB5 , 2);
  jmpPatch(giveUnits, Hook_GiveUnits);
  jmpPatch(createUnitWithProperties, Hook_CreateUnitWithProperties); 
  memoryPatch(0x004CBE3C, 0xB5);
  memoryPatch_ByteNo(0x004CD525, 0xB5 , 2);
  jmpPatch(getNextNearestResource, Hook_GetNextNearestResource); 
  jmpPatch(harvestNextNearestResourcesEx, Hook_HarvestNextNearestResourcesEx); 

  memoryPatch(0x004BB74F, SoundArray);
  memoryPatch(0x004BB7A4, &SoundArray->dbUnknown2);
  memoryPatch(0x004BB851, &SoundArray->ddUnknown0);
  memoryPatch(0x004BB85D, &SoundArray->dbUnknown4);
  memoryPatch(0x004BB865, &SoundArray->ddUnknown0);
  memoryPatch(0x004BB87D, &SoundArray->ddUnknown0);
  memoryPatch(0x004BBBAC, &SoundArray->ddUnknown0);
  memoryPatch(0x004BBBB6, &SoundArray->dwUnknown1);
  memoryPatch(0x004BBBBF, &SoundArray->dbUnknown2);
  memoryPatch(0x004BBBC8, &SoundArray->dbUnknown3);
  memoryPatch(0x004BBBD1, &SoundArray->dbUnknown4);
  memoryPatch(0x004BBBDB, &SoundArray->ddUnknown5);
  memoryPatch(0x004BBBE1, &SoundArray->ddUnknown6);
  memoryPatch(0x004BBEE5, &SoundArray->ddUnknown6);
  memoryPatch(0x004BBEF0, &SoundArray->dwUnknown1);
  memoryPatch(0x004BBEFC, &SoundArray->ddUnknown7);
  memoryPatch(0x004BBEF6, &SoundArray->dbUnknown2);
  memoryPatch(0x004BBF0B, &SoundArray->ddUnknown5);
  memoryPatch(0x004BBF11, &SoundArray->dbUnknown2);
  memoryPatch(0x004BBF62, SoundArray);
  memoryPatch(0x004BC00E, SoundArray);
  memoryPatch(0x004BC651, &SoundArray->ddUnknown6);
  memoryPatch(0x004BCB92, SoundArray);
  memoryPatch(0x004BB78C, &SoundArray[maxSound]);
  memoryPatch(0x004BC064, &SoundArray[maxSound]);
  memoryPatch(0x004BC856, &SoundArray[maxSound].ddUnknown6);
  memoryPatch(0x004BB748, maxSound);
  memoryPatch(0x004BBF67, maxSound);
  memoryPatch(0x004BCB8D, maxSound*sizeof(Sound)/4);
  memoryPatch(0x004BBB0C, minimumSound);
  jmpPatch(Sound0, Sound00);
  jmpPatch(Sound1, Sound01);
  jmpPatch(Sound2, Sound02);
  jmpPatch(Sound3, Sound03);
  
  memoryPatch(0x0048A691, CBulletArray);
  memoryPatch(0x0048A6BD, ((u32)&CBulletArray->previous) + sizeof(CBullet));
  memoryPatch(0x0048A6C7, CBulletArray);
  memoryPatch(0x0048A6E1, &CBulletArray->next);
  memoryPatch(0x0048A6EA, &CBulletArray->next);
  memoryPatch(0x0048A6F5, &CBulletArray->next);
  memoryPatch(0x0048A726, ((u32)&CBulletArray->sprite) + sizeof(CBullet));
  memoryPatch(0x0048A759, ((u32)&CBulletArray[maxCBullet].sprite) + sizeof(CBullet));
  memoryPatch(0x0048A78B, &CBulletArray->previous);
  memoryPatch(0x0048A7A5, CBulletArray);
  memoryPatch(0x0048A7E5, ((u32)&CBulletArray->sprite) + sizeof(CBullet));
  memoryPatch(0x0048A7DC, &CBulletArray[maxCBullet]);
  memoryPatch(0x0048A819, ((u32)&CBulletArray[maxCBullet].sprite) + sizeof(CBullet));
  memoryPatch(0x0048A851, &CBulletArray->previous);
  memoryPatch(0x0048A86F, &CBulletArray->next);
  memoryPatch(0x0048A87F, CBulletArray);
  memoryPatch(0x0048A8B5, &CBulletArray[maxCBullet]);
  memoryPatch(0x0048A973, ((u32)CBulletArray) - sizeof(CBullet));
  memoryPatch(0x0048A98C, ((u32)CBulletArray) - sizeof(CBullet));
  memoryPatch(0x0048AA39, CBulletArray);
  memoryPatch(0x0048AA62, CBulletArray);
  memoryPatch(0x0048AB94, CBulletArray);
  memoryPatch(0x0048AC64, CBulletArray);
  memoryPatch(0x0048AE4C, CBulletArray);
  memoryPatch(0x0048AE80, CBulletArray);
  memoryPatch(0x0048AEB9, CBulletArray);
  memoryPatch(0x0048AED7, &CBulletArray->sprite);
  memoryPatch(0x0048AF48, CBulletArray);
  memoryPatch(0x0048AFA4, CBulletArray);
  memoryPatch(0x0048A69B, maxCBullet*sizeof(CBullet) / 4);
  memoryPatch(0x0048A6C2, maxCBullet - 1);
  memoryPatch(0x0048AE47, maxCBullet*sizeof(CBullet) / 4);
  memoryPatch(0x0048AE85, maxCBullet);
  memoryPatch(0x0048AEBE, maxCBullet);
  memoryPatch(0x0048AEDC, maxCBullet);
  memoryPatch(0x0048AFA9, maxCBullet);
  jmpPatch(bullet1, bullet01);
  jmpPatch(bullet2, bullet02);
  jmpPatch(bullet3, bullet03);
  jmpPatch(bullet4, bullet04);
  jmpPatch(bullet5, bullet05);

  /*
  memoryPatch(0x00479E6D, ((u32)Sprites)-sizeof(CSprite));
  memoryPatch(0x00479F4F, Sprites);
  memoryPatch(0x00487559, Sprites);
  memoryPatch(0x004875A9, Sprites);
  memoryPatch(0x0048760D, ((u32)Sprites)-sizeof(CSprite));
  memoryPatch(0x00487655, ((u32)Sprites)-sizeof(CSprite));
  memoryPatch(0x00494D31, Sprites);
  memoryPatch(0x00495392, ((u32)Sprites)-sizeof(CSprite));
  memoryPatch(0x00497236, &Sprites->images.head);
  memoryPatch(0x00497240, (u32)(&Sprites->images.head)+sizeof(CSprite));
  memoryPatch(0x0049724A, (u32)(&Sprites->images.head)+sizeof(CSprite)*2);
  memoryPatch(0x00497254, (u32)(&Sprites->images.head)+sizeof(CSprite)*3);
  memoryPatch(0x0049725E, (u32)(&Sprites->images.head)+sizeof(CSprite)*4);
  memoryPatch(0x004972AB, &Sprites->link.prev);
  memoryPatch(0x004972D1, &Sprites->link.prev);
  memoryPatch(0x00497361, ((u32)Sprites)-sizeof(CSprite));
  memoryPatch(0x00497378, ((u32)Sprites)-sizeof(CSprite));
  memoryPatch(0x004973CD, Sprites);
  memoryPatch(0x004973ED, Sprites);
  memoryPatch(0x004985A3, Sprites);
  memoryPatch(0x00498631, &Sprites->link.prev);
  memoryPatch(0x0049865B, Sprites);
  memoryPatch(0x004986B2, ((u32)Sprites)-sizeof(CSprite));
  memoryPatch(0x00498702, ((u32)Sprites)-sizeof(CSprite));
  memoryPatch(0x00498718, &Sprites->index);
  memoryPatch(0x00498749, Sprites);
  memoryPatch(0x00498763, &(Sprites->images.head)+9);
  memoryPatch(0x00498793, &Sprites[maxCSprite].images.head);
  memoryPatch(0x004987FC, Sprites);
  memoryPatch(0x00498868, Sprites);
  memoryPatch(0x004988A2, Sprites);
  memoryPatch(0x004988F6, ((u32)Sprites)-sizeof(CSprite));
  memoryPatch(0x00498923, Sprites);
  memoryPatch(0x00498975, ((u32)Sprites)-sizeof(CSprite));
  memoryPatch(0x004989A8, &Sprites->flags);
  memoryPatch(0x00499915, Sprites);
  memoryPatch(0x0049993C, &Sprites->flags);
  memoryPatch(0x00499952, &(Sprites->link.prev)+9);
  memoryPatch(0x004D56B9, ((u32)Sprites)-sizeof(CSprite));
  memoryPatch(0x004D63DA, Sprites);
  memoryPatch(0x004E3199, ((u32)Sprites)-sizeof(CSprite));
  memoryPatch(0x004E394C, Sprites);
  memoryPatch(0x0042D47B, spriteListPointers);
  memoryPatch(0x0042D4B7, spriteListPointers);
  memoryPatch(0x00498D67, spriteListPointers);
  memoryPatch(0x00497271, maxCSprite*36);
  memoryPatch(0x0049728A, maxCSprite);
  memoryPatch(0x004972B1, maxCSprite);
  memoryPatch(0x004972CA, maxCSprite);
  memoryPatch(0x0049859E, maxCSprite*9);
  memoryPatch(0x00498660, maxCSprite);
  memoryPatch(0x00498721, maxCSprite*36);
  memoryPatch(0x0049874E, maxCSprite);
  memoryPatch(0x0049886D, maxCSprite);
  memoryPatch(0x004989AF, maxCSprite);
  memoryPatch(0x0049991A, maxCSprite*9);
  memoryPatch(0x00499957, (maxCSprite-1)/3);
  jmpPatch(sprite0, sprite00);
  */

  /*
  memoryPatch(0x0049797B, spriteHeads);
  memoryPatch(0x00497987, spriteHeads);
  memoryPatch(0x00497A7B, spriteHeads);
  memoryPatch(0x00497A87, spriteHeads);
  memoryPatch(0x00497AC4, spriteHeads);
  memoryPatch(0x00497AFC, spriteHeads);
  memoryPatch(0x00497B86, spriteHeads);
  memoryPatch(0x00497B92, spriteHeads);
  memoryPatch(0x00497C52, spriteHeads);
  memoryPatch(0x00498308, spriteHeads);
  memoryPatch(0x00498343, spriteHeads);
  memoryPatch(0x0049867D, spriteHeads);
  memoryPatch(0x004986BB, spriteHeads);
  memoryPatch(0x004988B6, spriteHeads);
  memoryPatch(0x004988C5, spriteHeads);
  memoryPatch(0x004988FF, spriteHeads);
  memoryPatch(0x00498CC3, spriteHeads);
  memoryPatch(0x00498D03, spriteHeads);
  memoryPatch(0x004991C3, spriteHeads);
  memoryPatch(0x00499201, spriteHeads);
  memoryPatch(0x00499928, spriteHeads);
  memoryPatch(0x0049798E, spriteTails);
  memoryPatch(0x00497999, spriteTails);
  memoryPatch(0x00497A8E, spriteTails);
  memoryPatch(0x00497A99, spriteTails);
  memoryPatch(0x00497ACF, spriteTails);
  memoryPatch(0x00497AD8, spriteTails);
  memoryPatch(0x00497AF5, spriteTails);
  memoryPatch(0x00497B99, spriteTails);
  memoryPatch(0x00497BA4, spriteTails);
  memoryPatch(0x004986A3, spriteTails+maxSprtieHeadTail-1);
  memoryPatch(0x004986CA, spriteTails);
  memoryPatch(0x0049870B, spriteTails);
  memoryPatch(0x00498893, spriteTails+maxSprtieHeadTail-1);
  memoryPatch(0x004988E7, spriteTails+maxSprtieHeadTail-1);
  memoryPatch(0x00498937, spriteTails);
  memoryPatch(0x00498946, spriteTails);
  memoryPatch(0x0049897E, spriteTails);
  memoryPatch(0x004991CE, spriteTails);
  memoryPatch(0x004991D7, spriteTails);
  memoryPatch(0x004991FA, spriteTails);
  memoryPatch(0x00499934, spriteTails);
  memoryPatch(0x00499923, maxSprtieHeadTail);
  memoryPatch(0x0049992F, maxSprtieHeadTail);
  memoryPatch(0x00498678, maxSprtieHeadTail);
  memoryPatch(0x004986C5, maxSprtieHeadTail);
  memoryPatch(0x004988C0, maxSprtieHeadTail);
  memoryPatch(0x00498941, maxSprtieHeadTail);
  memoryPatch(0x00497C7F, maxSprtieHeadTail);*/

  /*
  //memoryPatch(0x00403CDE, regionAccessibilityFlags+19988);
  memoryPatch(0x00437096, regionAccessibilityFlags);
  memoryPatch(0x004370D3, regionAccessibilityFlags);
  memoryPatch(0x004370F3, regionAccessibilityFlags);
  memoryPatch(0x00437116, regionAccessibilityFlags);
  memoryPatch(0x00437148, regionAccessibilityFlags);
  memoryPatch(0x00437A81, regionAccessibilityFlags);
  memoryPatch(0x00437F04, regionAccessibilityFlags);
  //memoryPatch(0x004E2CF9B, regionAccessibilityFlags+19988);
  memoryPatch(0x004037D5, maxCSprite);
  memoryPatch(0x00403865, maxCSprite);
  memoryPatch(0x00403902, maxCSprite);
  memoryPatch(0x004039A3, maxCSprite);
  memoryPatch(0x00403A44, maxCSprite);
  memoryPatch(0x00432265, maxCSprite);
  memoryPatch(0x004322AA, maxCSprite/4);
  memoryPatch(0x00432281, -maxCSprite);
  memoryPatch(0x004322A3, -maxCSprite);
  memoryPatch(0x004322E9, -maxCSprite);
  memoryPatch(0x004322F3, maxCSprite/4);
  memoryPatch(0x004329C9, maxCSprite);
  memoryPatch(0x00436CF5, maxCSprite+4);
  memoryPatch(0x00436CFC, maxCSprite/4);
  memoryPatch(0x00436D04, -(maxCSprite+4));
  memoryPatch(0x00436D12, -(maxCSprite+4));
  memoryPatch(0x00436D68, maxCSprite);
  memoryPatch(0x00436D85, maxCSprite+4);
  memoryPatch(0x00436D8C, maxCSprite/4);
  memoryPatch(0x00436D94, -(maxCSprite+4));
  memoryPatch(0x00436DA3, -(maxCSprite+4));
  memoryPatch(0x00436DFC, maxCSprite);
  memoryPatch(0x0043708F, maxCSprite);
  memoryPatch(0x0043709C, maxCSprite/4);
  memoryPatch(0x004372A6, maxCSprite);
  memoryPatch(0x00437790, maxCSprite/4);
  memoryPatch(0x00437A7C, maxCSprite*2);
  memoryPatch(0x00437B4C, maxCSprite*2);
  memoryPatch(0x00437EFD, maxCSprite);
  memoryPatch(0x004382B1, maxCSprite);
  memoryPatch(0x004399D3, maxCSprite*10);
  memoryPatch(0x00439A0C, -(maxCSprite*6));
  memoryPatch(0x00439A13, maxCSprite);
  memoryPatch(0x00439A24, -(maxCSprite*6));
  memoryPatch(0x00439A34, -(maxCSprite*4));
  memoryPatch(0x00439A3B, -(maxCSprite*10));
  memoryPatch(0x00439A42, maxCSprite);
  memoryPatch(0x00439C05, maxCSprite+4);
  memoryPatch(0x00439C1C, maxCSprite/4);
  memoryPatch(0x00439C22, -(maxCSprite+4));
  memoryPatch(0x00439CBB, -(maxCSprite+4));
  memoryPatch(0x00439CE7, -(maxCSprite+4));
  memoryPatch(0x0043ABB5, maxCSprite+4);
  memoryPatch(0x0043AC0B, maxCSprite/4);
  memoryPatch(0x0043AC11, -(maxCSprite+4));
  memoryPatch(0x0043AC18, -(maxCSprite+4));
  memoryPatch(0x0043AC42, -(maxCSprite+4));
  memoryPatch(0x0043ACAF, -(maxCSprite+4));
  memoryPatch(0x0043ACD2, -(maxCSprite+4));
  memoryPatch(0x0043ACFF, -(maxCSprite+4));
  memoryPatch(0x0043AD20, -(maxCSprite+4));
  memoryPatch(0x0043BA4F, maxCSprite);
  memoryPatch(0x0043BA66, maxCSprite);
  memoryPatch(0x0043BA70, maxCSprite);
  memoryPatch(0x0043DF73, maxCSprite);
  memoryPatch(0x0043DFC6, maxCSprite);
  memoryPatch(0x0043DFD4, maxCSprite);
  memoryPatch(0x0043DFDE, maxCSprite);
  memoryPatch(0x0043E0F0, maxCSprite);
  memoryPatch(0x0043E1E3, maxCSprite);
  memoryPatch(0x0043FDD9, maxCSprite);
  memoryPatch(0x00445AC5, maxCSprite);
  memoryPatch(0x00445BD9, maxCSprite);
  memoryPatch(0x00445D46, maxCSprite);
  memoryPatch(0x00446745, maxCSprite);
  memoryPatch(0x00446847, maxCSprite);
  memoryPatch(0x0044696B, maxCSprite);
  memoryPatch(0x0048300F, maxCSprite);
  jmpPatch(regionAccessibilityFlags0, regionAccessibilityFlags00);
  jmpPatch(regionAccessibilityFlags1, regionAccessibilityFlags01);
  jmpPatch(regionAccessibilityFlags2, regionAccessibilityFlags02);
  jmpPatch(regionAccessibilityFlags3, regionAccessibilityFlags03);
  //jmpPatch(regionAccessibilityFlags4, regionAccessibilityFlags04);
  jmpPatch(regionAccessibilityFlags5, regionAccessibilityFlags05);
  
  
  
  memoryPatch(0x00487542, &SpriteThingyArray->next);
  memoryPatch(0x00487573, SpriteThingyArray);
  memoryPatch(0x00487585, SpriteThingyArray);
  memoryPatch(0x004875F1, &SpriteThingyArray->next);
  memoryPatch(0x00487996, SpriteThingyArray);
  memoryPatch(0x004879B4, SpriteThingyArray);
  memoryPatch(0x00488106, SpriteThingyArray);
  memoryPatch(0x00488560, SpriteThingyArray);
  memoryPatch(0x00488572, SpriteThingyArray);
  memoryPatch(0x0048857C, SpriteThingyArray);
  memoryPatch(0x004885AB, &(SpriteThingyArray->prev)+4);
  memoryPatch(0x004C2B7D, SpriteThingyArray);
  memoryPatch(0x004E3927, SpriteThingyArray);
  memoryPatch(0x00487596, &THG2_Array->next);
  memoryPatch(0x004875C3, THG2_Array);
  memoryPatch(0x004875D5, THG2_Array);
  memoryPatch(0x0048763F, &THG2_Array->next);
  memoryPatch(0x004879D2, THG2_Array);
  memoryPatch(0x004879F0, THG2_Array);
  memoryPatch(0x0048811A, THG2_Array);
  memoryPatch(0x00488556, THG2_Array);
  memoryPatch(0x004885E9, &(THG2_Array->prev)+4);
  memoryPatch(0x004C2B8D, THG2_Array);
  memoryPatch(0x00487547, maxCThingy);
  memoryPatch(0x0048759B, maxCThingy);
  memoryPatch(0x004875F6, maxCThingy);
  memoryPatch(0x00487644, maxCThingy);
  memoryPatch(0x004879A5, maxCThingy);
  memoryPatch(0x004879C3, maxCThingy);
  memoryPatch(0x004879E1, maxCThingy);
  memoryPatch(0x004879FF, maxCThingy);
  memoryPatch(0x00488101, maxCThingy);
  memoryPatch(0x00488115, maxCThingy);
  memoryPatch(0x0048855B, maxCThingy*4);
  memoryPatch(0x00488567, maxCThingy*4);
  memoryPatch(0x004885B0, maxCThingy-1);
  memoryPatch(0x004885EE, maxCThingy-1);
  memoryPatch(0x004C2B78, maxCThingy);
  memoryPatch(0x004C2B88, maxCThingy);*/

  /*
  memoryPatch_Byte(0x0049738E, 7);
  memoryPatch_Byte(0x004973A5, 7);
  memoryPatch_Byte(0x004973BC, 7);
  memoryPatch_Byte(0x004D4C11, 7);
  memoryPatch_Byte(0x004D4C30, 7);
  memoryPatch_Byte(0x004D62DB, 7);
  memoryPatch_Byte(0x004D63F9, 7);
  memoryPatch_Byte(0x004D640F, 7);
  */

  /*
  memoryPatch_Byte(0x0042D8D8, 0x40);
  memoryPatch_Byte(0x0042D8E1, 0x48);
  memoryPatch_Byte(0x0042DC13, 0x40);
  memoryPatch_Byte(0x0042DC1C, 0x48);
  */
  memoryPatch_Byte(0x0049788A, 0x4C);
  memoryPatch_Byte(0x00497941, 0x40);
  memoryPatch_Byte(0x004D5A9C, 0x4C);
  memoryPatch_Byte(0x004D5AC6, 0x40);
  //memoryPatch_Byte(0x004D5ACB, 8);
  memoryPatch_Byte(0x004D69E0, SELECT_UNIT_COUNT);
  memoryPatch_Byte(0x004D74CE, 0x0D);
  memoryPatch_Byte(0x004D74D7, 0x0D);
  memoryPatch_Byte(0x004D7B99, 4);
  memoryPatch_Byte(0x004D7FE2, 4); 
  memoryPatch_Byte(0x004D8063, 4); 
  memoryPatch_Byte(0x004D8289, 4); 
  memoryPatch_Byte(0x004D685E, 0x40);
  //memoryPatch_Byte(0x004D6863, 8);
  
  jmpPatch(updateUnitSpeed_iscriptControlled, Hook_UpdateUnitSpeed_iscriptControlled);
  jmpPatch(loc_42DB50, Hook_42DB50);
  jmpPatch(loc_42D600, Hook_42D600);
  jmpPatch(iscript_OpcodeCases, Hook_Iscript_OpcodeCases);
  jmpPatch(OpcodeCases00, Hook_OpcodeCases00);
  jmpPatch(OpcodeCases01, Hook_OpcodeCases01);
  jmpPatch(OpcodeCases02, Hook_OpcodeCases02);
  jmpPatch(OpcodeCases03, Hook_OpcodeCases03);
  jmpPatch(OpcodeCases04, Hook_OpcodeCases04);
  jmpPatch(OpcodeCases05, Hook_OpcodeCases05);
  jmpPatch(OpcodeCases06, Hook_OpcodeCases06);
  jmpPatch(OpcodeCases07, Hook_OpcodeCases07);
  jmpPatch(OpcodeCases08, Hook_OpcodeCases08);
  jmpPatch(OpcodeCases09, Hook_OpcodeCases09);
  jmpPatch(OpcodeCases10, Hook_OpcodeCases10);
  jmpPatch(OpcodeCases11, Hook_OpcodeCases11);
  jmpPatch(playImageIscript, Hook_PlayImageIscript);
  jmpPatch(isValidScript, Hook_IsValidScript);
  jmpPatch(playWarpInOverlay, Hook_PlayWarpInOverlay);
  jmpPatch(iscriptSomething_Death, Hook_IscriptSomething_Death);
  jmpPatch(groundAttackInit, Hook_GroundAttackInit);
  jmpPatch(orders_BuildSelf2_0, Hook_Orders_BuildSelf2_0);
  jmpPatch(orders_BuildSelf2_1, Hook_Orders_BuildSelf2_1);
  jmpPatch(orders_BuildSelf2_2, Hook_Orders_BuildSelf2_2);

  memoryPatch(0x0042D8DB, &someIscriptStruct->iscriptHeaderOffsetEx);
  memoryPatch(0x0042D8E6, &someIscriptStruct->unknown2Ex);
  memoryPatch(0x0042D91D, someIscriptStruct);
  memoryPatch(0x0042DC16, &someIscriptStruct->iscriptHeaderOffsetEx);
  memoryPatch(0x0042DC1E, &someIscriptStruct->unknown2Ex);
  memoryPatch(0x0042DC24, &someIscriptStruct->unknown2Ex);
  memoryPatch(0x0042DC4C, someIscriptStruct);
  memoryPatch(0x0042DD1C, someIscriptStruct);

  memoryPatch_Byte(0x004D4BA9, (-(s8)sizeof(CImage)));
  memoryPatch_Byte(0x004D4BB5, (u8)sizeof(CImage));
  memoryPatch(0x004D4BBC, (u32)sizeof(CImage)*2);
  memoryPatch(0x004D4BC6, (u32)sizeof(CImage)*3);
  memoryPatch(0x004D4BD0, (u32)sizeof(CImage)*5);
  memoryPatch_Byte(0x004D4C65, (u8)sizeof(CImage));
  memoryPatch_Byte(0x004D626A, (u8)sizeof(CImage)+2);
  memoryPatch(0x004D62E5, ((u32)sizeof(CImage))/4);
  memoryPatch_Byte(0x004D62EB, (u8)sizeof(CImage));
  memoryPatch_Byte(0x004D6317, (u8)sizeof(CImage));
  memoryPatch_Byte(0x004D64D9, (u8)sizeof(CImage));
  memoryPatch(0x004D651C, (u32)sizeof(CImage)*5);
  memoryPatch_Byte(0x004D655E, (u8)sizeof(CImage)+2);
  memoryPatch(0x004D65A7, ((u32)sizeof(CImage))/4);
  memoryPatch_Byte(0x004D65B5, (u8)sizeof(CImage));
  memoryPatch_Byte(0x004D65BD, (u8)sizeof(CImage));
  memoryPatch_Byte(0x004D65FC, (u8)sizeof(CImage));
  memoryPatch(0x004D6952, (u32)SELECT_UNIT_COUNT*sizeof(CImage)/4);
  memoryPatch(0x004D6959, (u32)80*sizeof(CImage)/4);
  memoryPatch(0x004D6965, (u32)64*sizeof(CImage)/4);
  memoryPatch(0x004D6971, (u32)64*sizeof(CImage)/4);
  memoryPatch_Byte(0x004D69DD, (u8)sizeof(CImage));
  memoryPatch_Byte(0x004D6A2C, (u8)sizeof(CImage));
  memoryPatch_Byte(0x004D6A7B, (u8)sizeof(CImage));
  memoryPatch_Byte(0x004D6BB5, (u8)sizeof(CImage)-8);
  memoryPatch_Byte(0x004D6BBE, (u8)sizeof(CImage)-8);
  memoryPatch_Byte(0x004D6BCD, (u8)sizeof(CImage)-4);
  memoryPatch_Byte(0x004D6BE3, (u8)sizeof(CImage));
  memoryPatch_Byte(0x004D6BE6, ((u8)sizeof(CImage))+36);
  memoryPatch_Byte(0x004D6BEF, ((u8)sizeof(CImage))+4);
  memoryPatch_Byte(0x004D6C00, ((u8)sizeof(CImage))+48);
  memoryPatch_Byte(0x004D6C0A, ((u8)sizeof(CImage))+4);
  memoryPatch_Byte(0x004D6C0E, ((u8)sizeof(CImage))+16);
  memoryPatch_Byte(0x004D6C11, ((u8)sizeof(CImage))+3);
  memoryPatch_Byte(0x004D6C15, ((u8)sizeof(CImage))+18);
  memoryPatch_Byte(0x004D6C18, ((u8)sizeof(CImage))+2);
  memoryPatch_Byte(0x004D6C28, ((u8)sizeof(CImage))+4);
  /*memoryPatch_Byte(0x004D6C35, ((u8)sizeof(CImage))+44);
  memoryPatch_Byte(0x004D6C3A, (u8)(sizeof(CImage)*2-8));
  memoryPatch_Byte(0x004D6C43, (u8)(sizeof(CImage)*2-8));
  memoryPatch_Byte(0x004D6C52, (u8)(sizeof(CImage)*2-4));
  */
  memoryPatch_Byte(0x004D6D33, (u8)sizeof(CImage));

  memoryPatch(0x004D6C6A, (u32)(sizeof(CImage)*2+36));
  memoryPatch(0x004D6C71, (u32)sizeof(CImage)*2);
  memoryPatch(0x004D6C7D, (u32)sizeof(CImage)*2+4);
  memoryPatch(0x004D6C91, (u32)sizeof(CImage)*2+48);
  memoryPatch(0x004D6C9E, (u32)sizeof(CImage)*2+4);
  memoryPatch(0x004D6CA5, (u32)sizeof(CImage)*2+16);
  memoryPatch(0x004D6CAB, (u32)sizeof(CImage)*2+3);
  memoryPatch(0x004D6CB2, (u32)sizeof(CImage)*2+18);
  memoryPatch(0x004D6CB8, (u32)sizeof(CImage)*2+2);
  memoryPatch(0x004D6CCB, (u32)sizeof(CImage)*2+4);
  memoryPatch(0x004D6CD4, (u32)sizeof(CImage)*2+44);
  memoryPatch(0x004D6CD9, (u32)sizeof(CImage)*3);
  
  memoryPatch(0x004D5B69, &buildingPlacementImages->parentSprite);
  memoryPatch(0x004D5B71, buildingPlacementImages);
  memoryPatch(0x004D5B7C, &buildingPlacementImages->parentSprite);
  memoryPatch(0x004D5B82, &circleMarkers3->parentSprite);
  memoryPatch(0x004D5B8A, &circleMarkers3->link.prev);
  memoryPatch(0x004D5B95, &circleMarkers3->parentSprite);
  memoryPatch_Byte(0x004D5B9B, (u8)sizeof(CImage));
  memoryPatch(0x004D5B9E, 64*sizeof(CImage));

  memoryPatch(0x004D693F, playerCursorCircles);
  memoryPatch(0x004D695E, someImageArray80);
  memoryPatch(0x004D696A, buildingPlacementImages);
  memoryPatch(0x004D6976, circleMarkers3);
  memoryPatch(0x004D697D, playerCursorCircles);
  memoryPatch(0x004D6998, somePlayerImages);
  memoryPatch(0x004D69A6, playerCursorCircles);
  memoryPatch(0x004D69B4, playerCursorCircles);
  memoryPatch(0x004D69B9, &playerCursorCircles->link.next);
  memoryPatch(0x004D69C1, &playerCursorCircles->link.next);
  memoryPatch(0x004D69D1, &playerCursorCircles->link.next);
  memoryPatch(0x004D69E4, someImageArray80);
  memoryPatch(0x004D69F3, ((u32)&someImageArray80->link.prev)+sizeof(CImage));
  memoryPatch(0x004D69FD, someImageArray80);
  memoryPatch(0x004D6A12, &someImageArray80->link.next);
  memoryPatch(0x004D6A1B, &someImageArray80->link.next);
  memoryPatch(0x004D6A26, &someImageArray80->link.next);
  memoryPatch(0x004D6A31, buildingPlacementImages);
  memoryPatch(0x004D6A40, ((u32)buildingPlacementImages)+sizeof(CImage));
  memoryPatch(0x004D6A4A, buildingPlacementImages);
  memoryPatch(0x004D6A61, &buildingPlacementImages->link.next);
  memoryPatch(0x004D6A6A, &buildingPlacementImages->link.next);
  memoryPatch(0x004D6A75, &buildingPlacementImages->link.next);
  memoryPatch(0x004D6A81, &circleMarkers3->flags);
  memoryPatch(0x004D6A88, &circleMarkers3->flags);
  memoryPatch(0x004D6A94, circleMarkers3);
  memoryPatch(0x004D6AAF, &circleMarkers3->id);
  memoryPatch(0x004D6AB5, &circleMarkers3->grpOffset);
  memoryPatch(0x004D6AC6, &circleMarkers3->frameSet);
  memoryPatch(0x004D6ACC, &circleMarkers3->direction);
  memoryPatch(0x004D6AD3, &circleMarkers3->frameIndex);
  memoryPatch(0x004D6AD9, &circleMarkers3->paletteType);
  memoryPatch(0x004D6AE0, &circleMarkers3->updateFunction);
  memoryPatch(0x004D6AEE, &circleMarkers3->renderFunction);
  memoryPatch(0x004D6AFC, &circleMarkers3->renderFunction);
  memoryPatch(0x004D6B03, &circleMarkers3->flags);
  memoryPatch(0x004D6B0D, ((u32)&circleMarkers3->id)+sizeof(CImage));
  memoryPatch(0x004D6B26, circleMarkers3);
  memoryPatch(0x004D6B3A, circleMarkers3);
  memoryPatch(0x004D6B40, &circleMarkers3->link.next);
  memoryPatch(0x004D6B49, &circleMarkers3->link.next);
  memoryPatch(0x004D6B55, &circleMarkers3->link.next);
  memoryPatch(0x004D6BAA, circleMarkers3);
  memoryPatch(0x004D6BC1, circleMarkers3);
  memoryPatch(0x004D6BC7, &circleMarkers3->link.next);
  memoryPatch(0x004D6BD0, &circleMarkers3->link.next);
  memoryPatch(0x004D6BDC, &circleMarkers3->link.next);
  memoryPatch(0x004D6C2F, circleMarkers3);
  //memoryPatch(0x004D6C46, circleMarkers3);
  //memoryPatch(0x004D6C4C, &circleMarkers3->link.next);
  memoryPatch(0x004D6C55, &circleMarkers3->link.next);
  memoryPatch(0x004D6C61, &circleMarkers3->link.next);

  memoryPatch(0x00497388, Images);
  memoryPatch(0x0049739F, Images);
  memoryPatch(0x004973B6, Images);
  memoryPatch(0x004D4BA3, ((u32)&Images->grpOffset) + sizeof(CImage));
  memoryPatch(0x004D4BD9, ((u32)&Images[maxCImage].grpOffset) + sizeof(CImage));
  memoryPatch(0x004D4C67, (u32)&Images[maxCImage]);
  memoryPatch(0x004D4C14, Images);
  //memoryPatch(0x004D4C32, Images);
  memoryPatch(0x004D6252, Images);
  memoryPatch(0x004D62E0, Images);
  memoryPatch(0x004D6307, Images);
  memoryPatch(0x004D63F3, Images);
  memoryPatch(0x004D6409, Images);
  memoryPatch(0x004D64C9, Images);
  memoryPatch(0x004D64E6, &Images->grpOffset);
  memoryPatch(0x004D64F1, ((u32)&Images->grpOffset)+sizeof(CImage));
  memoryPatch(0x004D64FC, ((u32)&Images->grpOffset)+sizeof(CImage)*2);
  memoryPatch(0x004D6507, ((u32)&Images->grpOffset)+sizeof(CImage)*3);
  memoryPatch(0x004D6512, ((u32)&Images->grpOffset)+sizeof(CImage)*4);
  memoryPatch(0x004D658A, Images);
  memoryPatch(0x004D65EC, Images);
  memoryPatch(0x004D6949, Images);
  memoryPatch(0x004D6CEB, Images);
  memoryPatch(0x004D6CFA, ((u32)&Images->link.prev)+sizeof(CImage));
  memoryPatch(0x004D6D04, Images);
  memoryPatch(0x004D6D19, &Images->link.next);
  memoryPatch(0x004D6D22, &Images->link.next);
  memoryPatch(0x004D6D2D, &Images->link.next);
  memoryPatch(0x0042D377, imageListPointers+3);
  memoryPatch(0x0042D37E, imageListPointers+2);
  memoryPatch(0x0042D3B1, imageListPointers+2);
  memoryPatch(0x0042D488, imageListPointers+3);
  memoryPatch(0x0042D48E, imageListPointers+2);
  memoryPatch(0x0042D49E, imageListPointers+3);
  memoryPatch(0x0042D3AA, imageListPointers);
  memoryPatch(0x0042D3BB, imageListPointers);
  memoryPatch(0x0042D3C8, imageListPointers);
  memoryPatch(0x0042D3CF, imageListPointers+1);
  memoryPatch(0x0042D3D6, imageListPointers+1);
  memoryPatch(0x0042D3E6, imageListPointers);
  memoryPatch(0x0042D3ED, imageListPointers+1);
  memoryPatch(0x0042D418, imageListPointers+1);
  memoryPatch(0x0042D423, imageListPointers);
  memoryPatch(0x0042D42E, imageListPointers);
  memoryPatch(0x0042D435, imageListPointers+1);
  memoryPatch(0x0042D43C, imageListPointers+1);
  memoryPatch(0x0042D44D, imageListPointers+1);
  memoryPatch(0x0042D454, imageListPointers);
  memoryPatch(0x0042D483, imageListPointers);
  memoryPatch(0x0042D495, imageListPointers+1);
  memoryPatch(0x0042D4CA, imageListPointers+1);
  memoryPatch(0x0042D506, imageListPointers);
  memoryPatch(0x0042D409, imageListPointers);
  memoryPatch(0x004865BF, maxCImage/2);
  memoryPatch(0x004D4BF2, maxCImage);
  memoryPatch(0x004D4C1A, maxCImage);
  memoryPatch(0x004D624D, maxCImage*sizeof(CImage)/4);
  memoryPatch(0x004D630C, maxCImage);
  memoryPatch(0x004D64CE, maxCImage);
  memoryPatch(0x004D6521, maxCImage*sizeof(CImage));
  memoryPatch(0x004D65F1, maxCImage);
  memoryPatch(0x004D6944, maxCImage*sizeof(CImage)/4);
  memoryPatch(0x004D6CFF, maxCImage-1);
  jmpPatch(image0, image00);
  jmpPatch(image1, image01);
  jmpPatch(image2, image02);
  jmpPatch(image3, image03);
  jmpPatch(image4, image04);
  jmpPatch(image5, image05);
  jmpPatch(image6, image06);
  jmpPatch(image7, image07);
  jmpPatch(image8, image08);
  jmpPatch(image9, image09);
  jmpPatch(image_10, image10);
  jmpPatch(image_11, image11);
  jmpPatch(image_12, image12);
  jmpPatch(image_13, image13);
  jmpPatch(image_14, image14);
  jmpPatch(image_15, image15);
  jmpPatch(image_16, image16);
  
  memoryPatch(0x004D7184, IMAGE_TYPE_COUNT*24+8);
  memoryPatch(0x00401DFC, IMAGE_TYPE_COUNT);
  memoryPatch(0x00499678, IMAGE_TYPE_COUNT);
  memoryPatch(0x004D50DD, IMAGE_TYPE_COUNT);
  memoryPatch(0x004D5101, IMAGE_TYPE_COUNT);
  memoryPatch(0x004D510C, IMAGE_TYPE_COUNT*4);
  memoryPatch(0x004D5119, IMAGE_TYPE_COUNT);
  memoryPatch(0x004D5775, IMAGE_TYPE_COUNT);
  memoryPatch(0x004D5C24, IMAGE_TYPE_COUNT);
  memoryPatch(0x0047AE4A, IMAGE_TYPE_COUNT-0x0F4);
  memoryPatch(0x0047AF4A, IMAGE_TYPE_COUNT-0x0F4);
  memoryPatch(0x004D6357, IMAGE_TYPE_COUNT-0x0F4);
  memoryPatch(0x004D7250, -IMAGE_TYPE_COUNT*4-8);
  memoryPatch(0x004D7257, -IMAGE_TYPE_COUNT*24-8);
  memoryPatch(0x004D7278, -IMAGE_TYPE_COUNT*4-8);
  memoryPatch(0x004D727F, -IMAGE_TYPE_COUNT*24-8);
  memoryPatch(0x004D72A5, -IMAGE_TYPE_COUNT*4-8);
  memoryPatch(0x004D72AC, -IMAGE_TYPE_COUNT*24-8);
  memoryPatch(0x004D72C8, -IMAGE_TYPE_COUNT*4-8);
  memoryPatch(0x004D72CF, -IMAGE_TYPE_COUNT*24-8);
  memoryPatch(0x004D72EB, -IMAGE_TYPE_COUNT*4-8);
  memoryPatch(0x004D72F2, -IMAGE_TYPE_COUNT*24-8);
  memoryPatch(0x004D730E, -IMAGE_TYPE_COUNT*4-8);
  memoryPatch(0x004D7315, -IMAGE_TYPE_COUNT*24-8);
  memoryPatch(0x004D7331, -IMAGE_TYPE_COUNT*4-8);
  memoryPatch(0x004D7338, -IMAGE_TYPE_COUNT*24-8);
  memoryPatch(0x004D72D5, IMAGE_TYPE_COUNT);
  memoryPatch(0x004D72F8, IMAGE_TYPE_COUNT);
  memoryPatch(0x004D731B, IMAGE_TYPE_COUNT);
  memoryPatch_ByteNo(0x004EA60F, IMAGE_TYPE_COUNT, 2);
  memoryPatch(0x004D725D, IMAGE_TYPE_COUNT);
  memoryPatch(0x004D7285, IMAGE_TYPE_COUNT);
  memoryPatch(0x004D72B2, IMAGE_TYPE_COUNT);
  memoryPatch(0x004D733E, IMAGE_TYPE_COUNT);
  memoryPatch(0x004F397E, IMAGE_TYPE_COUNT);
  memoryPatch(0x00514018, IMAGE_TYPE_COUNT);
  memoryPatch(0x00514024, IMAGE_TYPE_COUNT);
  memoryPatch(0x00514030, IMAGE_TYPE_COUNT);
  memoryPatch(0x0051403C, IMAGE_TYPE_COUNT);
  memoryPatch(0x00514048, IMAGE_TYPE_COUNT);
  memoryPatch(0x00514054, IMAGE_TYPE_COUNT);
  memoryPatch(0x00514060, IMAGE_TYPE_COUNT);
  memoryPatch(0x0051406C, IMAGE_TYPE_COUNT);
  memoryPatch(0x00514078, IMAGE_TYPE_COUNT);
  memoryPatch(0x00514084, IMAGE_TYPE_COUNT);
  memoryPatch(0x00514090, IMAGE_TYPE_COUNT);
  memoryPatch(0x0051409C, IMAGE_TYPE_COUNT);
  memoryPatch(0x005140A8, IMAGE_TYPE_COUNT);
  memoryPatch(0x005140B4, IMAGE_TYPE_COUNT);
  
  memoryPatch(0x0048D770, iImagesGRPGraphic);
  memoryPatch(0x0048DAAA, iImagesGRPGraphic);
  memoryPatch(0x00493369, ((u32)iImagesGRPGraphic)+0x920);
  memoryPatch(0x004D50E2, iImagesGRPGraphic);
  memoryPatch(0x004D5417, ((u32)iImagesGRPGraphic)+0x348);
  memoryPatch(0x004D5639, ((u32)iImagesGRPGraphic)-4);
  memoryPatch(0x004D5A5A, iImagesGRPGraphic);
  memoryPatch(0x004D6028, ((u32)iImagesGRPGraphic)+0x8C4);
  memoryPatch(0x004D6363, iImagesGRPGraphic);
  memoryPatch(0x004D6832, iImagesGRPGraphic);
  memoryPatch(0x004D701E, iImagesGRPGraphic);
  memoryPatch(0x004D7268, iImagesGRPGraphic);
  memoryPatch(0x00401E12, &iLO_s->imagesAttackOverlayGraphic);
  memoryPatch(0x00478000, &iLO_s->imagesAttackOverlayGraphic);
  memoryPatch(0x00478074, &iLO_s->imagesAttackOverlayGraphic);
  memoryPatch(0x00499692, &iLO_s->imagesAttackOverlayGraphic);
  memoryPatch(0x004D50F6, &iLO_s->imagesAttackOverlayGraphic);
  memoryPatch(0x004D578B, &iLO_s->imagesAttackOverlayGraphic);
  memoryPatch(0x004D7290, &iLO_s->imagesAttackOverlayGraphic);
  memoryPatch(0x0049941D, &iLO_s->imagesDamageOverlayGraphic);
  memoryPatch(0x004994C5, &iLO_s->imagesDamageOverlayGraphic);
  memoryPatch(0x00499606, &iLO_s->imagesDamageOverlayGraphic);
  memoryPatch(0x004D5C3C, &iLO_s->imagesDamageOverlayGraphic);
  memoryPatch(0x004D72BD, &iLO_s->imagesDamageOverlayGraphic);
  memoryPatch(0x00498BE8, &iLO_s->imagesSpecialOverlayGraphic);
  memoryPatch(0x0049F741, &iLO_s->imagesSpecialOverlayGraphic);
  memoryPatch(0x004D5A0D, &iLO_s->imagesSpecialOverlayGraphic);
  memoryPatch(0x004D72E0, &iLO_s->imagesSpecialOverlayGraphic);
  memoryPatch(0x0049826D, &iLO_s->imagesLandingDustOverlayGraphic);
  memoryPatch(0x0049829D, &iLO_s->imagesLandingDustOverlayGraphic);
  memoryPatch(0x004D7303, &iLO_s->imagesLandingDustOverlayGraphic);
  memoryPatch(0x004D7326, &iLO_s->imagesLiftoffOverlayGraphic);
  memoryPatch(0x004D511E, iImagesShieldOverlayGraphic);
  memoryPatch(0x004D7349, iImagesShieldOverlayGraphic);
  memoryPatch(0x004E6152, iImagesShieldOverlayGraphic);
  memoryPatch(0x00498FCC, iGubImageRLE);
  memoryPatch(0x004D5AD2, iGubImageRLE);
  memoryPatch(0x004D5AE2, iGubImageRLE);
  memoryPatch(0x004D66BA, iGubImageRLE);
  memoryPatch(0x0051404C, iGubImageRLE);
  memoryPatch(0x004997ED, iImagesLandingDustOverlay);
  memoryPatch(0x004D72FE, iImagesLandingDustOverlay);
  memoryPatch(0x005140A0, iImagesLandingDustOverlay);
  memoryPatch(0x004976E6, iImagesDrawifCloaked);
  memoryPatch(0x00498186, iImagesDrawifCloaked);
  memoryPatch(0x004D6EC9, iImagesDrawifCloaked);
  memoryPatch(0x004D7A62, iImagesDrawifCloaked);
  memoryPatch(0x004D7AC5, iImagesDrawifCloaked);
  memoryPatch(0x00514040, iImagesDrawifCloaked);
  memoryPatch(0x004D72DB, iImagesSpecialOverlay);
  memoryPatch(0x00514094, iImagesSpecialOverlay);
  memoryPatch(0x004D7263, iImagesGRP);
  memoryPatch(0x00514010, iImagesGRP);
  memoryPatch(0x004D5AEC, iGubImageColorShift);
  memoryPatch(0x00514058, iGubImageColorShift);
  memoryPatch(0x00467387, iImagesDamageOverlay);
  memoryPatch(0x004798F2, iImagesDamageOverlay);
  memoryPatch(0x004993D4, iImagesDamageOverlay);
  memoryPatch(0x00499594, iImagesDamageOverlay);
  memoryPatch(0x004D72B8, iImagesDamageOverlay);
  memoryPatch(0x00514088, iImagesDamageOverlay);
  memoryPatch(0x004D728B, iImagesAttackOverlay);
  memoryPatch(0x0051407C, iImagesAttackOverlay);
  memoryPatch(0x004D7344, iImagesShieldOverlay);
  memoryPatch(0x00514070, iImagesShieldOverlay);
  memoryPatch(0x004D5A7E, iImagesClickable);
  memoryPatch(0x00514028, iImagesClickable);
  memoryPatch(0x0049982D, iImagesLiftoffOverlay);
  memoryPatch(0x004D7321, iImagesLiftoffOverlay);
  memoryPatch(0x005140AC, iImagesLiftoffOverlay);
  memoryPatch(0x00499002, iImagesIscriptID);
  memoryPatch(0x004D671F, iImagesIscriptID);
  memoryPatch(0x004D67AB, iImagesIscriptID);
  memoryPatch(0x004D689F, iImagesIscriptID);
  memoryPatch(0x004D6921, iImagesIscriptID);
  memoryPatch(0x004D8542, iImagesIscriptID);
  memoryPatch(0x00514064, iImagesIscriptID);
  memoryPatch(0x00498FDD, iGubImageUseScript);
  memoryPatch(0x004D6703, iGubImageUseScript);
  memoryPatch(0x00514034, iGubImageUseScript);
  memoryPatch(0x004D5A69, iImagesGFXTurns);
  memoryPatch(0x0051401C, iImagesGFXTurns);
  memoryPatch(0x004994AD, iImagesSomething);
  memoryPatch(0x0049EF39, iImagesSomething);
  memoryPatch(0x004D5C35, iImagesSomething);
  memoryPatch(0x004D5C79, iImagesSomething);
  memoryPatch(0x004E60B8, iImagesSomething);
  memoryPatch(0x004E669C, iImagesSomething);
  
  memoryPatch(0x00513FC0, SPRITE_TYPE_COUNT);
  memoryPatch(0x00513FCC, SPRITE_TYPE_COUNT-SPRITE_TYPE_START);
  memoryPatch(0x00513FD8, SPRITE_TYPE_COUNT);
  memoryPatch(0x00513FE4, SPRITE_TYPE_COUNT);
  memoryPatch(0x00513FF0, SPRITE_TYPE_COUNT-SPRITE_TYPE_START);
  memoryPatch(0x00513FFC, SPRITE_TYPE_COUNT-SPRITE_TYPE_START);

  memoryPatch(0x00463A8B, sImageID);
  memoryPatch(0x004683D9, sImageID);
  memoryPatch(0x0048D769, sImageID);
  memoryPatch(0x0048DAA3, sImageID);
  memoryPatch(0x00499081, sImageID);
  memoryPatch(0x004D7A5C, sImageID);
  memoryPatch(0x004D7ABF, sImageID);
  memoryPatch(0x004E4CC9, sImageID);
  memoryPatch(0x004E4FDF, sImageID);
  memoryPatch(0x004E501C, sImageID);
  memoryPatch(0x004E6644, sImageID);
  memoryPatch(0x00513FB8, sImageID);
  memoryPatch(0x0047A873, sHealthBarSize-SPRITE_TYPE_START);
  memoryPatch(0x0047A90F, sHealthBarSize-SPRITE_TYPE_START);
  memoryPatch(0x0047AA6E, sHealthBarSize-SPRITE_TYPE_START);
  memoryPatch(0x004D6045, sHealthBarSize-SPRITE_TYPE_START);
  memoryPatch(0x00513FC4, sHealthBarSize);
  memoryPatch(0x00513FD0, sUnknown2);
  memoryPatch(0x0049906D, sIsVisible);
  memoryPatch(0x00513FDC, sIsVisible);
  memoryPatch(0x004D6021, sSelCircleImage-SPRITE_TYPE_START);
  memoryPatch(0x004D681E, sSelCircleImage-SPRITE_TYPE_START);
  memoryPatch(0x00513FE8, sSelCircleImage);
  memoryPatch(0x004D6034, sSelCircleOffset-SPRITE_TYPE_START);
  memoryPatch(0x004D6842, sSelCircleOffset-SPRITE_TYPE_START);
  memoryPatch(0x00513FF4, sSelCircleOffset);
  
  memoryPatch(0x00515A40, FLINGY_TYPE_COUNT);
  memoryPatch(0x00515A4C, FLINGY_TYPE_COUNT);
  memoryPatch(0x00515A58, FLINGY_TYPE_COUNT);
  memoryPatch(0x00515A64, FLINGY_TYPE_COUNT);
  memoryPatch(0x00515A70, FLINGY_TYPE_COUNT);
  memoryPatch(0x00515A7C, FLINGY_TYPE_COUNT);
  memoryPatch(0x00515A88, FLINGY_TYPE_COUNT);
  
  memoryPatch(0x00463A7C, fSpriteID);
  memoryPatch(0x004683D1, fSpriteID);
  memoryPatch(0x0048D761, fSpriteID);
  memoryPatch(0x0048DA9B, fSpriteID);
  memoryPatch(0x00496415, fSpriteID);
  memoryPatch(0x00497153, fSpriteID);
  memoryPatch(0x0049717A, fSpriteID);
  memoryPatch(0x004971A1, fSpriteID);
  memoryPatch(0x004971C8, fSpriteID);
  memoryPatch(0x004971EF, fSpriteID);
  memoryPatch(0x00497216, fSpriteID);
  memoryPatch(0x004E9A1B, fSpriteID);
  memoryPatch(0x00515A38, fSpriteID);
  memoryPatch(0x00454337, fTopSpeed);
  memoryPatch(0x0047B8FE, fTopSpeed);
  memoryPatch(0x00494FA6, fTopSpeed);
  memoryPatch(0x00496381, fTopSpeed);
  memoryPatch(0x00515A44, fTopSpeed);
  memoryPatch(0x0047B8AF, fAcceleration);
  memoryPatch(0x00494FB4, fAcceleration);
  memoryPatch(0x0049638C, fAcceleration);
  memoryPatch(0x00515A50, fAcceleration);
  memoryPatch(0x00494FBD, fHaltDistance);
  memoryPatch(0x00515A5C, fHaltDistance);
  memoryPatch(0x0047B85E, fTurnRadius);
  memoryPatch(0x00496399, fTurnRadius);
  memoryPatch(0x00515A68, fTurnRadius);
  memoryPatch(0x00515A74, fUnused);
  memoryPatch(0x00454320, fMoveControl);
  memoryPatch(0x004963A2, fMoveControl);
  memoryPatch(0x00515A80, fMoveControl);
  
  memoryPatch(0x0045F042, SOUND_TYPE_COUNT);
  memoryPatch(0x004BBC2D, SOUND_TYPE_COUNT);
  memoryPatch(0x004BBCA5, SOUND_TYPE_COUNT-1);
  memoryPatch(0x004BBEB7, SOUND_TYPE_COUNT-1);
  memoryPatch(0x004BCA98, SOUND_TYPE_COUNT);
  memoryPatch(0x004BCD2A, SOUND_TYPE_COUNT);
  memoryPatch(0x004DB0FA, SOUND_TYPE_COUNT);
  memoryPatch(0x005154A0, SOUND_TYPE_COUNT);
  memoryPatch(0x005154AC, SOUND_TYPE_COUNT);
  memoryPatch(0x005154B8, SOUND_TYPE_COUNT);
  memoryPatch(0x005154C4, SOUND_TYPE_COUNT);
  memoryPatch(0x005154D0, SOUND_TYPE_COUNT);
  
  memoryPatch(0x00455BDC, SFXDAT_Volume);
  memoryPatch(0x0048D0F0, ((u32)SFXDAT_Volume)+0x17);
  memoryPatch(0x0048D16C, ((u32)SFXDAT_Volume)+2);
  memoryPatch(0x0048D409, ((u32)SFXDAT_Volume)+0x17);
  memoryPatch(0x0048ECED, SFXDAT_Volume);
  memoryPatch(0x0048EDD7, SFXDAT_Volume);
  memoryPatch(0x0048EE5C, SFXDAT_Volume);
  memoryPatch(0x0048EFA5, SFXDAT_Volume);
  memoryPatch(0x0048F028, SFXDAT_Volume);
  memoryPatch(0x0048F0CA, SFXDAT_Volume);
  memoryPatch(0x0048F1A7, SFXDAT_Volume);
  memoryPatch(0x004E3EB5, SFXDAT_Volume);
  memoryPatch(0x004E3F55, SFXDAT_Volume);
  memoryPatch(0x005154C8, SFXDAT_Volume);
  memoryPatch(0x004BB972, SFXDAT_Flags);
  memoryPatch(0x004BBC54, SFXDAT_Flags);
  memoryPatch(0x004BBC67, SFXDAT_Flags);
  memoryPatch(0x004BBF17, SFXDAT_Flags);
  memoryPatch(0x004BBF20, SFXDAT_Flags);
  memoryPatch(0x004BBFF3, SFXDAT_Flags);
  memoryPatch(0x004BC06C, SFXDAT_Flags);
  memoryPatch(0x004BC0C0, SFXDAT_Flags);
  memoryPatch(0x004BC7AE, SFXDAT_Flags);
  memoryPatch(0x004BC80A, SFXDAT_Flags);
  memoryPatch(0x004BC846, SFXDAT_Flags);
  memoryPatch(0x004BCAD6, SFXDAT_Flags);
  memoryPatch(0x004BCAF3, SFXDAT_Flags);
  memoryPatch(0x004BCB0B, SFXDAT_Flags);
  memoryPatch(0x004BCD44, SFXDAT_Flags);
  memoryPatch(0x005154B0, SFXDAT_Flags);
  memoryPatch(0x0048F127, SFXDAT_Race);
  memoryPatch(0x0048F204, SFXDAT_Race);
  memoryPatch(0x0048F2EB, SFXDAT_Race);
  memoryPatch(0x0048F376, ((u32)SFXDAT_Race)+0x10E);
  memoryPatch(0x0048F3D2, SFXDAT_Race);
  memoryPatch(0x0048F41B, ((u32)SFXDAT_Race)+0x10E);
  memoryPatch(0x0048F4A0, SFXDAT_Race);
  memoryPatch(0x0048F572, SFXDAT_Race);
  memoryPatch(0x0048F607, SFXDAT_Race);
  memoryPatch(0x0048F697, SFXDAT_Race);
  memoryPatch(0x0048F732, SFXDAT_Race);
  memoryPatch(0x0048F822, SFXDAT_Race);
  memoryPatch(0x0048F8E3, ((u32)SFXDAT_Race)+0x104);
  memoryPatch(0x0048FAEB, SFXDAT_Race);
  memoryPatch(0x005154BC, SFXDAT_Race);
  memoryPatch(0x004BBEC3, SFXDAT_Type);
  memoryPatch(0x004BC0D3, SFXDAT_Type);
  memoryPatch(0x004BC813, SFXDAT_Type);
  memoryPatch(0x005154A4, SFXDAT_Type);
  memoryPatch(0x0046102D, ((u32)SFXDAT_SoundFile)+0x58);
  memoryPatch(0x0046116D, ((u32)SFXDAT_SoundFile)+0x54);
  memoryPatch(0x004BC690, SFXDAT_SoundFile);
  memoryPatch(0x004BCA3C, SFXDAT_SoundFile);
  memoryPatch(0x004BCB2A, ((u32)SFXDAT_SoundFile)+0x3C);
  memoryPatch(0x004DB0E6, SFXDAT_SoundFile);
  memoryPatch(0x00515498, SFXDAT_SoundFile);
  memoryPatch(0x004BB8E8, SFXDAT_Unknown);
  memoryPatch(0x004BBC47, SFXDAT_Unknown);
  memoryPatch(0x004BC077, SFXDAT_Unknown);
  memoryPatch(0x004BC674, SFXDAT_Unknown);
  memoryPatch(0x004BCAA6, SFXDAT_Unknown);
  memoryPatch(0x0045EF66, ((u32)SFXDAT_Unknown)+8);
  memoryPatch(0x0048F11D, ((u32)SFXDAT_Unknown)+8);
  memoryPatch(0x0048F1FA, ((u32)SFXDAT_Unknown)+8);
  memoryPatch(0x0048F2DE, ((u32)SFXDAT_Unknown)+8);
  memoryPatch(0x0048F3CA, ((u32)SFXDAT_Unknown)+8);
  memoryPatch(0x0048F498, ((u32)SFXDAT_Unknown)+8);
  memoryPatch(0x0048F56A, ((u32)SFXDAT_Unknown)+8);
  memoryPatch(0x0048F5FF, ((u32)SFXDAT_Unknown)+8);
  memoryPatch(0x0048F68F, ((u32)SFXDAT_Unknown)+8);
  memoryPatch(0x0048F72A, ((u32)SFXDAT_Unknown)+8);
  memoryPatch(0x0048F81A, ((u32)SFXDAT_Unknown)+8);
  memoryPatch(0x0048FAE3, ((u32)SFXDAT_Unknown)+8);
  memoryPatch(0x0048F8DA, ((u32)SFXDAT_Unknown)+0x828);
  memoryPatch(0x0048F36F, ((u32)SFXDAT_Unknown)+0x878);
  memoryPatch(0x0048F412, ((u32)SFXDAT_Unknown)+0x878);

  memoryPatch(0x0044EC20, PORTDAT_IdleDir);
  memoryPatch(0x0044F1E5, PORTDAT_IdleDir);
  memoryPatch(0x0044F213, PORTDAT_IdleDir);
  memoryPatch(0x0044F253, ((u32)PORTDAT_IdleDir)+4);
  memoryPatch(0x0044F281, ((u32)PORTDAT_IdleDir)+4);
  memoryPatch(0x0044F2C1, ((u32)PORTDAT_IdleDir)+8);
  memoryPatch(0x0044F2EF, ((u32)PORTDAT_IdleDir)+8);
  memoryPatch(0x0044F32F, ((u32)PORTDAT_IdleDir)+0x0C);
  memoryPatch(0x0044F35D, ((u32)PORTDAT_IdleDir)+0x0C);
  memoryPatch(0x0044F39D, ((u32)PORTDAT_IdleDir)+0x10);
  memoryPatch(0x0044F3CB, ((u32)PORTDAT_IdleDir)+0x10);
  memoryPatch(0x0045E810, PORTDAT_IdleDir);
  memoryPatch(0x0045F3D5, PORTDAT_IdleDir);
  memoryPatch(0x0045F403, PORTDAT_IdleDir);
  memoryPatch(0x0045F443, ((u32)PORTDAT_IdleDir)+4);
  memoryPatch(0x0045F471, ((u32)PORTDAT_IdleDir)+4);
  memoryPatch(0x0045F4B1, ((u32)PORTDAT_IdleDir)+8);
  memoryPatch(0x0045F4DF, ((u32)PORTDAT_IdleDir)+8);
  memoryPatch(0x0045F51F, ((u32)PORTDAT_IdleDir)+0x0C);
  memoryPatch(0x0045F54D, ((u32)PORTDAT_IdleDir)+0x0C);
  memoryPatch(0x0045F58D, ((u32)PORTDAT_IdleDir)+0x10);
  memoryPatch(0x0045F5BB, ((u32)PORTDAT_IdleDir)+0x10);
  memoryPatch(0x00513780, PORTDAT_IdleDir);
  memoryPatch(0x0044ED04, PORTDAT_TalkingDir);
  memoryPatch(0x0044F21C, PORTDAT_TalkingDir);
  memoryPatch(0x0044F24A, PORTDAT_TalkingDir);
  memoryPatch(0x0044F28A, ((u32)PORTDAT_TalkingDir)+4);
  memoryPatch(0x0044F2B8, ((u32)PORTDAT_TalkingDir)+4);
  memoryPatch(0x0044F2F8, ((u32)PORTDAT_TalkingDir)+8);
  memoryPatch(0x0044F326, ((u32)PORTDAT_TalkingDir)+8);
  memoryPatch(0x0044F366, ((u32)PORTDAT_TalkingDir)+0x0C);
  memoryPatch(0x0044F394, ((u32)PORTDAT_TalkingDir)+0x0C);
  memoryPatch(0x0044F3D4, ((u32)PORTDAT_TalkingDir)+0x10);
  memoryPatch(0x0044F402, ((u32)PORTDAT_TalkingDir)+0x10);
  memoryPatch(0x0045E900, PORTDAT_TalkingDir);
  memoryPatch(0x0045F40C, PORTDAT_TalkingDir);
  memoryPatch(0x0045F43A, PORTDAT_TalkingDir);
  memoryPatch(0x0045F47A, ((u32)PORTDAT_TalkingDir)+4);
  memoryPatch(0x0045F4A8, ((u32)PORTDAT_TalkingDir)+4);
  memoryPatch(0x0045F4E8, ((u32)PORTDAT_TalkingDir)+8);
  memoryPatch(0x0045F516, ((u32)PORTDAT_TalkingDir)+8);
  memoryPatch(0x0045F556, ((u32)PORTDAT_TalkingDir)+0x0C);
  memoryPatch(0x0045F584, ((u32)PORTDAT_TalkingDir)+0x0C);
  memoryPatch(0x0045F5C4, ((u32)PORTDAT_TalkingDir)+0x10);
  memoryPatch(0x0045F5F2, ((u32)PORTDAT_TalkingDir)+0x10);
  memoryPatch(0x0051378C, PORTDAT_TalkingDir);
  memoryPatch(0x0044F41A, PORTDAT_IdleSMKChange);
  memoryPatch(0x0044F438, PORTDAT_IdleSMKChange);
  memoryPatch(0x0045F60A, PORTDAT_IdleSMKChange);
  memoryPatch(0x0045F628, PORTDAT_IdleSMKChange);
  /*
  memoryPatch(0x0044F41A, ((u32)PORTDAT_TalkingDir)+(PORTRAIT_TYPE_COUNT*sizeof(u32)));
  memoryPatch(0x0044F438, ((u32)PORTDAT_TalkingDir)+(PORTRAIT_TYPE_COUNT*sizeof(u32)));
  memoryPatch(0x0045F60A, ((u32)PORTDAT_TalkingDir)+(PORTRAIT_TYPE_COUNT*sizeof(u32)));
  memoryPatch(0x0045F628, ((u32)PORTDAT_TalkingDir)+(PORTRAIT_TYPE_COUNT*sizeof(u32)));
  */
  memoryPatch(0x00513798, PORTDAT_IdleSMKChange);
  memoryPatch(0x005137A4, PORTDAT_TalkingSMKChange);
  memoryPatch(0x005137B0, PORTDAT_IdleUnknown1);
  memoryPatch(0x005137BC, PORTDAT_TalkingUnknown1);

  memoryPatch(0x0044F40A, PORTRAIT_TYPE_COUNT*4);
  memoryPatch(0x0045F5FA, PORTRAIT_TYPE_COUNT*4);
  memoryPatch(0x00513788, PORTRAIT_TYPE_COUNT);
  memoryPatch(0x00513794, PORTRAIT_TYPE_COUNT);
  memoryPatch(0x005137A0, PORTRAIT_TYPE_COUNT);
  memoryPatch(0x005137AC, PORTRAIT_TYPE_COUNT);
  memoryPatch(0x005137B8, PORTRAIT_TYPE_COUNT);
  memoryPatch(0x005137C4, PORTRAIT_TYPE_COUNT);
}

} //hooks