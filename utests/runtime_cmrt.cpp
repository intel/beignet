/*
this test case shows how to execute CM kernel via OpenCL APIs.
the CM kernel source code is already compiled into file "cmrt_utest_genx.isa" with offline compiler.

I also copied the CM kernel source code and CM host source code here for your reference.

CM kernel source code:
#include <cm/cm.h>
extern "C" _GENX_MAIN_  void
simplemov(SurfaceIndex ibuf, SurfaceIndex obuf, uint d)
{
    matrix<uchar, 1, 4> in;
    matrix<uchar, 1, 4> out;

	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	read(ibuf, h_pos*4, v_pos, in);

	out = in / d;
	write(obuf, h_pos*4, v_pos, out);
}

CM host source code:
#include "cm_rt.h"

int main()
{
    FILE* pISA = fopen("cmrt_utest_genx.isa", "rb");
    if (pISA == NULL) {
        perror("cmrt_utest_genx.isa");
        return -1;
    }

    fseek (pISA, 0, SEEK_END);
    int codeSize = ftell (pISA);
    rewind(pISA);

    if(codeSize == 0)
    {
        perror("cmrt_utest_genx.isa");
        return -1;
    }

    void *pCommonISACode = (BYTE*) malloc(codeSize);
    if( !pCommonISACode )
    {
        return -1;
    }

    if (fread(pCommonISACode, 1, codeSize, pISA) != codeSize) {
        perror("cmrt_utest_genx.isa");
        return -1;
    }
    fclose(pISA);

    unsigned int  width = 256;
    unsigned int  height = 128;

    unsigned char *src;
    unsigned char *dst;
    src = (unsigned char*) malloc(width*height*4);
    dst = (unsigned char*) malloc(width*height*4);

    for (unsigned int i = 0; i < width*height*4; i++) {
        src[i] = i % 256;
        dst[i] = 0;
    }

    CmDevice* pCmDev = NULL;;
    UINT version = 0;

    int result = CreateCmDevice( pCmDev, version );
    if (result != CM_SUCCESS ) {
        printf("CmDevice creation error");
        return -1;
    }
    if( version < CM_1_0 ){
        printf(" The runtime API version is later than runtime DLL version");
        return -1;
    }

    CmProgram* program = NULL;
    result = pCmDev->LoadProgram(pCommonISACode, codeSize, program);
    if (result != CM_SUCCESS ) {
        perror("CM LoadProgram error");
        return -1;
    }

    CmKernel* kernel = NULL;
    result = pCmDev->CreateKernel(program, CM_KERNEL_FUNCTION(simplemov) , kernel);
    if (result != CM_SUCCESS ) {
        perror("CM CreateKernel error");
        return -1;
    }

    CmSurface2D*  pInputSurf = NULL;
    result = pCmDev->CreateSurface2D( width, height, CM_SURFACE_FORMAT_A8R8G8B8, pInputSurf );
    if (result != CM_SUCCESS ) {
        printf("CM CreateSurface2D error");
        return -1;
    }

    CmSurface2D*  pOutputSurf = NULL;
    result = pCmDev->CreateSurface2D( width, height, CM_SURFACE_FORMAT_A8R8G8B8, pOutputSurf );
    if (result != CM_SUCCESS ) {
        printf("CM CreateSurface2D error");
        return -1;
    }

    result = pInputSurf->WriteSurface( src, NULL );
    if (result != CM_SUCCESS ) {
        printf("CM WriteSurface error");
        return -1;
    }

    kernel->SetThreadCount( width * height );

    CmThreadSpace* pTS = NULL;
    result = pCmDev->CreateThreadSpace(width, height, pTS);
    if (result != CM_SUCCESS ) {
        printf("CM WriteSurface error");
        return -1;
    }

    SurfaceIndex * index0= NULL;
    pInputSurf->GetIndex(index0);
    kernel->SetKernelArg(0,sizeof(SurfaceIndex),index0);

    SurfaceIndex * index1= NULL;
    pOutputSurf->GetIndex(index1);
    kernel->SetKernelArg(1,sizeof(SurfaceIndex),index1);

	unsigned int  d = 3;
	kernel->SetKernelArg(2, sizeof(unsigned int), &d);

    CmQueue* pCmQueue = NULL;
    result = pCmDev->CreateQueue( pCmQueue );
    if (result != CM_SUCCESS ) {
        perror("CM CreateQueue error");
        return -1;
    }

    CmTask *pKernelArray = NULL;

    result = pCmDev->CreateTask(pKernelArray);
    if (result != CM_SUCCESS ) {
        printf("CmDevice CreateTask error");
        return -1;
    }

    result = pKernelArray-> AddKernel (kernel);
    if (result != CM_SUCCESS ) {
        printf("CmDevice AddKernel error");
        return -1;
    }

    CmEvent* e = NULL;
    result = pCmQueue->Enqueue(pKernelArray, e, pTS);
    if (result != CM_SUCCESS ) {
        printf("CmDevice enqueue error");
        return -1;
    }

    pCmDev->DestroyTask(pKernelArray);
    result = pOutputSurf->ReadSurface( dst, e );
    if (result != CM_SUCCESS ) {
        printf("CM ReadSurface error");
        return -1;
    }

	for (unsigned int i = 0; i < width*height*4; i++) {
        if (src[i] / d != dst[i]) {
			printf("test failed at %d, expected %d, got %d\n", i, src[i]/d, dst[i]);
			return -1;
		}
    }

	printf("test passed!\n");

    result = DestroyCmDevice( pCmDev );

    free(pCommonISACode);
    free(src);
    free(dst);

    return 0;
}

*/

#include "utest_helper.hpp"
#include "utest_file_map.hpp"
#include <string.h>

void runtime_cmrt(void)
{
  uint32_t w = 256;
  uint32_t h = 128;
  cl_int status;
  cl_int binary_status;
  char *ker_path = NULL;

  cl_file_map_t *fm = cl_file_map_new();
  ker_path = cl_do_kiss_path("cmrt_utest_genx.isa", NULL);
  OCL_ASSERT (cl_file_map_open(fm, ker_path) == CL_FILE_MAP_SUCCESS);

  const unsigned char *kbin = (const unsigned char *)cl_file_map_begin(fm);
  const size_t sz = cl_file_map_size(fm);

  program = clCreateProgramWithBinary(ctx, 1,
            &device, &sz, &kbin, &binary_status, &status);

  OCL_ASSERT(program && status == CL_SUCCESS);

  /* OCL requires to build the program even if it is created from a binary */
  OCL_ASSERT(clBuildProgram(program, 1, &device, NULL, NULL, NULL) == CL_SUCCESS);

  kernel = clCreateKernel(program, "simplemov", &status);
  OCL_ASSERT(status == CL_SUCCESS);


  cl_image_format format;
  cl_image_desc desc;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  format.image_channel_order = CL_BGRA;
  format.image_channel_data_type = CL_UNORM_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_row_pitch = 0;

  OCL_CREATE_IMAGE(buf[0], 0, &format, &desc, NULL);
  OCL_CREATE_IMAGE(buf[1], 0, &format, &desc, NULL);

  OCL_MAP_BUFFER_GTT(0);
  OCL_MAP_BUFFER_GTT(1);
  uint8_t* src = (uint8_t*)buf_data[0];
  uint8_t* dst = (uint8_t*)buf_data[1];
  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w*4; i++) {
      src[j * w * 4 + i] = i;
      dst[j * w * 4 + i] = 0;
    }
  OCL_UNMAP_BUFFER_GTT(0);
  OCL_UNMAP_BUFFER_GTT(1);

  unsigned int d = 3;
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(unsigned int), &d);
  globals[0] = w;
  globals[1] = h;

  //if kernel uses get_origin_thread_x/y, locals must be NULL to invoke pCmQueue->Enqueue
  //if kernel uses cm_linear_global_id, locals must be not NULL to invoke pCmQueue->EnqueueWithGroup
  OCL_CALL (clEnqueueNDRangeKernel, queue, kernel, 2, NULL, globals, NULL, 0, NULL, NULL);

  OCL_MAP_BUFFER_GTT(0);
  OCL_MAP_BUFFER_GTT(1);
  src = (uint8_t*)buf_data[0];
  dst = (uint8_t*)buf_data[1];
  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w*4; i++) {
      OCL_ASSERT(src[j * w * 4 + i] / d == dst[j * w * 4 + i]);
    }
  OCL_UNMAP_BUFFER_GTT(0);
  OCL_UNMAP_BUFFER_GTT(1);
}

MAKE_UTEST_FROM_FUNCTION(runtime_cmrt);
