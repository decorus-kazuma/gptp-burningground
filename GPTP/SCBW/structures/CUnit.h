//This header file contains member function declarations for CUnit.
//For the structure layout, see CUnitLayout.h

#pragma once
#include "CUnitLayout.h"
#include <SCBW/enumerations.h>
#include "../../graphics/graphics.h"

//Wrapper structure that provides methods for the CUnit structure.
struct CUnit: public CUnitLayout {

  /// @name Unit Stats and Properties
  //////////////////////////////////////////////////////////////// @{

  /// Checks if the unit is a clean detector (no Lockdown, Optical Flare, etc.)
  bool canDetect() const;
  
  /// Returns the amount of HP displayed in-game. This is equal to
  /// (this->hitPoints + 255) / 256.
  u32 getCurrentHpInGame() const;

  /// Returns the amount of shield points displayed in-game. This is equal to
  /// unit->shields / 256.
  u32 getCurrentShieldsInGame() const;

  /// Returns the amount of HP + shields displayed in game. If the unit does not
  /// have shields enabled, this returns the amount of HP displayed.
  u32 getCurrentLifeInGame() const;
  
  /// Returns the unit's ground weapon ID. If the unit is an unburrowed Lurker,
  /// returns WeaponId::None instead.
  u8 getGroundWeapon() const;

  /// Returns the unit's air weapon ID. This is identical to reading the value
  /// from units_dat::AirWeapon[unit->id].
  u8 getAirWeapon() const;
  
  /// Returns the total armor amount of this unit (with upgrades).
  /// To retrieve a unit's base armor amount, use units_dat::ArmorAmount[].
  u8 getArmor() const;

  /// Returns the bonus armor amount of this unit (from upgrades).
  /// To retrieve a unit's base armor amount, use units_dat::ArmorAmount[].
  u8 getArmorBonus() const;
  
  /// Returns the maximum energy amount of this unit (with upgrades).
  u16 getMaxEnergy() const;
  
  /// Returns the maximum range of a weapon in pixels. The weapon is assumed to
  /// be attached to this unit for calculating upgrade effects.
  /// This is affected by the Weapon Range hook module.
  u32 getMaxWeaponRange(u8 weaponId) const;

  bool isTargetWithinMinMovementRange(CUnit* target, u32 range) const;
  
  /// Retrieves the in-game name of this unit. In UMS maps, this returns custom
  /// names (if they exist).
  const char* getName() const;

  /// Retrieves the in-game name of the unit with @p unitId. In UMS maps, this
  /// returns custom unit names (if they exist).
  static const char* getName(u16 unitId);

  /// Returns the race of the current unit.
  /// @return   RaceId::Terran, Zerg, Protoss, or Neutral.
  RaceId::Enum getRace() const;
  
  /// Returns the race of the unit with @p unitId.
  /// @return   RaceId::Terran, Zerg, Protoss, or Neutral.
  static RaceId::Enum getRace(u16 unitId);
  
  /// Returns the unit's seek range (AKA target acquisition range) in matrices.
  /// This is affected by the Weapon Range hook module.
  u8 getSeekRange() const;
  
  /// Returns the sight range of this unit (with upgrades).
  /// If @p isForSpellCasting is true, also factors in status effects.
  u32 getSightRange(bool isForSpellCasting = false) const;
  
  /// Returns true if:
  /// * The unit has a weapon, or
  /// * The unit's subunit has a weapon, or
  /// * If the unit is a loaded Carrier/Reaver.
  bool hasWeapon() const;

  /// Checks if the unit is unpowered / lockdowned / stasised / maelstromed.
  bool isFrozen() const;

  /// Check if the unit is a remorphing building (i.e. is a Lair, Hive,
  /// Greater Spire, Sunken or Spore Colony under construction).
  bool isRemorphingBuilding() const;
  
  bool isConstructingAddon() const;
  
  bool isActiveTransport() const;
  
  /// Checks if the current unit is a subunit (i.e. not nullptr and has the
  /// UnitProperty::Subunit flag set).
  bool isSubunit() const;

  /// Checks if the unit is a spellcaster (has energy) and not a hallucination.
  bool isValidCaster() const;
  
  //////////////////////////////////////////////////////////////// @}

  /// @name Positions and Dimensions
  //////////////////////////////////////////////////////////////// @{

  /// Returns the X position of the unit's sprite on the map.
  u16 getX() const;
  /// Returns the Y position of the unit's sprite on the map.
  u16 getY() const;

  /// Returns the X position of the tile that the unit's sprite is on.
  u16 getTileX() const;
  /// Returns the Y position of the tile that the unit's sprite is on.
  u16 getTileY() const;

  /// Returns the position of the left edge of the unit's collision box.
  s16 getLeft() const;
  /// Returns the position of the right edge of the unit's collision box.
  s16 getRight() const;
  /// Returns the position of the top edge of the unit's collision box.
  s16 getTop() const;
  /// Returns the position of the bottom edge of the unit's collision box.
  s16 getBottom() const;

  s16 getLeftPlacement() const;
  s16 getRightPlacement() const;
  s16 getTopPlacement() const;
  s16 getBottomPlacement() const;

  //////////////////////////////////////////////////////////////// @}

  /// @name Modifying Unit State
  //////////////////////////////////////////////////////////////// @{

  /// Sets the unit's HP, and also updates burning / bleeding graphics and unit
  /// strength data (used by the AI). HP is guaranteed not to overflow.
  void setHp(s32 hitPoints);

  void setButtonSet(u16 buttonSet = -1);

  /// Deals damage to this unit, using a specific weapons.dat ID.
  void damageWith(u32 damage,               ///< Amount of damage dealt to this unit.
                  u8 weaponId,              ///< weapons.dat ID to use.
                  CUnit *attacker = nullptr,///< Attacking unit (for increasing kill count)
                  u8 attackingPlayer = -1,  ///< Attacking player (for increasing kill score)
                  s8 direction = 0,         ///< Attacked direction (for shield flicker overlays)
                  u8 damageDivisor = 1      ///< Damage divisor (for splash damage / glave wurm calculations)
                  );

  /// Deals damage directly to unit HP, killing it if possible.
  void damageHp(u32 damage, CUnit *attacker = nullptr,
                s32 attackingPlayer = -1, bool notify = true);

  /// Reduces Defensive Matrix by @p amount, removing it if possible.
  void reduceDefensiveMatrixHp(u32 amount);

  /// Immediately kills the unit. To silently remove the unit, 
  void remove();

  /// Removes the Lockdown effect from the unit.
  void removeLockdown();

  /// Removes the Maelstrom effect from the unit.
  void removeMaelstrom();
  
  /// Removes the Stasis Field effect from the unit.
  void removeStasisField();

  void restoreAllUnitStats();

  void removeDebuffEffect();

  //////////////////////////////////////////////////////////////// @}

  /// @name Unit Orders
  //////////////////////////////////////////////////////////////// @{

  /// Issue the @p order to the unit, using the given @p target unit.
  void orderTo(u8 orderId, const CUnit *target = nullptr);

  /// Issue the @p order to the unit, using the given position as the target.
  void orderTo(u8 orderId, u16 x, u16 y);

  /// Causes the unit to become idle.
  void orderToIdle();

  /// Issues a new order to the unit.
  void order(u8 orderId, u16 x, u16 y, const CUnit *target, u16 targetUnitId, bool stopPreviousOrders);

  /// Used by several hooks, details not completely understood.
  void setSecondaryOrder(u8 orderId);

  void setRetreatPoint(u8 direct);
  
  void resetUnitPowerup();
  void destroyPowerupImageOverlay();

  //////////////////////////////////////////////////////////////// @}
  
  /// @name Graphics and Animations
  /// Note: See CSprite::createOverlay() and CSprite::createTopOverlay() for
  ///       creating image overlays.
  //////////////////////////////////////////////////////////////// @{
  
  /// Returns the overlay image of this unit's sprite (or its subunit's sprite)
  /// that matches the @p imageId. If the image cannot be found, returns nullptr.
  CImage* getOverlay(u16 imageId) const;

  /// Makes the unit's sprite play the specified Iscript animation entry.
  void playIscriptAnim(IscriptAnimation::Enum animation);

  /// Removes the first image overlay with an ID value between @p imageIdStart
  /// and @p imageIdEnd.
  void removeOverlay(u32 imageIdStart, u32 imageIdEnd);

  /// Get the player (creator) color of current unit
  graphics::ColorId getColor();

  /// Removes the first image overlay with the given image ID.
  void removeOverlay(u32 imageId);
  
  void applyUnitEffects();

  //////////////////////////////////////////////////////////////// @}

  /// @name Distances and Terrain
  //////////////////////////////////////////////////////////////// @{
  
  /// Returns the distance between this unit and the @p target, taking unit
  /// collision size in units.dat into account.
  /// Internally, this function uses scbw::getDistanceFast().
  u32 getDistanceToTarget(const CUnit *target) const;

  Bool32 isDistanceGreaterThanHaltDistance(s32 x, s32 y, u32 distance) const;

  /// Checks whether this unit can reach the target position. This checks only
  /// for terrain, and does not consider obstacles (units and buildings).
  bool hasPathToPos(u32 x, u32 y) const;
  
  /// Checks whether this unit can reach the @p target unit. This checks only
  /// for terrain, and does not consider obstacles (units and buildings).
  bool hasPathToUnit(const CUnit *target) const;
  
  //////////////////////////////////////////////////////////////// @}

  /// @name DAT Requirements
  //////////////////////////////////////////////////////////////// @{
  
  /// Checks if the @p unit can build / train / morph into @p unitId. This checks:
  /// * If the unit is actually owned by the commanding @p playerId
  /// * Whether the unit is stunned / disabled.
  ///
  /// If the unit can build / train / morph, returns 1. If the tech needs to be
  /// researched, returns -1. If the unit cannot use the tech at all, returns 0.
  int canMakeUnit(u16 unitId, u8 playerId) const;

  /// Checks the following:
  ///  * If the unit is actually owned by the commanding @p playerId
  ///  * If the unit's owning player has the @p techId researched
  ///  * If the unit has enough energy (or energy cheat is enabled)
  ///  * If the unit is not stunned / is a hallucination / is being built
  ///  * If the unit meets the Use Tech Requirements (editable in FireGraft)
  ///
  /// If the unit can use the tech, returns 1. If the tech needs to be
  /// researched, returns -1. If the unit cannot use the tech at all, returns 0.
  int canUseTech(u8 techId, u8 playerId) const;
  
  //////////////////////////////////////////////////////////////// @}

  /// @name Utility Methods
  //////////////////////////////////////////////////////////////// @{
  
  /// Returns true if the current unit is not frozen and the @p target unit is
  /// not invincible and can be attacked or infested. If @p checkVisibility is
  /// set to false, the function will not check whether the unit can see the
  /// @p target (i.e. it is being detected properly). Additionally:
  ///
  /// * Queens (and Matriarchs) return true only if the @p target is an
  ///   infestable Command Center.
  /// * Reavers (and Warbringers) return true only if they have a ground path
  ///   to the @p target unit.
  /// * AI-controlled Arbiters (not including Danimoths) always return false.
  bool canAttackTarget(const CUnit* target, bool checkVisibility = true) const;

  /// Makes the unit use the specified weapon to attack its current target unit
  /// stored in the CUnit::orderTarget.unit member. This does not affect the
  /// unit's weapon cooldown. The spawned weapon sprite obeys the weapon
  /// behavior properties in weapons.dat.
  /// Note: This function is affected by the Fire Weapon hook module.
  void fireWeapon(u8 weaponId) const;

  /// Retrieves the unit pointer by @p index from the unit table (first unit is
  /// indexed at 1). If invalid, returns nullptr instead.
  static CUnit* getFromIndex(u16 index);

  /// Returns the index of this unit in the unit table. First unit == index 1.
  u16 getIndex() const;

  /// Returns the ID of the last player to own this unit. This is usually the
  /// same as CUnit::playerId, but if the unit belongs to a defeated player,
  /// this returns the correct player ID (instead of 11).
  u8 getLastOwnerId() const;

  /// Returns the loaded unit at @p index (value between 0-7). If no unit is
  /// loaded at the slot, returns nullptr instead.
  CUnit* getLoadedUnit(int index) const;

  /// Checks if this unit has other units loaded inside.
  bool hasLoadedUnit() const;
  
  /// Returns the first unit loaded in this unit. If this unit is not a
  /// transport, or has no units inside, this function returns nullptr instead.
  CUnit* getFirstLoadedUnit() const;
  
  /// Checks if the unit has an associated sprite and is not running the
  /// OrderId::Die order.
  bool isDead() const;

  /// Checks whether the @p target unit is an enemy of this unit.
  /// Internally, this calls scbw::isUnitEnemy().
  bool isTargetEnemy(const CUnit* target) const;

  /// Checks whether this unit can be seen by @playerId (i.e. not covered by the
  /// fog-of-war and is detectable).
  bool isVisibleTo(u8 playerId) const;
  
  void playCompleteEvent();

  /// Updates the unit's actual speed. This function should be called after
  /// changing any properties and status effects that affect movement speed.
  void updateSpeed();

  void updateUnitDamageOverlay();

  void MoveUnit(u16 x, u16 y);

  void refreshUnitVision();

  void refreshUnit();

  //////////////////////////////////////////////////////////////// @}
};

static_assert(sizeof(CUnit) == sizeof(CUnitLayout), "The size of the CUnit structure is invalid");
