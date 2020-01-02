//Source file for the Weapon Fire hook module.
//This hook controls how weapons are fired.
#include "weapon_fire.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>


//-------- Helper function declarations. Do NOT modify! ---------//

namespace {
typedef void (__stdcall *GetWeaponHitPosFunc)(const CUnit *unit, s32 *x, s32 *y);
GetWeaponHitPosFunc const getWeaponHitPos = (GetWeaponHitPosFunc) 0x004762C0;
void createBullet(u8 weaponId, CUnit *source, s16 x, s16 y, u8 attackingPlayer, u8 direction);
} //unnamed namespace


//-------- Actual hook functions --------//

namespace hooks {

//Fires a weapon from the @p unit.
//This hook affects the following iscript opcodes: attackwith, attack, castspell
//This also affects CUnit::fireWeapon().
void fireWeaponHook(CUnit *unit, u8 weaponId) {
  //Default StarCraft behavior

  if (unit->id == UnitId::jim_raynor_marine){
	  if(weaponId == unit->getGroundWeapon() && unit->orderTarget.unit && unit->orderTarget.unit->status & UnitStatus::GroundedBuilding && unit->orderTarget.unit->id != UnitId::UnusedRuins)
		  weaponId = WeaponId::GaussRifle13_Unused;
  }
  else if (unit->id == UnitId::jim_raynor_vulture){
	  if(weaponId == unit->getGroundWeapon() && unit->orderTarget.unit && unit->orderTarget.unit->status & UnitStatus::GroundedBuilding && unit->orderTarget.unit->id != UnitId::UnusedRuins)
		  weaponId = WeaponId::GaussRifle12_Unused;
  }

  //Retrieve the spawning position for the bullet.
  s32 x, y;

  if (weapons_dat::Behavior[weaponId] == WeaponBehavior::AppearOnTargetUnit) {
    if (!unit->orderTarget.unit)
      return;

    getWeaponHitPos(unit, &x, &y);
  }

  else if (weapons_dat::Behavior[weaponId] == WeaponBehavior::AppearOnTargetSite) {
    x = unit->orderTarget.pt.x;
    y = unit->orderTarget.pt.y;
  }

  else {
    s32 forwardOffset = weapons_dat::ForwardOffset[weaponId];

    x = unit->getX() + scbw::getPolarX(forwardOffset, unit->currentDirection1);
    y = unit->getY() + scbw::getPolarY(forwardOffset, unit->currentDirection1)
        - weapons_dat::VerticalOffset[weaponId];
  }

  if (weapons_dat::FlingyId[weaponId] != 0)
    createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);

  switch(unit->id){
  case UnitId::sarah_kerrigan://KYSXD Smart Pistol - START
	  if (weaponId == unit->getGroundWeapon() || weaponId == units_dat::AirWeapon[unit->id]) {
			//Max targets
			const int mtargets = 2;
			//Angle vision - binary radian (11.25 grades = 8 binary radian, 45 grades = 32 brads)
			const u8 vision = 21;// vision / 2
			// nearesttarget definition (array)
			u8 maxWeaponRange = unit->getMaxWeaponRange(weaponId);
			CUnit* nearesttarget[mtargets];
			//Damage divisor for the target unit
			u8 damage_divisor = 1;
	   
			s32 X_Dir = unit->getX() + scbw::getPolarX(maxWeaponRange, unit->currentDirection1);
			s32 Y_Dir = unit->getY() + scbw::getPolarY(maxWeaponRange, unit->currentDirection1);
			s32 X_LimitL = unit->getX() + scbw::getPolarX(maxWeaponRange, unit->currentDirection1 - vision);
			s32 Y_LimitL = unit->getY() + scbw::getPolarY(maxWeaponRange, unit->currentDirection1 - vision);
			s32 X_LimitR = unit->getX() + scbw::getPolarX(maxWeaponRange, unit->currentDirection1 + vision);
			s32 Y_LimitR = unit->getY() + scbw::getPolarY(maxWeaponRange, unit->currentDirection1 + vision);

			int left = std::max(std::min(std::min(X_Dir, (s32)unit->getX()), std::min(X_LimitL, X_LimitR)), 0);
			int top = std::max(std::min(std::min(Y_Dir, (s32)unit->getY()), std::min(Y_LimitL, Y_LimitR)), 0);
			int right = std::max(std::max(X_Dir, (s32)unit->getX()), std::max(X_LimitL, X_LimitR));
			int bottom = std::max(std::max(Y_Dir, (s32)unit->getY()), std::max(Y_LimitL, Y_LimitR));

			for (int i = 0; i < mtargets; ++i) {
				//Define each new nearesttarget, if...
				nearesttarget[i] = scbw::UnitFinder::getNearestTarget(
				left, top, right, bottom, unit->orderTarget.unit, 
				[unit, vision, X_Dir, Y_Dir, X_LimitR, Y_LimitR, &nearesttarget, maxWeaponRange, i] (CUnit *tunit) -> bool {

				//Some values:
				s32 X_Target = unit->getX() + scbw::getPolarX(maxWeaponRange, scbw::getAngle(tunit->getX(), tunit->getY(), unit->getX(), unit->getY()));
				s32 Y_Target = unit->getY() + scbw::getPolarY(maxWeaponRange, scbw::getAngle(tunit->getX(), tunit->getY(), unit->getX(), unit->getY()));

				// ... and is in range...
				if (unit->getDistanceToTarget(tunit) <= maxWeaponRange
					// ... and is in vision...
					&& scbw::getDistanceFast(X_Dir, Y_Dir, X_Target, Y_Target) <= scbw::getDistanceFast(X_Dir, Y_Dir, X_LimitR, Y_LimitR)
					//Unit is alive...
					&& tunit->hitPoints > 0
					// ... and different from the order target...
					//&& tunit != unit->orderTarget.unit
					// ... and isn't hidden by the fog of war...
					&& tunit->sprite->isVisibleTo(unit->playerId)
					&& tunit->isVisibleTo(unit->playerId)
					// ... and isn't invisible...
					&& !(tunit->status & UnitStatus::Invincible)
					// ... and isn't an allied unit...
					&& !scbw::isAlliedTo(unit->playerId, tunit->playerId)) {
						for (int j = 0; j < i; ++j){
							if(nearesttarget[j] == tunit)
								return false;
						}
						return true;
				}
				else
					return false;
				});

				if(nearesttarget[i] == nullptr)
					break;
				else{
				CUnit* tmpUnit = unit->orderTarget.unit;
				unit->orderTarget.unit = nearesttarget[i];
				createBullet(weaponId, unit, nearesttarget[i]->getX(), nearesttarget[i]->getY(), unit->playerId, unit->currentDirection1);
				unit->orderTarget.unit = tmpUnit;
				}
			}
			//KYSXD Smart Pistol - END
		}
	  break;
  case UnitId::UnusedKhaydarinCrystalFormation://·¹ºñ¾ÆÅº ÅÍ·¿
	  if (!unit->airWeaponCooldown && unit->orderTarget.unit 
		&& !(units_dat::SizeType[unit->orderTarget.unit->id] == 1 
		|| (units_dat::SizeType[unit->orderTarget.unit->id] == 2 && units_dat::BaseProperty[unit->orderTarget.unit->id] & UnitProperty::Organic))){
		s32 forwardOffset = weapons_dat::ForwardOffset[74];
		x = unit->getX() + scbw::getPolarX(forwardOffset, unit->currentDirection1);
		y = unit->getY() + scbw::getPolarY(forwardOffset, unit->currentDirection1) - weapons_dat::VerticalOffset[weaponId];
		createBullet(74, unit, x, y, unit->playerId, unit->currentDirection1);
		createBullet(74, unit, x, y, unit->playerId, unit->currentDirection1);
		unit->airWeaponCooldown = weapons_dat::Cooldown[74];
	  }
	  break;
  }
   
} //hooks

}


//-------- Helper function definitions. Do NOT modify! --------//

namespace {

const u32 Helper_CreateBullet = 0x0048C260;
void createBullet(u8 weaponId, CUnit *source, s16 x, s16 y, u8 attackingPlayer, u8 direction) {
  u32 attackingPlayer_ = attackingPlayer, direction_ = direction;
  s32 x_ = x, y_ = y;

  __asm {
    PUSHAD
    PUSH direction_
    PUSH attackingPlayer_
    PUSH y_
    PUSH x_
    MOV EAX, source
    MOVZX ECX, weaponId
    CALL Helper_CreateBullet
    POPAD
  }
}

} //unnamed namespace
