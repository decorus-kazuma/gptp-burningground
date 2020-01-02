//Contains hooks that control how rally points are used.

#include "rally_point.h"
#include "../SCBW/scbwdata.h"
#include "../SCBW/enumerations.h"
#include "../SCBW/api.h"

/// Custom function:
///   Checks whether the @p resource unit can be harvested by \p playerId.
bool canBeHarvestedBy(const CUnit* resource, u8 playerId) {
  using units_dat::BaseProperty;

  if (resource && BaseProperty[resource->id] & UnitProperty::ResourceContainer) {

    //Mineral fields can be harvested by anyone
    if (UnitId::ResourceMineralField <= resource->id
		&& resource->id <= UnitId::UnusedCantina)
      return true;

    //Gas buildings can only be harvested if it is owned by the current player
    if (resource->status & UnitStatus::Completed
        && resource->playerId == playerId)
      return true;
  }

  return false;
}

namespace hooks {

/// Orders newly-produced units to rally, based upon the properties of the
/// building that produced it.
///
/// @param  unit      The unit that needs to receive rally orders.
/// @param  factory   The unit (building) that created the given unit.
void orderNewUnitToRally(CUnit* unit, CUnit* factory) {
  using units_dat::BaseProperty;

  CUnit* rallyTarget = factory->rally.unit;

  //Do nothing if the rally target is the factory itself or the rally target position is 0
  if (rallyTarget == factory || (!factory->rally.pt.x && !factory->rally.pt.y)) {
	  if(BaseProperty[unit->id] & UnitProperty::Worker)
		  unit->orderToIdle();
	  return;
  }

  //Following should be allowed only on friendly units
  if (rallyTarget) {
    //Enter transport
	  if (BaseProperty[unit->id] & UnitProperty::Worker && canBeHarvestedBy(rallyTarget, unit->playerId)){
		  unit->orderTo(OrderId::Harvest1, rallyTarget);
		  return;
	  }
	  if (scbw::canBeEnteredBy(rallyTarget, unit)) {
		  unit->orderTo(OrderId::EnterTransport, rallyTarget);
		  return;
	  }
	  
	  //Follow rally target
	  if (rallyTarget->playerId == unit->playerId) {
		  unit->orderTo(OrderId::Follow, rallyTarget);
		  return;
	  }
	  //Cannot use rallyTarget, so move on
  }

  unit->orderTo(OrderId::Move, factory->rally.pt.x, factory->rally.pt.y);
}

/// Called when the player sets the rally point on the ground.
void setRallyPosition(CUnit *unit, u16 x, u16 y) {
  //Default StarCraft behavior
  unit->rally.unit = NULL;
  unit->rally.pt.x = x;
  unit->rally.pt.y = y;
}

/// Called when the player sets the rally point on a unit.
void setRallyUnit(CUnit *unit, CUnit *target) {
  if (!target) target = unit;
  unit->rally.unit = target;
  unit->rally.pt.x = target->getX();
  unit->rally.pt.y = target->getY();
}

} //hooks
