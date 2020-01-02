#include "spellcasting.h"
#include "spells/spells.h"
#include <algorithm>

//-------- Helper function declarations. Do NOT modify! --------//
namespace {

bool canCastSpellOrder(const CUnit *unit, u8 techId, u8 orderId);
bool aiCastSpellOrder(CUnit *unit, CUnit *target, u8 orderId, u8 aiActionFlag = 1);
u16 getOrderEnergyCost(u8 orderId);
bool isNukeTimerReady(u8 playerId);
CUnit* getLoadedSilo(CUnit *ghost);

} //unnamed namespace

namespace AI {

//Attempts make the @p unit cast a spell.
bool AI_spellcasterHook(CUnit *unit, bool isUnitBeingAttacked) {
  if (!isUnitBeingAttacked
      && AIScriptController[unit->playerId].spellcasterTimer != 0)
    return false;

  switch (unit->id) {
	case UnitId::norad_ii:
	case UnitId::mutalisk:
	case UnitId::hyperion:
      //Yamato Gun
      if (canCastSpellOrder(unit, TechId::YamatoGun, OrderId::FireYamatoGun1)) {
        CUnit *target = findBestYamatoGunTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::FireYamatoGun1
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::FireYamatoGun1))
          return true;
      }

      break;

    case UnitId::ghost:
      //Lockdown
      if (canCastSpellOrder(unit, TechId::Lockdown, OrderId::MagnaPulse)) {
        CUnit *target = findBestLockdownTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::MagnaPulse
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::MagnaPulse))
          return true;
      }
	  
    case UnitId::sarah_kerrigan:
      //EMP Shockwave
      if (canCastSpellOrder(unit, TechId::EMPShockwave, OrderId::EmpShockwave)) {
        CUnit *target = findBestEmpShockwaveTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::EmpShockwave
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::EmpShockwave))
          return true;
      }
	  
      //Launch Nuclear Missile
      if (isNukeTimerReady(unit->playerId)) {
        CUnit *silo = getLoadedSilo(unit);
        if (!silo) return false;

        CUnit *target = findBestNukeLaunchTarget(unit, isUnitBeingAttacked);

        if (aiCastSpellOrder(unit, target, OrderId::NukePaint)) {
          silo->building.silo.nuke->connectedUnit = unit;
          AIScriptController[unit->playerId].AI_LastNukeTime = *elapsedTimeSeconds;
          return true;
        }
      }

      break;
	  
	case UnitId::battlecruiser:
      //Defensive Matrix
      if (canCastSpellOrder(unit, TechId::DefensiveMatrix, OrderId::DefensiveMatrix)) {
        //CUnit *target = findBestDefensiveMatrixTarget(unit, isUnitBeingAttacked);
		  if(unit->defensiveMatrixHp || unit->defensiveMatrixTimer 
			  || unit->hitPoints >= units_dat::MaxHitPoints[unit->id] || !isUnitBeingAttacked)
		  return false;
        
        if (unit->mainOrderId == OrderId::DefensiveMatrix)
          return false;

        if (aiCastSpellOrder(unit, unit, OrderId::DefensiveMatrix))
          return true;
      }
	  
	  break;

    case UnitId::science_vessel:
	case UnitId::magellan:
      //Irradiate
      if (canCastSpellOrder(unit, TechId::Irradiate, OrderId::Irradiate)) {
        CUnit *target = findBestIrradiateTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::Irradiate
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::Irradiate))
          return true;
      }

      break;

	case UnitId::kukulza_guardian:
      //250mm Strike Cannons
      if (unit->getMaxEnergy() == unit->energy
          && canCastSpellOrder(unit, TechId::OpticalFlare, OrderId::CastOpticalFlare)) {
        CUnit *target = findBestStrikeCannonsTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::CastOpticalFlare
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::CastOpticalFlare))
          return true;
      }
	  
	  break;

    case UnitId::medic:
      //Restoration
      if (canCastSpellOrder(unit, TechId::Restoration, OrderId::Restoration)) {
        CUnit *target = findBestRestorationTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::Restoration
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::Restoration))
          return true;

        if (unit == AIScriptController[unit->id].mainMedic
            && unit->mainOrderId != OrderId::Restoration)
        {
          CUnit *targetSituational = findBestRestorationTargetSituational(unit, isUnitBeingAttacked);
          if (aiCastSpellOrder(unit, targetSituational, OrderId::Restoration))
            return true;
        }
      }

      break;

    case UnitId::arcturus_mengsk:
      //200mm Tank Buster
      if (canCastSpellOrder(unit, TechId::Parasite, OrderId::CastParasite)) {
        CUnit *target = findBestTankBusterTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::CastParasite
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::CastParasite))
          return true;
      }

      break;
	  
    case UnitId::queen:
      //Parasite
      if (isUmsMode(unit->playerId) && scbw::random() % 256 == 1
          || unit->getMaxEnergy() == unit->energy)
      {
        if (canCastSpellOrder(unit, TechId::Parasite, OrderId::CastParasite)) {
          CUnit *target = findBestParasiteTarget(unit, isUnitBeingAttacked);
          
          if (unit->mainOrderId == OrderId::CastParasite
              && unit->orderTarget.unit == target)
            return false;

          if (aiCastSpellOrder(unit, target, OrderId::CastParasite, 4))
            return true;

          return false;
        }
        if (isUmsMode(unit->playerId))
          return false;
      }

      //Spawn Broodlings
      if (canCastSpellOrder(unit, TechId::SpawnBroodlings, OrderId::SummonBroodlings)) {
        CUnit *target = findBestSpawnBroodlingsTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::SummonBroodlings
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::SummonBroodlings)) {
          unit->order(OrderId::Move, unit->getX(), unit->getY(), nullptr, UnitId::None, false);
          return true;
        }
      }

      //Ensnare
      if ((isUmsMode(unit->playerId) || unit->getMaxEnergy() == unit->energy)
          && canCastSpellOrder(unit, TechId::Ensnare, OrderId::Ensnare))
      {
        CUnit *target = findBestEnsnareTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::Ensnare
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::Ensnare))
          return true;
      }

      break;

	  
    case UnitId::alexei_stukov:
      //Spawn Medibot
      if (canCastSpellOrder(unit, TechId::SpawnBroodlings, OrderId::SummonBroodlings)) {
        CUnit *target = findBestSpawnBroodlingsTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::SummonBroodlings
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::SummonBroodlings)) {
          unit->order(OrderId::Move, unit->getX(), unit->getY(), nullptr, UnitId::None, false);
          return true;
        }
      }

	  break;

    case UnitId::defiler:
	case UnitId::unclean_one:
      //Plague
      if (canCastSpellOrder(unit, TechId::Plague, OrderId::Plague)) {
        CUnit *target = findBestPlagueTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::Plague
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::Plague))
          return true;
      }

	  break;

	case UnitId::torrasque:
	case UnitId::tassadar_zeratul:
      //330mm Strike Cannons
      if (canCastSpellOrder(unit, TechId::DarkSwarm, OrderId::DarkSwarm)) {
        CUnit *target = findBest330mmStrikeCannonsTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::DarkSwarm
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::DarkSwarm))
          return true;
      }
      break;

	  /*
    case UnitId::high_templar:
      //Psionic Storm
      if (canCastSpellOrder(unit, TechId::PsionicStorm, OrderId::PsiStorm)) {
        CUnit *target = findBestPsiStormTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::PsiStorm
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::PsiStorm))
          return true;
      }

      //Hallucination
      if (unit->getMaxEnergy() == unit->energy
          && canCastSpellOrder(unit, TechId::Hallucination, OrderId::Hallucianation1)) {
        CUnit *target = findBestHallucinationTarget(unit, isUnitBeingAttacked);

        if (unit->mainOrderId == OrderId::Hallucianation1
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::Hallucianation1))
          return true;
      }

      break;
	  */

    case UnitId::arbiter:
      //Stasis Field
      if (canCastSpellOrder(unit, TechId::StasisField, OrderId::StasisField)) {
        CUnit *target = findBestStasisFieldTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::StasisField
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::StasisField))
          return true;
      }
      
      //Recall
      if (canCastSpellOrder(unit, TechId::Recall, OrderId::Teleport)) {
        CUnit *target = findBestRecallTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::Teleport
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::Teleport))
          return true;
      }

      break;
	  
    case UnitId::corsair:
      if (canCastSpellOrder(unit, TechId::MindControl, OrderId::CastMindControl) && !unit->unusedTimer) {

		  for (CBullet* bullet = *firstBullet; bullet; bullet = bullet->next) {
			  if(bullet->attackTarget.unit == unit && bullet->behaviourTypeInternal == WeaponBehavior::Fly_FollowTarget+1){
				  if(!scbw::isCheatEnabled(CheatFlags::TheGathering))
					  unit->energy -= techdata_dat::EnergyCost[TechId::MindControl]<<8;
					scbw::createSprite(134, unit->playerId, unit->getX(), unit->getY(), unit->sprite->elevationLevel+1);
					unit->unusedTimer = 16;

					return true;
			  }
		  }

          return false;
      }
		/*
    case UnitId::dark_archon:
      //Feedback
      if (canCastSpellOrder(unit, TechId::Feedback, OrderId::CastFeedback)) {
        CUnit *target = findBestFeedbackTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::CastFeedback
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::CastFeedback))
          return true;
      }
      
      //Mind Control
      if (canCastSpellOrder(unit, TechId::MindControl, OrderId::CastMindControl)) {
        CUnit *target = findBestMindControlTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::CastMindControl
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::CastMindControl))
          return true;
      }
      
      //Maelstrom
      energyReserve = getOrderEnergyCost(OrderId::CastMaelstrom)
                      + getOrderEnergyCost(OrderId::CastMindControl);
      energyReserve = std::min(energyReserve, unit->getMaxEnergy());

      if ((unit->energy >= energyReserve || scbw::isCheatEnabled(CheatFlags::TheGathering))
          && unit->canUseTech(TechId::Maelstrom, unit->playerId) != 1)
      {
        CUnit *target = findBestMaelstromTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::CastMaelstrom
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::CastMaelstrom))
          return true;
      }

      break;
	  */

	case UnitId::high_templar:
      //Disruption Web
      if (canCastSpellOrder(unit, TechId::DisruptionWeb, OrderId::CastDisruptionWeb)) {
        CUnit *target = findBestForceFieldTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::CastDisruptionWeb
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::CastDisruptionWeb))
          return true;
      }

      //Psionic Storm
      if (canCastSpellOrder(unit, TechId::PsionicStorm, OrderId::PsiStorm)) {
        CUnit *target = findBestPorcupineMissileTarget(unit, isUnitBeingAttacked);
        
        if (unit->mainOrderId == OrderId::PsiStorm
            && unit->orderTarget.unit == target)
          return false;
		
        if (unit->connectedUnit)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::PsiStorm))
          return true;
      }

      break;

	case UnitId::dragoon:
	  if(canCastSpellOrder(unit, TechId::Consume, OrderId::Consume)){
		if (unit->mainOrderId == OrderId::Consume)
		  return false;

		s32 randDistance = scbw::randBetween(80, 160);
		if(isUnitBeingAttacked && unit->shields < 128 && unit->hitPoints < units_dat::MaxHitPoints[unit->id]){
			ActiveTile actTile = scbw::getActiveTileAt(unit->getX() + scbw::getPolarX(randDistance,(unit->currentDirection1+128)%256), 
				unit->getY() + scbw::getPolarY(randDistance,(unit->currentDirection1+128)%256));
			if(!(actTile.exploredFlags & (1 << unit->playerId)) //탐색되지 않은 지형에다가 점멸할 수 없음.
				&& !actTile.isUnwalkable && !actTile.cliffEdge && !actTile.currentlyOccupied){//이동 불가 지형에다가 점멸할 수 없음.
				  unit->orderTo(OrderId::Consume, unit->getX() + scbw::getPolarX(randDistance,(unit->currentDirection1+128)%256), 
					unit->getY() + scbw::getPolarY(randDistance,(unit->currentDirection1+128)%256));

				return true;
			}
		}

		return false;
	  }

	  break;

	case UnitId::UnusedTerran1:
      //Ensnare
      if (unit->_unknown_0x066) {
        CUnit *target = findBestTyphoonTarget(unit);
        
        if (unit->mainOrderId == OrderId::Ensnare
            && unit->orderTarget.unit == target)
          return false;

        if (aiCastSpellOrder(unit, target, OrderId::Ensnare))
          return true;
      }

      break;
  }

  return false;
}

} //AI


//-------- Helper function definitions. Do NOT modify! --------//
namespace {

//Logically equivalent to function @ 0x004A11E0
bool canCastSpellOrder(const CUnit *unit, u8 techId, u8 orderId) {
  u16 energyCost = 0;
  if (orders_dat::TechUsed[orderId] < TechId::None)
    energyCost = techdata_dat::EnergyCost[orders_dat::TechUsed[orderId]] * 256;

  if (unit->energy >= energyCost || scbw::isCheatEnabled(CheatFlags::TheGathering))
    return unit->canUseTech(techId, unit->playerId) == 1;

  return false;
}

//Logically equivalent to function @ 0x004A1290
bool aiCastSpellOrder(CUnit *unit, CUnit *target, u8 orderId, u8 aiActionFlag) {
  if (!target || target->aiActionFlags & aiActionFlag)
    return false;

  if (unit->mainOrderId != orderId) {
    unit->orderTo(orderId, target);
    target->aiActionFlags |= aiActionFlag;
  }
  return true;
}

//Logically equivalent to function @ 0x0049E1C0
u16 getOrderEnergyCost(u8 orderId) {
  if (orders_dat::TechUsed[orderId] < TechId::None)
    return techdata_dat::EnergyCost[orders_dat::TechUsed[orderId]] * 256;
  else
    return 0;
}

//Logically equivalent to function @ 0x00446E50
bool isNukeTimerReady(u8 playerId) {
  return *elapsedTimeSeconds >= AIScriptController[playerId].AI_LastNukeTime + 60 * AIScriptController[playerId].AI_NukeRate;
}

//Based on function @ 0x00463360
CUnit* getLoadedSilo(CUnit *ghost) {
  for (CUnit *unit = firstPlayerUnit->unit[ghost->playerId];
       unit; unit = unit->player_link.next)
  {
    if (unit->id == UnitId::nuclear_silo && unit->building.silo.isReady) {
      CUnit *nuke = unit->building.silo.nuke;
      if (!nuke->connectedUnit
          || nuke->connectedUnit == ghost
          || nuke->connectedUnit->id == UnitId::nuclear_silo)
        return unit;
    }
  }
  return nullptr;
}

} //unnamed namespace
