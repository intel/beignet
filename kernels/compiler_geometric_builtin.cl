kernel void compiler_geometric_builtin() {
  float x = 1, y = 2, z = 3;
  z = dot(x, y);
  z = cross(x, y);
  z = distance(x, y);
  z = length(x);
  z = normalize(x);
  z = fast_distance(x, y);
  z = fast_length(x, y);
  z = fast_normalize(x);
}
