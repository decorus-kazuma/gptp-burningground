#include <SCBW/api.h>
#include <SCBW/scbwdata.h>

//by TheBest(khkgn)
namespace hooks {

u8 getRightClickActionHook(const CUnit *unit) {

	u8 rightClkAct = 0;

	if (unit->id == UnitId::lurker 
		&& (unit->status & UnitStatus::Burrowed)){
		rightClkAct = 3;
		
		return rightClkAct;
	}
	
	using units_dat::GroupFlags;
	using units_dat::RightclickActionOrder;

	rightClkAct = RightclickActionOrder[unit->id];

	if (rightClkAct == 0){
		if ((unit->status & UnitStatus::GroundedBuilding) 
			&& GroupFlags[unit->id].isFactory){
			rightClkAct = 2;
		}
	}

	return rightClkAct;
}

bool setRightClickActionHook(const CUnit *unit){

	bool noRightClkAct = false;

	if(unit->status & UnitStatus::DoodadStatesThing)
			noRightClkAct = true;

	else if(unit->lockdownTimer 
		|| unit->stasisTimer 
		|| unit->maelstromTimer)
			noRightClkAct = true;

	else if (unit->status & UnitStatus::Burrowed 
		&& unit->id == UnitId::lurker)
			noRightClkAct = true;

	else if (unit->id == UnitId::scv
		|| unit->id == UnitId::drone){
		if (unit->mainOrderId == OrderId::ConstructingBuilding)
			noRightClkAct = true;
	}

	else if (unit->id == UnitId::ghost
		|| unit->id == UnitId::sarah_kerrigan
		|| unit->id == UnitId::alexei_stukov
		|| unit->id == UnitId::samir_duran
		|| unit->id == UnitId::Hero_InfestedDuran){
			if (unit->mainOrderId == OrderId::NukeTrack)
			noRightClkAct = true;
	}

	else if (unit->id == UnitId::archon){//ºíÀð°¡ ±î¸Ô°í ´ÙÄ­ µî·Ï ¾ÈÇÑ µí
		if (unit->mainOrderId == OrderId::CompletingArchonSummon)
			noRightClkAct = true;
	}


	return noRightClkAct;

}

bool getSCVBuild(const CUnit *unit){

	bool build = false;

	if((unit->id == UnitId::scv || unit->id == UnitId::drone)
		&& unit->mainOrderId == OrderId::ConstructingBuilding)
			build = true;

	return build;

}

}