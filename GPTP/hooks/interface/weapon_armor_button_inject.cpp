#include "weapon_armor_button.h"
#include <hook_tools.h>

//미완성 코드임.

//이 함수 뒤에 인터셉터나 스캐럽, 마인 버튼같은게 출력됨.
const u32 SetUnitDataButton = 0x00426E34;
const u32 SetUnitDataButtonBack = 0x00426E3C;
const u32 StatWeaponUpgradeCount = 0x00425790;
void __declspec(naked) setUnitDataButtonWrapper() {
	__asm{
		MOV     AX, [EDX+64h]
		CMP		AX, 4Bh
		JZ		short _Next0
		CMP		AX, 0BAh
		JNZ		short _Next1
_Next0:
		MOV		ECX, 0A1h
		MOVZX	ECX, BYTE PTR [ECX+6636B8h]
		PUSH	ECX
		PUSH	EDI
		MOV		EAX, EBX
		CALL	StatWeaponUpgradeCount
		INC		EDI
		JMP		_End
_Next1:
		CMP		AX, 14h
		JZ		short _Next2
		CMP		AX, 62h
		JNZ		short _Next3
_Next2:
		MOV		ECX, 81h
		PUSH	ECX
		PUSH	EDI
		MOV		EAX, EBX
		CALL	StatWeaponUpgradeCount
		INC		EDI
		JMP		_End
_Next3:
		CMP		AX, 13h
		JNZ		short _Next4
		MOV		ECX, 80h
		PUSH	ECX
		PUSH	EDI
		MOV		EAX, EBX
		CALL	StatWeaponUpgradeCount
		INC		EDI
		JMP		_End
_Next4:
		CMP		AX, 46h
		JNZ		short _End
		MOV		ECX, 4Ah
		PUSH	ECX
		PUSH	EDI
		MOV		EAX, EBX
		CALL	StatWeaponUpgradeCount
		INC		EDI
_End:
		MOV		EDX, [597248h]
		MOV		EDX, [EDX]
		MOV     AX, [EDX+64h]
		CMP     AX, 48h
		JMP		SetUnitDataButtonBack
	}
}


namespace hooks {

void injectUnitDataButtonHook() {
	memoryPatch_Byte(0x00426E8A, 9);
	jmpPatch(setUnitDataButtonWrapper, SetUnitDataButton);
}

} //hooks