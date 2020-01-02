#include "weapon_begin.h"
#include <SCBW\scbwdata.h>
#include <SCBW/api.h>

namespace {
typedef void (__stdcall *GetWeaponHitPosFunc)(const CUnit *unit, s32 *x, s32 *y);
GetWeaponHitPosFunc const getWeaponHitPos = (GetWeaponHitPosFunc) 0x004762C0;
} //unnamed namespace

const u32 Hook_DoWeaponIscript = 0x00476C90;

u32* dword_64DEB0 = (u32*)0x0064DEB0;
const u32 Hook_476640 = 0x00476640;
const u32 Hook_getUpgradedWpnCooldown = 0x00475DC0;
const u32 Hook_randomizeShort = 0x004DC4A0;

Bool32 sub_476640(CUnit* unit, u8 WeaponId){
	static Bool32 result;

	__asm{
		PUSHAD
		MOVZX	EBX, WeaponId
		PUSH	EBX
		MOV		EAX, unit
		CALL	Hook_476640
		MOV		result, EAX
		POPAD
	}

	return result;
}

u8 getUpgradedWpnCooldown(u8 WeaponId, CUnit* unit){
	static u8 result;

	__asm{
		PUSHAD
		MOV		AL, WeaponId
		MOV		ESI, unit
		CALL	Hook_getUpgradedWpnCooldown
		MOV		result, AL
		POPAD
	}

	return result;
}

u16 randomizeShort(u32 value){
	static u16 result;

	__asm{
		PUSHAD
		MOV		EAX, value
		CALL	Hook_randomizeShort
		MOV		result, AX
		POPAD
	}

	return result;
}

namespace hooks {

Bool32 doWeaponIscript(CUnit* unit, u8 weaponId, u32 GndOrAir, u8 animation){//GndOrAir: 1->target is ground, 2->target is air
	static Bool32 result;

	__asm{
		PUSHAD
		MOV		EAX, unit
		MOV		BL, weaponId
		MOV		EDI, GndOrAir
		MOVZX	EDX, animation
		PUSH	EDX
		CALL	Hook_DoWeaponIscript
		MOV		result, EAX
		POPAD
	}

	return result;
}

void weaponBeginHook(CUnit* unit, u8 iscriptAnimation){//default, iscriptAnimation is 2 or 5

	*dword_64DEB0 = 0;//Unknown

	if(unit->status & UnitStatus::CanNotAttack)
		return;

	if(unit->id == UnitId::battlecruiser || unit->id == UnitId::norad_ii|| unit->id == UnitId::mutalisk){
		s32 x, y;
  
		getWeaponHitPos(unit, &x, &y);

		if(unit->isDistanceGreaterThanHaltDistance(x, y, 160))//distance는 스크립트의 거리 값과 동일
		doWeaponIscript(unit, units_dat::AirWeapon[unit->id], 2, iscriptAnimation+1);
		else
		doWeaponIscript(unit, unit->getGroundWeapon(), 1, iscriptAnimation);

		return;
	}

	const CUnit* target = unit->orderTarget.unit;
	if(target && target->status & UnitStatus::InAir){
		doWeaponIscript(unit, units_dat::AirWeapon[unit->id], 2, iscriptAnimation+1);
		return;
	}

	switch(unit->id){
	case UnitId::lurker:
		if(!(unit->status & UnitStatus::Burrowed)){
			doWeaponIscript(unit, WeaponId::None, 1, iscriptAnimation);				//it's not mistake...
			return;
		}
		break;
	case UnitId::UnusedKhaydarinCrystalFormation:
		if(unit->orderTarget.unit && (units_dat::SizeType[unit->orderTarget.unit->id] == 1 
			|| (units_dat::SizeType[unit->orderTarget.unit->id] == 2 && units_dat::BaseProperty[unit->orderTarget.unit->id] & UnitProperty::Organic)))
			iscriptAnimation = 4;
		break;
	case UnitId::jim_raynor_marine:
		if(unit->orderTarget.unit && unit->orderTarget.unit->status & UnitStatus::GroundedBuilding && unit->orderTarget.unit->id != UnitId::UnusedRuins){
			iscriptAnimation = 10;
			doWeaponIscript(unit, WeaponId::GaussRifle13_Unused, 1, iscriptAnimation);
			return;
		}
		break;
	case UnitId::jim_raynor_vulture:
		if(unit->orderTarget.unit && unit->orderTarget.unit->status & UnitStatus::GroundedBuilding && unit->orderTarget.unit->id != UnitId::UnusedRuins){
			iscriptAnimation = 7;
			doWeaponIscript(unit, WeaponId::GaussRifle12_Unused, 1, iscriptAnimation);
			return;
		}
		break;
	}

	doWeaponIscript(unit, unit->getGroundWeapon(), 1, iscriptAnimation);

}

}