#include "utest_helper.hpp"

static void test_copy_buf(size_t sz, size_t src_off, size_t dst_off, size_t cb)
{
    unsigned int i;
    OCL_MAP_BUFFER(0);

    for (i=0; i < sz; i++) {
        ((char*)buf_data[0])[i] = (rand() & 63);
    }

    OCL_UNMAP_BUFFER(0);

    if (src_off + cb > sz || dst_off + cb > sz) {
        /* Expect Error. */
        OCL_ASSERT(clEnqueueCopyBuffer(queue, buf[0], buf[1],
                                       src_off, dst_off, cb*sizeof(char), 0, NULL, NULL));
        return;
    }

    OCL_ASSERT(!clEnqueueCopyBuffer(queue, buf[0], buf[1],
                                    src_off, dst_off, cb*sizeof(char), 0, NULL, NULL));

    OCL_MAP_BUFFER(0);
    OCL_MAP_BUFFER(1);

#if 0
    printf("\n########### Src buffer: \n");
    for (i = 0; i < cb; ++i)
        printf(" %2.2u", ((unsigned char*)buf_data[0])[i + src_off]);

    printf("\n########### dst buffer: \n");
    for (i = 0; i < cb; ++i)
        printf(" %2.2u", ((unsigned char*)buf_data[1])[i + dst_off]);
#endif

    // Check results
    for (i = 0; i < cb; ++i) {
        if (((char*)buf_data[0])[i + src_off] != ((char*)buf_data[1])[i + dst_off]) {
            printf ("different index is %d\n", i);
            OCL_ASSERT(0);
        }
    }

    OCL_UNMAP_BUFFER(0);
    OCL_UNMAP_BUFFER(1);

}

void enqueue_copy_buf(void)
{
    size_t i;
    size_t j;
    const size_t sz = 1024;

    OCL_CREATE_BUFFER(buf[0], 0, sz * sizeof(char), NULL);
    OCL_CREATE_BUFFER(buf[1], 0, sz * sizeof(char), NULL);

    for (i=0; i<sz; i+=7) {
        for (j=0; j<sz; j+=10) {
            test_copy_buf(sz, i, j, sz/2);
        }
    }
}

MAKE_UTEST_FROM_FUNCTION(enqueue_copy_buf);
