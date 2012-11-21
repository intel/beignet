#define X(x, y) x ## y
#define NAME(x, y) X(x, y)
#define S struct NAME(s, __LINE__) { \
  char c;  \
  int i;   \
  float f; \
}

S __attribute__((aligned(16)));
S __attribute__((aligned));
S __attribute__((packed));
S __attribute__((endian(host)));
S __attribute__((endian(device)));
S __attribute__((endian));

__kernel void compiler_structure_attributes() {
}
