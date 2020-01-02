#include "draw_Minimap_Unit_Box.h"
#include "hook_tools.h"

const u32 Hook_drawMinimapUnitBox0 = 0x004A46B7;
const u32 Hook_drawMinimapUnitBox0Back0 = 0x004A46EC;
const u32 Hook_drawMinimapUnitBox0Back1 = 0x004A478B;
void __declspec(naked) drawMinimapUnitBox0() {
  static u16 unitId;

  __asm{
	  PUSHAD
	  MOV unitId, cx
  }

  if(hooks::isDrawableUnit(unitId)){
	  __asm{
		  POPAD
		  JMP Hook_drawMinimapUnitBox0Back0
	  }
  }else{
	  __asm{
		  POPAD
		  JMP Hook_drawMinimapUnitBox0Back1
	  }
  }
}

const u32 Hook_drawMinimapUnitBox1 = 0x004A4817;
const u32 Hook_drawMinimapUnitBox1Back0 = 0x004A4849;
const u32 Hook_drawMinimapUnitBox1Back1 = 0x004A48C6;
void __declspec(naked) drawMinimapUnitBox1() {
  static u16 unitId;

  __asm{
	  PUSHAD
	  MOV unitId, ax
  }

  if(hooks::isDrawableUnit(unitId)){
	  __asm{
		  POPAD
		  JMP Hook_drawMinimapUnitBox1Back0
	  }
  }else{
	  __asm{
		  POPAD
		  JMP Hook_drawMinimapUnitBox1Back1
	  }
  }
}

const u32 Hook_drawAllMinimapUnitBox = 0x004A490A;
const u32 Hook_drawAllMinimapUnitBoxBack0 = 0x004A493B;
const u32 Hook_drawAllMinimapUnitBoxBack1 = 0x004A49D6;
void __declspec(naked) drawAllMinimapUnitBox() {
  static u16 unitId;

  __asm{
	  PUSHAD
	  MOV unitId, ax
  }

  if(hooks::isDrawableUnit(unitId)){
	  __asm{
		  POPAD
		  JMP Hook_drawAllMinimapUnitBoxBack0
	  }
  }else{
	  __asm{
		  POPAD
		  JMP Hook_drawAllMinimapUnitBoxBack1
	  }
  }
}

namespace hooks {

void injectdrawMinimapUnitBoxHooks() {
  jmpPatch(drawMinimapUnitBox0, Hook_drawMinimapUnitBox0);
  jmpPatch(drawMinimapUnitBox1, Hook_drawMinimapUnitBox1);
  jmpPatch(drawAllMinimapUnitBox, Hook_drawAllMinimapUnitBox);
}

} //hooks