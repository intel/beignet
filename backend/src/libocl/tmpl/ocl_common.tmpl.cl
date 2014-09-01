#include "ocl_common.h"
#include "ocl_float.h"

/////////////////////////////////////////////////////////////////////////////
// Common Functions
/////////////////////////////////////////////////////////////////////////////
PURE CONST float __gen_ocl_fmax(float a, float b);
PURE CONST float __gen_ocl_fmin(float a, float b);

OVERLOADABLE float step(float edge, float x) {
  return x < edge ? 0.0 : 1.0;
}

OVERLOADABLE float max(float a, float b) {
  return __gen_ocl_fmax(a, b);
}
OVERLOADABLE float min(float a, float b) {
  return __gen_ocl_fmin(a, b);
}
OVERLOADABLE float mix(float x, float y, float a) {
  return x + (y-x)*a;
}
OVERLOADABLE float clamp(float v, float l, float u) {
  return max(min(v, u), l);
}


OVERLOADABLE float degrees(float radians) {
  return (180 / M_PI_F) * radians;
}
OVERLOADABLE float radians(float degrees) {
  return (M_PI_F / 180) * degrees;
}

OVERLOADABLE float smoothstep(float e0, float e1, float x) {
  x = clamp((x - e0) / (e1 - e0), 0.f, 1.f);
  return x * x * (3 - 2 * x);
}

OVERLOADABLE float sign(float x) {
  if(x > 0)
    return 1;
  if(x < 0)
    return -1;
  if(x == -0.f)
    return -0.f;
  return 0.f;
}
