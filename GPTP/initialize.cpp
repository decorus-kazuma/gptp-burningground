#include "Plugin.h"
#include <Psapi.h>

#include "file_data.h"
using namespace dll;

//Hook header files
#include "hooks/game_hooks.h"
#include "graphics/draw_hook.h"

#include "hooks/apply_upgrade_flags.h"
#include "hooks/attack_priority.h"
//#include "hooks/bunker_hooks.h"
#include "hooks/change_buttonset.h"
//#include "hooks/cloak_nearby_units.h"
#include "hooks/cloak_tech.h"
//#include "hooks/consume.h"
//#include "hooks/detector.h"
#include "hooks/harvest.h"
#include "hooks/kill_count.h"
#include "hooks/rally_point.h"
//#include "hooks/recharge_shields.h"
#include "hooks/spider_mine.h"
#include "hooks/stim_packs.h"
#include "hooks/tech_target_check.h"
//#include "hooks/transfer_tech_upgrades.h"
#include "hooks/unit_speed.h"
#include "hooks/update_status_effects.h"
#include "hooks/update_unit_state.h"
#include "hooks/weapon_begin.h"
//#include "hooks/weapon_cooldown.h"
#include "hooks/weapon_damage.h"
#include "hooks/weapon_fire.h"

#include "hooks/unit_destructor_special.h"
#include "hooks/psi_field.h"

#include "hooks/unit_morph.h"
//#include "hooks/building_morph.h"

#include "hooks/unit_stats/armor_bonus.h"
//#include "hooks/unit_stats/sight_range.h"
#include "hooks/unit_stats/max_energy.h"
#include "hooks/unit_stats/weapon_range.h"
#include "hooks/interface/BG.h"
#include "hooks/interface/draw_Minimap_Unit_Box.h"
#include "hooks/interface/print_HP.h"
#include "hooks/interface/right_click_action.h"
#include "hooks/interface/select_one.h"
#include "hooks/interface/unit_rank.h"
#include "hooks/interface/weapon_armor_tooltip.h"
#include "hooks/interface/weapon_armor_button.h"

#include "AI/spellcasting.h"

#include "hooks/SFmpq_static.h"
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>

/// This function is called when the plugin is loaded into StarCraft.
/// You can enable/disable each group of hooks by commenting them.
/// You can also add custom modifications to StarCraft.exe by using:
///    memoryPatch(address_to_patch, value_to_patch_with);

BOOL WINAPI Plugin::InitializePlugin(IMPQDraftServer *lpMPQDraftServer) {

	/*//기존에 쓰던 코드
	int answer = MessageBox(NULL, "Do you want to inject W-MODE plugins?", "SC:BurningGround", MB_YESNOCANCEL);
	if (answer == IDYES) {
		HINSTANCE hDll;
		hDll = LoadLibrary("WMODE.dll");
		if(!hDll)
			MessageBox(NULL, "WMODE.dll cannot be found.", NULL, MB_ICONERROR);
		hDll = LoadLibrary("WMODE_FIX.dll");
		if(!hDll)
			MessageBox(NULL, "WMODE_FIX.dll cannot be found.", NULL, MB_ICONERROR);
	}
	else if (answer == IDCANCEL)
		exit(NULL);
	*/

  //StarCraft.exe version check
	char exePath[260];
	const DWORD pathLen = GetModuleFileName(NULL, exePath, sizeof(exePath));
	if (pathLen == sizeof(exePath)) {
		MessageBox(NULL, "Error: Cannot check version of StarCraft.exe. The file path is too long.", NULL, MB_OK);
		exit(1);
		return false;
	}
	if (!checkStarCraftExeVersion(exePath)){
		exit(1);
		return false;
	}

	switch(MessageBox(NULL, "Do you want to inject W-MODE plugins?", "SC:Burning_Ground", MB_YESNOCANCEL)){
		case IDYES:{
			char dll0Path[260];//WMode.dll
			strncpy_s(dll0Path, sizeof(dll0Path), exePath, strrchr(exePath, '\\') - exePath + 1);
			strcat_s(dll0Path, sizeof(dll0Path), "WMode.dll");
			HANDLE cFile0 = CreateFile(dll0Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if(cFile0 == (HANDLE)EOF){//WMode.dll 파일이 없으면
				FILE *fout0;
				fopen_s(&fout0, dll0Path,"wb");
				fwrite(wModeData, sizeof(wModeData), 1, fout0);
				fclose(fout0);
			}
			CloseHandle(cFile0);
			
			char dll1Path[260];//WMode_Fix.dll
			strncpy_s(dll1Path, sizeof(dll1Path), exePath, strrchr(exePath, '\\') - exePath + 1);
			strcat_s(dll1Path, sizeof(dll1Path), "WMode_Fix.dll");
			HANDLE cFile1 = CreateFile(dll1Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if(cFile1 == (HANDLE)EOF){//WMode_Fix.dll 파일이 없으면
				FILE *fout1;
				fopen_s(&fout1, dll1Path,"wb");
				fwrite(wModeFixData, sizeof(wModeFixData), 1, fout1);
				fclose(fout1);
			}
			CloseHandle(cFile1);

			HINSTANCE hDll;     
			hDll = LoadLibrary(dll0Path);
			if(!hDll)
				MessageBox(NULL, "Failed inject WMODE.dll.", NULL, MB_ICONERROR);
			hDll = LoadLibrary(dll1Path);
			if(!hDll)
				MessageBox(NULL, "Failed inject WMODE_Fix.dll.", NULL, MB_ICONERROR);
		}
			break;
		case IDCANCEL:
			exit(NULL);
			return false;
	}


  hooks::injectGameHooks();
  hooks::injectDrawHook();

  hooks::injectApplyUpgradeFlags();
  hooks::injectAttackPriorityHooks();
  //hooks::injectBunkerHooks();
  hooks::injectChangeUnitButtonSetHook();
  //hooks::injectCloakNearbyUnits();
  //hooks::injectCloakingTechHooks();
  //hooks::injectConsumeHooks();
  //hooks::injectDetectorHooks();
  hooks::injectGetUnitRankStringHook();
  hooks::injectdrawMinimapUnitBoxHooks();
  hooks::injectHarvestResource();
  hooks::injectIncrementUnitKillCountHook();
  hooks::injectSetTextStr();
  hooks::injectRallyHooks();
  //hooks::injectRechargeShieldsHooks();
  hooks::injectRightClickActionHooks();
  hooks::injectSelectOneHooks();
  hooks::injectSpiderMineHooks();
  hooks::injectStimPacksHooks();
  hooks::injectTechTargetCheckHooks();
  //hooks::injectTransferTechAndUpgradesHooks();
  hooks::injectUnitSpeedHooks();
  hooks::injectUpdateStatusEffects();
  hooks::injectUpdateUnitState();
  hooks::injectWeaponBeginHook();
  //hooks::injectWeaponCooldownHook();
  hooks::injectWeaponDamageHook();
  hooks::injectWeaponFireHooks();
  
  hooks::injectUnitDestructorSpecial();
  hooks::injectPsiFieldHooks();

  //hooks::injectUnitMorphHooks();
  //hooks::injectBuildingMorphHooks();
  
  hooks::injectArmorBonusHook();
  //hooks::injectSightRangeHook();
  hooks::injectUnitMaxEnergyHook();
  hooks::injectWeaponRangeHooks();
  
  hooks::injectUnitTooltipHook();
  hooks::injectUnitDataButtonHook();

  hooks::injectSpellcasterAI();
  
  hooks::inject_BG();

  return TRUE;
}
