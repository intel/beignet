#ifndef __OCL_COMMON_H__
#define __OCL_COMMON_H__

#include "ocl_types.h"

/////////////////////////////////////////////////////////////////////////////
// Common Functions
/////////////////////////////////////////////////////////////////////////////
OVERLOADABLE float step(float edge, float x);
OVERLOADABLE float max(float a, float b);
OVERLOADABLE float min(float a, float b);
OVERLOADABLE float mix(float x, float y, float a);
OVERLOADABLE float clamp(float v, float l, float u);

OVERLOADABLE float degrees(float radians);
OVERLOADABLE float radians(float degrees);
OVERLOADABLE float smoothstep(float e0, float e1, float x);

OVERLOADABLE float sign(float x);
