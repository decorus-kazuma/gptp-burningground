#include "draw_Minimap_Unit_Box.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>

//by TheBest(khkgn)
namespace hooks {

//��ŸƮ �����̼��̳� �� ������, ��ĵ ���� ������ �ٸ� ������� ó���ǳ� ��
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