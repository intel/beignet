inline int offset(int x, int y, int width) { return width*y + x; }
inline float mapX(float x) {return x*3.25f - 2.f;}
inline float mapY(float y) {return y*2.5f - 1.25f;}

__kernel void compiler_mandelbrot_alternate(__global uint *out,
                                            float rcpWidth,
                                            float rcpHeight,
                                            float criterium)
{
  int xDim = get_global_id(0);
  int yDim = get_global_id(1);
  int width = get_global_size(0);
  int height = get_global_size(1);
  int idx = offset(xDim, yDim, width);

  float xOrigin = mapX((float) xDim * rcpWidth);
  float yOrigin = mapY((float) yDim * rcpHeight);
  float x = 0.0f;
  float y = 0.0f;

  float iteration = 256.f;

  bool breakCond = true;
  while (breakCond) {
    const float xtemp = mad(-y,y,mad(x,x,xOrigin));
    y = mad(2.f*x, y, yOrigin);
    x = xtemp;
    iteration -= 1.f;
    breakCond = -mad(y,y,mad(x,x, -criterium)) * iteration > 0.f;
  }

  const uint iIteration = 256 - (uint) iteration;
  const uint isBlack = (iIteration == 256);
  const uint black = 255 << 24;
  const uint nonBlack = iIteration | (iIteration << 8) | (iIteration << 16) | (255 << 24);
  out[idx] = select(nonBlack, black, isBlack);
}

