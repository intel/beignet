/*
 * Copyright © 2014 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sys/alloc.cpp"
#include "sys/cvar.cpp"
#include "sys/assert.cpp"
#include "sys/platform.cpp"
#include "ir/constant.cpp"
#include "ir/printf.cpp"

#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"
#undef GBE_COMPILER_AVAILABLE
#include "backend/program.cpp"
#include "backend/gen_program.cpp"
#include "ir/sampler.cpp"
#include "ir/image.cpp"

struct BinInterpCallBackInitializer
{
  BinInterpCallBackInitializer() {
    gbe_program_new_from_binary = gbe::genProgramNewFromBinary;
    gbe_program_get_kernel_num = gbe::programGetKernelNum;
    gbe_program_get_kernel_by_name = gbe::programGetKernelByName;
    gbe_program_get_kernel = gbe::programGetKernel;
    gbe_kernel_get_code_size = gbe::kernelGetCodeSize;
    gbe_kernel_get_code = gbe::kernelGetCode;
    gbe_kernel_get_arg_num = gbe::kernelGetArgNum;
    gbe_kernel_get_curbe_size = gbe::kernelGetCurbeSize;
    gbe_kernel_get_sampler_size = gbe::kernelGetSamplerSize;
    gbe_kernel_get_compile_wg_size = gbe::kernelGetCompileWorkGroupSize;
    gbe_kernel_get_stack_size = gbe::kernelGetStackSize;
    gbe_kernel_get_image_size = gbe::kernelGetImageSize;
    gbe_kernel_get_name = gbe::kernelGetName;
    gbe_kernel_get_attributes = gbe::kernelGetAttributes;
    gbe_kernel_get_arg_type = gbe::kernelGetArgType;
    gbe_kernel_get_arg_size = gbe::kernelGetArgSize;
    gbe_kernel_get_arg_bti = gbe::kernelGetArgBTI;
    gbe_kernel_get_simd_width = gbe::kernelGetSIMDWidth;
    gbe_kernel_get_scratch_size = gbe::kernelGetScratchSize;
    gbe_kernel_use_slm = gbe::kernelUseSLM;
    gbe_kernel_get_required_work_group_size = gbe::kernelGetRequiredWorkGroupSize;
    gbe_kernel_get_curbe_offset = gbe::kernelGetCurbeOffset;
    gbe_kernel_get_slm_size = gbe::kernelGetSLMSize;
    gbe_kernel_get_arg_align = gbe::kernelGetArgAlign;
    gbe_program_get_global_constant_size = gbe::programGetGlobalConstantSize;
    gbe_program_delete = gbe::programDelete;
    gbe_program_get_global_constant_data = gbe::programGetGlobalConstantData;
    gbe_kernel_get_sampler_data = gbe::kernelGetSamplerData;
    gbe_kernel_get_image_data = gbe::kernelGetImageData;
    gbe_kernel_get_arg_info = gbe::kernelGetArgInfo;
    gbe_get_printf_num = gbe::kernelGetPrintfNum;
    gbe_get_printf_buf_bti = gbe::kernelGetPrintfBufBTI;
    gbe_get_printf_indexbuf_bti = gbe::kernelGetPrintfIndexBufBTI;
    gbe_dup_printfset = gbe::kernelDupPrintfSet;
    gbe_get_printf_sizeof_size = gbe::kernelGetPrintfSizeOfSize;
    gbe_release_printf_info = gbe::kernelReleasePrintfSet;
    gbe_output_printf = gbe::kernelOutputPrintf;
  }

  ~BinInterpCallBackInitializer() {
  }
};

static struct BinInterpCallBackInitializer binInterpCB;
