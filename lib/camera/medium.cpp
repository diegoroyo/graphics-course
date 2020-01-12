#include "medium.h"

// default value for air medium,
// can be configured by overwriting the value in another module
MediumPtr Medium::air = Medium::create(1.0f);