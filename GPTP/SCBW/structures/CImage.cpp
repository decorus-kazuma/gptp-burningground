#include "CImage.h"
#include "CSprite.h"
#include <hooks\interface\BG.h>
#include <SCBW/api.h>
#include <SCBW/scbwdata.h>
#include <cassert>

void CImage::playIscriptAnim(IscriptAnimation::Enum animation) {
  assert(this);
  u32 animation_ = (u8)animation;

  const u32 Func_PlayIscriptAnim = 0x004D8470;  //AKA playImageIscript();
  __asm {
    PUSHAD
    PUSH animation_
    MOV ECX, this
    CALL Func_PlayIscriptAnim
    POPAD
  }
}

void CImage::free() {
  assert(this);
  if (!(screenLayers->game.hasBeenRefreshed))
    scbw::refreshScreen(this->screenPosition.x,
                        this->screenPosition.y,
                        this->screenPosition.x + this->grpSize.right,
                        this->screenPosition.y + this->grpSize.bottom
                        );

  this->parentSprite->images.unlink<&CImage::link>(this);
  this->grpOffset = NULL;

  unusedImages.insertAfterHead(this);
}

//Loosely based on code at @ 0x004D5A50
void CImage::setRemapping(ColorRemapping::Enum remapping) {
  assert(this);
  this->coloringData = colorShiftData[remapping].data;
}

const u32 Func_SetImageDirection = 0x004D5F80;
void CImage::setDirection(u8 direction){
  assert(this);
  u32 direction_ = (u32)direction;

  __asm{
	  PUSHAD
	  PUSH		direction_
	  MOV		ESI, this
	  CALL		Func_SetImageDirection
	  POPAD
  }
}

//Identical to function @ 0x004D5A50
void CImage::initializeData(CSprite *parent, u16 imageId, s8 x, s8 y) {
  assert(this);
  this->id = imageId;
  this->grpOffset = iImagesGRPGraphic[imageId];
  this->flags = ((iImagesGFXTurns[imageId] & 1) << 3) | ((iImagesClickable[imageId] & 1) << 5);
  this->frameSet = 0;
  this->direction = 0;
  this->frameIndex = 0;
  this->parentSprite = parent;
  this->horizontalOffset = x;
  this->verticalOffset = y;
  this->grpSize = Box16();
  this->coloringData = 0;

  //Initialize iscript data
  this->iscriptHeaderOffset = 0;
  this->iscriptOffset = 0;
  this->unknown2[0] = 0;
  this->unknown2[1] = 0;
  this->animation = 0;
  this->wait = 0;
  this->iscriptHeaderOffsetEx = 0;
  this->iscriptOffsetEx = 0;
  this->unknown2Ex = 0;
  this->animationEx = 0;
  this->waitEx = 0;

  if (iGubImageRLE[imageId] == 14)
    *(u32*)(&this->coloringData) = parent->playerId;
  else if (iGubImageRLE[imageId] == 9)
	this->coloringData = colorShiftData[iGubImageColorShift[imageId]].data;
}