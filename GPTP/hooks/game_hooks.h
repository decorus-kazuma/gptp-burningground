#pragma once

namespace hooks {

#define BRONTES_ATTACKRADIUS 128

void setShock(unsigned char strength, unsigned short x, unsigned short y);

bool gameOn();
bool gameEnd();
bool nextFrame();

void injectGameHooks();

} //hooks
