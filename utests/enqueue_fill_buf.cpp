#include "utest_helper.hpp"
#include <string.h>

static char pattern_serials[128];

static void test_fill_buf(size_t sz, size_t offset, size_t size, size_t pattern_sz)
{
  unsigned int i;
  int ret = 0;
  OCL_MAP_BUFFER(0);
  memset(((char*)buf_data[0]), 0, sz);
  OCL_UNMAP_BUFFER(0);

  for (i=0; i < pattern_sz; i++) {
    pattern_serials[i] = (rand() & 63);
  }

  if (offset + size > sz) {
    /* Expect Error. */
    OCL_ASSERT(clEnqueueFillBuffer(queue, buf[0], pattern_serials,
                                   pattern_sz, offset, size, 0, NULL, NULL));
    return;
  }

  ret = clEnqueueFillBuffer(queue, buf[0], pattern_serials,
                            pattern_sz, offset, size, 0, NULL, NULL);
  OCL_ASSERT(!ret);

  OCL_MAP_BUFFER(0);

#if 0
  printf("\n==== pattern size is %d, offset is %d, size is %d ====\n",
         pattern_sz, offset, size);
  printf("\n###########  buffer: \n");
  for (i = 0; i < sz; ++i)
    printf(" %2.2u", ((unsigned char*)buf_data[0])[i]);

#endif

  // Check results
  int j = 0;
  for (i = 0; i < sz; ++i) {
    if (i < offset || i >= offset + size) {
      if (((char*)buf_data[0])[i] != 0) {
        printf ("\nnon zero index is %d\n", i);
        OCL_ASSERT(0);
      }
      continue;
    }

    if (((char*)buf_data[0])[i] != pattern_serials[j]) {
      printf ("\ndifferent index is %d\n", i);
      OCL_ASSERT(0);
    }
    j++;
    if (j == (int)pattern_sz) j = 0;
  }

  OCL_UNMAP_BUFFER(0);

}

void enqueue_fill_buf(void)
{
  size_t offset;
  size_t pattern_sz;
  const size_t sz = 1024;
  size_t size = 0;
  static int valid_sz[] = {1, 2, 4, 8, 16, 32, 64, 128};
  unsigned int i = 0;

  OCL_CREATE_BUFFER(buf[0], 0, sz * sizeof(char), NULL);

  for (i = 0; i < sizeof(valid_sz)/sizeof(int); i++) {

	pattern_sz = valid_sz[i];
	size = ((rand()%1024)/pattern_sz) * pattern_sz;
	offset = ((rand()%1024)/pattern_sz) * pattern_sz;
	while (size + offset + 1 > sz) {
      if (size > offset) {
        size = size - offset;
      } else
        offset = offset - size;
	}

	test_fill_buf(sz, offset, size, pattern_sz);
  }
}

MAKE_UTEST_FROM_FUNCTION(enqueue_fill_buf);
