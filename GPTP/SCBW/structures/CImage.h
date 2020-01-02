//Based on BWAPI's BW/CBullet.h

#pragma once
#include "common.h"
#include "CList.h"
#include <SCBW/enumerations.h>
#pragma pack(1)

struct CSprite;
struct GrpHead;

struct CImage {
//Utility method definitions added by pastelmind


  /// Makes the image play the specified Iscript animation entry.
  void playIscriptAnim(IscriptAnimation::Enum animation);

  /// AKA ImageDestructor() @ 0x004D4CE0
  /// Removes the image from the linked list of valid images. This is different
  /// from a class destructor, since CImage objects are stored in a static array
  /// and never truly destroyed.
  void free();

  /// Set the color remapping of this image to @ remapping.
  void setRemapping(ColorRemapping::Enum remapping);

  void setDirection(u8 direction);

  /// Resets most of the data member values. This is meant to be used in other
  /// functions, and should not be used by modders.
  void initializeData(CSprite *parent, u16 imageId, s8 x, s8 y);


////////////////////////////////////////////////////////////////
//Actual data structure
/*0x00*/  CLink<CImage> link;
/*0x08*/  u16         id;
/*0x0A*/  u8          paletteType;      //++Drawing function (values are the same as DataEdit)
                                // officially "ubRLE"
                                // RLE_SHADOW = 10
                                // RLE_HPFLOATDRAW = 11
                                // RLE_OUTLINE = 13
                                // RLE_PLAYER_SIDE = 14
/*0x0B*/  u8          direction;
/*0x0C*/  u16         flags;
              /*  0x0001  - Redraw
                  0x0002  - Don't update x? //thebest: 그래픽이 현재 미러상태인지의 여부(미러상태가 맞다면 2, 아니면 0) [가능한지의 여부가 아니다.]
                  0x0004  - Don't update y?
                  0x0008  - //thebest: 그래픽이 17방향을 사용하는지의 여부(17방향을 사용한다면 8, 아니면 0) [Datedit-Images 탭에서 Graphics Turns 체크 여부]
                  0x0010  - 
                  0x0020  - //thebest: 이 그래픽이 선택이 가능한지의 여부(가능하면 20, 아니면 0) [Datedit-Images 탭에서 Clickable 체크 여부]
                  0x0040  - Hidden/Invisible (don't draw)
                  0x0080  -
                  0x0100  -
                  0x0200  -
                  0x0400  -
                  0x0800  -
                  0x1000  -
                  0x2000  -
                  0x4000  -
                  0x8000  -
              */
/*0x0E*/  s8          horizontalOffset;
/*0x0F*/  s8          verticalOffset;
/*0x10*/  u16         iscriptHeaderOffset;
/*0x12*/  u16         iscriptOffset;
/*0x14*/  UNK         unknown2[2];
/*0x16*/  u8          animation;
/*0x17*/  u8          wait;
/*0x18*/  u16         frameSet;
/*0x1A*/  u16         frameIndex;
/*0x1C*/  Point16     mapPosition;
/*0x20*/  Point16     screenPosition;
/*0x24*/  Box16       grpSize;
/*0x2C*/  GrpHead     *grpOffset;
/*0x30*/  void*       coloringData;        //?
/*0x34*/  void*       renderFunction;
/*0x38*/  void*       updateFunction;
/*0x3C*/  CSprite*	  parentSprite;
/*0x40*/  u32         iscriptHeaderOffsetEx;
/*0x44*/  u32         iscriptOffsetEx;
/*0x48*/  u32         unknown2Ex;
/*0x4C*/  u8          animationEx;
/*0x4D*/  u8          waitEx;
};

static_assert(sizeof(CImage) == 78, "The size of the CImage structure is invalid");

struct IscriptEx{
	/*0x00*/  u32		iscriptHeaderOffsetEx;
	/*0x04*/  u32		iscriptOffsetEx;
	/*0x08*/  u32		unknown2Ex;
	/*0x0C*/  u8		animationEx;
	/*0x0D*/  u8		waitEx;
};

static_assert(sizeof(IscriptEx) == 14, "The size of the IscriptEx structure is invalid");

#pragma pack()
