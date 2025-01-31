/// Support for adding/removing psi fields. To use, you will also have to modify
/// the following hooks:
///
///   hooks/unit_destructor_special.cpp
///   hooks/unit_selection_graphics.cpp

#pragma once
#include "../SCBW/structures/CUnit.h"

namespace hooks {

/// Edit these functions in psi_field.cpp.
bool isReadyToMakePsiField(CUnit *unit);
bool canMakePsiField(u16 unitId);

//Call this in initialize.h.
void injectPsiFieldHooks();

} //hooks
