#include "draw_Minimap_Unit_Box.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>

//by TheBest(khkgn)
namespace hooks {

//스타트 로케이션이나 맵 리빌러, 스캔 같은 유닛은 다른 방식으로 처리되나 봄
bool isDrawableUnit(const u16 unitId) {
	
	if(units_dat::BaseProperty[unitId] & UnitProperty::Subunit)
		return false;

	if (unitId >= 203 && unitId <= 213) // 203 = Floor Gun Trap, 213 = Right Wall Flame Trap 
		return false;

	switch(unitId){
	case UnitId::Spell_DarkSwarm:
	case UnitId::Spell_DisruptionWeb:
	case UnitId::UnusedZergMarker:
	case UnitId::UnusedProtossMarker:
	case UnitId::UnusedProtoss1:
		return false;
	}
  
  return true;
}

}