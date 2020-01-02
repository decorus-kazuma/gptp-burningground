#pragma once
#include <SCBW/structures/CUnit.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

struct SightStruct {
  u32 tileSightWidth;
  u32 tileSightHeight;
  u32 unknown1;
  u32 unknown2;
  u32 unknown3;
  u32 unknown4;
  u32 unknown5;
};

static_assert(sizeof(SightStruct) == 28, "The size of the SightStruct structure is invalid");

const u32 mainBinOffset[] = {NULL, 0x004E17E0, 0x004E17E0, 0x004E17E0, 0x004E17E0, NULL, NULL, 0x004E1560, 
							0x004E1560, 0x00419190, 0x004E17E0, 0x004E17E0};

const u32 btnBinOffset[] = {0x004598D0, 0x004598D0, 0x004598D0, 0x004598D0, 0x004598D0, 0x004598D0, 0x004598D0, 0x004598D0, 
							0x004598D0, 0x004598D0, 0x004598D0, 0x004598D0, 0x004598D0, 0x004598D0, 0x004598D0, 0x00459890};

const u8 wireShieldColor[] = {2, 2, 0x1C, 2, 0x1B, 0x1C, 0x1A, 0x1B, 0x19, 0x1A, 0x18, 0x19, 0x17, 0x18 ,0x16, 0x17, 0x15, 0x16};
const u8 hpBarShieldColor[] = {18, 19, 20, 18};
const u32 statDataBinOffset[] = {0x00456A50, 0x00457F30, 0x00457F30, 0x00457F30, 0x00457F30, 0x00457F30, 0x004581E0, 0x00456EC0, 
								0x00457F30, 0x00457F30, 0x00457F30, 0x00457F30, 0x004581E0, 0x004581E0, 0x00457F30, 0x004581E0, 
								0x00457F30, 0x00457E90, 0x00457E90, 0x00457E90, 0x00457E90, 0x00457E90, 0x00457E90, 0x00457E90, 
								0x00457E90, 0x00457E90, 0x00457E90, 0x00457E90, 0x00457E90, 0x00457E90, 0x00457E90, 0x00457E90, 
								0x004583E0, 0x004583E0, 0x004583E0, 0x004583E0, 0x004583E0, 0x004583E0, 0x004583E0, 0x004583E0, 
								0x004583E0, 0x004583E0, 0x004583E0, 0x004583E0, 0x004583E0, 0x004583E0, 0x004583E0, 0x004583E0, 
								0x004583E0, 0x004583E0, 0x004583E0, 0x004583E0, 0x004583E0, 0x004583E0, 0x004583E0, 0x004583E0};

struct Sound{
	s32	ddUnknown0;
	s16	dwUnknown1;
	s8	dbUnknown2;
	s8	dbUnknown3;
	s8	dbUnknown4[4];
	s32	ddUnknown5;
	s32	ddUnknown6;
	s32	ddUnknown7;
};

static_assert(sizeof(Sound) == 24, "The size of the Sound structure is invalid");

const u32 maxCBullet = BULLET_ARRAY_LENGTH;	//기본 100
const u16 maxCBullet_16 = (u16)maxCBullet;	//기본 100
//const int maxCSprite = 8192;				//기본 2500
//const int maxSprtieHeadTail = 16383;		//기본 256
//const int maxCThingy = 32767;				//기본 2500
const u32 maxCImage = 5000;					//기본 5000
//const int maxCUnit = 1700;				//기본 1700
const u32 maxSound = 128;					//기본 8
//const u8 maxSelection = 18;				//기본 12, 6의 배수여야 하며, 최대 60
const u8 minimumSound = 28;					//기본 50, 최대 55

const int maxExtendCheat = 2;//최대 14개
const u32 extendCheatHashTable[maxExtendCheat][2] = {{0xA87CF579, 0x1C55AC27},  //1. the best overseer
													{0xAAAB0D6C, 0x82F5A3DA}};  //2. I am the crazy bastard around here

namespace hooks {

void inject_BG();

} //hooks