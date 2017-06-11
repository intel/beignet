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
 */

#include "cl_gen.h"

static int
cl_check_builtin_kernel_dimension(cl_kernel kernel, cl_device_id device)
{
  const char *n = kernel->name;
  const char *builtin_kernels_2d = "__cl_copy_image_2d_to_2d;__cl_copy_image_2d_to_buffer;"
                                   "__cl_copy_buffer_to_image_2d;__cl_fill_image_2d;"
                                   "__cl_fill_image_2d_array;";
  const char *builtin_kernels_3d = "__cl_copy_image_3d_to_2d;__cl_copy_image_2d_to_3d;"
                                   "__cl_copy_image_3d_to_3d;__cl_copy_image_3d_to_buffer;"
                                   "__cl_copy_buffer_to_image_3d;__cl_fill_image_3d";
  if (n == NULL || !strstr(device->built_in_kernels, n)) {
    return 0;
  } else if (strstr(builtin_kernels_2d, n)) {
    return 2;
  } else if (strstr(builtin_kernels_3d, n)) {
    return 3;
  } else
    return 1;
}

LOCAL size_t
cl_kernel_get_max_workgroup_size_gen(cl_kernel kernel, cl_device_id device)
{
  size_t work_group_size, thread_cnt;
  cl_kernel_gen kernel_gen;
  DEV_PRIVATE_DATA(kernel, device, kernel_gen);

  cl_uint simd_width = kernel_gen->simd_width;
  cl_uint local_mem_size = kernel_gen->local_mem_size;
  cl_int device_id = device->device_id;
  cl_bool use_local_mem = CL_FALSE;

  if (local_mem_size)
    use_local_mem = CL_TRUE;

  if (use_local_mem == CL_FALSE) {
    int i = 0;
    for (; i < kernel->arg_n; i++) {
      if (kernel->args[i].arg_type == ArgTypePointer &&
          kernel->args[i].arg_addrspace == AddressSpaceLocal) {
        use_local_mem = CL_TRUE;
        break;
      }
    }
  }

  if (use_local_mem == CL_FALSE) {
    if (!IS_BAYTRAIL_T(device_id) || simd_width == 16)
      work_group_size = simd_width * 64;
    else
      work_group_size = device->max_compute_unit *
                        device->max_thread_per_unit * simd_width;
  } else {
    thread_cnt = device->max_compute_unit * device->max_thread_per_unit /
                 device->sub_slice_count;
    if (thread_cnt > 64)
      thread_cnt = 64;
    work_group_size = thread_cnt * simd_width;
  }

  if (work_group_size > device->max_work_group_size)
    work_group_size = device->max_work_group_size;

  return work_group_size;
}

LOCAL void
cl_kernel_delete_gen(cl_device_id device, cl_kernel kernel)
{
  cl_kernel_gen kernel_gen = NULL;
  DEV_PRIVATE_DATA(kernel, device, kernel_gen);

  if (kernel_gen->samper_info) {
    CL_FREE(kernel_gen->samper_info);
    kernel_gen->samper_info = NULL;
  }
  if (kernel_gen->arg_extra_info) {
    CL_FREE(kernel_gen->arg_extra_info);
    kernel_gen->arg_extra_info = NULL;
  }
  if (kernel_gen->virt_reg_phy_offset) {
    CL_FREE(kernel_gen->virt_reg_phy_offset);
    kernel_gen->virt_reg_phy_offset = NULL;
  }
  if (kernel_gen->image_info) {
    CL_FREE(kernel_gen->image_info);
    kernel_gen->image_info = NULL;
  }

  CL_FREE(kernel_gen);
}

LOCAL cl_int
cl_kernel_get_info_gen(cl_device_id device, cl_kernel kernel, cl_uint param_name, void *param_value)
{
  cl_kernel_gen kernel_gen;
  DEV_PRIVATE_DATA(kernel, device, kernel_gen);

  if (param_name == CL_KERNEL_WORK_GROUP_SIZE) {
    *(size_t *)param_value = cl_kernel_get_max_workgroup_size_gen(kernel, device);
    return CL_SUCCESS;
  } else if (param_name == CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE) {
    *(size_t *)param_value = kernel_gen->simd_width;
    return CL_SUCCESS;
  } else if (param_name == CL_KERNEL_PRIVATE_MEM_SIZE) {
    *(size_t *)param_value = kernel_gen->stack_size;
    return CL_SUCCESS;
  } else if (param_name == CL_KERNEL_LOCAL_MEM_SIZE) {
    *(size_t *)param_value = kernel_gen->local_mem_size;
    return CL_SUCCESS;
  } else if (param_name == CL_KERNEL_GLOBAL_WORK_SIZE) {
    int dimension = cl_check_builtin_kernel_dimension(kernel, device);
    if (!dimension)
      return CL_INVALID_VALUE;

    if (dimension == 1) {
      memcpy(param_value, device->max_1d_global_work_sizes, 3 * sizeof(size_t));
      return CL_SUCCESS;
    } else if (dimension == 2) {
      memcpy(param_value, device->max_2d_global_work_sizes, 3 * sizeof(size_t));
      return CL_SUCCESS;
    } else if (dimension == 3) {
      memcpy(param_value, device->max_3d_global_work_sizes, 3 * sizeof(size_t));
      return CL_SUCCESS;
    } else
      return CL_INVALID_VALUE;
  } else if (param_name == CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE_KHR) {
    *(size_t *)param_value = kernel_gen->simd_width;
    return CL_SUCCESS;
  }

  return CL_INVALID_VALUE;
}

static cl_int
cl_program_gen_get_kernel_func_cl_info(cl_device_id device, cl_kernel kernel)
{
  cl_program prog = kernel->program;
  cl_program_gen prog_gen;
  cl_kernel_gen kernel_gen;
  cl_int offset;
  void *desc;
  void *ptr;
  cl_char *name;
  cl_uint name_size;
  cl_uint desc_size;
  cl_uint desc_type;
  cl_uint wg_sz_size;
  cl_uint attr_size;
  cl_uint arg_info_size;
  int i;
  char *arg_type_qual_str;
  char *arg_access_qualifier_str;

  DEV_PRIVATE_DATA(prog, device, prog_gen);
  DEV_PRIVATE_DATA(kernel, device, kernel_gen);

  assert(kernel->name);

  if (prog_gen->func_cl_info == NULL)
    return CL_SUCCESS;

  offset = 0;
  desc = NULL;
  while (offset < prog_gen->func_cl_info_data->d_size) {
    name_size = *(cl_uint *)(prog_gen->func_cl_info_data->d_buf + offset);
    desc_size = *(cl_uint *)(prog_gen->func_cl_info_data->d_buf + offset + sizeof(cl_uint));
    desc_type = *(cl_uint *)(prog_gen->func_cl_info_data->d_buf + offset + 2 * sizeof(cl_uint));
    name = prog_gen->func_cl_info_data->d_buf + offset + sizeof(cl_uint) * 3;

    if (desc_type != GEN_NOTE_TYPE_CL_INFO) {
      offset += 3 * sizeof(cl_uint) + ALIGN(name_size, 4) + ALIGN(desc_size, 4);
      continue;
    }

    if (strcmp((char *)name, (char *)kernel->name) == 0) { // Find the kernel info slot
      desc = prog_gen->func_cl_info_data->d_buf + offset + sizeof(cl_uint) * 3 + ALIGN(name_size, 4);
      break;
    }

    offset += 3 * sizeof(cl_uint) + ALIGN(name_size, 4) + ALIGN(desc_size, 4);
  }

  if (desc == NULL)
    return CL_SUCCESS;

  ptr = desc;
  attr_size = *(cl_uint *)ptr;
  ptr += sizeof(cl_uint);
  wg_sz_size = *(cl_uint *)ptr;
  ptr += sizeof(cl_uint);
  arg_info_size = *(cl_uint *)ptr;
  ptr += sizeof(cl_uint);

  if (attr_size) {
    if (kernel->kernel_attr && strcmp(kernel->kernel_attr, ptr) != 0)
      return CL_INVALID_KERNEL_DEFINITION;

    if (kernel->kernel_attr == NULL) {
      kernel->kernel_attr = CL_MALLOC(strlen(ptr) + 1);
      if (kernel->kernel_attr == NULL)
        return CL_OUT_OF_HOST_MEMORY;
      memcpy(kernel->kernel_attr, ptr, strlen(ptr) + 1);
    }

    ptr += ALIGN((strlen(kernel->kernel_attr) + 1), 4);
  }

  if (wg_sz_size) {
    for (i = 0; i < 3; i++) {
      if (kernel->compile_wg_sz[i] > 0) {
        if (kernel->compile_wg_sz[i] != *(size_t *)ptr)
          return CL_INVALID_KERNEL_DEFINITION;
      } else {
        kernel->compile_wg_sz[i] = *(size_t *)ptr;
      }
      ptr += sizeof(size_t);
    }
  }

  if (arg_info_size) {
    cl_uint arg_type_qualifier = 0;
    cl_uint arg_access_qualifier = 0;

    for (i = 0; i < kernel->arg_n; i++) {
      if (kernel->args[i].arg_type_name) {
        if (strcmp(kernel->args[i].arg_type_name, ptr) != 0)
          return CL_INVALID_KERNEL_DEFINITION;
      } else {
        kernel->args[i].arg_type_name = CL_MALLOC(strlen(ptr) + 1);
        if (kernel->args[i].arg_type_name == NULL)
          return CL_OUT_OF_HOST_MEMORY;
        memcpy(kernel->args[i].arg_type_name, ptr, strlen(ptr) + 1);
      }
      ptr += strlen(ptr) + 1;

      arg_access_qualifier_str = ptr;
      ptr += strlen(ptr) + 1;

      arg_type_qual_str = ptr;
      ptr += strlen(ptr) + 1;

      if (kernel->args[i].arg_name) {
        if (strcmp(kernel->args[i].arg_name, ptr) != 0)
          return CL_INVALID_KERNEL_DEFINITION;
      } else {
        kernel->args[i].arg_name = CL_MALLOC(strlen(ptr) + 1);
        if (kernel->args[i].arg_name == NULL)
          return CL_OUT_OF_HOST_MEMORY;
        memcpy(kernel->args[i].arg_name, ptr, strlen(ptr) + 1);
      }
      ptr += strlen(ptr) + 1;

      ptr = (void *)(ALIGN((long)ptr, 4));

      if (!strcmp(arg_access_qualifier_str, "write_only")) {
        arg_access_qualifier = CL_KERNEL_ARG_ACCESS_WRITE_ONLY;
      } else if (!strcmp(arg_access_qualifier_str, "read_only")) {
        arg_access_qualifier = CL_KERNEL_ARG_ACCESS_READ_ONLY;
      } else if (!strcmp(arg_access_qualifier_str, "read_write")) {
        arg_access_qualifier = CL_KERNEL_ARG_ACCESS_READ_WRITE;
      } else {
        arg_access_qualifier = CL_KERNEL_ARG_ACCESS_NONE;
      }
      if (kernel->args[i].arg_access_qualifier > 0) {
        if (kernel->args[i].arg_access_qualifier != arg_access_qualifier)
          return CL_INVALID_KERNEL_DEFINITION;
      } else {
        kernel->args[i].arg_access_qualifier = arg_access_qualifier;
      }

      arg_type_qualifier = 0;
      if (kernel->args[i].arg_type == ArgTypePointer) {
        if (strstr(arg_type_qual_str, "const"))
          arg_type_qualifier = arg_type_qualifier | CL_KERNEL_ARG_TYPE_CONST;
        if (strstr(arg_type_qual_str, "volatile"))
          arg_type_qualifier = arg_type_qualifier | CL_KERNEL_ARG_TYPE_VOLATILE;
      }
      if (strstr(arg_type_qual_str, "restrict"))
        arg_type_qualifier = arg_type_qualifier | CL_KERNEL_ARG_TYPE_RESTRICT;
      if (strstr(arg_type_qual_str, "pipe"))
        arg_type_qualifier = CL_KERNEL_ARG_TYPE_PIPE;
      if (kernel->args[i].arg_type_qualifier > 0) {
        if (kernel->args[i].arg_type_qualifier != arg_type_qualifier)
          return CL_INVALID_KERNEL_DEFINITION;
      } else {
        kernel->args[i].arg_type_qualifier = arg_type_qualifier;
      }
    }
  }

  if (ptr - desc != desc_size)
    return CL_INVALID_PROGRAM;

  return CL_SUCCESS;
}

static cl_int
cl_program_gen_get_one_kernel_func(cl_device_id device, cl_kernel kernel, GElf_Sym *p_sym_entry)
{
  cl_program prog = kernel->program;
  cl_program_gen prog_gen;
  cl_kernel_gen kernel_gen;
  cl_int offset;
  cl_char *name;
  cl_uint name_size;
  cl_uint desc_size;
  cl_uint desc_type;
  cl_uint arg_num;
  void *ptr;
  int i, j;
  int cmp_arg;
  DEV_PRIVATE_DATA(prog, device, prog_gen);
  DEV_PRIVATE_DATA(kernel, device, kernel_gen);

  assert(kernel->name && kernel->name[0] != 0); // Must have a name for kernel func
  assert(p_sym_entry->st_shndx == prog_gen->text_sec_index);
  kernel_gen->kern_base.exec_code = prog_gen->text_data->d_buf + p_sym_entry->st_value;
  kernel_gen->kern_base.exec_code_sz = p_sym_entry->st_size;
  assert(kernel_gen->kern_base.exec_code < prog_gen->text_data->d_buf + prog_gen->text_data->d_size);

  /* Get all GPU related info for the kernel. */
  offset = 0;
  while (offset < prog_gen->func_gpu_info_data->d_size) {
    name_size = *(cl_uint *)(prog_gen->func_gpu_info_data->d_buf + offset);
    desc_size = *(cl_uint *)(prog_gen->func_gpu_info_data->d_buf + offset + sizeof(cl_uint));
    desc_type = *(cl_uint *)(prog_gen->func_gpu_info_data->d_buf + offset + 2 * sizeof(cl_uint));
    name = prog_gen->func_gpu_info_data->d_buf + offset + sizeof(cl_uint) * 3;
    if (desc_type != GEN_NOTE_TYPE_GPU_INFO) {
      offset += 3 * sizeof(cl_uint) + ALIGN(name_size, 4) + ALIGN(desc_size, 4);
      continue;
    }

    if (strcmp((char *)name, (char *)kernel->name) != 0) {
      offset += 3 * sizeof(cl_uint) + ALIGN(name_size, 4) + ALIGN(desc_size, 4);
      continue;
    }

    // Find the kernel info slot
    cmp_arg = 0;
    ptr = prog_gen->func_gpu_info_data->d_buf + offset + sizeof(cl_uint) * 3 + ALIGN(name_size, 4);
    kernel_gen->simd_width = *(cl_uint *)(ptr);
    kernel_gen->local_mem_size = *(cl_uint *)(ptr + sizeof(cl_uint));
    kernel_gen->scratch_size = *(cl_uint *)(ptr + 2 * sizeof(cl_uint));
    kernel_gen->stack_size = *(cl_uint *)(ptr + 3 * sizeof(cl_uint));
    kernel_gen->barrier_slm_used = *(cl_uint *)(ptr + 4 * sizeof(cl_uint));
    arg_num = *(cl_uint *)(ptr + 5 * sizeof(cl_uint));
    if (kernel->arg_n > 0 && kernel->arg_n != arg_num)
      return CL_INVALID_KERNEL_DEFINITION;

    if (kernel->arg_n > 0) {
      cmp_arg = 1;
    } else {
      kernel->arg_n = arg_num;
      kernel->args = CL_CALLOC(arg_num, sizeof(_cl_argument));
      if (kernel->args == NULL)
        return CL_OUT_OF_HOST_MEMORY;
    }

    kernel_gen->arg_extra_info = CL_CALLOC(arg_num, sizeof(_cl_gen_arg_extra_info));
    if (kernel_gen->arg_extra_info == NULL)
      return CL_OUT_OF_HOST_MEMORY;

    kernel_gen->arg_extra_info_num = arg_num;

    ptr += 6 * sizeof(cl_uint);

    /* Setup all the arguments info or cmp them */
    if (cmp_arg == 0) {
      for (i = 0; i < arg_num; i++) {
        kernel->args[i].arg_no = *((cl_uint *)ptr);
        ptr += sizeof(cl_uint);
        kernel->args[i].arg_size = *((cl_uint *)ptr);
        ptr += sizeof(cl_uint);
        kernel->args[i].arg_type = *((cl_uint *)ptr);
        ptr += sizeof(cl_uint);
        kernel_gen->arg_extra_info[i].arg_offset = *((cl_int *)ptr);
        ptr += sizeof(cl_uint);
        kernel->args[i].arg_addrspace = *((cl_uint *)ptr);
        ptr += sizeof(cl_uint);
        kernel_gen->arg_extra_info[i].arg_align = *((cl_uint *)ptr);
        ptr += sizeof(cl_uint);
        kernel_gen->arg_extra_info[i].arg_misc = *((cl_int *)ptr);
        ptr += sizeof(cl_uint); // The BTI, image index, etc
      }
    } else {
      cl_uint arg_no;
      cl_argument k_arg;
      for (i = 0; i < arg_num; i++) {
        arg_no = *((cl_uint *)ptr);
        ptr += sizeof(cl_uint);

        k_arg = NULL;
        for (j = 0; j < arg_num; j++) { // Find the same index arg
          if (kernel->args[j].arg_no == arg_no) {
            k_arg = &kernel->args[j];
            break;
          }
        }
        assert(k_arg);

        if (k_arg->arg_size != *((cl_uint *)ptr))
          return CL_INVALID_KERNEL_DEFINITION;
        ptr += sizeof(cl_uint);
        if (k_arg->arg_type != *((cl_uint *)ptr))
          return CL_INVALID_KERNEL_DEFINITION;
        ptr += sizeof(cl_uint);

        kernel_gen->arg_extra_info[i].arg_offset = *((cl_int *)ptr);
        ptr += sizeof(cl_uint);

        if (k_arg->arg_type != *((cl_uint *)ptr))
          return CL_INVALID_KERNEL_DEFINITION;
        ptr += sizeof(cl_uint);

        kernel_gen->arg_extra_info[i].arg_align = *((cl_uint *)ptr);
        ptr += sizeof(cl_uint);
        kernel_gen->arg_extra_info[i].arg_misc = *((cl_int *)ptr);
        ptr += sizeof(cl_uint); // The BTI, image index, etc
      }
    }

    /* Setup the samplers. */
    kernel_gen->samper_info_num = *((cl_uint *)ptr);
    ptr += sizeof(cl_uint);
    if (kernel_gen->samper_info_num) {
      kernel_gen->samper_info = CL_CALLOC(kernel_gen->samper_info_num, sizeof(cl_uint));
      if (kernel_gen->samper_info == NULL)
        return CL_OUT_OF_HOST_MEMORY;

      for (i = 0; i < kernel_gen->samper_info_num; i++) {
        kernel_gen->samper_info[i] = *((cl_uint *)ptr);
        ptr += sizeof(cl_uint);
      }
    }

    /* Setup the image. */
    kernel_gen->image_info_num = *((cl_uint *)ptr);
    ptr += sizeof(cl_uint);
    if (kernel_gen->image_info_num) {
      kernel_gen->image_info =
        CL_CALLOC(kernel_gen->image_info_num, sizeof(_cl_gen_image_info_offset));
      if (kernel_gen->image_info == NULL)
        return CL_OUT_OF_HOST_MEMORY;

      for (i = 0; i < kernel_gen->image_info_num; i++) {
        kernel_gen->image_info[i].bti = *((cl_int *)ptr);
        ptr += sizeof(cl_uint);
        kernel_gen->image_info[i].width = *((cl_int *)ptr);
        ptr += sizeof(cl_uint);
        kernel_gen->image_info[i].height = *((cl_int *)ptr);
        ptr += sizeof(cl_uint);
        kernel_gen->image_info[i].depth = *((cl_int *)ptr);
        ptr += sizeof(cl_uint);
        kernel_gen->image_info[i].data_type = *((cl_int *)ptr);
        ptr += sizeof(cl_uint);
        kernel_gen->image_info[i].channel_order = *((cl_int *)ptr);
        ptr += sizeof(cl_uint);
      }
    }

    /* Setup the special virtual reg/offset mapping. */
    kernel_gen->virt_reg_phy_offset_num = *((cl_uint *)ptr);
    ptr += sizeof(cl_uint);
    if (kernel_gen->virt_reg_phy_offset_num) {
      kernel_gen->virt_reg_phy_offset =
        CL_CALLOC(kernel_gen->virt_reg_phy_offset_num, sizeof(_cl_gen_virt_phy_offset));
      if (kernel_gen->virt_reg_phy_offset == NULL)
        return CL_OUT_OF_HOST_MEMORY;

      for (i = 0; i < kernel_gen->virt_reg_phy_offset_num; i++) {
        if (*((cl_uint *)ptr) >= CL_GEN_VIRT_REG_LAST || *((cl_uint *)ptr) < 0)
          return CL_INVALID_PROGRAM;

        kernel_gen->virt_reg_phy_offset[i].virt_reg = *((cl_uint *)ptr);
        ptr += sizeof(cl_uint);
        kernel_gen->virt_reg_phy_offset[i].phy_offset = *((cl_uint *)ptr);
        ptr += sizeof(cl_uint);
        kernel_gen->virt_reg_phy_offset[i].size = *((cl_uint *)ptr);
        ptr += sizeof(cl_uint);
      }
    }

    return cl_program_gen_get_kernel_func_cl_info(device, kernel);
  }

  return CL_INVALID_PROGRAM;
}

LOCAL cl_int
cl_kernel_create_gen(cl_device_id device, cl_kernel kernel)
{
  cl_program prog = kernel->program;
  GElf_Sym sym_entry;
  GElf_Sym *p_sym_entry = NULL;
  cl_program_gen prog_gen;
  char *name = NULL;
  int i;
  cl_kernel_gen kernel_gen = CL_CALLOC(1, sizeof(_cl_kernel_gen));
  if (kernel_gen == NULL)
    return CL_OUT_OF_HOST_MEMORY;

  kernel_gen->kern_base.device = device;
  ASSIGN_DEV_PRIVATE_DATA(kernel, device, (cl_kernel_for_device)kernel_gen);
  DEV_PRIVATE_DATA(prog, device, prog_gen);

  assert(kernel->name && kernel->name[0] != 0); // Must have a name for kernel func

  for (i = 0; i < (int)(prog_gen->symtab_entry_num); i++) {
    p_sym_entry = gelf_getsym(prog_gen->symtab_data, i, &sym_entry);
    assert(p_sym_entry == &sym_entry);
    if (ELF32_ST_TYPE(p_sym_entry->st_info) != STT_FUNC)
      continue;
    if (ELF32_ST_BIND(p_sym_entry->st_info) != STB_GLOBAL)
      continue;

    name = p_sym_entry->st_name + prog_gen->strtab_data->d_buf;
    assert(name);
    if (strcmp((char *)name, (char *)kernel->name))
      continue;

    // Find the kernel info slot
    return cl_program_gen_get_one_kernel_func(device, kernel, p_sym_entry);
  }

  return CL_INVALID_KERNEL_NAME;
}

LOCAL cl_int
cl_enqueue_native_kernel(cl_event event, cl_int status)
{
  cl_mem_gen mem_gen = NULL;
  cl_mem mem = NULL;
  cl_mem *mem_list = event->exec_data.native_kernel.mem_list;
  cl_uint i;
  void *ptr;

  if (status == CL_QUEUED || status == CL_RUNNING || status == CL_SUBMITTED)
    return CL_SUCCESS;

  assert(status == CL_COMPLETE);

  for (i = 0; i < event->exec_data.native_kernel.mem_num; i++) {
    mem = mem_list[i];
    mem_gen = (cl_mem_gen)mem->each_device[0];
    assert(mem_gen->drm_bo);
    ptr = cl_mem_gen_drm_bo_map(mem_gen->drm_bo, CL_FALSE);
    assert(ptr);
    *(void **)(event->exec_data.native_kernel.mem_arg_loc[i]) = ptr;
  }

  /* Call the real func */
  event->exec_data.native_kernel.user_func(event->exec_data.native_kernel.args);

  for (i = 0; i < event->exec_data.native_kernel.mem_num; i++) {
    mem = mem_list[i];
    mem_gen = (cl_mem_gen)mem->each_device[0];
    cl_mem_gen_drm_bo_unmap(mem_gen->drm_bo);
    event->exec_data.native_kernel.mem_arg_loc[i] = NULL;
  }

  return CL_SUCCESS;
}
