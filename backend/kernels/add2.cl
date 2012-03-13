#include <stdlib.h>
struct big{
  unsigned int a, b;
};

__kernel struct big add(unsigned int x, unsigned int y)
{
  struct big p;
  p.a = x + y;
  p.b = x - y + 10;
  return p;
}

