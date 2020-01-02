#ifndef CORONA_OPEN_H
#define CORONA_OPEN_H


#include "corona.h"


namespace corona {
  Image* OpenPCX (File* file); // OpenPCX.cpp
#ifndef NO_PNG
  Image* OpenPNG (File* file); // OpenPNG.cpp
#endif
}


#endif
