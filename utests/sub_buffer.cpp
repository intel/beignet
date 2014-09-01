#include "utest_helper.hpp"

void sub_buffer_check(void)
{
    cl_int error;
    cl_ulong max_alloc_size;
    cl_uint address_align;
    cl_mem main_buf;
    cl_mem sub_buf;
    char *main_buf_content;
    char sub_buf_content[32];

    error = clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(max_alloc_size), &max_alloc_size, NULL);
    OCL_ASSERT(error == CL_SUCCESS);
    error = clGetDeviceInfo(device, CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(address_align ), &address_align, NULL );
    OCL_ASSERT(error == CL_SUCCESS);

    main_buf_content = (char *)malloc(sizeof(char) * max_alloc_size);

    for (cl_ulong i = 0; i < max_alloc_size; i++) {
        main_buf_content[i] = rand() & 63;
    }

    main_buf = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, max_alloc_size, main_buf_content, &error);
    OCL_ASSERT(error == CL_SUCCESS);

    /* Test read sub buffer. */
    for (cl_ulong sz = 64; sz < max_alloc_size; sz*=4) {
        for (cl_ulong off = 0; off < max_alloc_size; off += 1234) {
            cl_buffer_region region;
            region.origin = off;
            region.size = sz;

            sub_buf = clCreateSubBuffer(main_buf, 0, CL_BUFFER_CREATE_TYPE_REGION, &region, &error );

            /* invalid size, should be failed. */
            if(off + sz > max_alloc_size) {
                OCL_ASSERT(error != CL_SUCCESS);
                continue;
            }
            /* invalid align, should be failed. */
            if(off & ((address_align/8)-1)) {
                OCL_ASSERT(error != CL_SUCCESS);
                continue;
            }

            OCL_ASSERT(error == CL_SUCCESS);

            error = clEnqueueReadBuffer(queue, sub_buf, CL_TRUE, 0, 32, (void *)sub_buf_content, 0, NULL, NULL);
            OCL_ASSERT(error == CL_SUCCESS);

#if 0
            printf("\nRead ########### Src buffer: \n");
            for (int i = 0; i < 32; ++i)
                printf(" %2.2u", main_buf_content[off + i]);

            printf("\nRead ########### dst buffer: \n");
            for (int i = 0; i < 32; ++i)
                printf(" %2.2u", sub_buf_content[i]);
            printf("\n");
#endif
            for (int i = 0; i < 32; ++i) {

                if (main_buf_content[off + i] != sub_buf_content[i]) {
                    printf ("different index is %d\n", i);
                    OCL_ASSERT(0);
                }
            }

        }
    }


    for (cl_ulong sz = 64; sz < max_alloc_size; sz*=4) {
        for (cl_ulong off = 0; off < max_alloc_size; off += 1234) {
            cl_buffer_region region;
            region.origin = off;
            region.size = sz;

            sub_buf = clCreateSubBuffer(main_buf, 0, CL_BUFFER_CREATE_TYPE_REGION, &region, &error );

            /* invalid size, should be failed. */
            if(off + sz > max_alloc_size) {
                OCL_ASSERT(error != CL_SUCCESS);
                continue;
            }
            /* invalid align, should be failed. */
            if(off & (address_align/8-1)) {
                OCL_ASSERT(error != CL_SUCCESS);
                continue;
            }

            OCL_ASSERT(error == CL_SUCCESS);

            for (int i = 0; i < 32; i++) {
                sub_buf_content[i] = rand() & 63;
            }

            error = clEnqueueWriteBuffer(queue, main_buf, CL_TRUE, off, 32, sub_buf_content, 0, NULL, NULL);
            OCL_ASSERT(error == CL_SUCCESS);

            void * mapped_ptr = clEnqueueMapBuffer(queue, sub_buf, CL_TRUE, (cl_map_flags)( CL_MAP_READ | CL_MAP_WRITE ),
                    0, 32, 0, NULL, NULL, &error );
            OCL_ASSERT(error == CL_SUCCESS);

#if 0
            printf("\nMap ########### Src buffer: \n");
            for (int i = 0; i < 32; ++i)
                printf(" %2.2u", sub_buf_content[i]);

            printf("\nMap ########### dst buffer: \n");
            for (int i = 0; i < 32; ++i)
                printf(" %2.2u", ((char *)mapped_ptr)[i]);
            printf("\n");
#endif
            for (int i = 0; i < 32; i++) {

                if (((char *)mapped_ptr)[i] != sub_buf_content[i]) {
                    printf ("different index is %d\n", i);
                    OCL_ASSERT(0);
                }
            }

            error = clEnqueueUnmapMemObject(queue, sub_buf, mapped_ptr, 0, NULL, NULL );
            OCL_ASSERT(error == CL_SUCCESS);

            clReleaseMemObject(sub_buf);
        }
    }

    clReleaseMemObject(main_buf);
    free(main_buf_content);
}

MAKE_UTEST_FROM_FUNCTION(sub_buffer_check);
