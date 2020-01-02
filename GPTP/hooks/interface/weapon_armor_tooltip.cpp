#include "weapon_armor_tooltip.h"
#include <SCBW/api.h>
#include <SCBW/enumerations/WeaponId.h>
#include <cstdio>

char buffer[256];

//Returns the special damage multiplier factor for units that don't use the
//"Damage Factor" property in weapons.dat.
u8 getDamageFactorForTooltip(u8 weaponId, const CUnit *unit) {
  //Default StarCraft behavior
	switch(unit->id){
	case UnitId::kukulza_guardian:
	case UnitId::jim_raynor_marine:
	case UnitId::tassadar_zeratul:
		if(weaponId != unit->getGroundWeapon())
			break;
	case UnitId::tom_kazansky:
	case UnitId::firebat:
	case UnitId::missile_turret:
		return 2;
	}

  return weapons_dat::DamageFactor[weaponId];
}

//Returns the C-string for the tooltip text of the unit's weapon icon.
//This function is used for weapon icons and special icons.
//Precondition: @p entryStrIndex is a stat_txt.tbl string index.
const char* getWeaponTooltipString(u8 weaponId, const CUnit *unit, u16 entryStrIndex) {
  //Default StarCraft behavior

  const char *entryName = (*statTxtTbl)->getString(entryStrIndex);
  const char *damageStr = (*statTxtTbl)->getString(777);			//"Damage:"
  const char *typeStr = (*statTxtTbl)->getString(1645);				//"Type:"
  const char *rangeStr = (*statTxtTbl)->getString(1646);			//"Range:"

  const u8 damageFactor = getDamageFactorForTooltip(weaponId, unit);
  const u8 upgradeLevel = scbw::getUpgradeLevel(unit->playerId, weapons_dat::DamageUpgrade[weaponId]);
  const u16 baseDamage = weapons_dat::DamageAmount[weaponId] * damageFactor;
  const u16 bonusDamage = weapons_dat::DamageBonus[weaponId] * damageFactor * upgradeLevel;
  const u32 range = weapons_dat::MaxRange[weaponId];    //hooks::getMaxWeaponrangeHook(unit,weaponId);
  const u32 plusRange = unit->getMaxWeaponRange(weaponId);
  const char *damageTypeStr = (*statTxtTbl)->getString(1635+weapons_dat::DamageType[weaponId]);
  
  if (weaponId == WeaponId::HaloRockets || weaponId == WeaponId::GaussRifle1_Unused || weaponId == WeaponId::GaussRifle2_Unused 
	  || weaponId == WeaponId::GaussRifle3_Unused || weaponId == WeaponId::GaussRifle6_Unused || weaponId == WeaponId::Spines) {
	const char *perRocketStr = (*statTxtTbl)->getString(1301); //"per rocket"
	if (plusRange > *maxX && plusRange > *maxY){
		if (bonusDamage > 0) 
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d+%d %s\n%s Unlimited",
				entryName, typeStr, damageTypeStr, damageStr, baseDamage, bonusDamage, perRocketStr, rangeStr);
		else 
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d %s\n%s Unlimited",
				entryName, typeStr, damageTypeStr, damageStr, baseDamage, perRocketStr, rangeStr);
	}
	else{
		if (bonusDamage > 0) 
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d+%d %s\n%s %d",
				entryName, typeStr, damageTypeStr, damageStr, baseDamage, bonusDamage, perRocketStr, rangeStr, range>>5);
		else 
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d %s\n%s %d",
				entryName, typeStr, damageTypeStr, damageStr, baseDamage, perRocketStr, rangeStr, range>>5);
	}
  }
  else if (weaponId == WeaponId::GaussRifle12_Unused || weaponId == WeaponId::GaussRifle13_Unused) {
	const char *targetBuildingStr = (*statTxtTbl)->getString(1694); //""
	if (plusRange > *maxX && plusRange > *maxY){
		if (bonusDamage > 0) 
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d+%d \n%s Unlimited\n%s",
				entryName, typeStr, damageTypeStr, damageStr, baseDamage, bonusDamage, rangeStr, targetBuildingStr);
		else 
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d \n%s Unlimited\n%s",
				entryName, typeStr, damageTypeStr, damageStr, baseDamage, rangeStr, targetBuildingStr);
	}
	else{
		if (bonusDamage > 0) 
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d+%d \n%s %d\n%s",
				entryName, typeStr, damageTypeStr, damageStr, baseDamage, bonusDamage, rangeStr, range>>5, targetBuildingStr);
		else 
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d \n%s %d\n%s",
				entryName, typeStr, damageTypeStr, damageStr, baseDamage, rangeStr, range>>5, targetBuildingStr);
	}
  }
  else {
	  if (plusRange > *maxX && plusRange > *maxY){
		if (bonusDamage > 0)
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n\n%s %d+%d\n%s Unlimited",
					entryName, typeStr, damageTypeStr, damageStr, baseDamage, bonusDamage, rangeStr);
		else
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d\n%s Unlimited",
					entryName, typeStr, damageTypeStr, damageStr, baseDamage, rangeStr);
	  }

	  else if(range!=plusRange){
		if (bonusDamage > 0)
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d+%d\n%s %d+%d",
					entryName, typeStr, damageTypeStr, damageStr, baseDamage, bonusDamage, rangeStr, range>>5, (plusRange-range)>>5);
		else
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d\n%s %d+%d",
					entryName, typeStr, damageTypeStr, damageStr, baseDamage, rangeStr, range>>5, (plusRange-range)>>5);
	  }

	  else if(!(range>>5)){
		if (bonusDamage > 0)
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d+%d\n%s Melee",
					entryName, typeStr, damageTypeStr, damageStr, baseDamage, bonusDamage, rangeStr);
		else
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d\n%s Melee",
					entryName, typeStr, damageTypeStr, damageStr, baseDamage, rangeStr);
	  }

	  else{
		if (bonusDamage > 0)
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d+%d\n%s %d",
					entryName, typeStr, damageTypeStr, damageStr, baseDamage, bonusDamage, rangeStr, range>>5);
		else
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d\n%s %d",
					entryName, typeStr, damageTypeStr, damageStr, baseDamage, rangeStr, range>>5);
	  }
  }

  return buffer;
}

namespace hooks {

//Returns the C-string for the tooltip text of the unit's weapon icon.
const char* getWeaponTooltipString(u8 weaponId, const CUnit *unit) {
  return getWeaponTooltipString(weaponId, unit, weapons_dat::Label[weaponId]);
}

//Returns the C-string for the tooltip text of the unit's armor icon.
const char* getArmorTooltipString(const CUnit *unit) {
  //Default StarCraft behavior
  
  const u16 labelId = upgrades_dat::Label[units_dat::ArmorUpgrade[unit->id]];
  const char *armorUpgradeName = (*statTxtTbl)->getString(labelId);
  const char *armorStr = (*statTxtTbl)->getString(778);          //"Armor:"
  const char *sizeStr = (*statTxtTbl)->getString(1640);         //"Unit Size:"

  const u8 baseArmor = units_dat::ArmorAmount[unit->id];
  const u8 bonusArmor = unit->getArmorBonus();
  const char *unitSizeStr = (*statTxtTbl)->getString(1641+units_dat::SizeType[unit->id]);

  if (bonusArmor > 0)
    sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d+%d",
              armorUpgradeName, sizeStr, unitSizeStr, armorStr, baseArmor, bonusArmor);
  else
    sprintf_s(buffer, sizeof(buffer), "%s\n%s %s\n%s %d",
              armorUpgradeName, sizeStr, unitSizeStr, armorStr, baseArmor);

  return buffer;
}


//Returns the C-string for the tooltip text of the plasma shield icon.
const char* getShieldTooltipString(const CUnit *unit) {
  //Default StarCraft behavior

  const u16 labelId = upgrades_dat::Label[UpgradeId::ProtossPlasmaShields];
  const char *shieldUpgradeName = (*statTxtTbl)->getString(labelId);
  const char *shieldStr = (*statTxtTbl)->getString(779);         //"Shields:"

  const u8 shieldUpgradeLevel = scbw::getUpgradeLevel(unit->playerId, UpgradeId::ProtossPlasmaShields);

  if (shieldUpgradeLevel > 0)
    sprintf_s(buffer, sizeof(buffer), "%s\n%s %d+%d",
              shieldUpgradeName, shieldStr, 0, shieldUpgradeLevel);
  else
    sprintf_s(buffer, sizeof(buffer), "%s\n%s %d",
              shieldUpgradeName, shieldStr, 0);

  return buffer;
}

//Returns the C-string for the tooltip text of the Interceptor icon (Carriers),
//Scarab icon (Reavers), Nuclear Missile icon (Nuclear Silos), and Spider Mine
//icon (Vultures).
const char* getSpecialTooltipString(u16 iconUnitId, const CUnit *unit) {
  //Default StarCraft behavior

  if (iconUnitId == UnitId::interceptor) {
    return getWeaponTooltipString(WeaponId::PulseCannon, unit, 791);  //"Interceptors"
  }

  if (iconUnitId == UnitId::ProtossScarab) {
    return getWeaponTooltipString(WeaponId::Scarab, unit, 792);       //"Scarabs"
  }

  if (iconUnitId == UnitId::nuclear_missile) {
    return (*statTxtTbl)->getString(793);  //"Nukes"
  }

  if (iconUnitId == UnitId::spider_mine) {
    return getWeaponTooltipString(WeaponId::SpiderMines, unit, 794);  //"Spider Mines"
  }

  //Should never reach here
  return "";
}

} //hooks

