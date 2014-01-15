struct sfloat8 {
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;
    float g;
    float h;
};


__kernel void compiler_function_argument3(
struct sfloat8 f, __global struct sfloat8 *result)
{
  result[0].a = f.a;
  result[0].b = 12.0f;
  result[0].c = 12.0f;
  result[0].d = 12.0f;
  result[0].e = 12.0f;
  result[0].f = 12.0f;
  result[0].g = 12.0f;
  result[0].h = f.a + f.h;

  result[1].a = f.a;
  result[1].b = 12.0f;
  result[1].c = 12.0f;
  result[1].d = 12.0f;
  result[1].e = 12.0f;
  result[1].f = 12.0f;
  result[1].g = 12.0f;
  result[1].h = f.a + f.h;

  result[2].a = f.a;
  result[2].b = 12.0f;
  result[2].c = 12.0f;
  result[2].d = 12.0f;
  result[2].e = 12.0f;
  result[2].f = 12.0f;
  result[2].g = 12.0f;
  result[2].h = f.a + f.h;

  result[3].a = f.a;
  result[3].b = 12.0f;
  result[3].c = 12.0f;
  result[3].d = 12.0f;
  result[3].e = 12.0f;
  result[3].f = 12.0f;
  result[3].g = 12.0f;
  result[3].h = f.a + f.h;

  result[4].a = f.a;
  result[4].b = 12.0f;
  result[4].c = 12.0f;
  result[4].d = 12.0f;
  result[4].e = 12.0f;
  result[4].f = 12.0f;
  result[4].g = 12.0f;
  result[4].h = f.a + f.h;

  result[5].a = f.a;
  result[5].b = 12.0f;
  result[5].c = 12.0f;
  result[5].d = 12.0f;
  result[5].e = 12.0f;
  result[5].f = 12.0f;
  result[5].g = 12.0f;
  result[5].h = f.a + f.h;

  result[6] = result[0];
}
