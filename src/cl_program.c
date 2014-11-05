/*
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "cl_kernel.h"
#include "cl_program.h"
#include "cl_device_id.h"
#include "cl_context.h"
#include "cl_alloc.h"
#include "cl_utils.h"
#include "cl_khr_icd.h"
#include "cl_gbe_loader.h"
#include "CL/cl.h"
#include "CL/cl_intel.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>

static void
cl_program_release_sources(cl_program p)
{
  if (p->source) {
    cl_free(p->source);
    p->source = NULL;
  }
}

static void
cl_program_release_binary(cl_program p)
{
  if (p->binary) {
    cl_free(p->binary);
    p->binary = NULL;
  }
}

LOCAL void
cl_program_delete(cl_program p)
{
  uint32_t ref, i;

  if (p == NULL)
    return;

  /* We are not done with it yet */
  if ((ref = atomic_dec(&p->ref_n)) > 1) return;

  /* Destroy the sources and binary if still allocated */
  cl_program_release_sources(p);
  cl_program_release_binary(p);

  /* Release the build options. */
  if (p->build_opts) {
    cl_free(p->build_opts);
    p->build_opts = NULL;
  }

  if (p->build_log) {
    free(p->build_log);
    p->build_log = NULL;
  }

  /* Remove it from the list */
  assert(p->ctx);
  pthread_mutex_lock(&p->ctx->program_lock);
    if (p->prev)
      p->prev->next = p->next;
    if (p->next)
      p->next->prev = p->prev;
    if (p->ctx->programs == p)
      p->ctx->programs = p->next;
  pthread_mutex_unlock(&p->ctx->program_lock);

  cl_free(p->bin);               /* Free the blob */
  for (i = 0; i < p->ker_n; ++i) /* Free the kernels */
    cl_kernel_delete(p->ker[i]);
  cl_free(p->ker);

  /* Program belongs to their parent context */
  cl_context_delete(p->ctx);

  /* Free the program as allocated by the compiler */
  if (p->opaque) {
    if (CompilerSupported())
      compiler_program_clean_llvm_resource(p->opaque);
    interp_program_delete(p->opaque);
  }

  p->magic = CL_MAGIC_DEAD_HEADER; /* For safety */
  cl_free(p);
}

LOCAL cl_program
cl_program_new(cl_context ctx)
{
  cl_program p = NULL;

  /* Allocate the structure */
  TRY_ALLOC_NO_ERR (p, CALLOC(struct _cl_program));
  SET_ICD(p->dispatch)
  p->build_status = CL_BUILD_NONE;
  p->ref_n = 1;
  p->magic = CL_MAGIC_PROGRAM_HEADER;
  p->ctx = ctx;
  p->build_log = calloc(1000, sizeof(char));
  if (p->build_log)
    p->build_log_max_sz = 1000;
  /* The queue also belongs to its context */
  cl_context_add_ref(ctx);

exit:
  return p;
error:
  cl_program_delete(p);
  goto exit;
}

LOCAL void
cl_program_add_ref(cl_program p)
{
  assert(p);
  atomic_inc(&p->ref_n);
}

static cl_int
cl_program_load_gen_program(cl_program p)
{
  cl_int err = CL_SUCCESS;
  uint32_t i;

  assert(p->opaque != NULL);
  p->ker_n = interp_program_get_kernel_num(p->opaque);

  /* Allocate the kernel array */
  TRY_ALLOC (p->ker, CALLOC_ARRAY(cl_kernel, p->ker_n));

  for (i = 0; i < p->ker_n; ++i) {
    const gbe_kernel opaque = interp_program_get_kernel(p->opaque, i);
    assert(opaque != NULL);
    TRY_ALLOC (p->ker[i], cl_kernel_new(p));
    cl_kernel_setup(p->ker[i], opaque);
  }

error:
  return err;
}

inline cl_bool isBitcodeWrapper(const unsigned char *BufPtr, const unsigned char *BufEnd)
{
  // See if you can find the hidden message in the magic bytes :-).
  // (Hint: it's a little-endian encoding.)
  return BufPtr != BufEnd &&
    BufPtr[0] == 0xDE &&
    BufPtr[1] == 0xC0 &&
    BufPtr[2] == 0x17 &&
    BufPtr[3] == 0x0B;
}

inline cl_bool isRawBitcode(const unsigned char *BufPtr, const unsigned char *BufEnd)
{
  // These bytes sort of have a hidden message, but it's not in
  // little-endian this time, and it's a little redundant.
  return BufPtr != BufEnd &&
    BufPtr[0] == 'B' &&
    BufPtr[1] == 'C' &&
    BufPtr[2] == 0xc0 &&
    BufPtr[3] == 0xde;
}

#define isBitcode(BufPtr,BufEnd)  (isBitcodeWrapper(BufPtr, BufEnd) || isRawBitcode(BufPtr, BufEnd))

LOCAL cl_program
cl_program_create_from_binary(cl_context             ctx,
                              cl_uint                num_devices,
                              const cl_device_id *   devices,
                              const size_t *         lengths,
                              const unsigned char ** binaries,
                              cl_int *               binary_status,
                              cl_int *               errcode_ret)
{
  cl_program program = NULL;
  cl_int err = CL_SUCCESS;

  assert(ctx);
  INVALID_DEVICE_IF (num_devices != 1);
  INVALID_DEVICE_IF (devices == NULL);
  INVALID_DEVICE_IF (devices[0] != ctx->device);
  INVALID_VALUE_IF (binaries == NULL);
  INVALID_VALUE_IF (lengths == NULL);

  if (binaries[0] == NULL) {
    err = CL_INVALID_VALUE;
    if (binary_status)
      binary_status[0] = CL_INVALID_VALUE;
    goto error;
  }

  if (lengths[0] == 0) {
    err = CL_INVALID_VALUE;
    if (binary_status)
      binary_status[0] = CL_INVALID_VALUE;
    goto error;
  }

  program = cl_program_new(ctx);

  // TODO:  Need to check the binary format here to return CL_INVALID_BINARY.
  TRY_ALLOC(program->binary, cl_calloc(lengths[0], sizeof(char)));
  memcpy(program->binary, binaries[0], lengths[0]);
  program->binary_sz = lengths[0];
  program->source_type = FROM_BINARY;

  if(isBitcode((unsigned char*)program->binary+1, (unsigned char*)program->binary+program->binary_sz)) {
    if(*program->binary == 1){
      program->binary_type = CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT;
    }else if(*program->binary == 2){
      program->binary_type = CL_PROGRAM_BINARY_TYPE_LIBRARY;
    }else{
      err= CL_INVALID_BINARY;
      goto error;
    }
    program->opaque = compiler_program_new_from_llvm_binary(program->ctx->device->vendor_id, program->binary, program->binary_sz);

    if (UNLIKELY(program->opaque == NULL)) {
      err = CL_INVALID_PROGRAM;
      goto error;
    }
    program->source_type = FROM_LLVM;
  }
  else if (*program->binary == 0) {
    program->opaque = interp_program_new_from_binary(program->ctx->device->vendor_id, program->binary, program->binary_sz);
    if (UNLIKELY(program->opaque == NULL)) {
      err = CL_INVALID_PROGRAM;
      goto error;
    }

    /* Create all the kernels */
    TRY (cl_program_load_gen_program, program);
    program->binary_type = CL_PROGRAM_BINARY_TYPE_EXECUTABLE;
  }

  if (binary_status)
    binary_status[0] = CL_SUCCESS;

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return program;
error:
  cl_program_delete(program);
  program = NULL;
  goto exit;

  return CL_SUCCESS;
}

LOCAL cl_program
cl_program_create_with_built_in_kernles(cl_context     ctx,
                                  cl_uint              num_devices,
                                  const cl_device_id * devices,
                                  const char *         kernel_names,
                                  cl_int *             errcode_ret)
{
  cl_int err = CL_SUCCESS;

  assert(ctx);
  INVALID_DEVICE_IF (num_devices != 1);
  INVALID_DEVICE_IF (devices == NULL);
  INVALID_DEVICE_IF (devices[0] != ctx->device);

  cl_int binary_status = CL_SUCCESS;
  extern char cl_internal_built_in_kernel_str[];
  extern size_t cl_internal_built_in_kernel_str_size;
  char* p_built_in_kernel_str =cl_internal_built_in_kernel_str;

  ctx->built_in_prgs = cl_program_create_from_binary(ctx, 1,
                                                          &ctx->device,
                                                          (size_t*)&cl_internal_built_in_kernel_str_size,
                                                          (const unsigned char **)&p_built_in_kernel_str,
                                                          &binary_status, &err);
  if (!ctx->built_in_prgs)
    return NULL;

  err = cl_program_build(ctx->built_in_prgs, NULL);
  if (err != CL_SUCCESS)
    return NULL;

  ctx->built_in_prgs->is_built = 1;

  char delims[] = ";";
  char* saveptr = NULL;
  char* local_kernel_names;
  char* kernel = NULL;
  char* matched_kernel;
  int i = 0;

  //copy the content to local_kernel_names to protect the kernel_names.
  TRY_ALLOC(local_kernel_names, cl_calloc(strlen(kernel_names)+1, sizeof(char) ) );
  memcpy(local_kernel_names, kernel_names, strlen(kernel_names)+1);

  kernel = strtok_r( local_kernel_names, delims , &saveptr);
  while( kernel != NULL ) {
    matched_kernel = strstr(ctx->device->built_in_kernels, kernel);
    if(matched_kernel){
      for (i = 0; i < ctx->built_in_prgs->ker_n; ++i) {
        assert(ctx->built_in_prgs->ker[i]);
        const char *ker_name = cl_kernel_get_name(ctx->built_in_prgs->ker[i]);
        if (strcmp(ker_name, kernel) == 0) {
          break;
        }
      }

      ctx->built_in_kernels[i] = cl_program_create_kernel(ctx->built_in_prgs, kernel, NULL);
    }
    kernel = strtok_r((char*)saveptr , delims, &saveptr );
  }

  cl_free(local_kernel_names);

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return ctx->built_in_prgs;
error:
  goto exit;

  return CL_SUCCESS;
}

LOCAL cl_program
cl_program_create_from_llvm(cl_context ctx,
                            cl_uint num_devices,
                            const cl_device_id *devices,
                            const char *file_name,
                            cl_int *errcode_ret)
{
  cl_program program = NULL;
  cl_int err = CL_SUCCESS;

  assert(ctx);
  INVALID_DEVICE_IF (num_devices != 1);
  INVALID_DEVICE_IF (devices == NULL);
  INVALID_DEVICE_IF (devices[0] != ctx->device);
  INVALID_VALUE_IF (file_name == NULL);

  program = cl_program_new(ctx);
  program->opaque = compiler_program_new_from_llvm(ctx->device->vendor_id, file_name, NULL, NULL, program->build_log_max_sz, program->build_log, &program->build_log_sz, 1);
  if (UNLIKELY(program->opaque == NULL)) {
    err = CL_INVALID_PROGRAM;
    goto error;
  }

  /* Create all the kernels */
  TRY (cl_program_load_gen_program, program);
  program->source_type = FROM_LLVM;

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return program;
error:
  cl_program_delete(program);
  program = NULL;
  goto exit;
}

LOCAL cl_program
cl_program_create_from_source(cl_context ctx,
                              cl_uint count,
                              const char **strings,
                              const size_t *lengths,
                              cl_int *errcode_ret)

{
  cl_program program = NULL;
  cl_int err = CL_SUCCESS;
  cl_uint i;
  int32_t * lens = NULL;
  int32_t len_total = 0;
  assert(ctx);
  char * p = NULL;
  // the real compilation step will be done at build time since we do not have
  // yet the compilation options
  program = cl_program_new(ctx);
  TRY_ALLOC (lens, cl_calloc(count, sizeof(int32_t)));
  for (i = 0; i < (int) count; ++i) {
    size_t len;
    if (lengths == NULL || lengths[i] == 0)
      len = strlen(strings[i]);
    else
      len = lengths[i];
    lens[i] = len;
    len_total += len;
  }
  TRY_ALLOC(program->source, cl_calloc(len_total+1, sizeof(char)));
  p = program->source;
  for (i = 0; i < (int) count; ++i) {
    memcpy(p, strings[i], lens[i]);
    p += lens[i];
  }
  *p = '\0';

  program->source_type = FROM_SOURCE;
  program->binary_type = CL_PROGRAM_BINARY_TYPE_NONE;

exit:
  cl_free(lens);
  lens = NULL;
  if (errcode_ret)
    *errcode_ret = err;
  return program;
error:
  cl_program_delete(program);
  program = NULL;
  goto exit;
}

/* Before we do the real work, we need to check whether our platform
   cl version can meet -cl-std= */
static int check_cl_version_option(cl_program p, const char* options) {
  const char* s = NULL;
  int ver1 = 0;
  int ver2 = 0;
  char version_str[64];

  if (options && (s = strstr(options, "-cl-std="))) {

    if (s + strlen("-cl-std=CLX.X") > options + strlen(options)) {
      return 0;
    }

    if (s[8] != 'C' || s[9] != 'L' || s[10] > '9' || s[10] < '0' || s[11] != '.'
        || s[12] > '9' || s[12] < '0') {
      return 0;
    }

    ver1 = (s[10] - '0') * 10 + (s[12] - '0');

    if (cl_get_device_info(p->ctx->device, CL_DEVICE_OPENCL_C_VERSION, sizeof(version_str),
                                  version_str, NULL) != CL_SUCCESS)
      return 0;

    assert(strstr(version_str, "OpenCL") && version_str[0] == 'O');
    ver2 = (version_str[9] - '0') * 10 + (version_str[11] - '0');

    if (ver2 < ver1)
      return 0;

    return 1;
  }

  return 1;
}

LOCAL cl_int
cl_program_build(cl_program p, const char *options)
{
  cl_int err = CL_SUCCESS;
  int i = 0;
  int copyed = 0;

  if (p->ref_n > 1) {
    err = CL_INVALID_OPERATION;
    goto error;
  }

  if (!check_cl_version_option(p, options)) {
    err = CL_BUILD_PROGRAM_FAILURE;
    goto error;
  }
  if (options) {
    if(p->build_opts == NULL || strcmp(options, p->build_opts) != 0) {
      if(p->build_opts) {
        cl_free(p->build_opts);
        p->build_opts = NULL;
      }
      TRY_ALLOC (p->build_opts, cl_calloc(strlen(options) + 1, sizeof(char)));
      memcpy(p->build_opts, options, strlen(options));

      p->source_type = p->source ? FROM_SOURCE : p->binary ? FROM_BINARY : FROM_LLVM;
    }
  }

  if (options == NULL && p->build_opts) {
    p->source_type = p->source ? FROM_SOURCE : p->binary ? FROM_BINARY : FROM_LLVM;

    cl_free(p->build_opts);
    p->build_opts = NULL;
  }

  if (p->source_type == FROM_SOURCE) {
    if (!CompilerSupported()) {
      err = CL_COMPILER_NOT_AVAILABLE;
      goto error;
    }

    p->opaque = compiler_program_new_from_source(p->ctx->device->vendor_id, p->source, p->build_log_max_sz, options, p->build_log, &p->build_log_sz);
    if (UNLIKELY(p->opaque == NULL)) {
      if (p->build_log_sz > 0 && strstr(p->build_log, "error: error reading 'options'"))
        err = CL_INVALID_BUILD_OPTIONS;
      else
        err = CL_BUILD_PROGRAM_FAILURE;
      goto error;
    }

    /* Create all the kernels */
    TRY (cl_program_load_gen_program, p);
  } else if (p->source_type == FROM_LLVM) {
    if (!CompilerSupported()) {
      err = CL_COMPILER_NOT_AVAILABLE;
      goto error;
    }

    compiler_program_build_from_llvm(p->opaque, p->build_log_max_sz, p->build_log, &p->build_log_sz, options);
    if (UNLIKELY(p->opaque == NULL)) {
      if (p->build_log_sz > 0 && strstr(p->build_log, "error: error reading 'options'"))
        err = CL_INVALID_BUILD_OPTIONS;
      else
        err = CL_BUILD_PROGRAM_FAILURE;
      goto error;
    }
    /* Create all the kernels */
    TRY (cl_program_load_gen_program, p);
  } else if (p->source_type == FROM_BINARY) {
    p->opaque = interp_program_new_from_binary(p->ctx->device->vendor_id, p->binary, p->binary_sz);
    if (UNLIKELY(p->opaque == NULL)) {
      err = CL_BUILD_PROGRAM_FAILURE;
      goto error;
    }

    /* Create all the kernels */
    TRY (cl_program_load_gen_program, p);
  }
  p->binary_type = CL_PROGRAM_BINARY_TYPE_EXECUTABLE;

  for (i = 0; i < p->ker_n; i ++) {
    const gbe_kernel opaque = interp_program_get_kernel(p->opaque, i);
    p->bin_sz += interp_kernel_get_code_size(opaque);
  }

  TRY_ALLOC (p->bin, cl_calloc(p->bin_sz, sizeof(char)));
  for (i = 0; i < p->ker_n; i ++) {
    const gbe_kernel opaque = interp_program_get_kernel(p->opaque, i);
    size_t sz = interp_kernel_get_code_size(opaque);

    memcpy(p->bin + copyed, interp_kernel_get_code(opaque), sz);
    copyed += sz;
  }
  p->is_built = 1;
  p->build_status = CL_BUILD_SUCCESS;
  return CL_SUCCESS;

error:
  p->build_status = CL_BUILD_ERROR;
  return err;
}

cl_program
cl_program_link(cl_context            context,
                cl_uint               num_input_programs,
                const cl_program *    input_programs,
                const char *          options,
                cl_int*               errcode_ret)
{
  cl_program p = NULL;
  cl_int err = CL_SUCCESS;
  cl_int i = 0;
  int copyed = 0;
  p = cl_program_new(context);

  if (!check_cl_version_option(p, options)) {
    err = CL_BUILD_PROGRAM_FAILURE;
    goto error;
  }

  p->opaque = compiler_program_new_gen_program(context->device->vendor_id, NULL, NULL);

  for(i = 0; i < num_input_programs; i++) {
    // if program create with llvm binary, need deserilize first to get module.
    if(input_programs[i])
      compiler_program_link_program(p->opaque, input_programs[i]->opaque,
        p->build_log_max_sz, p->build_log, &p->build_log_sz);
    if (UNLIKELY(p->opaque == NULL)) {
      err = CL_LINK_PROGRAM_FAILURE;
      goto error;
    }
  }

  if(options && strstr(options, "-create-library")){
    p->binary_type = CL_PROGRAM_BINARY_TYPE_LIBRARY;
    goto done;
  }else{
    p->binary_type = CL_PROGRAM_BINARY_TYPE_EXECUTABLE;
  }

  compiler_program_build_from_llvm(p->opaque, p->build_log_max_sz, p->build_log, &p->build_log_sz, options);

  /* Create all the kernels */
  TRY (cl_program_load_gen_program, p);

  for (i = 0; i < p->ker_n; i ++) {
    const gbe_kernel opaque = interp_program_get_kernel(p->opaque, i);
    p->bin_sz += interp_kernel_get_code_size(opaque);
  }

  TRY_ALLOC (p->bin, cl_calloc(p->bin_sz, sizeof(char)));
  for (i = 0; i < p->ker_n; i ++) {
    const gbe_kernel opaque = interp_program_get_kernel(p->opaque, i);
    size_t sz = interp_kernel_get_code_size(opaque);

    memcpy(p->bin + copyed, interp_kernel_get_code(opaque), sz);
    copyed += sz;
  }
done:
  p->is_built = 1;
  p->build_status = CL_BUILD_SUCCESS;
  if (errcode_ret)
    *errcode_ret = err;
  return p;

error:
  p->build_status = CL_BUILD_ERROR;
  if (errcode_ret)
    *errcode_ret = err;
  return p;
}

LOCAL cl_int
cl_program_compile(cl_program            p,
                   cl_uint               num_input_headers,
                   const cl_program *    input_headers,
                   const char **         header_include_names,
                   const char*           options)
{
  cl_int err = CL_SUCCESS;
  int i = 0;

  if (p->ref_n > 1) {
    err = CL_INVALID_OPERATION;
    goto error;
  }

  if (!check_cl_version_option(p, options)) {
    err = CL_BUILD_PROGRAM_FAILURE;
    goto error;
  }

  if (options) {
    if(p->build_opts == NULL || strcmp(options, p->build_opts) != 0) {
      if(p->build_opts) {
        cl_free(p->build_opts);
        p->build_opts = NULL;
      }
      TRY_ALLOC (p->build_opts, cl_calloc(strlen(options) + 1, sizeof(char)));
      memcpy(p->build_opts, options, strlen(options));

      p->source_type = p->source ? FROM_SOURCE : p->binary ? FROM_BINARY : FROM_LLVM;
    }
  }

  if (options == NULL && p->build_opts) {
    p->source_type = p->source ? FROM_SOURCE : p->binary ? FROM_BINARY : FROM_LLVM;

    cl_free(p->build_opts);
    p->build_opts = NULL;
  }

  char temp_header_template[]= "/tmp/beignet.XXXXXX";
  char* temp_header_path = mkdtemp(temp_header_template);

  if (p->source_type == FROM_SOURCE) {

    if (!CompilerSupported()) {
      err = CL_COMPILER_NOT_AVAILABLE;
      goto error;
    }

    //write the headers to /tmp/beignet.XXXXXX for include.
    for (i = 0; i < num_input_headers; i++) {
      if(header_include_names[i] == NULL || input_headers[i] == NULL)
        continue;

      char temp_path[255]="";
      strncpy(temp_path, temp_header_path, strlen(temp_header_path));
      strncat(temp_path, "/", 1);
      strncat(temp_path, header_include_names[i], strlen(header_include_names[i]));
      char* dirc = strdup(temp_path);
      char* dir = dirname(dirc);
      mkdir(dir, 0755);
      if(access(dir, R_OK|W_OK) != 0){
        err = CL_COMPILE_PROGRAM_FAILURE;
        goto error;
      }
      free(dirc);

      FILE* pfile = fopen(temp_path, "wb");
      if(pfile){
        fwrite(input_headers[i]->source, strlen(input_headers[i]->source), 1, pfile);
        fclose(pfile);
      }else{
        err = CL_COMPILE_PROGRAM_FAILURE;
        goto error;
      }
    }

    p->opaque = compiler_program_compile_from_source(p->ctx->device->vendor_id, p->source, temp_header_path,
        p->build_log_max_sz, options, p->build_log, &p->build_log_sz);

    char rm_path[255]="rm ";
    strncat(rm_path, temp_header_path, strlen(temp_header_path));
    strncat(rm_path, " -rf", 4);
    int temp = system(rm_path);

    if(temp){
      assert(0);
    }

    if (UNLIKELY(p->opaque == NULL)) {
      if (p->build_log_sz > 0 && strstr(p->build_log, "error: error reading 'options'"))
        err = CL_INVALID_BUILD_OPTIONS;
      else
        err = CL_BUILD_PROGRAM_FAILURE;
      goto error;
    }

    /* Create all the kernels */
    p->source_type = FROM_LLVM;
    p->binary_type = CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT;
  }else if(p->source_type == FROM_BINARY){
    err = CL_INVALID_OPERATION;
    return err;
  }

  p->is_built = 1;
  p->build_status = CL_BUILD_SUCCESS;
  return CL_SUCCESS;

error:
  p->build_status = CL_BUILD_ERROR;
  cl_program_delete(p);
  p = NULL;
  return err;
}

LOCAL cl_kernel
cl_program_create_kernel(cl_program p, const char *name, cl_int *errcode_ret)
{
  cl_kernel from = NULL, to = NULL;
  cl_int err = CL_SUCCESS;
  uint32_t i = 0;

  /* Find the program first */
  for (i = 0; i < p->ker_n; ++i) {
    assert(p->ker[i]);
    const char *ker_name = cl_kernel_get_name(p->ker[i]);
    if (strcmp(ker_name, name) == 0) {
      from = p->ker[i];
      break;
    }
  }

  /* We were not able to find this named kernel */
  if (UNLIKELY(from == NULL)) {
    err = CL_INVALID_KERNEL_NAME;
    goto error;
  }

  TRY_ALLOC(to, cl_kernel_dup(from));

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return to;
error:
  cl_kernel_delete(to);
  to = NULL;
  goto exit;
}

LOCAL cl_int
cl_program_create_kernels_in_program(cl_program p, cl_kernel* ker)
{
  int i = 0;

  if(ker == NULL)
    return CL_SUCCESS;

  for (i = 0; i < p->ker_n; ++i) {
    TRY_ALLOC_NO_ERR(ker[i], cl_kernel_dup(p->ker[i]));
  }

  return CL_SUCCESS;

error:
  do {
    cl_kernel_delete(ker[i]);
    ker[i--] = NULL;
  } while(i > 0);

  return CL_OUT_OF_HOST_MEMORY;
}

LOCAL void
cl_program_get_kernel_names(cl_program p, size_t size, char *names, size_t *size_ret)
{
  int i = 0;
  const char *ker_name = NULL;
  size_t len = 0;
  if(size_ret) *size_ret = 0;

  if(p->ker == NULL) {
    return;
  }

  ker_name = cl_kernel_get_name(p->ker[i]);
  len = strlen(ker_name);
  if(names) {
    strncpy(names, cl_kernel_get_name(p->ker[0]), size - 1);
    if(size < len - 1) {
      if(size_ret) *size_ret = size;
      return;
    }
    size = size - len - 1;  //sub \0
  }
  if(size_ret) *size_ret = strlen(ker_name) + 1;  //add NULL

  for (i = 1; i < p->ker_n; ++i) {
    ker_name = cl_kernel_get_name(p->ker[i]);
    len = strlen(ker_name);
    if(names) {
      strncat(names, ";", size);
      if(size >= 1)
        strncat(names, ker_name, size - 1);
      if(size < len + 1) {
        if(size_ret) *size_ret = size;
        break;
      }
      size = size - len - 1;
    }
    if(size_ret) *size_ret += len + 1; //add ';'
  }
}
