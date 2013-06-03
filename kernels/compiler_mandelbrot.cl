// Used to ID into the 1D array, so that we can use
// it effectively as a 2D array
inline int ID(int x, int y, int width) { return 4*width*y + x*4; }
inline float mapX(float x) { return x*3.25f - 2.f; }
inline float mapY(float y) { return y*2.5f - 1.25f; }

__kernel void compiler_mandelbrot(__global char *out) {
  int x_dim = get_global_id(0);
  int y_dim = get_global_id(1);
  int width = get_global_size(0);
  int height = get_global_size(1);
  int idx = ID(x_dim, y_dim, width);

  float x_origin = mapX((float) x_dim / (float) width);
  float y_origin = mapY((float) y_dim / (float) height);

  // The Escape time algorithm, it follows the pseduocode from Wikipedia
  // _very_ closely
  float x = 0.0f;
  float y = 0.0f;

  int iteration = 0;

  // This can be changed, to be more or less precise
  int max_iteration = 256;
  while(x*x + y*y <= 4 && iteration < max_iteration) {
    float xtemp = x*x - y*y + x_origin;
    y = 2*x*y + y_origin;
    x = xtemp;
    iteration++;
  }

  if(iteration == max_iteration) {
    // This coordinate did not escape, so it is in the Mandelbrot set
    out[idx] = 0;
    out[idx + 1] = 0;
    out[idx + 2] = 0;
    out[idx + 3] = 255;
  } else {
    // This coordinate did escape, so color based on quickly it escaped
    out[idx] = iteration;
    out[idx + 1] = iteration;
    out[idx + 2] = iteration;
    out[idx + 3] = 255;
  }

}
