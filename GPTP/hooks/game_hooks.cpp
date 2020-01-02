/// This is where the magic happens; program your plug-in's core behavior here.

#include "game_hooks.h"
#include <hooks\interface\BG.h>
#include <hook_tools.h>
#include <graphics/graphics.h>
#include <SCBW/api.h>
#include <SCBW/scbwdata.h>
#include <SCBW/UnitFinder.h>
#include "tech_target_check.h"
#include "weapon_damage.h"
#include "psi_field.h"
#include <time.h>
//using namespace std;

using namespace std;
using namespace scbw;
using namespace CheatFlags;

const u32 Func_MoveScreen = 0x0049C440;
void MoveScreen(u32 x, u32 y){
	__asm {
		PUSHAD
			MOV EAX, x
			MOV ECX, y
			CALL Func_MoveScreen
			POPAD
	}
}

const u32 Func_UpdateScreenPosition = 0x0049BFD0;
void updateScreenPosition(u32 x, u32 y){

	*moveToX = x;
	*moveToY = y;
	*moveToTileX = (u16)x>>5;
	*moveToTileY = (u16)y>>5;

	__asm {
		PUSHAD
			CALL Func_UpdateScreenPosition
			POPAD
	}
}

const u32 Func_MoveScreenToUnit = 0x004E6020;
void MoveScreenToUnit(CUnit *unit){
	__asm {
		PUSHAD
			MOV EAX, unit
			CALL Func_MoveScreenToUnit
			POPAD
	}
}

const u32 Func_minimapPing = 0x004A34C0;
void __stdcall minimapPing(u32 x, u32 y, s32 color){
	__asm{
		PUSHAD
			PUSH color
			PUSH y
			PUSH x
			CALL Func_minimapPing
			POPAD
	}
}

const u32 Func_updateButtonSet = 0x00458DE0;
void __stdcall updateButtonSet(){
	__asm{
		CALL Func_updateButtonSet
	}
}
//-------- draw!! --------//

inline void DrawRallyPoint(CUnit *selectedUnit) {

	u16 xdest, ydest;

	if(selectedUnit->rally.pt.x == 0 && selectedUnit->rally.pt.y == 0) {
		xdest = selectedUnit->position.x;
		ydest = selectedUnit->position.y;
	}
	else {
		xdest = selectedUnit->rally.pt.x;
		ydest = selectedUnit->rally.pt.y;
	}

	graphics::drawLine(
		selectedUnit->position.x,
		selectedUnit->position.y,
		xdest,
		ydest,
		selectedUnit->getColor(),
		graphics::CoordType::ON_MAP);

	graphics::drawFilledCircle(
		xdest,
		ydest,
		3,
		selectedUnit->getColor(),
		graphics::CoordType::ON_MAP);

}

inline void DrawTargetedOrder(CUnit *selectedUnit) {

#define GET_TARGET_WIDTH(x) (u8)x->sprite->mainGraphic->grpOffset->frames[x->sprite->mainGraphic->frameIndex].width
#define GET_TARGET_HEIGHT(x) (u8)x->sprite->mainGraphic->grpOffset->frames[x->sprite->mainGraphic->frameIndex].height

	if(selectedUnit->orderTarget.pt.x != 0 && selectedUnit->orderTarget.pt.y != 0) {	//Attack or Attack Move Order (I supposed, but not right)

		if(selectedUnit->orderTarget.unit != NULL) {

			graphics::drawDottedLine(
				selectedUnit->position.x,
				selectedUnit->position.y,
				selectedUnit->orderTarget.unit->position.x, 
				selectedUnit->orderTarget.unit->position.y,
				selectedUnit->getColor(),
				graphics::CoordType::ON_MAP
			);

			graphics::drawFilledCircle(
				selectedUnit->orderTarget.unit->sprite->mainGraphic->mapPosition.x,
				selectedUnit->orderTarget.unit->sprite->mainGraphic->mapPosition.y,
				4,
				selectedUnit->orderTarget.unit->getColor(),
				graphics::CoordType::ON_MAP
			);

		}
		else {

			graphics::drawDottedLine(
				selectedUnit->position.x,
				selectedUnit->position.y,
				selectedUnit->moveTarget.pt.x,
				selectedUnit->moveTarget.pt.y,
				selectedUnit->getColor(),
				graphics::CoordType::ON_MAP
				);

			graphics::drawFilledCircle(
				selectedUnit->orderTarget.pt.x - units_dat::UnitBounds[selectedUnit->id].left,
				selectedUnit->orderTarget.pt.y - units_dat::UnitBounds[selectedUnit->id].top,
				4,
				selectedUnit->orderTarget.unit->getColor(),
				graphics::CoordType::ON_MAP
			);

		}

	}
	else {																				//Move or Follow Order (I supposed, but not right

		if(selectedUnit->moveTarget.unit != NULL) {

			graphics::drawDottedLine(
				selectedUnit->position.x,
				selectedUnit->position.y,
				selectedUnit->moveTarget.unit->position.x,
				selectedUnit->moveTarget.unit->position.y,
				selectedUnit->getColor(),
				graphics::CoordType::ON_MAP
			);

			graphics::drawFilledCircle(
				selectedUnit->moveTarget.unit->sprite->mainGraphic->mapPosition.x,
				selectedUnit->moveTarget.unit->sprite->mainGraphic->mapPosition.y,
				4,
				selectedUnit->orderTarget.unit->getColor(),
				graphics::CoordType::ON_MAP
			);
		}
		else {

			graphics::drawDottedLine(
				selectedUnit->position.x,
				selectedUnit->position.y,
				selectedUnit->moveTarget.pt.x,
				selectedUnit->moveTarget.pt.y,
				selectedUnit->getColor(),
				graphics::CoordType::ON_MAP);

			graphics::drawFilledCircle(
				selectedUnit->moveTarget.pt.x - units_dat::UnitBounds[selectedUnit->id].left,
				selectedUnit->moveTarget.pt.y - units_dat::UnitBounds[selectedUnit->id].top,
				4,
				selectedUnit->orderTarget.unit->getColor(),
				graphics::CoordType::ON_MAP
			);
		}

	}

#undef GET_TARGET_WIDTH
#undef GET_TARGET_HEIGHT

}

inline void DrawQueuedOrders(CUnit *selectedUnit) {

	COrder* current_order = selectedUnit->orderQueueHead;

	if(current_order != NULL) {

		//SCV destination while constructing
		if(current_order->target.pt.x == 0 && current_order->target.pt.y == 0) {

			if (selectedUnit->orderQueueTail != NULL)
				graphics::drawDottedLine(
					selectedUnit->position.x,
					selectedUnit->position.y,
					selectedUnit->orderQueueTail->target.pt.x,
					selectedUnit->orderQueueTail->target.pt.y,
					selectedUnit->getColor(),
					graphics::CoordType::ON_MAP);

		}
		else {

			//line between first order destination and first queued order
			graphics::drawDottedLine(
				selectedUnit->moveTarget.pt.x,
				selectedUnit->moveTarget.pt.y,
				current_order->target.pt.x,
				current_order->target.pt.y,
				selectedUnit->getColor(),
				graphics::CoordType::ON_MAP);

			current_order = current_order->next;

			while(current_order != NULL && ( current_order->target.pt.x != 0 && current_order->target.pt.y != 0 ) ) {

				//line between previous order destination and current one
				graphics::drawDottedLine(
					current_order->prev->target.pt.x,
					current_order->prev->target.pt.y,
					current_order->target.pt.x,
					current_order->target.pt.y,
					selectedUnit->getColor(),
					graphics::CoordType::ON_MAP);

				current_order = current_order->next;

			}

		}

	}

}


//-------- Helper function declarations. Do NOT modify! ---------//

namespace {
	typedef void (__stdcall *GetWeaponHitPosFunc)(const CUnit *unit, s32 *x, s32 *y);
	GetWeaponHitPosFunc const getWeaponHitPos = (GetWeaponHitPosFunc) 0x004762C0;
	void createBullet(u8 weaponId, const CUnit *source, s16 x, s16 y, u8 attackingPlayer, u8 direction);
} //unnamed namespace

//화면 흔들림 효과 함수
void shockScreen(u8 *shock, u8 *power, s16 saveXY[]){
	u32 moveX, moveY;
	s8 randomX, randomY;

	//흔들림 마지막 단계에선 원위치로 돌림
	if(*shock == 1){
		updateScreenPosition((u32)std::max(std::min((int)*screenX - saveXY[0], (int)*maxX), 0), (u32)std::max(std::min((int)*screenY - saveXY[1], (int)*maxY), 0));
		saveXY[0] = saveXY[1] = 0;
		*power = 0;
	}else{
		srand(*elapsedTimeFrames);
		
		do{
			//랜덤 X좌표
			if(saveXY[0] == 0)
				randomX = rand()%((*power<<1)+1)-*power;//흔들림 범위 -*power~*power
			else{
				randomX = rand()%(*power+1);//흔들림 범위 0~*power
				if(saveXY[0] > 0)
					randomX -= *power;//흔들림 범위 -*power~0
			}

			//랜덤 Y좌표
			if(saveXY[1] == 0)
				randomY = rand()%((*power<<1)+1)-*power;//흔들림 범위 -*power~*power
			else{
				randomY = rand()%(*power+1);//흔들림 범위 0~*power
				if(saveXY[1] > 0)
					randomY -= *power;//흔들림 범위 -*power~0
			}
		}while(!randomX && !randomY);//랜덤 값이 X, Y 둘 다 0이면 다시 실행
		
		moveX = (u32)std::max(std::min((int)*screenX + randomX, (int)*maxX), 0);
		saveXY[0] += moveX-*screenX;

		moveY = (u32)std::max(std::min((int)*screenY + randomY, (int)*maxY), 0);
		saveXY[1] += moveY-*screenY;

		updateScreenPosition(moveX, moveY);
		*power = (u8)std::max(*power-1, 2);
	}

	*shock -= 1;

}

bool setFinalDestination(CUnit* unit, u16 X, u16 Y, u8 OrderId){
	const MapSize map = *mapTileSize;
	const s16 tdX = X - unit->getX();
	const s16 tdY = Y - unit->getY();

	if(tdX){
		if(tdY){
			float chkX, chkY;

			if(tdX > 0)
				chkX = ((map.width<<5) - unit->getX()) / (float)tdX;
			else
				chkX = unit->getX() / (float)abs((float)tdX);

			if(tdY > 0)
				chkY = ((map.height<<5) - unit->getY()) / (float)tdY;
			else
				chkY = unit->getY() / (float)abs(tdY);
							
			if(chkX < chkY){
				if(tdX > 0){
					if(tdY > 0)
						unit->orderTo(OrderId, map.width<<5, unit->getY() + (s16)floor(tdY*chkX+0.5));
					else
						unit->orderTo(OrderId, map.width<<5, unit->getY() + (s16)floor(tdY*chkX-0.5));
				}else{
					if(tdY > 0)
						unit->orderTo(OrderId, 0, unit->getY() + (s16)floor(tdY*chkX+0.5));
					else
						unit->orderTo(OrderId, 0, unit->getY() + (s16)floor(tdY*chkX-0.5));
				}
			}else{
				if(tdY > 0){
					if(tdX > 0)
						unit->orderTo(OrderId, unit->getX() + (s16)floor(tdX*chkY+0.5), map.height<<5);
					else
						unit->orderTo(OrderId, unit->getX() + (s16)floor(tdX*chkY-0.5), map.height<<5);
				}else{
					if(tdX > 0)
						unit->orderTo(OrderId, unit->getX() + (s16)floor(tdX*chkY+0.5), 0);
					else
						unit->orderTo(OrderId, unit->getX() + (s16)floor(tdX*chkY-0.5), 0);
				}
			}
		}else{
			if(tdX > 0)
				unit->orderTo(OrderId, map.width<<5, unit->getY());
			else
				unit->orderTo(OrderId, 0, unit->getY());
		}
	}else if(tdY){
		if(tdY > 0)
			unit->orderTo(OrderId, unit->getX(), map.height<<5);
		else
			unit->orderTo(OrderId, unit->getX(), 0);
	}else
		return false;

	return true;
}

void BomberAI(CUnit *unit){ //폭격기 플러그인
	const MapSize map = *mapTileSize;
	const u16 minVal = 48;
	const u32 xLim = (map.width<<5)-minVal;
	const u32 yLim = (map.height<<5)-minVal-32;
	const u16 bombRange = 256;
	const u8 bomberWeapon = unit->getGroundWeapon();
	const u32 bomberWeaponRange = unit->getMaxWeaponRange(bomberWeapon);

	switch(unit->_padding_0x132) {
	case 1://생성된 직후 상태 (unusedTimer가 0이 되기 전)
		if(unit->unusedTimer == 0){ //첫번째 폭격기 소환 후 8프레임이 지나면
			if(unit->building.upgradeLevel == 1){//첫번째 폭격기에만 1로 스위치되어있음
				//각도 계산
				const u8 B2Ang = (unit->currentDirection1+128) % 256;
				const u16 distanDiff = 64;

				CUnit *bomber2 = scbw::createUnitAtPos(80, unit->playerId, unit->getX()+scbw::getPolarX(distanDiff, B2Ang), unit->getY()+scbw::getPolarY(distanDiff,B2Ang));
				if(bomber2){
					bomber2->currentButtonSet = UnitId::None;
					bomber2->_padding_0x132 = 1;
					if(scbw::getDistanceFast(bomber2->getX(), bomber2->getY(), unit->secondaryOrderPos.x, unit->secondaryOrderPos.y) > distanDiff){
						bomber2->secondaryOrderPos.x = unit->secondaryOrderPos.x+scbw::getPolarX(distanDiff, B2Ang);
						bomber2->secondaryOrderPos.y = unit->secondaryOrderPos.y+scbw::getPolarY(distanDiff, B2Ang);
					}
					else{
						bomber2->secondaryOrderPos.x = unit->secondaryOrderPos.x-scbw::getPolarX(distanDiff, B2Ang);
						bomber2->secondaryOrderPos.y = unit->secondaryOrderPos.y-scbw::getPolarY(distanDiff, B2Ang);
					}
					if(!setFinalDestination(bomber2, bomber2->secondaryOrderPos.x, bomber2->secondaryOrderPos.y, OrderId::Move)){
						bomber2->userActionFlags |= 4;
						bomber2->remove();
						unit->_padding_0x132 = 2;
						return;
					}

					bomber2->unusedTimer = 1;
					bomber2->currentDirection1 = scbw::getAngle(bomber2->secondaryOrderPos.x, bomber2->secondaryOrderPos.y, bomber2->getX(), bomber2->getY());
					scbw::replaceSpriteImages(bomber2->sprite, bomber2->sprite->mainGraphic->id, bomber2->currentDirection1);
					bomber2->status |= UnitStatus::CanNotReceiveOrders;
				}
				unit->building.upgradeLevel = 0;
			}
			unit->_padding_0x132 = 2;
		}
		break;
	case 2://생성된 직후의 상태를 벗어난 후 (목표지점까지 비행하는 상태)
	if(scbw::getDistanceFast(unit->getX(), unit->getY(), unit->secondaryOrderPos.x, unit->secondaryOrderPos.y) <= bombRange){//폭격 시작 거리
		unit->_padding_0x132 = 3;
	}
		break;
	case 3://폭격 중
	if(scbw::getDistanceFast(unit->getX(), unit->getY(), unit->secondaryOrderPos.x, unit->secondaryOrderPos.y) > bombRange)//폭격 끝 거리 
		unit->_padding_0x132 = 4;
	else{
		if(unit->unusedTimer == 0){ //기총
			unit->unusedTimer = 1;
			createBullet(bomberWeapon, unit, unit->getX()+scbw::getPolarX(bomberWeaponRange, unit->currentDirection1), 
				unit->getY()+scbw::getPolarY(bomberWeaponRange, unit->currentDirection1), unit->playerId, unit->currentDirection1);
		}
		if(unit->spellCooldown == 0){ //기총프레임
			unit->playIscriptAnim(IscriptAnimation::Unused1);
			unit->spellCooldown = 10;
		}
	}
	//break 넣지 마셈
	case 4:
		if(unit->getX() < minVal || unit->getX() > xLim || unit->getY() < minVal || unit->getY() > yLim){
			unit->userActionFlags |= 4;
			unit->remove();
		}
	}
}

void AC130AI(CUnit *unit){
	if(scbw::getDistanceFast(unit->getX(), unit->getY(), unit->orderTarget.pt.x, unit->orderTarget.pt.y) < 32){
		if(unit->_unused_0x106 >= 253){
			unit->_unused_0x106 -= 253;
		}
		unit->_padding_0x132 = 3;
		unit->_unused_0x106 +=3;
	}
	if(unit->_padding_0x132 == 3){
		if(unit->orderSignal == 0 && 
			(unit->sprite->mainGraphic->animationEx != IscriptAnimation::Unused1 
			||unit->sprite->mainGraphic->animationEx != IscriptAnimation::Unused2
			||unit->sprite->mainGraphic->animationEx != IscriptAnimation::GndAttkToIdle
			||unit->sprite->mainGraphic->animationEx != IscriptAnimation::AirAttkToIdle)){
				if(unit->currentDirection1 > 128)
					unit->playIscriptAnim(IscriptAnimation::Unused2);
				else
					unit->playIscriptAnim(IscriptAnimation::Unused1);
		}
		if(unit->currentDirection1 >= 121 && unit->currentDirection1 < 134 && unit->airWeaponCooldown == 0){
			unit->sprite->mainGraphic->flags |= 0x0040;
			unit->subunit->sprite->mainGraphic->flags |= 0x0040;
			unit->sprite->getOverlay(52)->flags |= 0x0040;
			unit->sprite->createTopOverlay(542, 0, 0, 128);
			scbw::playFrame(unit->getOverlay(542), 118);
			unit->airWeaponCooldown = 13;
		}
		if(unit->currentDirection1 == 128){
			unit->playIscriptAnim(IscriptAnimation::GndAttkToIdle);
		}
		if((unit->currentDirection1 >= 248 || unit->currentDirection1 <= 4) && unit->airWeaponCooldown == 0){
			unit->sprite->mainGraphic->flags |= 0x0040;
			unit->subunit->sprite->mainGraphic->flags |= 0x0040;
			unit->sprite->getOverlay(52)->flags |= 0x0040;
			unit->sprite->createTopOverlay(542, 0, 0, 0);
			scbw::playFrame(unit->getOverlay(542), 102);
			unit->airWeaponCooldown = 13;
		}
		if(unit->currentDirection1 == 0){
			unit->playIscriptAnim(IscriptAnimation::AirAttkToIdle);
		}
		if((unit->currentDirection1 < 248 && unit->currentDirection1 >= 135) || (unit->currentDirection1 <= 120 && unit->currentDirection1 >= 7)){
			unit->sprite->mainGraphic->flags &= ~(0x0040);
			unit->subunit->sprite->mainGraphic->flags &= ~(0x0040);
			unit->sprite->getOverlay(52)->flags &= ~(0x0040);
			unit->sprite->removeOverlay(542);
		}
		unit->flingyAcceleration = unit->flingyTopSpeed;
		if(unit->groundWeaponCooldown == 0){
			if(unit->subunit){
				unit->subunit->status &= ~(UnitStatus::CanNotAttack);
				if(unit->subunit->orderTarget.unit){
					if(scbw::getDistanceFast(unit->subunit->orderTarget.pt.x, unit->subunit->orderTarget.pt.y, unit->subunit->getX(), unit->subunit->getY()) 
						> unit->getMaxWeaponRange(unit->subunit->getGroundWeapon())){
							unit->subunit->orderTo(OrderId::Stop, unit->getX(), unit->getY());
					}else{
					scbw::playSound(793, unit);
					createBullet(115, unit->subunit, unit->subunit->orderTarget.pt.x, unit->subunit->orderTarget.pt.y, unit->playerId, unit->subunit->currentDirection1);
					}
				}
			}
			unit->groundWeaponCooldown = 8;
			++unit->building.hatcheryHarvestValue.bottom;
		}
		if(unit->building.hatcheryHarvestValue.bottom > 180){
			if(unit->subunit){
				unit->subunit->status |= UnitStatus::CanNotAttack;
			}
			unit->_padding_0x132 = 4;
		}
	}
	if(unit->_padding_0x132 == 4){
		unit->sprite->mainGraphic->flags &= ~(0x0040);
		if(unit->orderSignal == 0 && 
			(unit->sprite->mainGraphic->animationEx != IscriptAnimation::WorkingToIdle 
			||unit->sprite->mainGraphic->animationEx != IscriptAnimation::IsWorking)){
				if(unit->currentDirection1 > 128)
					unit->playIscriptAnim(IscriptAnimation::WorkingToIdle);
				else
					unit->playIscriptAnim(IscriptAnimation::IsWorking);
				unit->orderSignal = 1;
		}
	}
	int fintarX = unit->secondaryOrderPos.x + scbw::getPolarX(192, unit->_unused_0x106);
	int fintarY = unit->secondaryOrderPos.y + scbw::getPolarY(192, unit->_unused_0x106);
	unit->orderTo(OrderId::Move,fintarX, fintarY);
}

void exploreMap(){ 
	for(int x = 0; x < mapTileSize->width; x++) { 
		for(int y = 0; y < mapTileSize->height; y++) { 
			ActiveTile *currentTile = &(*activeTileArray)[(x) + mapTileSize->width * (y)]; 
			currentTile->exploredFlags = 0; 					
		} 
	} 
	//if vespene geyser or a mineral field, reveal it to all players
	for (CUnit *unit = *firstVisibleUnit; unit; unit = unit->link.next) {
		if (unit->id == UnitId::ResourceVespeneGeyser || (unit->id >= UnitId::mineral_field_1 && unit->id <= UnitId::UnusedCantina)){
			unit->sprite->visibilityFlags=0xFF; 
		}
	}
}

//옵저버 플러그인 기능, Draw


//Defined in psi_field_util.cpp.
void removePsiField(CUnit *unit);

bool firstRun = true;

namespace hooks {

//수리가 가능한 유닛인지 확인
bool unitCanRepair(const CUnit *unit) {
  return unit->id == UnitId::scv
         || unit->id == UnitId::drone
         || unit->id == UnitId::science_vessel
         || unit->id == UnitId::magellan;
}

//수리가 되는 유닛인지 확인
bool unitCanBeRepaired(const CUnit *unit) {
	using units_dat::BaseProperty;
	using units_dat::GroupFlags;
	using units_dat::MaxHitPoints;

	if (unit->hitPoints < MaxHitPoints[unit->id] 
	 && BaseProperty[unit->id] & UnitProperty::Mechanical
	 && unit->getRace() == RaceId::Terran
	 && unit->status & UnitStatus::Completed)
		 return true;

  return false;
}

u16 getBuildingAddonType(const u16 unitId, bool *isComsat) {

	switch(unitId){
	case UnitId::barracks:
		return UnitId::Special_IonCannon;
	case UnitId::factory:
		return UnitId::machine_shop;
	case UnitId::starport:
		return UnitId::control_tower;
	case UnitId::spawning_pool:
		return UnitId::covert_ops;
	case UnitId::ultralisk_cavern:
		return UnitId::Special_IndependentStarport;
	case UnitId::UnusedIndependentCommandCenter:
		return UnitId::UnusedIndependentJumpGate;
	case UnitId::command_center:
		if(isComsat != nullptr)
			*isComsat = true;
		return UnitId::comsat_station;
	case 121:
	case UnitId::infested_command_center:
		if(isComsat != nullptr)
			*isComsat = true;
		return UnitId::Special_MatureChrysalis; 
	}

	return UnitId::None;
}

bool earthquakeEffectMode = true;
u32 shockCheck = 0;//흔들림의 근원지와 현재 화면의 거리를 저장하는 변수
u8 shock = 0;//흔들릴 횟수를 저장하는 변수
u8 power = 0;//흔들림의 강도를 저장하는 변수
s16 saveXY[2] = {0, 0};//흔들리는 동안 이동한 값을 저장하여 다음 흔들림의 방향을 결정하고, 흔들림 효과가 끝난 후 원래 자리로 복구시키기 위한 변수

//__43 스크립트 명령어가 실행되면 BG_inject.cpp에서 이 함수를 호출해서 발생지점의 좌표를 받아옴.
void setShock(unsigned char strength, unsigned short x, unsigned short y){
	if(!earthquakeEffectMode)
		return;
	
	shockCheck = scbw::getDistanceFast(x, y, *screenX+320, *screenY+200)>>8;
	if(shockCheck <= strength){
		const u8 result = (u8)std::max(std::min((int)(strength-shockCheck)*3, 127), 2);
		if(shock < result)
			shock = power = result;
	}
}

u8 debugMode = 0;
bool checkF11 = false, checkF12 = false;
u8 vik_G, vik_A, inq_S, inq_N, igo_O, igo_D;
u8 saveCDirection, saveVDirection;
char buffer[64]; //앞으로 출력할 텍스트 저장할 땐 char 배열 또 새로 선언하지 말고 여기다 쑤셔박자.(sprintf_s 쓸 때라던가 말이지) 부족하면 배열 늘리셈. 좋은데?
bool ifObs = false;
bool obMode = false;
bool checkF8 = false, checkF9 = false;
bool drawMove = false;

//옵저버모드
u8 beforeSelect = NULL, selectPlayerId = NULL;

u16 myunitEx = 0;

/// 이 훅은 매 프레임마다 불러옵니다.
bool nextFrame() {

	//디버그(딜레이 측정)
	int startClock = clock();

	//길게 누르면 쭉 적용되는 문제 때문에 check 변수 쓰는데 스타 내부에 같은 기능의 변수나 함수가 있으면 그걸로 연결해줘야 함.
	//디버그 모드 키인식
	if (*(VK_Array+VK_F12)){
		if(!checkF12){
			switch(debugMode){
			case 0:
				debugMode = 1;
				scbw::printText("Enable Debug Mode (1)", GameTextColor::Yellow);
				break;
			case 1:
				debugMode = 2;
				scbw::printText("Enable Debug Mode (2)", GameTextColor::Yellow);
				break;
			case 2:
				debugMode = 0;
				scbw::printText("Disable Debug Mode", GameTextColor::Yellow);
				break;
			}
			scbw::playSound(23);
			checkF12 = true;
		}
	}
	else
		checkF12 = false;
	
	//화면충격 모드 키인식
	if (*(VK_Array+VK_F11)){
		if(!checkF11){
			if(earthquakeEffectMode){
				earthquakeEffectMode = false;
				scbw::printText("Earthquake Effect Off", GameTextColor::Yellow);
			}
			else{
				earthquakeEffectMode = true;
				scbw::printText("Earthquake Effect On", GameTextColor::Yellow);
			}
			scbw::playSound(23);
			checkF11 = true;
		}
	}
	else
		checkF11 = false;

	//화면 흔들림
	if(shock){
		if(!earthquakeEffectMode && shock != 1)
			shock = 1;

		shockScreen(&shock, &power, saveXY);
	}

	using scbw::isCheatEnabled;
	using CheatFlags::OperationCwal;
	using CheatFlags::TheGathering;
	using CheatFlags::ModifyThePhaseVariance;
	using CheatFlags::FoodForThought;
	using scbw::getUpgradeLevel;
	using scbw::setTechResearchState;
	
	if (!scbw::isGamePaused()) { //만일 게임이 정지되지 않았다면
	scbw::setInGameLoopState(true); //Needed for scbw::random() to work
	graphics::resetAllGraphics();//draw된 텍스트나 서클 등을 제거함

	//0프레임
    if (*elapsedTimeFrames == 0) {
		for(CUnit* setting=*firstVisibleUnit;setting;setting=setting->link.next){
			if (setting->playerId == *LOCAL_HUMAN_ID) {						//현재 플레이어 소유의 유닛인지 확인
				if(myunitEx < 1 && setting->id != UnitId::Special_StartLocation) //옵저버 모드
					++myunitEx;
			}

			if(units_dat::MaxHitPoints[setting->id] > 42949632)
				setting->hitPoints = units_dat::MaxHitPoints[setting->id];

			if(setting->id == UnitId::raszagal){
				saveCDirection = setting->currentDirection1;
				saveVDirection = setting->velocityDirection1;
				scbw::changeUnitType(setting, UnitId::jim_raynor_marine);
				setting->currentDirection1 = saveCDirection;
				setting->velocityDirection1 = saveVDirection;
				setting->sprite->setDirectionAll(setting->currentDirection1);
			}
			else if(setting->id == UnitId::kukulza_mutalisk){
				saveCDirection = setting->currentDirection1;
				saveVDirection = setting->velocityDirection1;
				scbw::changeUnitType(setting, UnitId::unclean_one);
				setting->currentDirection1 = saveCDirection;
				setting->velocityDirection1 = saveVDirection;
				setting->sprite->setDirectionAll(setting->currentDirection1);
			}
			else if (179 <= setting->id && setting->id <= 181 && !setting->building.resource.resourceAmount)
				setting->building.resource.resourceAmount = 2000;
			else if(setting->id == UnitId::dark_templar_hero)
				setting->status |= UnitStatus::NoCollide;
		}

		if(*GAME_TYPE != GameType::UseMapSettings) { //not an UMS map
			exploreMap();
		}
    }//0프레임 종료

	//변수 초기화
	u16 idleWorkerCount = 0;
	bool canBuildtechBuilding[PLAYER_COUNT] = {true, true, true, true, true, true, true, true, true, true, true, true};
	bool vikingButtonChange = false, inquisitorButtonChange = false, igorButtonChange = false;
	vik_G = vik_A = inq_S = inq_N = igo_O = igo_D = 0;

	//선택되어 있는 유닛
	for (int i = 0, j = 0; j < *clientSelectionCount && i < SELECT_UNIT_COUNT; ++i) {
		CUnit *selUnit = localSelectionGroup[i];

		if(selUnit == nullptr)
			continue;

		++j;

		//옵저버 모드
		
		if(obMode == true){
			if(selUnit->playerId < PLAYER_COUNT){
				beforeSelect = selectPlayerId;
				selectPlayerId = selUnit->playerId;
			}else{
				selectPlayerId = 0;
				beforeSelect = 0;
			}
			//랠리포인트 유닛 선 만듬
			if(selUnit->rally.unit){
				if(selUnit->rally.unit != selUnit 
					&& !(selUnit->rally.pt.x == selUnit->position.x && selUnit->rally.pt.y == selUnit->position.y)
					&& !canMakePsiField(selUnit->id)){
					graphics::drawFilledCircle(selUnit->rally.unit->position.x, selUnit->rally.unit->position.y, 6, graphics::WHITE, graphics::ON_MAP);
					graphics::drawLine(selUnit->position.x,selUnit->position.y,selUnit->rally.unit->position.x, selUnit->rally.unit->position.y,graphics::WHITE,graphics::ON_MAP);
				}
			}
			else{
				if(selUnit->rally.pt.x 
					&& selUnit->rally.pt.y 
					&& !(selUnit->rally.pt.x == selUnit->position.x && selUnit->rally.pt.y == selUnit->position.y)
					&& !canMakePsiField(selUnit->id)){
					graphics::drawFilledCircle(selUnit->rally.pt.x, selUnit->rally.pt.y, 6, graphics::WHITE, graphics::ON_MAP);
					graphics::drawLine(selUnit->position.x,selUnit->position.y,selUnit->rally.pt.x, selUnit->rally.pt.y,graphics::WHITE,graphics::ON_MAP);
					}	
				}
			/*if(drawMove == true){
				//QOrder
				if(selUnit->status & UnitStatus::InAir)			//Terran Flying Buildings especially
					DrawTargetedOrder(selUnit);
				else if(selUnit->hasWeapon() || !units_dat::GroupFlags[selUnit->id].isBuilding)	//include armed building, exclude others buildings and include all other units
					DrawTargetedOrder(selUnit);

				DrawQueuedOrders(selUnit);
			}*/

			//내가 버닝 코드를 잘 몰라서 그냥 생각난 것만 넣음. 
			switch(selUnit->id){
				case UnitId::infested_kerrigan:
					if(selUnit->secondaryOrderPos.x || selUnit->secondaryOrderPos.y){
						graphics::drawCircle(selUnit->secondaryOrderPos.x,selUnit->secondaryOrderPos.y,BRONTES_ATTACKRADIUS,graphics::GREEN,graphics::ON_MAP);
					}
				break;
				case UnitId::alan_schezar_turret:
					graphics::drawCircle(selUnit->position.x,selUnit->position.y,747,graphics::GREEN,graphics::ON_MAP);
				break;
				case UnitId::gerard_dugalle:
					graphics::drawCircle(selUnit->position.x,selUnit->position.y,471,graphics::GREEN,graphics::ON_MAP);
				break;
				case UnitId::corsair:
					if(selUnit->unusedTimer){//플레어 지속시간이 16이므로 -16 했으니 지속시간 수정했으면 이 것도 수정하는게 보기 좋음
						graphics::drawFilledBox(selUnit->getX()-17, selUnit->getTop() - 6, selUnit->getX()-15 + selUnit->unusedTimer*2, selUnit->getTop() - 3, graphics::GREY, graphics::ON_MAP);
						graphics::drawFilledBox(selUnit->getX()-16, selUnit->getTop() - 5, selUnit->getX()-16 + selUnit->unusedTimer*2, selUnit->getTop() - 4, graphics::CYAN, graphics::ON_MAP);
					}
				break;
				case UnitId::Special_CrashedNoradII:
				case UnitId::Special_PsiDisrupter:
					if(selUnit->status & UnitStatus::Completed){
						sprintf_s(buffer, "\x04 Available Packages: %d \x01", selUnit->_unknown_0x066);
						graphics::drawText(selUnit->getX()-50, selUnit->getY()+60, buffer, graphics::FONT_MEDIUM, graphics::ON_MAP);
						sprintf_s(buffer, "\x1F(ETA: %d sec)", 240-(selUnit->_padding_0x132));
						graphics::drawText(selUnit->getX()-50, selUnit->getY()+70, buffer, graphics::FONT_MEDIUM, graphics::ON_MAP);
					}
				break;
				case UnitId::UnusedTerran1:
					if(selUnit->status & UnitStatus::Completed){
						sprintf_s(buffer, "\x04 Available Packages: %d \x01", selUnit->_unknown_0x066);
						graphics::drawText(selUnit->getX()-50, selUnit->getY()+48, buffer, graphics::FONT_MEDIUM, graphics::ON_MAP);
						sprintf_s(buffer, "\x1F(ETA: %d sec)", 240-(selUnit->_padding_0x132));
						graphics::drawText(selUnit->getX()-50, selUnit->getY()+58, buffer, graphics::FONT_MEDIUM, graphics::ON_MAP);
					}
				break;
				case UnitId::bunker:
				if(selUnit->removeTimer){//벙커 파괴시간 40이니 수정되면 여기 수치도 수정
					graphics::drawFilledBox(selUnit->getX()-41, selUnit->getTop() - 9, selUnit->getX()-39 + selUnit->removeTimer*2, selUnit->getTop() - 5, graphics::GREY, graphics::ON_MAP);
					graphics::drawFilledBox(selUnit->getX()-40, selUnit->getTop() - 8, selUnit->getX()-40 + selUnit->removeTimer*2, selUnit->getTop() - 6, graphics::YELLOW, graphics::ON_MAP);
				}
				break;
			case UnitId::high_templar:
				if(selUnit->building.silo.isReady){
					if(*clientSelectionCount == 1){
						const u16 Hwidth = 112;
						const u16 Hheight = 73;
						const s16 focus = 86;
						const s16 resetUnitY = selUnit->getY()+12;
						scbw::UnitFinder sUnit(selUnit->getX()-Hwidth, resetUnitY-Hheight, selUnit->getX()+Hwidth, resetUnitY+Hheight);
						if(sUnit.getUnitCount()){
							const u32 maxDistance = scbw::getDistanceFast(selUnit->getX()-focus, resetUnitY, selUnit->getX(), resetUnitY-Hheight)<<1;
							sUnit.forEach([&selUnit, maxDistance, resetUnitY, focus] (const CUnit *target) {
								if(target->playerId != selUnit->playerId)
									return;

								if(units_dat::BaseProperty[target->id] & UnitProperty::Building)
									return;

								if(target->status & UnitStatus::InAir)
									return;

								if(scbw::getDistanceFast(selUnit->getX()-focus, resetUnitY, target->getX(), target->getY()) 
									+ scbw::getDistanceFast(selUnit->getX()+focus, resetUnitY, target->getX(), target->getY()) > maxDistance)
									return;
							
								graphics::drawCircle(target->getX(), target->getY(), 4, graphics::CYAN, graphics::ON_MAP);
							});
						}
					}
				}

				if(selUnit->connectedUnit && *clientSelectionCount == 1){
					graphics::drawFilledCircle(selUnit->connectedUnit->getX(), selUnit->connectedUnit->getY(), 3, graphics::RED, graphics::ON_MAP);
					graphics::drawCircle(selUnit->connectedUnit->getX(), selUnit->connectedUnit->getY(), 5, graphics::YELLOW, graphics::ON_MAP);
					graphics::drawCircle(selUnit->connectedUnit->getX(), selUnit->connectedUnit->getY(), 7, graphics::WHITE, graphics::ON_MAP);
				}
			break;
			case UnitId::sarah_kerrigan:
				if(*clientSelectionCount == 1){
					const u16 mtargets = 2;
					const u8 vision = 21;// vision / 2
					u8 maxWeaponRange = selUnit->getMaxWeaponRange(selUnit->getGroundWeapon());

					s32 X_Dir = selUnit->getX() + scbw::getPolarX(maxWeaponRange, selUnit->currentDirection1);
					s32 Y_Dir = selUnit->getY() + scbw::getPolarY(maxWeaponRange, selUnit->currentDirection1);
					s32 X_LimitL = selUnit->getX() + scbw::getPolarX(maxWeaponRange, selUnit->currentDirection1 - vision);
					s32 Y_LimitL = selUnit->getY() + scbw::getPolarY(maxWeaponRange, selUnit->currentDirection1 - vision);
					s32 X_LimitR = selUnit->getX() + scbw::getPolarX(maxWeaponRange, selUnit->currentDirection1 + vision);
					s32 Y_LimitR = selUnit->getY() + scbw::getPolarY(maxWeaponRange, selUnit->currentDirection1 + vision);
						
					int left = std::max(std::min(std::min(X_Dir, (s32)selUnit->getX()), std::min(X_LimitL, X_LimitR)), 0);
					int top = std::max(std::min(std::min(Y_Dir, (s32)selUnit->getY()), std::min(Y_LimitL, Y_LimitR)), 0);
					int right = std::max(std::max(X_Dir, (s32)selUnit->getX()), std::max(X_LimitL, X_LimitR));
					int bottom = std::max(std::max(Y_Dir, (s32)selUnit->getY()), std::max(Y_LimitL, Y_LimitR));

					scbw::UnitFinder spTarget(left, top, right, bottom);
					spTarget.forEach([selUnit, maxWeaponRange, X_Dir, Y_Dir, X_LimitR, Y_LimitR, mtargets] (CUnit *spTarget) {
						s32 X_Target = selUnit->getX() + scbw::getPolarX(maxWeaponRange, scbw::getAngle(spTarget->getX(), spTarget->getY(), selUnit->getX(), selUnit->getY()));
						s32 Y_Target = selUnit->getY() + scbw::getPolarY(maxWeaponRange, scbw::getAngle(spTarget->getX(), spTarget->getY(), selUnit->getX(), selUnit->getY()));

						if (selUnit->getDistanceToTarget(spTarget) <= maxWeaponRange
							&& scbw::getDistanceFast(X_Dir, Y_Dir, X_Target, Y_Target) <= scbw::getDistanceFast(X_Dir, Y_Dir, X_LimitR, Y_LimitR)
							&& !(selUnit->orderTarget.unit && selUnit->orderTarget.unit == spTarget)
							&& spTarget->hitPoints > 0
							&& spTarget->sprite->isVisibleTo(selUnit->playerId)
							&& spTarget->isVisibleTo(selUnit->playerId)
							&& !(spTarget->status & UnitStatus::Invincible)
							&& !scbw::isAlliedTo(selUnit->playerId, spTarget->playerId))
							graphics::drawFilledCircle(spTarget->getX(), spTarget->getY(), 3, graphics::GREEN, graphics::ON_MAP);
					});
							
					if((selUnit->mainOrderId == OrderId::AttackUnit || selUnit->mainOrderId == OrderId::HoldPosition2) 
						&& selUnit->orderTarget.unit){
								
						graphics::drawFilledCircle(selUnit->orderTarget.unit->getX(), selUnit->orderTarget.unit->getY(), 3, graphics::RED, graphics::ON_MAP);
						graphics::drawCircle(selUnit->orderTarget.unit->getX(), selUnit->orderTarget.unit->getY(), 5, graphics::WHITE, graphics::ON_MAP);
						graphics::drawCircle(selUnit->orderTarget.unit->getX(), selUnit->orderTarget.unit->getY(), 7, graphics::WHITE, graphics::ON_MAP);

						CUnit* nearestTarget[mtargets];

						for (int i = 0; i < mtargets; ++i) {
							nearestTarget[i] = scbw::UnitFinder::getNearestTarget(left, top, right, bottom, selUnit->orderTarget.unit, 
								[selUnit, vision, X_Dir, Y_Dir, X_LimitR, Y_LimitR, &nearestTarget, maxWeaponRange, i] (CUnit *tunit) -> bool {

							s32 X_Target = selUnit->getX() + scbw::getPolarX(maxWeaponRange, scbw::getAngle(tunit->getX(), tunit->getY(), selUnit->getX(), selUnit->getY()));
							s32 Y_Target = selUnit->getY() + scbw::getPolarY(maxWeaponRange, scbw::getAngle(tunit->getX(), tunit->getY(), selUnit->getX(), selUnit->getY()));

							if (selUnit->getDistanceToTarget(tunit) <= maxWeaponRange
								&& scbw::getDistanceFast(X_Dir, Y_Dir, X_Target, Y_Target) <= scbw::getDistanceFast(X_Dir, Y_Dir, X_LimitR, Y_LimitR)
								&& tunit->hitPoints > 0
								&& tunit->sprite->isVisibleTo(selUnit->playerId)
								&& tunit->isVisibleTo(selUnit->playerId)
								&& !(tunit->status & UnitStatus::Invincible)
								&& !scbw::isAlliedTo(selUnit->playerId, tunit->playerId)) {
									for (int j = 0; j < i; ++j){
										if(nearestTarget[j] == tunit)
											return false;
									}
									return true;
							}
							else
								return false;
							});

							if(nearestTarget[i] == nullptr)
								break;
							else{
								graphics::drawFilledCircle(nearestTarget[i]->getX(), nearestTarget[i]->getY(), 2, graphics::YELLOW, graphics::ON_MAP);
								graphics::drawCircle(nearestTarget[i]->getX(), nearestTarget[i]->getY(), 5, graphics::WHITE, graphics::ON_MAP);
							}
						}
					}
				}
			break;
			}
		}
		if(selUnit->playerId == *LOCAL_HUMAN_ID){

			//selUnit,선택유닛 아이디
			switch(selUnit->id){
			case UnitId::infested_kerrigan:
				if(selUnit->secondaryOrderPos.x || selUnit->secondaryOrderPos.y){
					graphics::drawCircle(selUnit->secondaryOrderPos.x,selUnit->secondaryOrderPos.y,BRONTES_ATTACKRADIUS,graphics::GREEN,graphics::ON_MAP);
				}
			break;
			case UnitId::bunker:
				if(selUnit->removeTimer){//벙커 파괴시간 40이니 수정되면 여기 수치도 수정
					graphics::drawFilledBox(selUnit->getX()-41, selUnit->getTop() - 9, selUnit->getX()-39 + selUnit->removeTimer*2, selUnit->getTop() - 5, graphics::GREY, graphics::ON_MAP);
					graphics::drawFilledBox(selUnit->getX()-40, selUnit->getTop() - 8, selUnit->getX()-40 + selUnit->removeTimer*2, selUnit->getTop() - 6, graphics::YELLOW, graphics::ON_MAP);
				}
				break;
			case UnitId::high_templar:
				if(selUnit->building.silo.isReady){
					++inq_S;
					if(*clientSelectionCount == 1){
						const u16 Hwidth = 112;
						const u16 Hheight = 73;
						const s16 focus = 86;
						const s16 resetUnitY = selUnit->getY()+12;
						scbw::UnitFinder sUnit(selUnit->getX()-Hwidth, resetUnitY-Hheight, selUnit->getX()+Hwidth, resetUnitY+Hheight);
						if(sUnit.getUnitCount()){
							const u32 maxDistance = scbw::getDistanceFast(selUnit->getX()-focus, resetUnitY, selUnit->getX(), resetUnitY-Hheight)<<1;
							sUnit.forEach([&selUnit, maxDistance, resetUnitY, focus] (const CUnit *target) {
								if(target->playerId != selUnit->playerId)
									return;

								if(units_dat::BaseProperty[target->id] & UnitProperty::Building)
									return;

								if(target->status & UnitStatus::InAir)
									return;

								if(scbw::getDistanceFast(selUnit->getX()-focus, resetUnitY, target->getX(), target->getY()) 
									+ scbw::getDistanceFast(selUnit->getX()+focus, resetUnitY, target->getX(), target->getY()) > maxDistance)
									return;
							
								graphics::drawCircle(target->getX(), target->getY(), 4, graphics::CYAN, graphics::ON_MAP);
							});
						}
					}
				}
				else
					++inq_N;

				if(selUnit->connectedUnit && *clientSelectionCount == 1){
					graphics::drawFilledCircle(selUnit->connectedUnit->getX(), selUnit->connectedUnit->getY(), 3, graphics::RED, graphics::ON_MAP);
					graphics::drawCircle(selUnit->connectedUnit->getX(), selUnit->connectedUnit->getY(), 5, graphics::YELLOW, graphics::ON_MAP);
					graphics::drawCircle(selUnit->connectedUnit->getX(), selUnit->connectedUnit->getY(), 7, graphics::WHITE, graphics::ON_MAP);
				}
			break;
			case UnitId::sarah_kerrigan:
				if(*clientSelectionCount == 1){
					const u16 mtargets = 2;
					const u8 vision = 21;// vision / 2
					u8 maxWeaponRange = selUnit->getMaxWeaponRange(selUnit->getGroundWeapon());

					s32 X_Dir = selUnit->getX() + scbw::getPolarX(maxWeaponRange, selUnit->currentDirection1);
					s32 Y_Dir = selUnit->getY() + scbw::getPolarY(maxWeaponRange, selUnit->currentDirection1);
					s32 X_LimitL = selUnit->getX() + scbw::getPolarX(maxWeaponRange, selUnit->currentDirection1 - vision);
					s32 Y_LimitL = selUnit->getY() + scbw::getPolarY(maxWeaponRange, selUnit->currentDirection1 - vision);
					s32 X_LimitR = selUnit->getX() + scbw::getPolarX(maxWeaponRange, selUnit->currentDirection1 + vision);
					s32 Y_LimitR = selUnit->getY() + scbw::getPolarY(maxWeaponRange, selUnit->currentDirection1 + vision);
						
					int left = std::max(std::min(std::min(X_Dir, (s32)selUnit->getX()), std::min(X_LimitL, X_LimitR)), 0);
					int top = std::max(std::min(std::min(Y_Dir, (s32)selUnit->getY()), std::min(Y_LimitL, Y_LimitR)), 0);
					int right = std::max(std::max(X_Dir, (s32)selUnit->getX()), std::max(X_LimitL, X_LimitR));
					int bottom = std::max(std::max(Y_Dir, (s32)selUnit->getY()), std::max(Y_LimitL, Y_LimitR));

					scbw::UnitFinder spTarget(left, top, right, bottom);
					spTarget.forEach([selUnit, maxWeaponRange, X_Dir, Y_Dir, X_LimitR, Y_LimitR, mtargets] (CUnit *spTarget) {
						s32 X_Target = selUnit->getX() + scbw::getPolarX(maxWeaponRange, scbw::getAngle(spTarget->getX(), spTarget->getY(), selUnit->getX(), selUnit->getY()));
						s32 Y_Target = selUnit->getY() + scbw::getPolarY(maxWeaponRange, scbw::getAngle(spTarget->getX(), spTarget->getY(), selUnit->getX(), selUnit->getY()));

						if (selUnit->getDistanceToTarget(spTarget) <= maxWeaponRange
							&& scbw::getDistanceFast(X_Dir, Y_Dir, X_Target, Y_Target) <= scbw::getDistanceFast(X_Dir, Y_Dir, X_LimitR, Y_LimitR)
							&& !(selUnit->orderTarget.unit && selUnit->orderTarget.unit == spTarget)
							&& spTarget->hitPoints > 0
							&& spTarget->sprite->isVisibleTo(selUnit->playerId)
							&& spTarget->isVisibleTo(selUnit->playerId)
							&& !(spTarget->status & UnitStatus::Invincible)
							&& !scbw::isAlliedTo(selUnit->playerId, spTarget->playerId))
							graphics::drawFilledCircle(spTarget->getX(), spTarget->getY(), 3, graphics::GREEN, graphics::ON_MAP);
					});
							
					if((selUnit->mainOrderId == OrderId::AttackUnit || selUnit->mainOrderId == OrderId::HoldPosition2) 
						&& selUnit->orderTarget.unit){
								
						graphics::drawFilledCircle(selUnit->orderTarget.unit->getX(), selUnit->orderTarget.unit->getY(), 3, graphics::RED, graphics::ON_MAP);
						graphics::drawCircle(selUnit->orderTarget.unit->getX(), selUnit->orderTarget.unit->getY(), 5, graphics::WHITE, graphics::ON_MAP);
						graphics::drawCircle(selUnit->orderTarget.unit->getX(), selUnit->orderTarget.unit->getY(), 7, graphics::WHITE, graphics::ON_MAP);

						CUnit* nearestTarget[mtargets];

						for (int i = 0; i < mtargets; ++i) {
							nearestTarget[i] = scbw::UnitFinder::getNearestTarget(left, top, right, bottom, selUnit->orderTarget.unit, 
								[selUnit, vision, X_Dir, Y_Dir, X_LimitR, Y_LimitR, &nearestTarget, maxWeaponRange, i] (CUnit *tunit) -> bool {

							s32 X_Target = selUnit->getX() + scbw::getPolarX(maxWeaponRange, scbw::getAngle(tunit->getX(), tunit->getY(), selUnit->getX(), selUnit->getY()));
							s32 Y_Target = selUnit->getY() + scbw::getPolarY(maxWeaponRange, scbw::getAngle(tunit->getX(), tunit->getY(), selUnit->getX(), selUnit->getY()));

							if (selUnit->getDistanceToTarget(tunit) <= maxWeaponRange
								&& scbw::getDistanceFast(X_Dir, Y_Dir, X_Target, Y_Target) <= scbw::getDistanceFast(X_Dir, Y_Dir, X_LimitR, Y_LimitR)
								&& tunit->hitPoints > 0
								&& tunit->sprite->isVisibleTo(selUnit->playerId)
								&& tunit->isVisibleTo(selUnit->playerId)
								&& !(tunit->status & UnitStatus::Invincible)
								&& !scbw::isAlliedTo(selUnit->playerId, tunit->playerId)) {
									for (int j = 0; j < i; ++j){
										if(nearestTarget[j] == tunit)
											return false;
									}
									return true;
							}
							else
								return false;
							});

							if(nearestTarget[i] == nullptr)
								break;
							else{
								graphics::drawFilledCircle(nearestTarget[i]->getX(), nearestTarget[i]->getY(), 2, graphics::YELLOW, graphics::ON_MAP);
								graphics::drawCircle(nearestTarget[i]->getX(), nearestTarget[i]->getY(), 5, graphics::WHITE, graphics::ON_MAP);
							}
						}
					}
				}
			break;
			case UnitId::edmund_duke://바이킹(지상)
				++vik_G;
			break;
			case UnitId::edmund_duke_s://바이킹(공중)
				++vik_A;
			break;
			case UnitId::tassadar://이고르(공격)
				++igo_O;
			break;
			case UnitId::aldaris://이고르(방어)
				++igo_D;
			break;
			case UnitId::siege_tank_s:
				graphics::drawCircle(selUnit->position.x, selUnit->position.y,423,graphics::GREEN,graphics::ON_MAP);
			break;
			case UnitId::alan_schezar_turret:
				graphics::drawCircle(selUnit->position.x,selUnit->position.y,747,graphics::GREEN,graphics::ON_MAP);
			break;
			case UnitId::gerard_dugalle:
				graphics::drawCircle(selUnit->position.x,selUnit->position.y,471,graphics::GREEN,graphics::ON_MAP);
			break;
			case UnitId::corsair:
				if(selUnit->unusedTimer){//플레어 지속시간이 16이므로 -16 했으니 지속시간 수정했으면 이 것도 수정하는게 보기 좋음
					graphics::drawFilledBox(selUnit->getX()-17, selUnit->getTop() - 6, selUnit->getX()-15 + selUnit->unusedTimer*2, selUnit->getTop() - 3, graphics::GREY, graphics::ON_MAP);
					graphics::drawFilledBox(selUnit->getX()-16, selUnit->getTop() - 5, selUnit->getX()-16 + selUnit->unusedTimer*2, selUnit->getTop() - 4, graphics::CYAN, graphics::ON_MAP);
				}
			break;
			case UnitId::Special_CrashedNoradII:
			case UnitId::Special_PsiDisrupter:
				if(selUnit->status & UnitStatus::Completed){
					sprintf_s(buffer, "\x04 Available Packages: %d \x01", selUnit->_unknown_0x066);
					graphics::drawText(selUnit->getX()-50, selUnit->getY()+60, buffer, graphics::FONT_MEDIUM, graphics::ON_MAP);
					sprintf_s(buffer, "\x1F(ETA: %d sec)", 240-(selUnit->_padding_0x132));
					graphics::drawText(selUnit->getX()-50, selUnit->getY()+70, buffer, graphics::FONT_MEDIUM, graphics::ON_MAP);
				}
			break;
			case UnitId::UnusedTerran1:
				if(selUnit->status & UnitStatus::Completed){
					sprintf_s(buffer, "\x04 Available Packages: %d \x01", selUnit->_unknown_0x066);
					graphics::drawText(selUnit->getX()-50, selUnit->getY()+48, buffer, graphics::FONT_MEDIUM, graphics::ON_MAP);
					sprintf_s(buffer, "\x1F(ETA: %d sec)", 240-(selUnit->_padding_0x132));
					graphics::drawText(selUnit->getX()-50, selUnit->getY()+58, buffer, graphics::FONT_MEDIUM, graphics::ON_MAP);
				}
			break;
			}
			
			//랠리포인트 유닛 선 만듬
			if(selUnit->rally.unit){
				if(selUnit->rally.unit != selUnit 
					&& !(selUnit->rally.pt.x == selUnit->position.x && selUnit->rally.pt.y == selUnit->position.y)
					&& !canMakePsiField(selUnit->id)){
					graphics::drawFilledCircle(selUnit->rally.unit->position.x, selUnit->rally.unit->position.y, 6, graphics::WHITE, graphics::ON_MAP);
					graphics::drawLine(selUnit->position.x,selUnit->position.y,selUnit->rally.unit->position.x, selUnit->rally.unit->position.y,graphics::WHITE,graphics::ON_MAP);
				}
			}
			else{
				if(selUnit->rally.pt.x 
					&& selUnit->rally.pt.y 
					&& !(selUnit->rally.pt.x == selUnit->position.x && selUnit->rally.pt.y == selUnit->position.y)
					&& !canMakePsiField(selUnit->id)){
					graphics::drawFilledCircle(selUnit->rally.pt.x, selUnit->rally.pt.y, 6, graphics::WHITE, graphics::ON_MAP);
					graphics::drawLine(selUnit->position.x,selUnit->position.y,selUnit->rally.pt.x, selUnit->rally.pt.y,graphics::WHITE,graphics::ON_MAP);
					}	
				}
		}
		else if(selUnit->playerId != 11){
			if(scbw::isAlliedTo(*LOCAL_HUMAN_ID, selUnit->playerId)){
				switch(selUnit->id){
					case UnitId::infested_kerrigan:
					if(selUnit->secondaryOrderPos.x || selUnit->secondaryOrderPos.y){
						graphics::drawCircle(selUnit->secondaryOrderPos.x,selUnit->secondaryOrderPos.y,BRONTES_ATTACKRADIUS,graphics::RED,graphics::ON_MAP);
					}
					break;
					case UnitId::siege_tank_s:
						graphics::drawCircle(selUnit->position.x,selUnit->position.y,423,graphics::YELLOW,graphics::ON_MAP);
					break;
					case UnitId::alan_schezar_turret:
						graphics::drawCircle(selUnit->position.x,selUnit->position.y,747,graphics::YELLOW,graphics::ON_MAP);
					break;
					case UnitId::gerard_dugalle:
						graphics::drawCircle(selUnit->position.x,selUnit->position.y,471,graphics::YELLOW,graphics::ON_MAP);
					break;
				}
			}
			else{
				switch(selUnit->id){
					case UnitId::infested_kerrigan:
					if(selUnit->secondaryOrderPos.x || selUnit->secondaryOrderPos.y){
						graphics::drawCircle(selUnit->secondaryOrderPos.x,selUnit->secondaryOrderPos.y,BRONTES_ATTACKRADIUS,graphics::RED,graphics::ON_MAP);
					}
					break;
					case UnitId::siege_tank_s:
						graphics::drawCircle(selUnit->position.x,selUnit->position.y,423,graphics::RED,graphics::ON_MAP);
					break;
					case UnitId::alan_schezar_turret:
						graphics::drawCircle(selUnit->position.x,selUnit->position.y,747,graphics::RED,graphics::ON_MAP);
					break;
					case UnitId::gerard_dugalle:
						graphics::drawCircle(selUnit->position.x,selUnit->position.y,471,graphics::RED,graphics::ON_MAP);
					break;
				}
			}
		}
	} //선택유닛 종료

	if (*clientSelectionCount && vik_G+vik_A == *clientSelectionCount)
		vikingButtonChange = true;
	else if (*clientSelectionCount && inq_S+inq_N == *clientSelectionCount)
		inquisitorButtonChange = true;
	else if (*clientSelectionCount && igo_O+igo_D == *clientSelectionCount)
		igorButtonChange = true;

	//유닛 루프문 시작
    for (CUnit *unit = *firstVisibleUnit; unit; unit = unit->link.next) {
		
		//쉬는 일꾼 검사 (FOKKIA/ pastelmind) && 옵저버 모드때 사용할놈
		if (!(*IS_IN_REPLAY)) {   //리플레이 시청 모드에서는 안 씀
			
			if (unit->playerId == *LOCAL_HUMAN_ID) {						//현재 플레이어 소유의 유닛인지 확인
				if(units_dat::BaseProperty[unit->id] & UnitProperty::Worker 
					&& unit->mainOrderId == OrderId::PlayerGuard)           //놀고 있는 상태인지 확인
					++idleWorkerCount;


				//버튼셋 교체코드
				//바이킹
				if(vikingButtonChange){
					switch(unit->id){
					case UnitId::edmund_duke://바이킹(지상모드)
					case UnitId::edmund_duke_s://바이킹(공중모드)
						if(unit->currentButtonSet != 228){//락다운 상태같이 버튼이 없는 상태에선 적용안함
							if(vik_A){//공중모드 상태인 바이킹이 포함되어 있는가
								if(vik_G)//지상모드 상태인 바이킹도 포함되어 있는가
									unit->currentButtonSet = unit->id;
								else
									unit->currentButtonSet = 26;
							}else
								unit->currentButtonSet = 24;
						}
					}
				}
				else if(inquisitorButtonChange){
					if(unit->id == UnitId::high_templar){
						if(unit->currentButtonSet != 228){// 버튼이 없는 상태에선 적용안함
							if(inq_S){//쉴드 킨 상태인 인퀴지터가 포함되어 있는가
								if(inq_N)//쉴드 안 킨 상태인 바이킹도 포함되어 있는가
									unit->currentButtonSet = 158;
								else
									unit->currentButtonSet = 161;
							}else
								unit->currentButtonSet = UnitId::high_templar;
						}
					}
				}
				else if(igorButtonChange){
					switch(unit->id){
					case UnitId::tassadar://바이킹(지상모드)
					case UnitId::aldaris://바이킹(공중모드)
						if(unit->currentButtonSet != 228){//락다운 상태같이 버튼이 없는 상태에선 적용안함
							if(igo_D && igo_O)//방어모드 상태와 공격모드 상태인 이고르가 같이 포함되어 있는가
								unit->currentButtonSet = 78;
							else
								unit->currentButtonSet = unit->id;
						}
					}
				}
			}

		}
		
	if(canBuildtechBuilding[unit->playerId] == true){
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker
			&& (unit->mainOrderId == OrderId::BuildTerran || unit->mainOrderId == OrderId::BuildProtoss1) 
			&& (unit->buildQueue[unit->buildQueueSlot] == UnitId::Special_CrashedNoradII 
			|| unit->buildQueue[unit->buildQueueSlot] == UnitId::Special_PsiDisrupter 
			|| unit->buildQueue[unit->buildQueueSlot] == UnitId::UnusedTerran1)
			&& !(isCheatEnabled(ModifyThePhaseVariance)))
			canBuildtechBuilding[unit->playerId] = false;
		else if ((unit->id == UnitId::Special_CrashedNoradII || unit->id == UnitId::Special_PsiDisrupter || unit->id == UnitId::UnusedTerran1) 
			&& !(isCheatEnabled(ModifyThePhaseVariance)))
			canBuildtechBuilding[unit->playerId] = false;
	}

	//자동 수리 기능
	//scv가 unit->connectedUnit 메모리를 쓰지 않는다는 가정하에 작성됨.
	if (unitCanRepair(unit)) {
		switch(unit->mainOrderId) {
		case OrderId::ReaverStop: 
			if(unit->_unused_0x106){
				unit->_unused_0x106 = 0; 
				unit->removeOverlay(976);
			}
			else{
				unit->_unused_0x106 = 1;
				unit->sprite->createOverlay(976);
			}

			if(unit->connectedUnit){
				unit->mainOrderId = OrderId::Follow;
				unit->orderTarget.unit = unit->connectedUnit;
			}
			else
				unit->orderToIdle();
		break;

		case OrderId::Follow:
			if (unit->connectedUnit != unit->orderTarget.unit)
				unit->connectedUnit = unit->orderTarget.unit;
		break;

		case OrderId::Repair1:
		break;

		default:
			if (unit->connectedUnit)
				unit->connectedUnit = nullptr;
		}

        //치료 대상을 탐색
        if (unit->_unused_0x106 
			&& unit->mainOrderId != OrderId::Repair1 
			&& !unit->isFrozen()
			&& !(unit->status & (UnitStatus::CanNotReceiveOrders | UnitStatus::NoBrkCodeStart | UnitStatus::Unmovable))) {
				switch(unit->mainOrderId) {
				case OrderId::Follow:
					//따라다니는 대상이 있다면 우선적으로 치료한다
				   if(unit->orderTarget.unit && unitCanBeRepaired(unit->orderTarget.unit)){
					   unit->orderTo(OrderId::Repair1, unit->orderTarget.unit);
					   break;
				   }//밑에 브레이크 넣지 마셈
				case OrderId::PlayerGuard:
				case OrderId::AttackMove:
					//자동 수리 최대 거리는 176
					CUnit* bestRepairTarget = scbw::UnitFinder::getNearestTarget(unit->getX() - 176, unit->getY() - 176, unit->getX() + 176, unit->getY() + 176, unit, 
						[&unit] (const CUnit *target) -> bool {//타겟이 본인인지 아닌지 검사하는 것은 유닛파인더 함수 안에 있으니까 넣을 필요 없음.
						if (unit->playerId != target->playerId) // 아군인가
							return false;
					
						if (!unitCanBeRepaired(target)) // 타겟이 수리가능한가
							return false;

						if ((units_dat::MineralCost[target->id] && !resources->minerals[unit->playerId]) // 유닛 수리에 필요한 자원이 부족한가
							|| (units_dat::GasCost[target->id] && !resources->gas[unit->playerId]))
							return false;

						return true;
					});
				
					if (bestRepairTarget != nullptr)
						unit->orderTo(OrderId::Repair1, bestRepairTarget);

					break;
				}
		}
	}
	 
	/*
	if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker){  //일꾼인지 확인
		if(unit->worker.powerup){ //자원 들고 있는지 확인
			switch(unit->worker.powerup->id){
			case UnitId::Powerup_MineralClusterType1:
				resources->minerals[unit->playerId] += unit->worker.powerup->_padding_0x132;
				unit->worker.powerup->remove();
				unit->worker.powerup = nullptr;
				unit->resourceType = NULL;
				unit->destroyPowerupImageOverlay();
				break;
			case UnitId::Powerup_TerranGasTankType1:
			case UnitId::Powerup_ProtossGasOrbType1:
				resources->gas[unit->playerId] += unit->worker.powerup->_padding_0x132;
				unit->worker.powerup->remove();
				unit->worker.powerup = nullptr;
				unit->resourceType = NULL;
				unit->destroyPowerupImageOverlay();
				break;
			}
		}
	}
	*/

	/*
	if(unit->id)를 switch로 변경함.
	앞으로 이곳에 추가바람.
	*/
	switch(unit->id){

		//뉴크 사일로에 핵이 장전되면 그래픽 변경
		case UnitId::nuclear_silo:
			if(unit->status & UnitStatus::Completed) {
				CImage* unitGraphic = unit->sprite->mainGraphic;
				if (unit->building.silo.isReady && unitGraphic->frameSet == 0)
				  scbw::playFrame(unitGraphic, 2);
				else if (!unit->building.silo.isReady && unitGraphic->frameSet == 2)
				  scbw::playFrame(unitGraphic, 0);
			}
		break;

		//노라드2와 테사다르? 뭔지 모르겠는데 상태이상에 대해 무적으로 만듬
		//hooks/update_status_effects.cpp 참고하시랍니다
		//기타 코드
		case UnitId::tassadar_zeratul:
			if(unit->mainOrderId == OrderId::DarkSwarm && unit->orderTarget.unit)
				unit->orderTarget.unit = nullptr;
		break;

		case UnitId::zeratul:
			if(!unit->groundWeaponCooldown && unit->mainOrderId != OrderId::AttackUnit){
				const u32 range = unit->getMaxWeaponRange(unit->getGroundWeapon());
				CUnit* target;

				target = scbw::UnitFinder::getNearestTarget(
					unit->getX() - range, unit->getY() - range,
					unit->getX() + range, unit->getY() + range,
					unit, [&unit, range] (const CUnit *target) -> bool {
						if (scbw::isAlliedTo(unit->playerId, target->getLastOwnerId()))
							return false;
						
						if (unit->getDistanceToTarget(target) > range)
							return false;

						if (!unit->canAttackTarget(target))
							return false;

						const u8 angleCheck = unit->currentDirection1 - (u8)scbw::getAngle(target->getX(), target->getY(), unit->getX(), unit->getY());
						if (angleCheck > weapons_dat::AttackAngle[unit->getGroundWeapon()] && angleCheck < (256-weapons_dat::AttackAngle[unit->getGroundWeapon()]))
							return false;

						return true;
				});

				if(target){
					scbw::playSound(scbw::randBetween(585, 586));
					const CImage* mainImg = unit->sprite->mainGraphic;
					unit->sprite->createOverlay(926, 0, 0, (mainImg->flags & 0x0002) ? 32-mainImg->direction : mainImg->direction);
					CUnit *tmp = unit->orderTarget.unit;
					unit->orderTarget.unit = target;
					createBullet(unit->getGroundWeapon(), unit, unit->getX(), unit->getY(), unit->playerId, unit->currentDirection1);
					unit->orderTarget.unit = tmp;
					setShock(7, unit->getX(), unit->getY());
					unit->groundWeaponCooldown = weapons_dat::Cooldown[unit->getGroundWeapon()];
				}
			}
			
			/*
			if(unit->subunit){
				if(unit->subunit->mainOrderId == OrderId::AttackUnit){
					if(unit->mainOrderId != OrderId::StayinRange && unit->subunit->orderTarget.unit
						&& ((unit->subunit->orderTarget.unit->status & UnitStatus::InAir 
						&& (unit->subunit->getAirWeapon() == WeaponId::None || !unit->isTargetWithinMinMovementRange(unit->subunit->orderTarget.unit, unit->getMaxWeaponRange(unit->subunit->getAirWeapon()))))
						|| (unit->subunit->getGroundWeapon() == WeaponId::None || !unit->isTargetWithinMinMovementRange(unit->subunit->orderTarget.unit, unit->getMaxWeaponRange(unit->subunit->getGroundWeapon())))))
							unit->subunit->orderToIdle();
					}
				else
					unit->subunit->setRetreatPoint(unit->currentDirection1 >> 3);
			}
			*/
		break;

		case UnitId::mojo:
			if(unit->_padding_0x132 > 0)
				BomberAI(unit);
		break;

		case UnitId::artanis:
			if(unit->beacon._unknown_00 == 12){
				unit->currentButtonSet = UnitId::None;
				if(unit->_padding_0x132 >= 4 && scbw::getDistanceFast(unit->building.powerupOrigin.x, unit->building.powerupOrigin.y, unit->getX(), unit->getY()) < 96){
					unit->userActionFlags |= 4;
					unit->remove();
				}
				if(unit->_padding_0x132 > 0 && unit->spellCooldown == 0){
					if(unit->_padding_0x132 == 1){
						if(unit->subunit){
							unit->subunit->status |= UnitStatus::CanNotAttack;
						}
						if(unit->currentDirection1 <64)
							unit->_unused_0x106 = unit->currentDirection1+192;
						else
							unit->_unused_0x106 = unit->currentDirection1-64;
						unit->_padding_0x132 = 2;
					}
					if(unit->_padding_0x132 != 4){
						AC130AI(unit);
					} else{
						unit->orderTo(OrderId::Move, unit->building.powerupOrigin.x, unit->building.powerupOrigin.y);
						unit->status |= UnitStatus::CanNotReceiveOrders;
					}
				}
			}
		break;
		//여기까지 상태이상에 대해 무적으로 만듬 및 기타코드~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		//서플 올리고 내리기
		case UnitId::supply_depot:
			if(unit->mainOrderId == OrderId::Stop || unit->mainOrderId == OrderId::Burrow){
				unit->orderToIdle();
				//올리기
				if (unit->status & UnitStatus::NoCollide) {
					//위에 다른 유닛이 있는지 확인
					bool isCollide = false;

					scbw::UnitFinder Collidefinder;
					Collidefinder.search(unit->getLeft(), unit->getTop(), unit->getRight(), unit->getBottom());

					CUnit *getUnit;
					for (int i = 0; i < Collidefinder.getUnitCount(); ++i){
						getUnit = Collidefinder.getUnit(i);

						//검사하는 유닛이 본인인지 아닌지 확인하는 코드는 건물인지 아닌지 검사하는 코드때문에 필요없음. 서플이 건물이니까
						if(!(units_dat::BaseProperty[getUnit->id] & (UnitProperty::Subunit | UnitProperty::Building))
							&& !(getUnit->status & (UnitStatus::InAir | UnitStatus::InBuilding | UnitStatus::InTransport))){
								isCollide = true;
								break;
						}
					}

					if (!isCollide) {
						unit->playIscriptAnim(IscriptAnimation::Landing);
						unit->status &= ~UnitStatus::NoCollide;
						unit->status |= UnitStatus::CanNotReceiveOrders;
						unit->sprite->elevationLevel = units_dat::Elevation[unit->id];
						unit->currentButtonSet = UnitId::None;
					}
				} else {
					unit->playIscriptAnim(IscriptAnimation::LiftOff);
					unit->status |= (UnitStatus::NoCollide | UnitStatus::CanNotReceiveOrders);
					unit->sprite->elevationLevel = 0;
					unit->currentButtonSet = UnitId::None;
				}
			}

			if(unit->orderSignal & 0x04){
				unit->orderSignal &= ~0x04;
				unit->status &= ~UnitStatus::CanNotReceiveOrders;
				if(unit->status & UnitStatus::NoCollide)
					unit->currentButtonSet = 103;  //내렸을 때 버튼셋
				else
					unit->currentButtonSet = unit->id;  //올렸을 때 버튼셋
			}
		break;

		//벙커에 회수 기능 추가
		case UnitId::bunker:
			if (unit->mainOrderId == OrderId::ReaverStop) {
				//75%의 미네랄만 회수 (미네랄 증가 코드는 update_unit_state.cpp에 들어감. 벙커가 파괴될 때 미네랄이 회수되도록)
				//resources->minerals[unit->playerId] += (int) (units_dat::MineralCost[unit->id] * 0.75);
				//안에 있는 모든 유닛을 내린다
				unit->orderTo(OrderId::Unload, unit);
				//유닛의 상태를 Disabled , CanNotReceiveOrders로 변경 (명령불가)
				unit->status |= UnitStatus::Disabled & UnitStatus::CanNotReceiveOrders;
				unit->currentButtonSet = UnitId::None;

				//회수시간
				unit->removeTimer = 40;
			}
		break;

		//배틀에 자체 디펜시브 스킬 추가
		case UnitId::battlecruiser:
			if (unit->mainOrderId == OrderId::CarrierStop || unit->mainOrderId == OrderId::DefensiveMatrix){
				if(isCheatEnabled(TheGathering) || unit->energy >= techdata_dat::EnergyCost[TechId::DefensiveMatrix]<<8)
					unit->orderTo(OrderId::DefensiveMatrix, unit);
				else {
					scbw::showErrorMessageWithSfx(unit->playerId, 864 + unit->getRace(), 156 + unit->getRace());
					unit->orderToIdle();
				}
			}
		break;

		//Case 바이킹 코드(시즈모드가 공중모드 바이킹)~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		case UnitId::edmund_duke:
			if(unit->mainOrderId == OrderId::CarrierStop){
				unit->orderToIdle();
				saveCDirection = unit->currentDirection1;
				saveVDirection = unit->velocityDirection1;
				scbw::changeUnitType(unit, UnitId::edmund_duke_s);
				unit->status &= ~UnitStatus::IgnoreTileCollision;
				unit->currentDirection1 = saveCDirection;
				unit->velocityDirection1 = saveVDirection;
				unit->sprite->setDirectionAll(unit->currentDirection1);
				unit->playIscriptAnim(IscriptAnimation::Unused1);
			}
			if(unit->orderSignal){
				unit->orderToIdle();
				unit->orderSignal = 0;
				unit->status &= ~(UnitStatus::Disabled | UnitStatus::CanNotReceiveOrders);
			}	
		break;

		case UnitId::edmund_duke_s:
			if(unit->mainOrderId == OrderId::ReaverStop){
				unit->orderToIdle();
				ActiveTile tile = scbw::getActiveTileAt(unit->position.x, unit->position.y);
				if(tile.cliffEdge || tile.isUnwalkable) {//이동 불가 지형에서는 지상모드로 변환 불가
					scbw::showErrorMessageWithSfx(unit->playerId, 902, 2);
				}
				else
					unit->playIscriptAnim(IscriptAnimation::Unused2);
			}
			if(unit->orderSignal){
				if(unit->sprite->mainGraphic->animationEx == IscriptAnimation::Unused2){
					saveCDirection = unit->currentDirection1;
					saveVDirection = unit->velocityDirection1;
					scbw::changeUnitType(unit, UnitId::edmund_duke);
					unit->currentDirection1 = saveCDirection;
					unit->velocityDirection1 = saveVDirection;
					unit->sprite->setDirectionAll(unit->currentDirection1);
				}
				unit->orderSignal = 0;
				unit->status &= ~(UnitStatus::Disabled | UnitStatus::CanNotReceiveOrders);
			}
		break;
		//바이킹 관련 코드~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		//탱크 코드
		case UnitId::siege_tank:
			if(unit->mainOrderId == OrderId::WatchTarget && unit->subunit->mainOrderId == OrderId::TurretGuard)//원래 스타에도 있던 버그를 고치기 위함
				unit->orderToIdle();

			if(unit->orderSignal){
				unit->orderToIdle();
				unit->subunit->orderToIdle();
				unit->orderSignal = 0;
			}
		break;

		//시즈모드 탱크에 홀드가 박혔을때 공격 또는 정지로 다시 바꾸는 코드
		case UnitId::siege_tank_s:
			if(unit->mainOrderId == OrderId::HoldPosition2){
				if(unit->subunit->orderTarget.unit && unit->subunit->mainOrderId==OrderId::TurretAttack){
					unit->orderTarget.unit = unit->subunit->orderTarget.unit;
					unit->mainOrderId=units_dat::AttackUnitOrder[unit->id];
				}
				else {
					unit->mainOrderId = units_dat::ReturnToIdleOrder[unit->id];
					unit->subunit->mainOrderId = units_dat::ReturnToIdleOrder[unit->subunit->id];
				}
			}
		break;

		//토르를 실은 수송 유닛의 이미지 변화
		//쿠쿨자 가디언을 토르(56), 악튜러스 멩스크를 워메크(27)로 사용
		case UnitId::dropship:
		case UnitId::yggdrasill: {
			CUnit* loadedUnit = unit->getFirstLoadedUnit();
			if(loadedUnit) {
				switch(loadedUnit->id){
				case 27:
					scbw::playFrame(unit->sprite->mainGraphic, 0x22);  //playfram 0x22
					break;
				case 56:
					scbw::playFrame(unit->sprite->mainGraphic, 0x11);  //playfram 0x11
					break;
				default:
					scbw::playFrame(unit->sprite->mainGraphic, 0);     //playfram 0
				}
			}
			else
				scbw::playFrame(unit->sprite->mainGraphic, 0);     //playfram 0
		}
		break;

		//다시 짬. 필요없는 검사식 다 지우고 맵에 미리 배치된 탱크도 이제 클로킹됨.
		case UnitId::alan_schezar:
			switch(unit->sprite->mainGraphic->animationEx){
				case IscriptAnimation::GndAttkInit:
				case IscriptAnimation::GndAttkRpt:
				case IscriptAnimation::Walking:
					unit->unusedTimer = 22;
				break;
			}

			if (unit->secondaryOrderId != OrderId::Cloak) {
				if (!unit->unusedTimer) {
					unit->sprite->createOverlay(978);
					unit->setSecondaryOrder(OrderId::Cloak);
				}
			} else {
				if (unit->unusedTimer) {
					unit->sprite->createOverlay(978);
					unit->setSecondaryOrder(OrderId::Decloak);
				}
			}
		break;

		//리퍼관련
		case UnitId::raszagal:
			unit->status&=~UnitStatus::Disabled;
			unit->status&=~UnitStatus::CanNotReceiveOrders;

			if(unit->orderSignal == 1){
				saveCDirection = unit->currentDirection1;
				saveVDirection = unit->velocityDirection1;
				scbw::changeUnitType(unit, UnitId::jim_raynor_marine);
				unit->currentDirection1 = saveCDirection;
				unit->velocityDirection1 = saveVDirection;
				unit->sprite->setDirectionAll(unit->currentDirection1);
				unit->orderTo(unit->secondaryOrderId, unit->secondaryOrderPos.x, unit->secondaryOrderPos.y);
				unit->secondaryOrderId = OrderId::Nothing2;
			}
		break;
		case UnitId::jim_raynor_marine:
			//리퍼 점프코드. 최적화 완료됨
			//이거 코드 분석해보니 정사각형 범위를 사용하는지라 대각선으로는 점프허용범위가 넓고 직선 방향으로는 좁음
			if(unit->mainOrderId == OrderId::Move || unit->mainOrderId == OrderId::Patrol || unit->mainOrderId == OrderId::AttackMove){
				u16 tileX = 0;
				u16 tileY = 0;
				u8 adusVal = 144; //점프허용범위

				if(unit->orderTarget.pt.x - unit->getX() > adusVal)
					tileX = unit->getX()+adusVal;
				else if(unit->orderTarget.pt.x - unit->getX() < -adusVal)
					tileX = unit->getX()-adusVal;
				else
					tileX = unit->orderTarget.pt.x;

				if(unit->orderTarget.pt.y - unit->getY() > adusVal)
					tileY = unit->getY()+adusVal;
				else if(unit->orderTarget.pt.y - unit->getY() < -adusVal)
					tileY = unit->getY()-adusVal;
				else
					tileY = unit->orderTarget.pt.y;

				ActiveTile actTile = scbw::getActiveTileAt(unit->getX(), unit->getY());
				ActiveTile tarTile = scbw::getActiveTileAt(tileX, tileY);

				if(!tarTile.cliffEdge
					&& !tarTile.hasDoodadCover
					&& !tarTile.currentlyOccupied
					&& !tarTile.isUnwalkable
					&& !actTile.hasDoodadCover
					&& actTile.groundHeight != tarTile.groundHeight){
						unit->secondaryOrderId = unit->mainOrderId;
						unit->secondaryOrderPos = unit->orderTarget.pt;
						saveCDirection = unit->currentDirection1;
						saveVDirection = unit->velocityDirection1;
						scbw::changeUnitType(unit, UnitId::raszagal);
						unit->currentDirection1 = saveCDirection;
						unit->velocityDirection1 = saveVDirection;
						unit->sprite->setDirectionAll(unit->currentDirection1);
						unit->orderTo(6, tileX, tileY);
					}
			}
		break;
		//리퍼관련 종료 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		//메디봇 시전범위에 이동불가 지형을 제외함. 메디봇을 물, 우주, 벽 등에다가 던지면 튕김
		case UnitId::alexei_stukov:
			if(unit->mainOrderId == OrderId::SummonBroodlings){
				ActiveTile tile = scbw::getActiveTileAt(unit->orderTarget.pt.x, unit->orderTarget.pt.y);
				if(tile.cliffEdge || tile.hasDoodadCover || tile.isUnwalkable) {
					scbw::showErrorMessageWithSfx(unit->playerId, 902, 2);
					unit->orderTo(OrderId::Move, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
				}
			}
		break;

		//나이트 쉐이드
		case UnitId::zealot:
			if(unit->status & UnitStatus::SpeedUpgrade){
				//질럿 돌진
				if (unit->sprite->mainGraphic->animationEx == IscriptAnimation::Walking) {

					unit->sprite->removeOverlay(153);

					if (unit->mainOrderId == OrderId::AttackUnit 
						&& unit->orderTarget.unit
						&& unit->canAttackTarget(unit->orderTarget.unit)) {
							if(unit->beacon._unknown_00 == (u32)unit->orderTarget.unit){
								if(isCheatEnabled(TheGathering) || unit->energy >= 64){
									unit->playIscriptAnim(IscriptAnimation::Unused1);
								}
							}
							else{
								unit->beacon._unknown_00 = NULL;
								if(unit->getDistanceToTarget(unit->orderTarget.unit) <= 384 
									&& (isCheatEnabled(TheGathering) || unit->energy == 2560)){
									unit->beacon._unknown_00 = (u32)unit->orderTarget.unit;
									unit->playIscriptAnim(IscriptAnimation::Unused1);
									scbw::playSound(665, unit);
								}
							}
					}
				}
				else if(unit->sprite->mainGraphic->animationEx == IscriptAnimation::Unused1){
					if(!isCheatEnabled(TheGathering)){
						if(unit->energy < 64){
							unit->beacon._unknown_00 = NULL;
							unit->sprite->removeOverlay(153);
							unit->playIscriptAnim(IscriptAnimation::Unused2);
						}
						else
							unit->energy = (u16)std::max((s32)unit->energy-64, 0);
					}
				}
				else {
					unit->beacon._unknown_00 = NULL;
					unit->sprite->removeOverlay(153);
				}
			}
		break;

		//크로노 배틀슈트
		case UnitId::dragoon:
			if(unit->mainOrderId == OrderId::Consume){
				if(unit->orderTarget.pt.x != unit->getX() && unit->orderTarget.pt.y != unit->getY()){ //시전 지점이 자신의 위치면 명령 무시
					if(isCheatEnabled(TheGathering) || unit->energy >= techdata_dat::EnergyCost[TechId::Consume]<<8){//마나가 10 이상이거나 마나무한 치트가 있으면
						ActiveTile actTile = scbw::getActiveTileAt(unit->orderTarget.pt.x, unit->orderTarget.pt.y);
						if(!(actTile.exploredFlags & (1 << unit->playerId)) //탐색되지 않은 지형에다가 점멸할 수 없음.
							&& !actTile.isUnwalkable && !actTile.cliffEdge && !actTile.currentlyOccupied){//이동 불가 지형에다가 점멸할 수 없음.
							if(scbw::getDistanceFast(unit->getX(), unit->getY(), unit->orderTarget.pt.x, unit->orderTarget.pt.y) <= 176) {//대상 거리가 176 이하면
								unit->status &= ~(UnitStatus::NoBrkCodeStart | UnitStatus::CanNotReceiveOrders);
								unit->sprite->playIscriptAnim(IscriptAnimation::Unused2);
								unit->orderTo(OrderId::Move, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
								unit->status |= (UnitStatus::NoBrkCodeStart | UnitStatus::CanNotReceiveOrders);
								unit->building.silo.isReady = true;
							}
						}else
							unit->orderTo(OrderId::Move, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
					}else
						unit->orderTo(OrderId::Move, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
				}else
					unit->orderToIdle();
			}else if(unit->building.silo.isReady){
				unit->status &= ~(UnitStatus::NoBrkCodeStart | UnitStatus::CanNotReceiveOrders);
				unit->MoveUnit(unit->orderTarget.pt.x, unit->orderTarget.pt.y);
				if(!isCheatEnabled(TheGathering))
					unit->energy = (u16)std::max((s32)unit->energy-(techdata_dat::EnergyCost[TechId::Consume]<<8), 0);
				unit->refreshUnitVision();
				unit->orderToIdle();
				unit->sprite->playIscriptAnim(IscriptAnimation::Unused1);
				unit->building.silo.isReady = false;
				scbw::playSound(scbw::randBetween(504, 505), unit);
			}
		break;

		case UnitId::dark_templar:
			if(!(unit->status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection)) && !unit->unusedTimer){
				unit->setSecondaryOrder(OrderId::Cloak);
				unit->updateSpeed();
			}
		break;
			
		case UnitId::scourge:
			if(unit->sprite->mainGraphic->animationEx == IscriptAnimation::GndAttkInit 
				|| unit->sprite->mainGraphic->animationEx == IscriptAnimation::GndAttkRpt
				|| unit->sprite->mainGraphic->animationEx == IscriptAnimation::AirAttkInit
				|| unit->sprite->mainGraphic->animationEx == IscriptAnimation::AirAttkRpt
				|| unit->sprite->mainGraphic->animationEx == IscriptAnimation::CastSpell){
					unit->unusedTimer = 24;
					if (unit->secondaryOrderId == OrderId::Cloak 
						&& (unit->status & UnitStatus::Cloaked) && !(unit->status & UnitStatus::CloakingForFree)){
							unit->setSecondaryOrder(OrderId::Decloak);
					}
			}
			else if(!(unit->status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection)) && !unit->unusedTimer)
				unit->setSecondaryOrder(OrderId::Cloak);
		break;

		//인퀴지터
		case UnitId::high_templar:
			switch(unit->mainOrderId){
			//유도 미사일
			case OrderId::PsiStorm:
				if(unit->orderTarget.unit && unit->getDistanceToTarget(unit->orderTarget.unit) <= unit->getMaxWeaponRange(WeaponId::PsiStorm)){
					if(isCheatEnabled(TheGathering) || unit->energy >= (techdata_dat::EnergyCost[TechId::PsionicStorm]<<8)){
						unit->connectedUnit = unit->orderTarget.unit;
						unit->_unused_0x106 = 4;
						if(!isCheatEnabled(TheGathering))
							unit->energy = (u16)std::max((s32)unit->energy-(techdata_dat::EnergyCost[TechId::PsionicStorm]<<8), 0);
					}
					unit->orderTo(OrderId::Move, unit->getX(), unit->getY());
				}
				break;
			//쉴드 작동
			case OrderId::ReaverStop:
				if(!unit->building.silo.isReady){
					if(unit->energy >= techdata_dat::EnergyCost[TechId::PersonnelCloaking]<<8 || isCheatEnabled(TheGathering)){
						unit->building.silo.isReady = true;
						unit->sprite->createOverlay(449, 0, -23, NULL);
						if(!isCheatEnabled(TheGathering))
							unit->energy = (u16)std::max((s32)unit->energy-(techdata_dat::EnergyCost[TechId::PersonnelCloaking]<<8), 0);
					}else
						scbw::showErrorMessageWithSfx(unit->playerId, 864 + unit->getRace(), 156 + unit->getRace());
				}
				unit->orderToIdle();
				break;
			//쉴드 끄기
			case OrderId::CarrierStop:
				if(unit->building.silo.isReady){
					unit->building.silo.isReady = false;
					CImage *CImg = unit->sprite->getOverlay(449);
					if(CImg)
						CImg->playIscriptAnim(IscriptAnimation::Death);
				}
				unit->orderToIdle();
				break;
			}
			
			if(unit->building.silo.isReady){
				if(!isCheatEnabled(TheGathering))
					unit->energy = (u16)std::max((s32)unit->energy-28, 0);
				if(!unit->energy){
					unit->building.silo.isReady = false;
					CImage *CImg = unit->sprite->getOverlay(449);
					if(CImg)
						CImg->playIscriptAnim(IscriptAnimation::Death);
				}
			}

			if(unit->connectedUnit){
				if(unit->_unused_0x106 && !unit->unusedTimer){
					CUnit *tmpUnit = unit->orderTarget.unit;
					unit->orderTarget.unit = unit->connectedUnit;
					createBullet(WeaponId::GaussRifle5_Unused, unit, unit->getX(), unit->getY() - weapons_dat::VerticalOffset[WeaponId::GaussRifle5_Unused], unit->playerId, unit->currentDirection1);
					unit->orderTarget.unit = tmpUnit;
					--unit->_unused_0x106;
					unit->unusedTimer = 3;
				}
				else if(!unit->_unused_0x106 && !unit->unusedTimer)
					unit->connectedUnit = nullptr;
			}
		break;

		case UnitId::infested_terran:
			if(unit->mainOrderId == OrderId::ReaverStop)
				unit->orderTo(OrderId::SapUnit, unit);
		break;

		case UnitId::tassadar:
			if(unit->mainOrderId == OrderId::ReaverStop){
				unit->orderTo(units_dat::ReturnToIdleOrder[UnitId::aldaris]);
				unit->building.silo.isReady = true;
				unit->status |= UnitStatus::CanNotReceiveOrders;
			}
			
			if(unit->building.silo.isReady && unit->sprite->mainGraphic->animationEx != IscriptAnimation::Unused1)
				unit->sprite->playIscriptAnim(IscriptAnimation::Unused1);

			if(unit->orderSignal & 1){
				unit->building.silo.isReady = false;
				unit->status &= ~UnitStatus::CanNotReceiveOrders;
				saveCDirection = unit->currentDirection1;
				saveVDirection = unit->velocityDirection1;
				scbw::changeUnitType(unit, UnitId::aldaris);
				unit->currentDirection1 = saveCDirection;
				unit->velocityDirection1 = saveVDirection;
				unit->sprite->setDirectionAll(unit->currentDirection1);
			}
		break;

		case UnitId::aldaris:
			if(unit->mainOrderId == OrderId::CarrierStop){
				unit->orderTo(units_dat::ReturnToIdleOrder[UnitId::tassadar]);
				unit->mainOrderId = OrderId::Stop;
				saveCDirection = unit->currentDirection1;
				saveVDirection = unit->velocityDirection1;
				scbw::changeUnitType(unit, UnitId::tassadar);
				unit->currentDirection1 = saveCDirection;
				unit->velocityDirection1 = saveVDirection;
				unit->sprite->setDirectionAll(unit->currentDirection1);
				unit->sprite->playIscriptAnim(IscriptAnimation::Unused2);
			}
		break;

		case UnitId::defiler:
			if(unit->mainOrderId == OrderId::StasisField &&//unit->mainOrderId == OrderId::CarrierIgnore1 && 
				scbw::getDistanceFast(unit->orderTarget.pt.x, unit->orderTarget.pt.y, unit->getX(), unit->getY()) <= 
				weapons_dat::MaxRange[units_dat::GroundWeapon[units_dat::SubUnit[UnitId::infested_kerrigan]]] - BRONTES_ATTACKRADIUS){
					unit->secondaryOrderPos.x = unit->orderTarget.pt.x;
					unit->secondaryOrderPos.y = unit->orderTarget.pt.y;
					unit->orderTo(units_dat::ReturnToIdleOrder[UnitId::infested_kerrigan]);
					unit->building.silo.isReady = true;
					unit->status |= UnitStatus::CanNotReceiveOrders;
			}
			
			if(unit->building.silo.isReady){
				if(unit->sprite->mainGraphic->animationEx != IscriptAnimation::Unused1)
					unit->sprite->playIscriptAnim(IscriptAnimation::Unused1);

				/*
				if(unit->subunit->sprite->mainGraphic->animationEx != IscriptAnimation::Unused1)
					unit->subunit->sprite->playIscriptAnim(IscriptAnimation::Unused1);
				*/

				u8 requireAngle = (u8)scbw::getAngle(unit->secondaryOrderPos.x, unit->secondaryOrderPos.y, unit->getX(), unit->getY());

				if(requireAngle != unit->currentDirection1){
					if(requireAngle > unit->currentDirection1){
						if(requireAngle - unit->currentDirection1 < 128){
							unit->currentDirection1 = requireAngle - unit->currentDirection1 > unit->flingyTurnSpeed ? 
								unit->currentDirection1 + unit->flingyTurnSpeed : requireAngle;
						}
						else{
							unit->currentDirection1 = requireAngle - unit->currentDirection1 > unit->flingyTurnSpeed ? 
								unit->currentDirection1 - unit->flingyTurnSpeed : requireAngle;
						}
					}
					else{
						if(unit->currentDirection1 - requireAngle < 128){
							unit->currentDirection1 = unit->currentDirection1 - requireAngle > unit->flingyTurnSpeed ? 
								unit->currentDirection1 - unit->flingyTurnSpeed : requireAngle;
						}
						else{
							unit->currentDirection1 = unit->currentDirection1 - requireAngle > unit->flingyTurnSpeed ? 
								unit->currentDirection1 + unit->flingyTurnSpeed : requireAngle;
						}
					}
					
					unit->currentDirection2 = unit->currentDirection1;
					unit->velocityDirection1 = unit->currentDirection1;
					unit->velocityDirection2 = unit->currentDirection1;
					unit->sprite->setDirectionAll(unit->currentDirection1);
				}

				if(requireAngle != unit->subunit->currentDirection1){
					if(requireAngle > unit->subunit->currentDirection1){
						if(requireAngle - unit->subunit->currentDirection1 < 128){
							unit->subunit->currentDirection1 = requireAngle - unit->subunit->currentDirection1 > unit->subunit->flingyTurnSpeed ? 
								unit->subunit->currentDirection1 + unit->subunit->flingyTurnSpeed : requireAngle;
						}
						else{
							unit->subunit->currentDirection1 = requireAngle - unit->subunit->currentDirection1 > unit->subunit->flingyTurnSpeed ? 
								unit->subunit->currentDirection1 - unit->subunit->flingyTurnSpeed : requireAngle;
						}
					}
					else{
						if(unit->subunit->currentDirection1 - requireAngle < 128){
							unit->subunit->currentDirection1 = unit->subunit->currentDirection1 - requireAngle > unit->subunit->flingyTurnSpeed ? 
								unit->subunit->currentDirection1 - unit->subunit->flingyTurnSpeed : requireAngle;
						}
						else{
							unit->subunit->currentDirection1 = unit->subunit->currentDirection1 - requireAngle > unit->subunit->flingyTurnSpeed ? 
								unit->subunit->currentDirection1 + unit->subunit->flingyTurnSpeed : requireAngle;
						}
					}
					
					unit->subunit->currentDirection2 = unit->subunit->currentDirection1;
					unit->subunit->velocityDirection1 = unit->subunit->currentDirection1;
					unit->subunit->velocityDirection2 = unit->subunit->currentDirection1;
					unit->subunit->sprite->setDirectionAll(unit->subunit->currentDirection1);
				}
			}

			if(unit->orderSignal & 1){
				unit->building.silo.isReady = false;
				unit->status &= ~UnitStatus::CanNotReceiveOrders;
				saveCDirection = unit->currentDirection1;
				saveVDirection = unit->velocityDirection1;
				scbw::changeUnitType(unit, UnitId::infested_kerrigan);
				unit->currentDirection1 = saveCDirection;
				unit->velocityDirection1 = saveVDirection;
				unit->sprite->setDirectionAll(unit->currentDirection1);
				if(unit->subunit)
					unit->status &= ~UnitStatus::CanTurnAroundToAttack;
				unit->orderToIdle();
				unit->subunit->orderToIdle();
			}
		break;

		case UnitId::infested_kerrigan:
			if(unit->mainOrderId == OrderId::CarrierStop){
				unit->orderTo(units_dat::ReturnToIdleOrder[UnitId::defiler]);
				unit->mainOrderId = OrderId::Stop;
				saveCDirection = unit->currentDirection1;
				saveVDirection = unit->velocityDirection1;
				u8 saveSCDirection, saveSVDirection;
				if(unit->subunit){
					saveSCDirection = unit->subunit->currentDirection1;
					saveSVDirection = unit->subunit->velocityDirection1;
				}
				scbw::changeUnitType(unit, UnitId::defiler);
				unit->currentDirection1 = saveCDirection;
				unit->velocityDirection1 = saveVDirection;
				if(unit->subunit){
					unit->subunit->currentDirection1 = saveSCDirection;
					unit->subunit->velocityDirection1 = saveSVDirection;
				}
				unit->sprite->setDirectionAll(unit->currentDirection1);
				unit->sprite->playIscriptAnim(IscriptAnimation::Unused2);
				unit->secondaryOrderPos.x = unit->secondaryOrderPos.y = NULL;
				if(unit->subunit)
					unit->status |= UnitStatus::CanTurnAroundToAttack;
				unit->orderToIdle();
				unit->subunit->orderToIdle();
			}
		break;

		case UnitId::dark_archon:
			if (unit->mainOrderId == OrderId::ReaverStop){
				if(isCheatEnabled(TheGathering) || unit->energy >= techdata_dat::EnergyCost[TechId::DarkArchonMeld]<<8){
					if(!isCheatEnabled(TheGathering))
						unit->energy -= techdata_dat::EnergyCost[TechId::MindControl]<<8;
					unit->unusedTimer = 28;
					scbw::playSound(scbw::randBetween(813, 814), unit);
					if(unit->sprite->mainGraphic->animationEx == IscriptAnimation::Walking){
						unit->sprite->mainGraphic->playIscriptAnim(IscriptAnimation::Unused1);
						unit->_padding_0x132 = 1;
					}
					unit->building.silo.isReady = true;
				}
				else 
					scbw::showErrorMessageWithSfx(unit->playerId, 864 + unit->getRace(), 156 + unit->getRace());

				unit->orderToIdle();
			}

			if(unit->unusedTimer){
				switch(unit->sprite->mainGraphic->animationEx){
				case IscriptAnimation::Walking:
				case IscriptAnimation::SpecialState1:
					unit->sprite->mainGraphic->playIscriptAnim(IscriptAnimation::Unused2);
					unit->_padding_0x132 = 1;
					break;
				case IscriptAnimation::WalkingToIdle:
					if(unit->_padding_0x132){
						unit->sprite->mainGraphic->playIscriptAnim(IscriptAnimation::SpecialState2);
						unit->_padding_0x132 = 0;
					}
					break;
				}
			}
			else if(unit->building.silo.isReady){
				unit->building.silo.isReady = false;
				if(unit->sprite->mainGraphic->animationEx == IscriptAnimation::Unused1 || unit->sprite->mainGraphic->animationEx == IscriptAnimation::Unused2){
					unit->sprite->mainGraphic->playIscriptAnim(IscriptAnimation::SpecialState1);
					unit->_padding_0x132 = 0;
				}
			}
		break;

		case UnitId::shuttle:
			if(unit->mainOrderId == OrderId::ReaverStop){
				unit->orderTo(OrderId::Stop);
				unit->status |= (UnitStatus::NoCollide | UnitStatus::CanNotReceiveOrders);
				unit->flingyTopSpeed = 0;
				unit->building.silo.isReady = true;
				unit->unusedTimer = 24;
			}

			if(unit->building.silo.isReady){
				if(!unit->unusedTimer){//만약에 버그로 고정된 채로 멈추는 현상 방지를 위함
					unit->status &= ~(UnitStatus::NoCollide | UnitStatus::CanNotReceiveOrders);
					unit->orderToIdle();
					unit->building.silo.isReady = false;
					unit->updateSpeed();
				}
				else
					unit->sprite->playIscriptAnim(IscriptAnimation::Unused1);
			}

			if(unit->orderSignal & 1){
				unit->unusedTimer = NULL;
				unit->orderSignal &= ~0x01;
				scbw::changeUnitType(unit, UnitId::dark_templar_hero);
				unit->currentDirection1 = unit->currentDirection2 = unit->velocityDirection1 = unit->velocityDirection2 = 0;
				unit->sprite->setDirectionAll(unit->currentDirection1);
				unit->currentButtonSet = UnitId::None;
			}
		break;

		case UnitId::dark_templar_hero:
			if(unit->mainOrderId == OrderId::ReaverStop)
				unit->orderTo(OrderId::Unload, unit);
			else if(unit->mainOrderId == OrderId::CarrierStop){
				unit->orderTo(OrderId::Stop);
				unit->sprite->playIscriptAnim(IscriptAnimation::Unused1);
				unit->currentButtonSet = UnitId::None;
				removePsiField(unit);
			} 

			if(unit->building.pylonAura){
				if(unit->isFrozen())
					removePsiField(unit);
			}
			else if(!unit->isFrozen() && unit->sprite->mainGraphic->animationEx != IscriptAnimation::Unused1 
				&& !unit->building.silo.isReady && !(unit->status & UnitStatus::CanNotReceiveOrders))
				unit->orderTo(OrderId::InitPsiProvider);

			if(unit->orderSignal & 1){
				unit->orderSignal &= ~0x01;
				unit->status &= ~UnitStatus::NoCollide;
				scbw::changeUnitType(unit, UnitId::shuttle);
				unit->currentDirection1 = unit->currentDirection2 = unit->velocityDirection1 = unit->velocityDirection2 = 96;
				unit->sprite->setDirectionAll(unit->currentDirection1);
				unit->updateSpeed();
			}

			if(unit->orderSignal & 2){
				unit->orderSignal &= ~0x02;
				unit->status &= ~UnitStatus::CanNotReceiveOrders;
				unit->building.silo.isReady = false;
				unit->orderTo(OrderId::InitPsiProvider);
				unit->currentButtonSet = unit->id;
			}
		break;

		case UnitId::scout:
			if(unit->subunit){
				if(unit->subunit->mainOrderId != OrderId::AttackUnit && unit->subunit->mainOrderId != OrderId::AttackFixedRange)
					unit->subunit->setRetreatPoint(unit->currentDirection1 >> 3);
			}
		break;

		case UnitId::corsair:
			if (unit->mainOrderId == OrderId::CarrierStop || unit->mainOrderId == OrderId::CastMindControl){
				if(isCheatEnabled(TheGathering) || unit->energy >= techdata_dat::EnergyCost[TechId::MindControl]<<8){
					if(!isCheatEnabled(TheGathering))
						unit->energy -= techdata_dat::EnergyCost[TechId::MindControl]<<8;
					scbw::createSprite(134, unit->playerId, unit->getX(), unit->getY(), unit->sprite->elevationLevel+1);
					unit->unusedTimer = 16;
				}
				else
					scbw::showErrorMessageWithSfx(unit->playerId, 864 + unit->getRace(), 156 + unit->getRace());
				unit->orderToIdle();
			}
		break;

		case UnitId::arbiter:
		case UnitId::danimoth:
			if(unit->secondaryOrderId == OrderId::ShieldBattery){
				if(unit->mainOrderId == OrderId::Teleport || unit->mainOrderId == OrderId::StasisField){
					for(CUnit *search = *firstVisibleUnit; search; search = search->link.next){
						if((search->mainOrderId == OrderId::RechargeShields1 || search->mainOrderId == OrderId::Rechargeshields2) 
							&& search->orderTarget.unit == unit)
							search->orderToIdle();
					}
					unit->setSecondaryOrder(OrderId::Nothing2);
				}
			}
			if(unit->mainOrderId == OrderId::Teleport){
				if(!unit->mainOrderState){
					unit->status |= UnitStatus::CanNotReceiveOrders;
					unit->currentButtonSet = UnitId::None;
				}
				else if(!unit->mainOrderTimer && !unit->isFrozen()){
					unit->status &= ~UnitStatus::CanNotReceiveOrders;
					unit->currentButtonSet = unit->id;
				}
			}
		break;

		//언클린 원이랑 쿠쿨자 뮤탈(점프)이 아라크네 영역. 점프범위는 320이니 필요시 이 값 수정
		case UnitId::unclean_one:
			if(unit->mainOrderId == OrderId::StasisField){
				if(isCheatEnabled(TheGathering) || unit->energy >= techdata_dat::EnergyCost[TechId::StasisField]<<8){//마나가 150 이상이거나 마나무한 치트가 있으면
					ActiveTile actTile = scbw::getActiveTileAt(unit->orderTarget.pt.x, unit->orderTarget.pt.y);
					if(!actTile.cliffEdge && !actTile.isUnwalkable){//이동 불가 지형에다가 점멸할 수 없음.
						unit->mainOrderId = OrderId::CarrierIgnore1;
					}else
						unit->orderTo(OrderId::Move, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
				}else
					unit->orderTo(OrderId::Move, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
			}

			if(unit->sprite->mainGraphic->animationEx != IscriptAnimation::Unused1){
				if(unit->mainOrderId == OrderId::CarrierIgnore1){
					if(isCheatEnabled(TheGathering) || unit->energy >= techdata_dat::EnergyCost[TechId::StasisField]<<8){
						if(scbw::getDistanceFast(unit->getX(), unit->getY(), unit->orderTarget.pt.x, unit->orderTarget.pt.y) <= 320){
							if(!isCheatEnabled(TheGathering))
								unit->energy -= techdata_dat::EnergyCost[TechId::StasisField]<<8;
							unit->sprite->playIscriptAnim(IscriptAnimation::Unused1);
							unit->secondaryOrderPos.x = unit->orderTarget.pt.x;
							unit->secondaryOrderPos.y = unit->orderTarget.pt.y;
							unit->orderToIdle();
						}
					}
					else{
						unit->orderTo(OrderId::Move, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
					}
				}
			}
			else{
				u8 requireAngle = (u8)scbw::getAngle(unit->secondaryOrderPos.x, unit->secondaryOrderPos.y, unit->getX(), unit->getY());

				if(requireAngle != unit->currentDirection1){
					if(requireAngle > unit->currentDirection1){
						if(requireAngle - unit->currentDirection1 < 128){
							unit->currentDirection1 = requireAngle - unit->currentDirection1 > unit->flingyTurnSpeed ? 
								unit->currentDirection1 + unit->flingyTurnSpeed : requireAngle;
						}
						else{
							unit->currentDirection1 = requireAngle - unit->currentDirection1 > unit->flingyTurnSpeed ? 
								unit->currentDirection1 - unit->flingyTurnSpeed : requireAngle;
						}
					}
					else{
						if(unit->currentDirection1 - requireAngle < 128){
							unit->currentDirection1 = unit->currentDirection1 - requireAngle > unit->flingyTurnSpeed ? 
								unit->currentDirection1 - unit->flingyTurnSpeed : requireAngle;
						}
						else{
							unit->currentDirection1 = unit->currentDirection1 - requireAngle > unit->flingyTurnSpeed ? 
								unit->currentDirection1 + unit->flingyTurnSpeed : requireAngle;
						}
					}

					unit->currentDirection2 = unit->currentDirection1;
					unit->velocityDirection1 = unit->currentDirection1;
					unit->velocityDirection2 = unit->currentDirection1;
					unit->sprite->setDirectionAll(unit->currentDirection1);
				}
			}

			if(unit->orderSignal & 1){
				unit->orderSignal &= ~0x01;
				unit->orderToIdle();
			}
			if(unit->orderSignal & 8){
				unit->orderSignal &= ~0x08;
				
				u16 x = unit->secondaryOrderPos.x;
				u16 y = unit->secondaryOrderPos.y;

				saveCDirection = unit->currentDirection1;
				saveVDirection = unit->velocityDirection1;
				scbw::changeUnitType(unit, UnitId::kukulza_mutalisk);
				unit->currentDirection1 = saveCDirection;
				unit->velocityDirection1 = saveVDirection;
				if(!unit->sprite->getOverlay(1013))
					unit->sprite->createOverlay(1013);
				unit->sprite->setDirectionAll(unit->currentDirection1);

				s16 plusX = (units_dat::UnitBounds[unit->id].left - units_dat::UnitBounds[unit->id].right)/2;
				s16 plusY = (units_dat::UnitBounds[unit->id].top - units_dat::UnitBounds[unit->id].bottom)/2;
				unit->orderTo(6, x+plusX, y+plusY);
				unit->secondaryOrderPos.x = unit->secondaryOrderPos.y = NULL;
				unit->flingyTopSpeed = scbw::getDistanceFast(unit->getX(), unit->getY(), unit->orderTarget.pt.x, unit->orderTarget.pt.y)*13/2;
				unit->flingyAcceleration = unit->flingyTopSpeed/2;
				unit->status |= UnitStatus::NoCollide;
			}
			else if(unit->orderSignal & 0x20){
				unit->orderSignal &= ~0x20;

				scbw::UnitFinder Collidefinder;
				Collidefinder.search(unit->getLeft()-16, unit->getTop()-16, unit->getRight()+16, unit->getBottom()+16);

				CUnit *getUnit;
				for (int i = 0; i < Collidefinder.getUnitCount(); ++i){
					getUnit = Collidefinder.getUnit(i);

					if(getUnit != unit && !(units_dat::BaseProperty[getUnit->id] & UnitProperty::Subunit)
						&& !(getUnit->status & (UnitStatus::InAir | UnitStatus::InBuilding | UnitStatus::InTransport))){
							weaponDamageHook((weapons_dat::DamageAmount[WeaponId::GaussRifle7_Unused] + 
								weapons_dat::DamageBonus[WeaponId::GaussRifle7_Unused] * getUpgradeLevel(unit->playerId, weapons_dat::DamageUpgrade[WeaponId::GaussRifle7_Unused])) << 8
							, getUnit, WeaponId::GaussRifle7_Unused, unit, unit->playerId, scbw::getAngle(getUnit->getX(), getUnit->getY(), unit->getX(), unit->getY()), 1);
					}
				}
			}
		break;

		case UnitId::kukulza_mutalisk:
			if(unit->orderSignal & 0x20){
				unit->orderSignal &= ~0x20;

				scbw::UnitFinder Collidefinder;
				Collidefinder.search(unit->getLeft(), unit->getTop(), unit->getRight(), unit->getBottom());

				CUnit *getUnit;
				for (int i = 0; i < Collidefinder.getUnitCount(); ++i){
					getUnit = Collidefinder.getUnit(i);

					if(getUnit != unit && !(units_dat::BaseProperty[getUnit->id] & UnitProperty::Subunit)
						&& !(getUnit->status & (UnitStatus::InAir | UnitStatus::InBuilding | UnitStatus::InTransport))){
							weaponDamageHook((weapons_dat::DamageAmount[WeaponId::GaussRifle7_Unused] + 
								weapons_dat::DamageBonus[WeaponId::GaussRifle7_Unused] * getUpgradeLevel(unit->playerId, weapons_dat::DamageUpgrade[WeaponId::GaussRifle7_Unused])) << 8
							, getUnit, WeaponId::GaussRifle7_Unused, unit, unit->playerId, scbw::getAngle(getUnit->getX(), getUnit->getY(), unit->getX(), unit->getY()), 1);
					}
				}
				
				saveCDirection = unit->currentDirection1;
				saveVDirection = unit->velocityDirection1;
				scbw::changeUnitType(unit, UnitId::unclean_one);
				unit->currentDirection1 = saveCDirection;
				unit->velocityDirection1 = saveVDirection;
				if(!unit->sprite->getOverlay(1013))
					unit->sprite->createOverlay(1013);
				unit->sprite->setDirectionAll(unit->currentDirection1);
				unit->status &= ~UnitStatus::NoCollide;
				unit->sprite->playIscriptAnim(IscriptAnimation::Unused2);
			}
		break;

		//UED 오비탈 스트라이크 센터
		case UnitId::gateway:
			switch(unit->mainOrderId){
			case OrderId::ReaverStop:
				if(unit->currentButtonSet == UnitId::gateway){
					if(unit->secondaryOrderId != OrderId::Train)
						unit->currentButtonSet = UnitId::UnusedZerg1;
				}
				else
					unit->currentButtonSet = UnitId::gateway;
				unit->orderToIdle();
			case OrderId::BuildProtoss1:
				if(unit->buildQueue[unit->buildQueueSlot] != UnitId::None){
					CUnit* dropUnit = scbw::createUnitAtPos(UnitId::UnusedZerg1, unit->playerId, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
					if(dropUnit){
						dropUnit->displayedUnitId = unit->buildQueue[unit->buildQueueSlot];
						raceSupply[CUnit::getRace(dropUnit->displayedUnitId)].used[dropUnit->playerId] += units_dat::SupplyRequired[dropUnit->displayedUnitId];
						dropUnit->unusedTimer = 5;
						CSprite* dropPod = scbw::createSprite(131, dropUnit->playerId, dropUnit->getX(), dropUnit->getY(), dropUnit->sprite->elevationLevel+1);
						for (CImage *img = dropPod->images.head; img; img = img->link.next)
							img->verticalOffset = -84;
						if(!isCheatEnabled(OperationCwal)){
							unit->_unknown_0x066 = (units_dat::TimeCost[unit->buildQueue[unit->buildQueueSlot]]<<5)/3;
							unit->_unknown_0x066 += 256 - (unit->_unknown_0x066 % 256);
							unit->orderToIdle();
							unit->status |= UnitStatus::CanNotReceiveOrders;
							unit->currentButtonSet = UnitId::None;
						}
						else
							unit->orderToIdle();
					}
					else{
						scbw::showErrorMessageWithSfx(unit->playerId, 902, 2);
						resources->minerals[unit->playerId] += units_dat::MineralCost[unit->buildQueue[unit->buildQueueSlot]];
						resources->gas[unit->playerId] += units_dat::GasCost[unit->buildQueue[unit->buildQueueSlot]];
						unit->orderToIdle();
					}
					unit->buildQueue[unit->buildQueueSlot] = UnitId::None;
				}
				else
					unit->orderToIdle();
				break;
			}

			if(unit->secondaryOrderId == OrderId::Train && unit->currentButtonSet == UnitId::UnusedZerg1){
				unit->currentButtonSet = UnitId::gateway;
			}

			if(unit->status & UnitStatus::CanNotReceiveOrders && (unit->energy == unit->_unknown_0x066 || isCheatEnabled(OperationCwal))){
				unit->status &= ~(UnitStatus::CanNotReceiveOrders);
				unit->_unknown_0x066 = 0;
				unit->currentButtonSet = UnitId::UnusedZerg1;
			}
		break;

		case UnitId::UnusedZerg1:
			if(!unit->unusedTimer){
				scbw::changeUnitType(unit, unit->displayedUnitId);
				if(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster)
					unit->energy = unit->getMaxEnergy()>>2;
				raceSupply[unit->getRace()].used[unit->playerId] -= units_dat::SupplyRequired[unit->id];
				unit->currentDirection1 = unit->currentDirection2 = unit->velocityDirection1 = unit->velocityDirection2 = scbw::randBetween(0, 255);
				unit->sprite->setDirectionAll(unit->currentDirection1);
				unit->playCompleteEvent();
				unit->orderToIdle();
			}
		break;

		case UnitId::nexus:
			if(unit->mainOrderId == OrderId::ReaverStop){
				if(unit->secondaryOrderId != OrderId::Train){
					unit->orderTo(units_dat::ReturnToIdleOrder[UnitId::infested_duran]);
					unit->sprite->mainGraphic->playIscriptAnim(IscriptAnimation::Unused2);
					if(unit->sprite->mainGraphic->animationEx == IscriptAnimation::Unused2){
						unit->status |= UnitStatus::CanNotReceiveOrders;
						unit->currentButtonSet = UnitId::None;
					}
				}else
					unit->orderToIdle();
			}
			
			if(unit->orderSignal & 0x10){
				unit->orderSignal &= ~0x10;
				scbw::changeUnitType(unit, UnitId::infested_duran);
				unit->currentDirection1 = unit->currentDirection2 = unit->velocityDirection1 = unit->velocityDirection2 = 152;
				unit->sprite->setDirectionAll(unit->currentDirection1);
				unit->updateUnitDamageOverlay();
				unit->rally.pt.x = unit->rally.pt.y = NULL;
				unit->rally.unit = nullptr;
			}
			
			if(unit->orderSignal & 0x08){
				unit->orderSignal &= ~0x08;
				unit->status &= ~UnitStatus::CanNotReceiveOrders;
				unit->currentButtonSet = unit->id;
			}
		break;

		case UnitId::infested_duran:
			if(unit->mainOrderId == OrderId::BuildingLand){
				unit->building.silo.isReady = true;
				unit->secondaryOrderPos.x = unit->orderTarget.pt.x;
				unit->secondaryOrderPos.y = unit->orderTarget.pt.y;
				unit->orderTo(OrderId::CarrierIgnore1, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
			}
			else if(unit->mainOrderId != OrderId::CarrierIgnore1 && unit->mainOrderId != OrderId::ReaverStop && unit->mainOrderId != OrderId::Nothing2)
				unit->building.silo.isReady = false;

			if((unit->building.silo.isReady == true || unit->mainOrderId == OrderId::ReaverStop) 
				&& unit->secondaryOrderPos.x == unit->getX() && unit->secondaryOrderPos.y == unit->getY()){
				unit->sprite->playIscriptAnim(IscriptAnimation::Unused1);
				if(unit->sprite->mainGraphic->animationEx == IscriptAnimation::Unused1){
					unit->status |= UnitStatus::CanNotReceiveOrders;
					unit->currentButtonSet = UnitId::None;
				}
				else
					unit->mainOrderId = OrderId::ReaverStop;
			}

			if(unit->orderSignal & 0x10){
				//주위에 다른 유닛이 있는지 확인
				bool isCollide = false;

				scbw::UnitFinder Collidefinder;
				Collidefinder.search(unit->getX() - units_dat::UnitBounds[UnitId::nexus].left, unit->getY() - units_dat::UnitBounds[UnitId::nexus].top, 
					unit->getX() + units_dat::UnitBounds[UnitId::nexus].right, unit->getY() + units_dat::UnitBounds[UnitId::nexus].bottom);

				CUnit *getUnit;
				for (int i = 0; i < Collidefinder.getUnitCount(); ++i){
					getUnit = Collidefinder.getUnit(i);

					if(getUnit != unit && !(units_dat::BaseProperty[getUnit->id] & (UnitProperty::Subunit | UnitProperty::Building))
						&& !(getUnit->status & (UnitStatus::InAir | UnitStatus::InBuilding | UnitStatus::InTransport))){
							isCollide = true;
							break;
					}
				}

				if(isCollide){
					scbw::showErrorMessageWithSfx(unit->playerId, 863, 142);
					minimapPing(unit->getX(), unit->getY(), 0x11);
					unit->status &= ~UnitStatus::CanNotReceiveOrders;
					unit->currentButtonSet = unit->id;
				}
				else{
					const s32 saveShield = units_dat::MaxShieldPoints[unit->id] ? ((unit->shields>>8) * units_dat::MaxShieldPoints[UnitId::nexus] / units_dat::MaxShieldPoints[unit->id])<<8 : 0;
					scbw::changeUnitType(unit, UnitId::nexus);
					unit->shields = saveShield;
					unit->sprite->mainGraphic->playIscriptAnim(IscriptAnimation::Unused1);
					unit->currentDirection1 = unit->currentDirection2 = unit->velocityDirection1 = unit->velocityDirection2 = 0;
					unit->sprite->setDirectionAll(unit->currentDirection1);
					unit->updateUnitDamageOverlay();
					unit->status |= UnitStatus::CanNotReceiveOrders;
					unit->currentButtonSet = UnitId::None;
				}
				unit->building.silo.isReady = false;
				unit->orderSignal &= ~0x10;
				unit->orderToIdle();
			}
		break;

		case UnitId::photon_cannon:
			if(unit->mainOrderId == OrderId::ReaverStop && !(unit->status & UnitStatus::DoodadStatesThing)){
				//올리기
				if (unit->status & UnitStatus::NoCollide) {
					//위에 다른 유닛이 있는지 확인
					bool isCollide = false;

					scbw::UnitFinder Collidefinder;
					Collidefinder.search(unit->getLeft(), unit->getTop(), unit->getRight(), unit->getBottom());

					CUnit *getUnit;
					for (int i = 0; i < Collidefinder.getUnitCount(); ++i){
						getUnit = Collidefinder.getUnit(i);

						if(!(units_dat::BaseProperty[getUnit->id] & (UnitProperty::Subunit | UnitProperty::Building))
							&& !(getUnit->status & (UnitStatus::InAir | UnitStatus::InBuilding | UnitStatus::InTransport))
							&& getUnit->playerId == unit->playerId){
								isCollide = true;
								break;
						}
					}

					if (!isCollide) {
						unit->mainOrderId = OrderId::Nothing2;
						unit->playIscriptAnim(IscriptAnimation::UnBurrow);
						unit->status &= ~(UnitStatus::NoCollide);
						unit->status |= UnitStatus::CanNotReceiveOrders;
						unit->sprite->elevationLevel = units_dat::Elevation[unit->id];
						unit->currentButtonSet = UnitId::None;
					} else
						unit->orderToIdle();
				} else {
					unit->mainOrderId = OrderId::Nothing2;
					unit->playIscriptAnim(IscriptAnimation::Burrow);
					unit->status |= (UnitStatus::NoCollide | UnitStatus::CanNotReceiveOrders);
					unit->sprite->elevationLevel = 0;
					unit->currentButtonSet = UnitId::None;
				}
			}

			if(unit->orderSignal & 0x04){
				unit->orderSignal &= ~0x04;
				unit->status &= ~UnitStatus::CanNotReceiveOrders;
				if(unit->status & UnitStatus::NoCollide)
					unit->currentButtonSet = 187;  //내렸을 때 버튼셋
				else{
					unit->currentButtonSet = unit->id;  //올렸을 때 버튼셋
					unit->orderToIdle();
				}
			}
		break;

		case UnitId::UnusedRuins:
			if(unit->status & UnitStatus::Completed && !(unit->status & UnitStatus::DoodadStatesThing)){
				unit->building.silo.isReady = false;
				const s32 saveShield = units_dat::MaxShieldPoints[unit->id] ? ((unit->shields>>8) * units_dat::MaxShieldPoints[UnitId::zeratul] / units_dat::MaxShieldPoints[unit->id])<<8 : 0;
				scbw::changeUnitType(unit, UnitId::zeratul);
				unit->shields = saveShield;
				unit->currentDirection1 = unit->currentDirection2 = unit->velocityDirection1 = unit->velocityDirection2 = 96;
				unit->sprite->setDirectionAll(unit->currentDirection1);
				unit->playCompleteEvent();
				unit->orderToIdle();
			}
			else if (!unit->building.silo.isReady){
				unit->sprite->createUnderlay(358, 0, 0, NULL);
				unit->building.silo.isReady = true;
			}
		break;

		//켈모리안 실험건물
		case UnitId::Special_PsiDisrupter:
			if(unit->status & UnitStatus::Completed){
				unit->energy = 65535;

				//패키지 증가 딜레이
				if(unit->_unknown_0x066 < 10){
					if(!unit->unusedTimer){
						if(unit->_padding_0x132 >= 239){
							unit->_padding_0x132 = 0;
							++unit->_unknown_0x066;
						}
						else
							++unit->_padding_0x132;
						unit->unusedTimer = 3;
					}
				}

				//몽구스 호출
				if(unit->mainOrderId == OrderId::Plague){
					if(unit->_unknown_0x066 > 0 || isCheatEnabled(TheGathering)){
						const RaceId::Enum mongRace = CUnit::getRace(UnitId::gerard_dugalle);
						if(raceSupply[mongRace].used[unit->playerId] + (units_dat::SupplyRequired[UnitId::gerard_dugalle]<<1) <= raceSupply[mongRace].max[unit->playerId]){
							if(isCheatEnabled(FoodForThought) || scbw::getSupplyRemaining(unit->playerId, mongRace) >= units_dat::SupplyRequired[UnitId::gerard_dugalle]<<1){
								if(scbw::getDistanceFast(unit->getX(), unit->getY(), unit->orderTarget.pt.x, unit->orderTarget.pt.y) <= 1280){
									ActiveTile actTileA = scbw::getActiveTileAt(unit->orderTarget.pt.x - 30, unit->orderTarget.pt.y - 20);
									ActiveTile actTileB = scbw::getActiveTileAt(unit->orderTarget.pt.x + 30, unit->orderTarget.pt.y + 20);

									if(!(actTileA.exploredFlags & (1 << unit->playerId)) && !(actTileB.exploredFlags & (1 << unit->playerId))){//탐색 되었는가
										if(!actTileA.isUnwalkable && !actTileA.cliffEdge && !actTileB.isUnwalkable && !actTileB.cliffEdge){
											CUnit* dropUnitA = scbw::createUnitAtPos(UnitId::UnusedZerg1, unit->playerId, unit->orderTarget.pt.x - 20, unit->orderTarget.pt.y -10);
											CUnit* dropUnitB = scbw::createUnitAtPos(UnitId::UnusedZerg1, unit->playerId, unit->orderTarget.pt.x + 20, unit->orderTarget.pt.y +10);
											if(dropUnitA || dropUnitB){
												if(dropUnitA){
													dropUnitA->displayedUnitId = UnitId::gerard_dugalle;
													raceSupply[mongRace].used[dropUnitA->playerId] += units_dat::SupplyRequired[UnitId::gerard_dugalle];
													dropUnitA->unusedTimer = 5;
													CSprite* dropPod1 = scbw::createSprite(131, dropUnitA->playerId, dropUnitA->getX(), dropUnitA->getY(), dropUnitA->sprite->elevationLevel+1);
													for (CImage *img = dropPod1->images.head; img; img = img->link.next){
														img->verticalOffset = -72;
														img->playIscriptAnim(IscriptAnimation::GndAttkInit);
													}
												}
												if(dropUnitB){
													dropUnitB->displayedUnitId = UnitId::gerard_dugalle;
													raceSupply[mongRace].used[dropUnitB->playerId] += units_dat::SupplyRequired[UnitId::gerard_dugalle];
													dropUnitB->unusedTimer = 5;
													CSprite* dropPod2 = scbw::createSprite(131, dropUnitB->playerId, dropUnitB->getX(), dropUnitB->getY(), dropUnitB->sprite->elevationLevel+1);
													for (CImage *img = dropPod2->images.head; img; img = img->link.next){
														img->verticalOffset = -72;
														img->playIscriptAnim(IscriptAnimation::GndAttkInit);
													}
												}
												if(!isCheatEnabled(TheGathering))
													--unit->_unknown_0x066;
											}else{
												scbw::showErrorMessageWithSfx(unit->playerId, 843, 2);
											}
										}else{
											scbw::showErrorMessageWithSfx(unit->playerId, 843, 2);
										}
									}else{
										scbw::showErrorMessageWithSfx(unit->playerId, 857, 2);
									}
								}else{
									scbw::showErrorMessageWithSfx(unit->playerId, 888, 2);
								}
							}else{
								scbw::showErrorMessageWithSfx(unit->playerId, 844 + mongRace, 153 + mongRace);
							}
						}else{
							scbw::showErrorMessageWithSfx(unit->playerId, 847 + mongRace, 2);
						}
					}else{
						scbw::showErrorMessageWithSfx(unit->playerId, 860, 2);
					}

					unit->orderToIdle();

				}

				//폭격기 호출
				else if(unit->mainOrderId == OrderId::Ensnare){
					if(isCheatEnabled(TheGathering) || unit->_unknown_0x066 > 0){
						Target target = unit->orderTarget;
						CUnit *bomber = scbw::createUnitAtPos(UnitId::mojo, unit->playerId, unit->getX(), unit->getY());
						if(bomber){
							for(int i = 0; i < PLAYER_COUNT; i++){
								if(i != unit->playerId){
									minimapPing(target.pt.x, target.pt.y, 0x11);
									scbw::showErrorMessageWithSfx(i, 1614, 1075);
								}
								else{
									minimapPing(target.pt.x, target.pt.y, 0x10);
									scbw::showErrorMessageWithSfx(i, 1613, 838);
								}
							}

							const MapSize map = *mapTileSize;
							const u32 bomberWeaponRange = weapons_dat::MaxRange[bomber->getGroundWeapon()];

							bomber->currentButtonSet = UnitId::None;
							bomber->_padding_0x132 = 1; //유닛의 상태를 구분하는 변수
							bomber->unusedTimer = 2;
							if(scbw::getDistanceFast(bomber->getX(), bomber->getY(), target.pt.x, target.pt.y) > bomberWeaponRange){
								bomber->secondaryOrderPos.x = target.pt.x + scbw::getPolarX(bomberWeaponRange, 
									scbw::getAngle(bomber->getX(), bomber->getY(), target.pt.x, target.pt.y));
								bomber->secondaryOrderPos.y = target.pt.y + scbw::getPolarY(bomberWeaponRange, 
									scbw::getAngle(bomber->getX(), bomber->getY(), target.pt.x, target.pt.y));
							}
							else
								bomber->secondaryOrderPos = target.pt;
							if(!setFinalDestination(bomber, bomber->secondaryOrderPos.x, bomber->secondaryOrderPos.y, OrderId::Move)){
								bomber->userActionFlags |= 4;
								bomber->remove();
								break;
							}
							bomber->currentDirection1 = bomber->currentDirection2 = bomber->velocityDirection1 = bomber->velocityDirection2 = 
								scbw::getAngle(bomber->secondaryOrderPos.x, bomber->secondaryOrderPos.y, bomber->getX(), bomber->getY());
							bomber->sprite->setDirectionAll(bomber->currentDirection1);
							scbw::replaceSpriteImages(bomber->sprite, bomber->sprite->mainGraphic->id, bomber->currentDirection1);
							bomber->building.upgradeLevel = 1;
							bomber->status |= UnitStatus::CanNotReceiveOrders;
							CUnit *marker = scbw::createUnitAtPos(UnitId::UnusedProtossMarker, unit->playerId, target.pt.x, target.pt.y);
							marker->sprite->mainGraphic->setRemapping(ColorRemapping::OFire);
							marker->_padding_0x132 = 1;
							marker->unusedTimer = 60;
							if(!isCheatEnabled(TheGathering))
								--unit->_unknown_0x066;
						}else{
							scbw::showErrorMessageWithSfx(unit->playerId, 843, 2);
						}
					}else{
						scbw::showErrorMessageWithSfx(unit->playerId, 860, 2);
					}
					unit->orderToIdle();
				}
			}
		break;
		//자치령 실험건물
		case UnitId::Special_CrashedNoradII:
			if(unit->status & UnitStatus::Completed){
				unit->energy = 65535;

				//패키지 증가 딜레이
				if(unit->_unknown_0x066 < 10){
					if(!unit->unusedTimer){
						if(unit->_padding_0x132 >= 239){
							unit->_padding_0x132 = 0;
							++unit->_unknown_0x066;
						}
						else
							++unit->_padding_0x132;
						unit->unusedTimer = 3;
					}
				}

				//AC130 소환
				if(unit->mainOrderId == OrderId::Plague){
					if(isCheatEnabled(TheGathering) || unit->_unknown_0x066 > 0){
						const MapSize map = *mapTileSize;

						//맵 끝자락에 호출되는 것을 방지함
						if(unit->orderTarget.pt.x < 208){ 
							unit->orderTarget.pt.x = 208; 
						} else if (unit->orderTarget.pt.x > (map.width<<5)-208){ 
							unit->orderTarget.pt.x = (map.width<<5)-208; 
						}
						if(unit->orderTarget.pt.y < 228){ 
							unit->orderTarget.pt.y = 228; 
						} else if (unit->orderTarget.pt.y > (map.height<<5)-272){ 
							unit->orderTarget.pt.y = (map.height<<5)-272; 
						}
						
						Target target = unit->orderTarget;
						CUnit* ac130 = scbw::createUnitAtPos(UnitId::artanis, unit->playerId, unit->getX(), unit->getY());
						if(ac130){
							for(int i = 0; i < PLAYER_COUNT; i++){
								if(i != unit->playerId){
									minimapPing(target.pt.x, target.pt.y, 0x11);
									scbw::showErrorMessageWithSfx(i, 1612, 1075);
								}
								else{
									minimapPing(target.pt.x, target.pt.y, 0x10);
									scbw::showErrorMessageWithSfx(i, 1611, scbw::randBetween(796,798));
								}
							}
							ac130->currentButtonSet = UnitId::None;
							ac130->beacon._unknown_00 = 12;
							ac130->repulseUnknown = unit->playerId;
							ac130->building.powerupOrigin.x = unit->getX();
							ac130->building.powerupOrigin.y = unit->getY();
							ac130->orderTo(OrderId::Move, target.pt.x, target.pt.y);
							ac130->_padding_0x132 = 1;
							ac130->secondaryOrderPos = target.pt;
							CUnit *marker = scbw::createUnitAtPos(UnitId::UnusedProtossMarker, unit->playerId, target.pt.x, target.pt.y+16);
							marker->_padding_0x132 = 1;
							marker->unusedTimer = 60;
							ac130->spellCooldown = 6;
							if(!isCheatEnabled(TheGathering))
								--unit->_unknown_0x066;
							}
						else{
							scbw::showErrorMessageWithSfx(unit->playerId, 843, 2);
						}
					}else{
						scbw::showErrorMessageWithSfx(unit->playerId, 860, 2);
					}
					unit->orderToIdle();
				}
			}
		break;
		//UED 실험건물(요르문간드)
		case UnitId::UnusedTerran1:
			if(unit->status & UnitStatus::Completed && !(unit->status & UnitStatus::DoodadStatesThing)){
				unit->energy = 65535;

				//패키지 증가 딜레이
				if(unit->_unknown_0x066 < 10){
					if(!unit->unusedTimer){
						if(unit->_padding_0x132 >= 239
							|| (unit->pAI && unit->_padding_0x132 >= 20)){
							unit->_padding_0x132 = 0;
							++unit->_unknown_0x066;
						}
						else
							++unit->_padding_0x132;
						unit->unusedTimer = 3;
					}
				}

				//공격
				if(unit->mainOrderId == OrderId::Ensnare && !(unit->status & UnitStatus::CanNotReceiveOrders) && !unit->spellCooldown) {
					if(isCheatEnabled(TheGathering) || unit->_unknown_0x066){
						const Target target = unit->orderTarget;
						for(int i = 0; i < PLAYER_COUNT; i++){
							if(i != unit->playerId){
								minimapPing(target.pt.x, target.pt.y, 0x11);
								scbw::showErrorMessageWithSfx(i, 1672, 763);
							}
							else{
								minimapPing(target.pt.x, target.pt.y, 0x10);
								scbw::showErrorMessageWithSfx(i, 1671, 770);
							}
						}
						CUnit *marker = scbw::createUnitAtPos(UnitId::UnusedProtossMarker, unit->playerId, target.pt.x, target.pt.y);
						marker->sprite->mainGraphic->setRemapping(ColorRemapping::OFire);
						marker->removeTimer = 120;
						unit->orderTo(OrderId::Plague, target.pt.x, target.pt.y);
						unit->status |= UnitStatus::CanNotReceiveOrders;
						if(!isCheatEnabled(TheGathering))
							--unit->_unknown_0x066;
					}
					else{
						scbw::showErrorMessageWithSfx(unit->playerId, 860, 2);
						unit->orderToIdle();
					}
				}

				if(unit->orderSignal & 0x02)
					unit->status &= ~UnitStatus::CanNotReceiveOrders;
			}
		break;

		//건물 버그 수정 및 애드온 변경 시스템
		case UnitId::barracks:
		case UnitId::factory:
		case UnitId::starport:
		case UnitId::spawning_pool:
		case UnitId::ultralisk_cavern:
		case UnitId::UnusedIndependentCommandCenter:
		case UnitId::command_center:
		case 121:
		case UnitId::infested_command_center:
			switch(unit->mainOrderId) {
			case OrderId::BuildingLand:
				if(unit->orderSignal == 16){
					unit->unusedTimer = 2;
					if(unit->building.addonBuildType && unit->building.addonBuildType != 228)
						unit->_padding_0x132 = unit->building.addonBuildType;
				}
				break;
			case OrderId::BuildingLiftoff://맵 맨위에다가 랜드시킬때 리프트랜드 반복하는 버그 수정
				if(unit->getY() < 64 && unit->unusedTimer){
					unit->mainOrderId = 23;
					unit->orderQueueCount = 0;
					unit->orderQueueHead = NULL;
					unit->orderQueueTail = NULL;
					unit->orderSignal = 0;
					unit->status &= ~UnitStatus::CanNotReceiveOrders;
					unit->unusedTimer = 0;
					if(unit->_padding_0x132){
						unit->building.addonBuildType = unit->_padding_0x132;
						unit->orderTo(OrderId::PlaceAddon, unit->getX(), unit->getY());
						unit->_padding_0x132 = NULL;
					}
					else
						unit->mainOrderId = 23;
				}
				break;
			case OrderId::ReaverStop:
				if(unit->secondaryOrderId != OrderId::BuildAddon){
					const u16 addonId = getBuildingAddonType(unit->id, nullptr);
					unit->building.addonBuildType = addonId;
					unit->orderTo(OrderId::PlaceAddon, unit->getX(), unit->getY());
				}else
					unit->orderToIdle();
				break;
			}

			//애드온 자동 연결
			if(unit->unusedTimer || (unit->_unused_0x106 == 0 && unit->remainingBuildTime <= 16)){
				bool findComsat = false;
				u16 changeId = getBuildingAddonType(unit->id, &findComsat);//미리 바꿔져야할 유닛id를 저장해둔다. 

				if(findComsat == true){//컴셋 찾을 때
					for(CUnit* searchAddon=*firstVisibleUnit;searchAddon;searchAddon=searchAddon->link.next){
						if(searchAddon->playerId == 11
							&& unit->moveTarget.pt.x+80 == searchAddon->getX() && unit->moveTarget.pt.y+16 == searchAddon->getY()
							&& searchAddon->id != changeId
							&& (searchAddon->id == UnitId::comsat_station || searchAddon->id == UnitId::Special_MatureChrysalis)){
								searchAddon->id = changeId;
								searchAddon->currentButtonSet = changeId;
								break;
						}
					}
				}
				else{
					for(CUnit* searchAddon=*firstVisibleUnit;searchAddon;searchAddon=searchAddon->link.next){
						if(searchAddon->playerId == 11
							&& unit->moveTarget.pt.x+96 == searchAddon->getX() && unit->moveTarget.pt.y+16 == searchAddon->getY()
							&& searchAddon->id != changeId
							&& (searchAddon->id == UnitId::Special_IonCannon || searchAddon->id == UnitId::machine_shop 
							|| searchAddon->id == UnitId::control_tower || searchAddon->id == UnitId::covert_ops 
							|| searchAddon->id == UnitId::Special_IndependentStarport || searchAddon->id == UnitId::UnusedIndependentJumpGate)){
								searchAddon->id = changeId;
								searchAddon->currentButtonSet = changeId;
								break;
						}
					}
				}

				unit->_unused_0x106 = 1;//build 도중에는 이 코드가 한번만 실행되도록 함
			}
		break;

		//과거 벙커터렛이었던 애드온 건물
		case UnitId::physics_lab:
			unit->remove();
		break;

		case UnitId::UnusedProtoss1:{//역장
			//EMP맞으면 터지는 코드는 BG_inject.cpp에 넣음
			scbw::UnitFinder sUnit(unit->getLeft()-16, unit->getTop()-16, unit->getRight()+16, unit->getBottom()+16);
			
			for (int i = 0; i < sUnit.getUnitCount(); ++i){
				CUnit* search = sUnit.getUnit(i);
				
				if((!(search->status & UnitStatus::InAir) && isNoDebuffUnit(search))
					|| search->id == UnitId::kukulza_guardian
					|| search->id == UnitId::arcturus_mengsk
					|| search->id == UnitId::dark_archon
					|| search->id == UnitId::infested_duran){
					unit->remove();
					break;
				}
			}
		}
		break;

		//초반에 저그인구수 주는 저그마커가 중립(P12)에게 넘어가면 삭제
		case 191:
			if(unit->playerId == 11)
				unit->remove();
		break;

		//플토 마커
		case 193:
			if(unit->_padding_0x132 == 1 && unit->unusedTimer == 0)
				unit->remove();
		break;

		//메디봇과 마인
		case UnitId::broodling:
			unit->energy = 65535;//밑에 break 넣지 마셈.
		case UnitId::spider_mine:
			unit->status |= UnitStatus::NoCollide;
		break;

		case UnitId::Powerup_ZergGasSacType1:
			for (CImage *img = scbw::createSprite(131, unit->playerId, unit->getX(), unit->getY(), 11)->images.head; img; img = img->link.next)
				img->verticalOffset = -84;

			unit->remove();
		break;

		case UnitId::Powerup_ZergGasSacType2:
			for (CImage *img = scbw::createSprite(131, unit->playerId, unit->getX(), unit->getY(), 11)->images.head; img; img = img->link.next){
				img->verticalOffset = -78;
				img->playIscriptAnim(IscriptAnimation::GndAttkInit);
			}

			unit->remove();
		break;

		case UnitId::Powerup_YoungChrysalis:
			hooks::setShock(5, unit->getX(), unit->getY());

			unit->remove();
		break;

		case UnitId::Powerup_MineralClusterType2:
			if(unit->orderSignal & 0x08){
				auto goldGathererFinder = [] (const CUnit *search) -> bool {
					if(!search->hitPoints || search->mainOrderId == OrderId::Die)
						return false;

					if(search->playerId >= 8)
						return false;

					if(search->status & (UnitStatus::GroundedBuilding | UnitStatus::InBuilding | UnitStatus::InAir | UnitStatus::IsSelfDestructing))
						return false;

					return true;
				};
			
				CUnit* goldGatherer = scbw::UnitFinder::getNearestTarget(
					unit->getLeft() - 48, unit->getTop() - 48,
					unit->getRight() + 48, unit->getBottom() + 48,
					unit, goldGathererFinder);

				if(goldGatherer){
					resources->minerals[goldGatherer->playerId] += unit->secondaryOrderPos.x;
					resources->gas[goldGatherer->playerId] += unit->secondaryOrderPos.y;
					unit->userActionFlags |= 4;
					unit->remove();
				}
			}
		break;

	}//unit->id 스위치문 종료

	}//유닛 루프문 끝남

	
	//옵저버
	if (!(*IS_IN_REPLAY)) {

		if(obMode == true){
			graphics::drawText(547,17,"\x03Observer Mode",graphics::FONT_MEDIUM, graphics::ON_SCREEN); 
				memoryPatch(0x004E5738,0xB9);
				memoryPatch_ByteNo(0x004E573D,0x909090,3);
				memoryPatch(0x004E5739,resources->gas[selectPlayerId]);
		
				memoryPatch(0x004E575A,0xB9);
				memoryPatch_ByteNo(0x004E575F,0x9090,2);
				memoryPatch(0x004E575B,resources->minerals[selectPlayerId]);	

			if(beforeSelect != selectPlayerId){
				memoryPatch_ByteNo(0x00459255,0xBA,1);
				memoryPatch_ByteNo(0x00459256,selectPlayerId,1);
				memoryPatch_ByteNo(0x00459257,0x000000,3);
				memoryPatch_ByteNo(0x0045925A,0x9090,2);
			
				memoryPatch_ByteNo(0x004E566C,0xBA,1);
				memoryPatch_ByteNo(0x004E566D,selectPlayerId,1);
				memoryPatch_ByteNo(0x004E566E,0x000000,3);
				memoryPatch_ByteNo(0x004E5671,0x90,1);

				memoryPatch_ByteNo(0x004E56E0,0xBA,1);
				memoryPatch_ByteNo(0x004E56E1,selectPlayerId,1);
				memoryPatch_ByteNo(0x004E56E2,0x000000,3);
				memoryPatch_ByteNo(0x004E56E5,0x90,1);

				memoryPatch_ByteNo(0x004E5709,0xBA,1);
				memoryPatch_ByteNo(0x004E570A,selectPlayerId,1);
				memoryPatch_ByteNo(0x004E570B,0x000000,3);
				memoryPatch_ByteNo(0x004E570E,0x90,1);

				memoryPatch_ByteNo(0x004E5732,0xBA,1);
				memoryPatch_ByteNo(0x004E5733,selectPlayerId,1);
				memoryPatch_ByteNo(0x004E5734,0x000000,3);
				memoryPatch_ByteNo(0x004E5737,0x90,1);
			}
			
			/*if (*(VK_Array+VK_F8)){
				if(!checkF8){
					if(drawMove == false){
						scbw::playSound(23);
						drawMove = true;
						scbw::printText("View Point ON");
					} else {
						scbw::playSound(23);
						drawMove = false;
						scbw::printText("View Point OFF");
					}
					checkF8 = true;
				}
			} else {
				checkF8 = false;
			}*/
		} else{
			checkF8 = false;
			drawMove = false;
		}
		

			if (*(VK_Array+VK_F9)){
				if(!checkF9){
					if(myunitEx == 0){
						if(ifObs == false){
							if(obMode == false){
				
								memoryPatch_ByteNo(0x00427271,0x10EB,2); //ObsMode 에그
								memoryPatch_ByteNo(0x004278F5,0x2DEB,2); //ObsMode 건설중 - UnitStatAct_
								memoryPatch_ByteNo(0x0042690C,0x10EB,2); //ObsMode 
								memoryPatch_ByteNo(0x00427557,0x19EB,2); //ObsMode 
								memoryPatch_ByteNo(0x00425F3D,0x10EB,2); //ObsMode 
								memoryPatch_ByteNo(0x00426511,0x10EB,2); //ObsMode 
								memoryPatch_ByteNo(0x00427557,0x19EB,2); //ObsMode 
								memoryPatch_ByteNo(0x00426C85,0x19EB,2); //ObsMode ??
								memoryPatch_ByteNo(0x00427D4B,0x4AEB,2); //ObsMode 	
								memoryPatch_ByteNo(0x004273EF,0x10EB,2); //ObsMode 아콘
								memoryPatch_ByteNo(0x004274B1,0x0CEB,2); //ObsMode 캐리어 리버
								memoryPatch_ByteNo(0x00425B7C,0x43EB,2); //ObsMode 
								memoryPatch_ByteNo(0x004266FF,0x10EB,2); //ObsMode 
								memoryPatch_ByteNo(0x00427CA3,0x0CEB,2); //ObsMode 
								memoryPatch_ByteNo(0x00427CAF,0x5BEB,2); //ObsMode 드랍쉽 
								memoryPatch_ByteNo(0x00427CDB,0x9090,2); //ObsMode 드랍쉽 
								memoryPatch_ByteNo(0x00425B70,0x0CEB,2); //ObsMode 할루시네이션 
								memoryPatch_ByteNo(0x00425B7E,0x0D8B,2); //ObsMode 할루시네이션 
								memoryPatch_ByteNo(0x00425A66,0x08EB,2); //ObsMode 마나 
								memoryPatch_ByteNo(0x00459288,0x10EB,2); //ObsMode 버튼 B60F
								memoryPatch_ByteNo(0x0045927F,0x19EB,2); //ObsMode 버튼 B60F

								memoryPatch_ByteNo(0x00493663,0x9090,2); //ObsMode 
								


								obMode = true;
							} else {
								memoryPatch_ByteNo(0x004E575A,0x0057F0F0850C8B,7);
								memoryPatch_ByteNo(0x004E5738,0x0057F120950C8B,7);
								memoryPatch_ByteNo(0x004E566C,0x00512684158B,6); //상단에 표시하는 미네랄 가스 인구
								memoryPatch_ByteNo(0x004E56E0,0x00512684158B,6); //상단에 표시하는 미네랄 가스 인구
								memoryPatch_ByteNo(0x004E5709,0x00512684158B,6); //상단에 표시하는 미네랄 가스 인구
								memoryPatch_ByteNo(0x004E5732,0x00512684158B,6); //상단에 표시하는 미네랄 가스 인구
								memoryPatch_ByteNo(0x00493663,0x3375,2); //ObsMode 
								memoryPatch_ByteNo(0x004266FF,0x1075,2); //ObsMode 
								memoryPatch_ByteNo(0x00427271,0x1075,2); //ObsMode 에그
								memoryPatch_ByteNo(0x004278F5,0x2D75,2); //ObsMode 건설중 - UnitStatAct_
								memoryPatch_ByteNo(0x0042690C,0x1075,2); //ObsMode 
								memoryPatch_ByteNo(0x00427557,0x1975,2); //ObsMode 
								memoryPatch_ByteNo(0x00425F3D,0x1075,2); //ObsMode 
								memoryPatch_ByteNo(0x00426511,0x1075,2); //ObsMode 
								memoryPatch_ByteNo(0x00427557,0x1975,2); //ObsMode 
								memoryPatch_ByteNo(0x00426C85,0x850F,2); //ObsMode ??
								memoryPatch_ByteNo(0x00427D4B,0x4A75,2); //ObsMode 	
								memoryPatch_ByteNo(0x004273EF,0x1075,2); //ObsMode 아콘
								memoryPatch_ByteNo(0x004274B1,0x0C75,2); //ObsMode 캐리어 리버
								memoryPatch_ByteNo(0x00425B7C,0x4375,2); //ObsMode 
								memoryPatch_ByteNo(0x00427CA3,0x0C75,2); //ObsMode 
								memoryPatch_ByteNo(0x00427CAF,0x5B75,2); //ObsMode 드랍쉽 
								memoryPatch_ByteNo(0x00427CDB,0x9075,2); //ObsMode 드랍쉽 
								memoryPatch_ByteNo(0x00425B70,0x0C75,2); //ObsMode 할루시네이션 
								memoryPatch_ByteNo(0x00425B7E,0x0D8B,2); //ObsMode 할루시네이션 
								memoryPatch_ByteNo(0x00425A66,0x0875,2); //ObsMode 마나 
								memoryPatch_ByteNo(0x00459288,0x1074,2); //ObsMode 버튼 B60F
								memoryPatch_ByteNo(0x0045927F,0x1975,2); //ObsMode 버튼 B60F

								memoryPatch_ByteNo(0x00459255,0x0051268415B60F,7);		 //ObsMode 버튼셋 플레이어
								memoryPatch_ByteNo(0x0045925C,0x500C4FB60F66,6);		 //ObsMode 버튼셋 플레이어

								obMode = false;
							}
						}
						scbw::playSound(23);
						ifObs=true;
					} 
					checkF9 = true;
				}
			} else {
				ifObs = false;
				checkF9 = false;
			}
		}
		

	//실험건물 제한 시스템 이제 애드온방식 안쓰고 테크방식 씀
	for(int i = 0; i < PLAYER_COUNT; i++)
		setTechResearchState(i, TechId::UnusedTech26, canBuildtechBuilding[i]);

    // Loop through the bullet table.
    // Warning: There is no guarantee that the current bullet is actually a
    // bullet rather than an unused space in memory
    //for (int i = 0; i < BULLET_ARRAY_LENGTH; ++i) {
    //  BULLET* bullet = &bulletTable[i];
    //  //Write your code here
    //}

    // Alternative looping method
    // Guarantees that [bullet] points to an actual bullet.

	//탄환 루프문 시작
    for (CBullet* bullet = *firstBullet; bullet; bullet = bullet->next) {

		if(bullet->sourceUnit != nullptr){
			if(bullet->sourceUnit->id == UnitId::gerard_dugalle){
				if(bullet->unknown_0x4E == 0){
					bullet->unknown_0x4E = 1;
					u32 disToTar = scbw::getDistanceFast(bullet->targetPosition.x, bullet->targetPosition.y, bullet->sourceUnit->getX(), bullet->sourceUnit->getY()) << 7;
					u32 weaRange = bullet->sourceUnit->getMaxWeaponRange(bullet->sourceUnit->getGroundWeapon());
					u32 mulVal = disToTar / weaRange;
					bullet->flingyTopSpeed *= mulVal;
					bullet->flingyAcceleration *= mulVal;
					bullet->flingyTopSpeed >>= 7;
					bullet->flingyAcceleration >>= 7;
				}
				break;
			}
		}
		
		CUnit* targetUnit = bullet->attackTarget.unit;
		if(targetUnit != nullptr){
			switch(targetUnit->id) {
			case UnitId::corsair:
				//무기타입+1 해줘야 제대로 작동함
				if(bullet->attackTarget.unit->unusedTimer && bullet->behaviourTypeInternal == WeaponBehavior::Fly_FollowTarget+1){
					const MapSize map = *mapTileSize;

					if(scbw::randBetween(0, 1))
						bullet->targetPosition.x += std::min(scbw::randBetween(units_dat::UnitBounds[targetUnit->id].right+1, 64),
							(u32)(map.width<<8)-bullet->targetPosition.x-1);
					else
						bullet->targetPosition.x -= std::min(scbw::randBetween(units_dat::UnitBounds[targetUnit->id].left+1, 64), 
							(u32)bullet->targetPosition.x);

					if(scbw::randBetween(0, 1))
						bullet->targetPosition.y += std::min(scbw::randBetween(units_dat::UnitBounds[targetUnit->id].bottom+1, 64),
							(u32)(map.width<<8)-bullet->targetPosition.y-1);
					else
						bullet->targetPosition.y -= std::min(scbw::randBetween(units_dat::UnitBounds[targetUnit->id].top+1, 64), 
							(u32)bullet->targetPosition.y);

					bullet->moveTarget.pt.x = bullet->targetPosition2.x = bullet->attackTarget.pt.x = bullet->targetPosition.x;
					bullet->moveTarget.pt.y = bullet->targetPosition2.y = bullet->attackTarget.pt.y = bullet->targetPosition.y;
					bullet->moveTarget.unit = bullet->attackTarget.unit = nullptr;
				}
				break;
			}
		}

	}//탄환 루프문 끝

	//쉬는 일꾼이 있다면 표시해준다
	if (idleWorkerCount) {
		sprintf_s(buffer, sizeof(buffer), "Idle Worker: %d", idleWorkerCount);
		graphics::drawText(554, 16, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN); 
	}

	//매 프레임마다 항상 콘솔 업데이트
	scbw::refreshConsole();

	if (debugMode){
		if(debugMode == 2){
			if(*clientSelectionCount == 1 && localSelectionGroup[0]){
				sprintf_s(buffer, sizeof(buffer), "MainOrderId: %3d", localSelectionGroup[0]->mainOrderId);
				graphics::drawText(527, 40, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "secondaryOrderId: %3d", localSelectionGroup[0]->secondaryOrderId);
				graphics::drawText(495, 52, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "Status: 0x%08x", localSelectionGroup[0]->status);
				graphics::drawText(527, 64, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "orderTargetUnit: 0x%08x", localSelectionGroup[0]->orderTarget.unit);
				graphics::drawText(475, 76, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "connectedUnit: 0x%08x", localSelectionGroup[0]->connectedUnit);
				graphics::drawText(475, 88, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "currentBuildUnit: 0x%08x", localSelectionGroup[0]->currentBuildUnit);
				graphics::drawText(475, 100, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "addon: %3d", localSelectionGroup[0]->building.addon);
				graphics::drawText(500, 112, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "currentButtonSet: %3d", localSelectionGroup[0]->currentButtonSet);
				graphics::drawText(500, 124, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "orderSignal: %x", localSelectionGroup[0]->orderSignal);
				graphics::drawText(500, 136, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "visibilityStatus: 0x%08x", localSelectionGroup[0]->visibilityStatus);
				graphics::drawText(475, 148, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "MovementFlags: 0x%08x", units_dat::MovementFlags[localSelectionGroup[0]->id]);
				graphics::drawText(475, 160, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "currentDirection1: %3d", localSelectionGroup[0]->currentDirection1);
				graphics::drawText(500, 172, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "currentDirection2: %3d", localSelectionGroup[0]->currentDirection2);
				graphics::drawText(500, 184, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "velocityDirection1: %3d", localSelectionGroup[0]->velocityDirection1);
				graphics::drawText(500, 196, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "velocityDirection2: %3d", localSelectionGroup[0]->velocityDirection2);
				graphics::drawText(500, 208, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "groundWeaponCooldown: %2u", localSelectionGroup[0]->groundWeaponCooldown);
				graphics::drawText(475, 220, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "airWeaponCooldown: %2u", localSelectionGroup[0]->airWeaponCooldown);
				graphics::drawText(475, 232, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "unitAddress: 0x%08x", localSelectionGroup[0]);
				graphics::drawText(475, 244, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "OrderTargetXY: %05d, %05d", localSelectionGroup[0]->orderTarget.pt.x, localSelectionGroup[0]->orderTarget.pt.y);
				graphics::drawText(475, 256, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "AnimationEx: %2d", localSelectionGroup[0]->sprite->mainGraphic->animationEx);
				graphics::drawText(4, 0, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "iscriptHeaderOffsetEx: %2d", localSelectionGroup[0]->sprite->mainGraphic->iscriptHeaderOffsetEx);
				graphics::drawText(4, 12, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "iscriptOffsetEx: %2d", localSelectionGroup[0]->sprite->mainGraphic->iscriptOffsetEx);
				graphics::drawText(4, 24, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "frameSet: %2d", localSelectionGroup[0]->sprite->mainGraphic->frameSet);
				graphics::drawText(4, 36, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "frameIndex: %2d", localSelectionGroup[0]->sprite->mainGraphic->frameIndex);
				graphics::drawText(4, 48, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "horizontalOffset: %2d", localSelectionGroup[0]->sprite->mainGraphic->horizontalOffset);
				graphics::drawText(4, 60, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "verticalOffset: %2d", localSelectionGroup[0]->sprite->mainGraphic->verticalOffset);
				graphics::drawText(4, 72, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "mainGraphicId: %2d", localSelectionGroup[0]->sprite->mainGraphic->id);
				graphics::drawText(4, 84, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "paletteType: %2d", localSelectionGroup[0]->sprite->mainGraphic->paletteType);
				graphics::drawText(4, 96, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "direction: %2d", localSelectionGroup[0]->sprite->mainGraphic->direction);
				graphics::drawText(4, 108, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "mainGraphic flags: 0x%04x", localSelectionGroup[0]->sprite->mainGraphic->flags);
				graphics::drawText(4, 120, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "flags: 0x%04x", localSelectionGroup[0]->sprite->flags);
				graphics::drawText(4, 132, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "visibilityFlags: 0x%04x", localSelectionGroup[0]->sprite->visibilityFlags);
				graphics::drawText(4, 144, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "elevationLevel: %2d", localSelectionGroup[0]->sprite->elevationLevel);
				graphics::drawText(4, 156, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "playerId: %3d", localSelectionGroup[0]->sprite->playerId);
				graphics::drawText(4, 168, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "spriteId: %3d", localSelectionGroup[0]->sprite->spriteId);
				graphics::drawText(4, 180, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "position.x: %3d", localSelectionGroup[0]->sprite->position.x);
				graphics::drawText(4, 192, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "position.y: %3d", localSelectionGroup[0]->sprite->position.y);
				graphics::drawText(4, 204, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				if(localSelectionGroup[0]->subunit){
					sprintf_s(buffer, sizeof(buffer), "subunit->MainOrderId: %3d", localSelectionGroup[0]->subunit->mainOrderId);
					graphics::drawText(4, 216, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
					sprintf_s(buffer, sizeof(buffer), "subunit->secondaryOrderId: %3d", localSelectionGroup[0]->subunit->secondaryOrderId);
					graphics::drawText(4, 228, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
					sprintf_s(buffer, sizeof(buffer), "subunit->Status: %#08x", localSelectionGroup[0]->subunit->status);
					graphics::drawText(4, 240, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
					sprintf_s(buffer, sizeof(buffer), "subunit->orderTargetUnit: %3d", localSelectionGroup[0]->subunit->orderTarget.unit);
					graphics::drawText(4, 252, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
					sprintf_s(buffer, sizeof(buffer), "subunit->currentDirection1: %3d", localSelectionGroup[0]->subunit->currentDirection1);
					graphics::drawText(4, 264, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
					sprintf_s(buffer, sizeof(buffer), "subunit->groundWeaponCooldown: %2u", localSelectionGroup[0]->subunit->groundWeaponCooldown);
					graphics::drawText(4, 276, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
					sprintf_s(buffer, sizeof(buffer), "subunit->airWeaponCooldown: %2u", localSelectionGroup[0]->subunit->airWeaponCooldown);
					graphics::drawText(4, 288, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				}
			}
		}else if(*clientSelectionCount == 1){
			sprintf_s(buffer, sizeof(buffer), "bulletCount: %u", *bulletCount);
			graphics::drawText(500, 40, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
			sprintf_s(buffer, sizeof(buffer), "spriteCount: %u", *spriteCount);
			graphics::drawText(500, 52, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
			if(localSelectionGroup[0]){
				sprintf_s(buffer, sizeof(buffer), "iscriptHeaderOffsetEx: %2d", localSelectionGroup[0]->sprite->mainGraphic->iscriptHeaderOffsetEx);
				graphics::drawText(476, 64, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "iscriptOffsetEx: %2d", localSelectionGroup[0]->sprite->mainGraphic->iscriptOffsetEx);
				graphics::drawText(490, 76, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "unknown2Ex: %2d", localSelectionGroup[0]->sprite->mainGraphic->unknown2Ex);
				graphics::drawText(500, 88, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "animationEx: %2d", localSelectionGroup[0]->sprite->mainGraphic->animationEx);
				graphics::drawText(500, 100, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
				sprintf_s(buffer, sizeof(buffer), "waitEx: %2d", localSelectionGroup[0]->sprite->mainGraphic->waitEx);
				graphics::drawText(500, 112, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
			}
		}
		sprintf_s(buffer, sizeof(buffer), "Plugin Delay: %3dms", clock()-startClock);
		graphics::drawText(527, 28, buffer, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
	}

  scbw::setInGameLoopState(false);
  }

  return true;
}

bool gameOn() {
	firstRun = true;

  return true;
}

bool gameEnd() {
  shock = 0;
  power = 0;
  saveXY[0] = 0;
  saveXY[1] = 0;
  
	memoryPatch_ByteNo(0x004E575A,0x0057F0F0850C8B,7);
	memoryPatch_ByteNo(0x004E5738,0x0057F120950C8B,7);
	memoryPatch_ByteNo(0x004E566C,0x00512684158B,6); //상단에 표시하는 미네랄 가스 인구
	memoryPatch_ByteNo(0x004E56E0,0x00512684158B,6); //상단에 표시하는 미네랄 가스 인구
	memoryPatch_ByteNo(0x004E5709,0x00512684158B,6); //상단에 표시하는 미네랄 가스 인구
	memoryPatch_ByteNo(0x004E5732,0x00512684158B,6); //상단에 표시하는 미네랄 가스 인구
	memoryPatch_ByteNo(0x00493663,0x3375,2); //ObsMode 
	memoryPatch_ByteNo(0x004266FF,0x1075,2); //ObsMode 
	memoryPatch_ByteNo(0x00427271,0x1075,2); //ObsMode 에그
	memoryPatch_ByteNo(0x004278F5,0x2D75,2); //ObsMode 건설중 - UnitStatAct_
	memoryPatch_ByteNo(0x0042690C,0x1075,2); //ObsMode 
	memoryPatch_ByteNo(0x00427557,0x1975,2); //ObsMode 
	memoryPatch_ByteNo(0x00425F3D,0x1075,2); //ObsMode 
	memoryPatch_ByteNo(0x00426511,0x1075,2); //ObsMode 
	memoryPatch_ByteNo(0x00427557,0x1975,2); //ObsMode 
	memoryPatch_ByteNo(0x00426C85,0x850F,2); //ObsMode ??
	memoryPatch_ByteNo(0x00427D4B,0x4A75,2); //ObsMode 	
	memoryPatch_ByteNo(0x004273EF,0x1075,2); //ObsMode 아콘
	memoryPatch_ByteNo(0x004274B1,0x0C75,2); //ObsMode 캐리어 리버
	memoryPatch_ByteNo(0x00425B7C,0x4375,2); //ObsMode 
	memoryPatch_ByteNo(0x00427CA3,0x0C75,2); //ObsMode 
	memoryPatch_ByteNo(0x00427CAF,0x5B75,2); //ObsMode 드랍쉽 
	memoryPatch_ByteNo(0x00427CDB,0x9075,2); //ObsMode 드랍쉽 
	memoryPatch_ByteNo(0x00425B70,0x0C75,2); //ObsMode 할루시네이션 
	memoryPatch_ByteNo(0x00425B7E,0x0D8B,2); //ObsMode 할루시네이션 
	memoryPatch_ByteNo(0x00425A66,0x0875,2); //ObsMode 마나 
	memoryPatch_ByteNo(0x00459288,0x1074,2); //ObsMode 버튼 B60F
	memoryPatch_ByteNo(0x0045927F,0x1975,2); //ObsMode 버튼 B60F

	memoryPatch_ByteNo(0x00459255,0x0051268415B60F,7);		 //ObsMode 버튼셋 플레이어
	memoryPatch_ByteNo(0x0045925C,0x500C4FB60F66,6);		 //ObsMode 버튼셋 플레이어

	myunitEx = 0;
	obMode = false;
	

  return true;
}

} //hooks

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

const u32 Helper_CreateBullet = 0x0048C260;
void createBullet(u8 weaponId, const CUnit *source, s16 x, s16 y, u8 attackingPlayer, u8 direction) {
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
