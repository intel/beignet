#include "utest_helper.hpp"

static void test_copy_buf(size_t sz, size_t src_off, size_t dst_off, size_t cb)
{
    unsigned int i;
    OCL_MAP_BUFFER(0);

    for (i=0; i < sz; i++) {
        ((char*)buf_data[0])[i] = (rand() & 31);
    }

    OCL_UNMAP_BUFFER(0);

    OCL_MAP_BUFFER(1);

    for (i=0; i < sz; i++) {
        ((char*)buf_data[1])[i] = 64;
    }

    OCL_UNMAP_BUFFER(1);

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
    printf ("@@@@@@@@@ cb is %d\n", cb);
    printf ("@@@@@@@@@ src_off is %d\n", src_off);
    printf ("@@@@@@@@@ dst_off is %d\n", dst_off);
    printf("\n########### Src buffer: \n");
    for (i = 0; i < sz; ++i)
        printf(" %2.2u", ((unsigned char*)buf_data[0])[i]);

    printf("\n########### dst buffer: \n");
    for (i = 0; i < sz; ++i)
        printf(" %2.2u", ((unsigned char*)buf_data[1])[i]);
#endif

    // Check results
    for (i = 0; i < cb; ++i) {
        if (((char*)buf_data[0])[i +src_off] != ((char*)buf_data[1])[i + dst_off]) {
            printf ("different index is %d\n", i);
            OCL_ASSERT(0);
        }
    }

    for (i = 0; i < dst_off; ++i) {
        if (((char*)buf_data[1])[i] != 64) {
            printf ("wrong write, different index is %d\n", i);
            OCL_ASSERT(0);
        }
    }

    for (i = dst_off + cb; i < sz; ++i) {
        if (((char*)buf_data[1])[i] != 64) {
            printf ("wrong write, different index is %d\n", i);
            OCL_ASSERT(0);
        }
    }

    OCL_UNMAP_BUFFER(0);
    OCL_UNMAP_BUFFER(1);

}

void enqueue_copy_buf_unaligned(void)
{
    size_t i;
    size_t j;
    const size_t sz = 1024;
    int offset = 0;

    OCL_CREATE_BUFFER(buf[0], 0, sz * sizeof(char), NULL);
    OCL_CREATE_BUFFER(buf[1], 0, sz * sizeof(char), NULL);

#if 1
    /* Test the same offset cases. */
    for (i=0; i<sz; i+=32) {
        for (j=64; j<sz; j+=32) {
	    offset = (rand() & 3);
            test_copy_buf(sz, i + offset, j + offset, ((rand() & 31) + 1));
        }
    }
#endif

#if 1
    /* Test the dst small offset cases. */
    for (i=0; i<sz; i+=32) {
        for (j=64; j<sz; j+=32) {
	    offset = (rand() & 2);
            test_copy_buf(sz, i + offset + 1, j + offset, ((rand() & 31) + 1));
        }
    }
#endif

#if 1
    /* Test the dst big offset cases. */
    for (i=0; i<sz; i+=32) {
        for (j=64; j<sz; j+=32) {
	    offset = (rand() & 2);
            test_copy_buf(sz, i + offset, j + offset + 1, ((rand() & 31) + 1));
        }
    }
#endif
//            test_copy_buf(sz, 0, 1, 17);

}

MAKE_UTEST_FROM_FUNCTION(enqueue_copy_buf_unaligned);
