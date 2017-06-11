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
#include <unistd.h>

struct binary_type_header_info {
  unsigned char header[7];
  cl_uint size;
  cl_uint type;
};

static struct binary_type_header_info binary_type_header[4] = {
  {{'B', 'C', 0xC0, 0xDE}, 4, CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT},
  {{'L', 'I', 'B', 'B', 'C', 0xC0, 0xDE}, 7, CL_PROGRAM_BINARY_TYPE_LIBRARY},
  {{0x7f, 'E', 'L', 'F'}, 4, CL_PROGRAM_BINARY_TYPE_EXECUTABLE}};

static cl_int
cl_program_get_binary_type_gen(const char *buf)
{
  int i;
  for (i = 0; i < sizeof(binary_type_header) / sizeof(struct binary_type_header_info); i++) {
    if (memcmp((char *)buf, binary_type_header[i].header, binary_type_header[i].size) == 0) {
      return binary_type_header[i].type;
    }
  }

  return CL_PROGRAM_BINARY_TYPE_NONE;
}

static Elf *
cl_program_parse_gen_elf_stream(cl_char *bit_stream, size_t size)
{
  Elf_Kind ek;
  Elf *elf_program = NULL;

  elf_program = elf_memory((char *)bit_stream, size);
  if (elf_program == NULL) {
    return NULL;
  }

  ek = elf_kind(elf_program);
  if (ek != ELF_K_ELF) {
    elf_end(elf_program);
    return NULL;
  }

  return elf_program;
}

LOCAL cl_int
cl_program_create_gen(cl_device_id device, cl_program p)
{
  cl_program_gen gen_elf = CL_CALLOC(1, sizeof(_cl_program_gen));
  if (gen_elf == NULL)
    return CL_OUT_OF_HOST_MEMORY;

  gen_elf->prog_base.device = device;
  gen_elf->prog_base.build_log_max_sz = BUILD_LOG_MAX_SIZE;
  gen_elf->prog_base.binary_type = CL_PROGRAM_BINARY_TYPE_NONE;
  ASSIGN_DEV_PRIVATE_DATA(p, device, (cl_program_for_device)gen_elf);
  return CL_SUCCESS;
}

LOCAL void
cl_program_delete_gen(cl_device_id device, cl_program p)
{
  cl_program_gen gen_elf = NULL;
  cl_program_for_device pd;
  DEV_PRIVATE_DATA(p, device, gen_elf);
  pd = &gen_elf->prog_base;
  int i;

  if (pd->kernel_names) {
    assert(pd->kernel_num > 0);
    for (i = 0; i < pd->kernel_num; i++) {
      if (pd->kernel_names[i])
        CL_FREE(pd->kernel_names[i]);
    }
    CL_FREE(pd->kernel_names);
  }
  pd->kernel_names = NULL;

  if (gen_elf->device_enqueue_info)
    CL_FREE(gen_elf->device_enqueue_info);
  gen_elf->device_enqueue_info = NULL;

  if (gen_elf->compiler_name)
    CL_FREE(gen_elf->compiler_name);
  gen_elf->compiler_name = NULL;

  if (gen_elf->gpu_name)
    CL_FREE(gen_elf->gpu_name);
  gen_elf->gpu_name = NULL;

  if (gen_elf->cl_version_str)
    CL_FREE(gen_elf->cl_version_str);
  gen_elf->cl_version_str = NULL;

  if (gen_elf->global_mem_data) {
    CL_FREE(gen_elf->global_mem_data);
    assert(gen_elf->global_mem_data_size > 0);
  }
  gen_elf->global_mem_data = NULL;

  if (gen_elf->elf)
    elf_end(gen_elf->elf);
  gen_elf->elf = NULL;

  CL_FREE(gen_elf);
}

static cl_int
cl_program_gen_alloc_global_mem(cl_device_id device, cl_program prog, cl_program_gen prog_gen)
{
  int i;
  cl_uint const_buf_size = 0;
  cl_uint aligned_const_buf_size = 0;

  if (prog_gen->cl_version < 200 || prog_gen->rodata_data == NULL)
    return CL_SUCCESS;

  const_buf_size = prog_gen->rodata_data->d_size;
  aligned_const_buf_size = ALIGN(const_buf_size, getpagesize());
  prog_gen->global_mem_data = CL_MEMALIGN(getpagesize(), aligned_const_buf_size);
  if (prog_gen->global_mem_data == NULL)
    return CL_OUT_OF_RESOURCES;

  prog_gen->global_mem_data_size = aligned_const_buf_size;
  memset(prog_gen->global_mem_data, 0, aligned_const_buf_size);
  memcpy(prog_gen->global_mem_data, prog_gen->rodata_data->d_buf, prog_gen->rodata_data->d_size);

  /* Do some reloc setting */
  if (prog_gen->ro_reloc) {
    GElf_Rela entry;
    GElf_Rela *p_entry;
    cl_int ro_reloc_num;
    GElf_Shdr *p_sec_header = NULL;
    GElf_Shdr sec_header;
    GElf_Sym *p_sym_entry;
    GElf_Sym sym_entry;
    char *const_buf_addr = prog_gen->global_mem_data;
    assert(prog_gen->ro_reloc_data);

    p_sec_header = gelf_getshdr(prog_gen->ro_reloc, &sec_header);
    ro_reloc_num = p_sec_header->sh_size / p_sec_header->sh_entsize;
    for (i = 0; i < ro_reloc_num; i++) {
      p_entry = gelf_getrela(prog_gen->ro_reloc_data, i, &entry);
      if (p_entry == NULL) {
        return CL_INVALID_PROGRAM;
      }

      if ((cl_uint)(GEN_ELF_RELOC_GET_TYPE(prog_gen, p_entry)) != R_386_32) {
        return CL_INVALID_PROGRAM;
      }

      p_sym_entry = gelf_getsym(prog_gen->symtab_data,
                                GEN_ELF_RELOC_GET_SYM(prog_gen, p_entry), &sym_entry);
      if (p_sym_entry == NULL) {
        return CL_INVALID_PROGRAM;
      }

      assert(p_entry->r_offset > 0);
      assert(sizeof(void *) == 8); // Must be 64 bits
      *(char **)(const_buf_addr + p_entry->r_offset) =
        (char *)(const_buf_addr + p_sym_entry->st_value + p_entry->r_addend);
    }
  }

  return CL_SUCCESS;
}

static cl_int
cl_program_load_binary_gen_elf(cl_device_id device, cl_program prog)
{
  cl_program_for_device pd;
  cl_program_gen elf = NULL;
  Elf *elf_p = NULL;
  GElf_Shdr sec_header;
  GElf_Shdr *p_sec_header = NULL;
  Elf_Scn *elf_sec = NULL;
  Elf_Scn *sh_strtab;
  Elf_Data *sh_strtab_data;
  GElf_Sym *p_sym_entry;
  GElf_Sym sym_entry;
  char *name;
  size_t val = 0;
  int i, j;
  cl_int offset;
  cl_uint name_size;
  cl_uint desc_size;
  cl_uint desc_type;
  cl_int ret;

  DEV_PRIVATE_DATA(prog, device, elf);
  pd = &elf->prog_base;

  assert(pd->binary != NULL);
  assert(pd->binary_sz > 4);
  assert(pd->binary_type == CL_PROGRAM_BINARY_TYPE_EXECUTABLE);

  elf_p = cl_program_parse_gen_elf_stream((cl_char *)pd->binary, pd->binary_sz);
  if (elf_p == NULL) {
    return CL_INVALID_PROGRAM;
  }

  elf->elf = elf_p;

  ret = elf_getphdrnum(elf_p, &val);
  if (ret < 0) {
    elf_end(elf_p);
    elf->elf = NULL;
    return CL_INVALID_PROGRAM;
  }

  /* Should always have sections. */
  ret = elf_getshdrnum(elf_p, &val);
  if (ret < 0 || val <= 0) {
    elf_end(elf_p);
    elf->elf = NULL;
    return CL_INVALID_PROGRAM;
  }
  elf->sec_num = val;

  /* Should always have a .shstrtab section. */
  ret = elf_getshdrstrndx(elf_p, &val);
  if (ret < 0) {
    elf_end(elf_p);
    elf->elf = NULL;
    return CL_INVALID_PROGRAM;
  }

  /* Get the section name string buffer. */
  sh_strtab = elf_getscn(elf_p, val);
  assert(sh_strtab);
  sh_strtab_data = elf_getdata(sh_strtab, NULL);
  if (sh_strtab_data == NULL) {
    elf_end(elf_p);
    elf->elf = NULL;
    return CL_INVALID_PROGRAM;
  }

  /* Find all the special sections. */
  for (i = 0; i < (int)(elf->sec_num); i++) {
    elf_sec = elf_getscn(elf_p, i);
    assert(elf_sec);
    p_sec_header = gelf_getshdr(elf_sec, &sec_header);
    assert(p_sec_header == &sec_header);
    if (strcmp(sh_strtab_data->d_buf + p_sec_header->sh_name, ".text") == 0) {
      elf->text = elf_sec;
      elf->text_sec_index = i;
    } else if (strcmp(sh_strtab_data->d_buf + p_sec_header->sh_name, ".symtab") == 0) {
      elf->symtab = elf_sec;
      elf->symtab_sec_index = i;
    } else if (strcmp(sh_strtab_data->d_buf + p_sec_header->sh_name, ".strtab") == 0) {
      elf->strtab = elf_sec;
      elf->strtab_sec_index = i;
    } else if (strcmp(sh_strtab_data->d_buf + p_sec_header->sh_name, ".note.gpu_info") == 0) {
      elf->func_gpu_info = elf_sec;
      elf->func_gpu_info_sec_index = i;
    } else if (strcmp(sh_strtab_data->d_buf + p_sec_header->sh_name, ".note.cl_info") == 0) {
      elf->func_cl_info = elf_sec;
      elf->func_cl_info_sec_index = i;
    } else if (strcmp(sh_strtab_data->d_buf + p_sec_header->sh_name, ".rodata") == 0) {
      elf->rodata = elf_sec;
      elf->rodata_sec_index = i;
    } else if (strcmp(sh_strtab_data->d_buf + p_sec_header->sh_name, ".rel.rodata") == 0) {
      elf->ro_reloc = elf_sec;
      elf->ro_reloc_index = i;
    }
  }

  if (elf->text == NULL || elf->symtab == NULL || elf->strtab == NULL ||
      elf->func_gpu_info == NULL || elf->func_cl_info == NULL) {
    elf_end(elf_p);
    elf->elf = NULL;
    return CL_INVALID_PROGRAM;
  }

  elf->strtab_data = elf_getdata(elf->strtab, NULL);
  assert(elf->strtab_data);
  elf->text_data = elf_getdata(elf->text, NULL);
  assert(elf->text_data);
  elf->symtab_data = elf_getdata(elf->symtab, NULL);
  assert(elf->symtab_data);
  p_sec_header = gelf_getshdr(elf->symtab, &sec_header);
  assert(p_sec_header == &sec_header);
  elf->symtab_entry_num = p_sec_header->sh_size / p_sec_header->sh_entsize;
  assert(p_sec_header->sh_size % p_sec_header->sh_entsize == 0);
  elf->func_gpu_info_data = elf_getdata(elf->func_gpu_info, NULL);
  assert(elf->func_gpu_info_data);
  elf->func_cl_info_data = elf_getdata(elf->func_cl_info, NULL);
  assert(elf->func_cl_info_data);
  if (elf->rodata) {
    elf->rodata_data = elf_getdata(elf->rodata, NULL);
    assert(elf->rodata_data);
  }
  if (elf->ro_reloc) {
    elf->ro_reloc_data = elf_getdata(elf->ro_reloc, NULL);
    assert(elf->ro_reloc_data);
  }

  /* Add all kernel names */
  assert(pd->kernel_names == NULL);
  assert(pd->kernel_num == 0);
  for (i = 0; i < (int)(elf->symtab_entry_num); i++) {
    p_sym_entry = gelf_getsym(elf->symtab_data, i, &sym_entry);
    assert(p_sym_entry == &sym_entry);
    if (ELF32_ST_TYPE(p_sym_entry->st_info) != STT_FUNC)
      continue;
    if (ELF32_ST_BIND(p_sym_entry->st_info) != STB_GLOBAL)
      continue;

    name = p_sym_entry->st_name + elf->strtab_data->d_buf;
    assert(name);

    pd->kernel_num++;
  }
  if (pd->kernel_num == 0) { // A program without kernel ?
    elf_end(elf_p);
    elf->elf = NULL;
    return CL_INVALID_PROGRAM;
  }

  pd->kernel_names = CL_CALLOC(pd->kernel_num, sizeof(char *));
  if (pd->kernel_names == NULL) {
    elf_end(elf_p);
    elf->elf = NULL;
    return CL_OUT_OF_HOST_MEMORY;
  }
  j = 0;
  for (i = 0; i < (int)(elf->symtab_entry_num); i++) {
    p_sym_entry = gelf_getsym(elf->symtab_data, i, &sym_entry);
    assert(p_sym_entry == &sym_entry);
    if (ELF32_ST_TYPE(p_sym_entry->st_info) != STT_FUNC)
      continue;
    if (ELF32_ST_BIND(p_sym_entry->st_info) != STB_GLOBAL)
      continue;

    pd->kernel_names[j] =
      CL_CALLOC(1, strlen(p_sym_entry->st_name + elf->strtab_data->d_buf) + 1);
    if (pd->kernel_names[j] == NULL) {
      elf_end(elf_p);
      elf->elf = NULL;
      return CL_OUT_OF_HOST_MEMORY;
    }

    memcpy(pd->kernel_names[j], p_sym_entry->st_name + elf->strtab_data->d_buf,
           strlen(p_sym_entry->st_name + elf->strtab_data->d_buf) + 1);
    j++;
  }
  assert(j == pd->kernel_num);

  /* Get the compiler name and gpu version */
  offset = 0;
  while (offset < elf->func_gpu_info_data->d_size) {
    name_size = *(cl_uint *)(elf->func_gpu_info_data->d_buf + offset);
    desc_size = *(cl_uint *)(elf->func_gpu_info_data->d_buf + offset + sizeof(cl_uint));
    desc_type = *(cl_uint *)(elf->func_gpu_info_data->d_buf + offset + 2 * sizeof(cl_uint));
    if (desc_type == GEN_NOTE_TYPE_COMPILER_INFO) {
      elf->compiler_name = CL_CALLOC(name_size + 1, sizeof(char));
      if (elf->compiler_name == NULL) {
        elf_end(elf_p);
        elf->elf = NULL;
        return CL_OUT_OF_HOST_MEMORY;
      }
      memcpy(elf->compiler_name, elf->func_gpu_info_data->d_buf + offset + sizeof(cl_uint) * 3, name_size);
      elf->compiler_name[name_size] = 0;
      elf->compiler_version_major = *(cl_uint *)(elf->func_gpu_info_data->d_buf + offset +
                                                 3 * sizeof(cl_uint) + ALIGN(name_size, 4));
      elf->compiler_version_minor = *(cl_uint *)(elf->func_gpu_info_data->d_buf + offset +
                                                 3 * sizeof(cl_uint) + ALIGN(name_size, 4) + sizeof(cl_uint));
    } else if (desc_type == GEN_NOTE_TYPE_GPU_VERSION) {
      elf->gpu_name = CL_CALLOC(name_size + 1, sizeof(char));
      if (elf->gpu_name == NULL) {
        elf_end(elf_p);
        elf->elf = NULL;
        return CL_OUT_OF_HOST_MEMORY;
      }
      memcpy(elf->gpu_name, elf->func_gpu_info_data->d_buf + offset + sizeof(cl_uint) * 3, name_size);
      elf->gpu_name[name_size] = 0;
      elf->gpu_version_major = *(cl_uint *)(elf->func_gpu_info_data->d_buf + offset +
                                            3 * sizeof(cl_uint) + ALIGN(name_size, 4));
      elf->gpu_version_minor = *(cl_uint *)(elf->func_gpu_info_data->d_buf + offset +
                                            3 * sizeof(cl_uint) + ALIGN(name_size, 4) + sizeof(cl_uint));
    }

    offset += 3 * sizeof(cl_uint) + ALIGN(name_size, 4) + ALIGN(desc_size, 4);
  }

  /* Get the OpenCL version */
  offset = 0;
  while (offset < elf->func_cl_info_data->d_size) {
    name_size = *(cl_uint *)(elf->func_cl_info_data->d_buf + offset);
    desc_size = *(cl_uint *)(elf->func_cl_info_data->d_buf + offset + sizeof(cl_uint));
    desc_type = *(cl_uint *)(elf->func_cl_info_data->d_buf + offset + 2 * sizeof(cl_uint));
    if (desc_type == GEN_NOTE_TYPE_CL_VERSION) {
      elf->cl_version_str = CL_CALLOC(name_size + 1, sizeof(char));
      if (elf->cl_version_str == NULL) {
        elf_end(elf_p);
        elf->elf = NULL;
        return CL_OUT_OF_HOST_MEMORY;
      }
      memcpy(elf->cl_version_str, elf->func_cl_info_data->d_buf + offset + sizeof(cl_uint) * 3, name_size);
      elf->cl_version_str[name_size] = 0;
      elf->cl_version = *(cl_uint *)(elf->func_cl_info_data->d_buf + offset +
                                     3 * sizeof(cl_uint) + ALIGN(name_size, 4));
    } else if (desc_type == GEN_NOTE_TYPE_CL_DEVICE_ENQUEUE_INFO) {
      cl_uint n;
      cl_uint *ptr;

      elf->device_enqueue_info_num = desc_size / (sizeof(cl_uint) * 2);
      assert(elf->device_enqueue_info_num > 0);
      elf->device_enqueue_info = CL_CALLOC(elf->device_enqueue_info_num,
                                           sizeof(_cl_program_gen_device_enqueue_info));
      if (elf->device_enqueue_info == NULL) {
        elf_end(elf_p);
        elf->elf = NULL;
        return CL_OUT_OF_HOST_MEMORY;
      }

      ptr = elf->func_cl_info_data->d_buf + offset + 3 * sizeof(cl_uint) + ALIGN(name_size, 4);
      for (n = 0; n < elf->device_enqueue_info_num; n++) {
        elf->device_enqueue_info[n].index = ptr[n * 2];
        p_sym_entry = gelf_getsym(elf->symtab_data, ptr[n * 2 + 1], &sym_entry);
        assert(p_sym_entry == &sym_entry);
        assert(ELF32_ST_TYPE(p_sym_entry->st_info) == STT_FUNC);
        elf->device_enqueue_info[n].kernel_name = p_sym_entry->st_name + elf->strtab_data->d_buf;
      }
    }

    offset += 3 * sizeof(cl_uint) + ALIGN(name_size, 4) + ALIGN(desc_size, 4);
  }

  ret = cl_program_gen_alloc_global_mem(device, prog, elf);
  if (ret != CL_SUCCESS) {
    elf_end(elf_p);
    elf->elf = NULL;
  }

  return ret;
}

LOCAL cl_int
cl_program_load_binary_gen(cl_device_id device, cl_program prog)
{
  cl_program_gen pg = NULL;
  cl_program_for_device pd = NULL;

  DEV_PRIVATE_DATA(prog, device, pg);
  pd = &pg->prog_base;

  assert(pd->binary != NULL);

  //need at least bytes to check the binary type.
  if (pd->binary_sz < 7)
    return CL_INVALID_PROGRAM_EXECUTABLE;

  if (pd->binary_type == CL_PROGRAM_BINARY_TYPE_NONE) { // Need to recognize it first
    pd->binary_type = cl_program_get_binary_type_gen(pd->binary);
    if (pd->binary_type == CL_PROGRAM_BINARY_TYPE_NONE)
      return CL_INVALID_PROGRAM_EXECUTABLE;
  }

  if (pd->binary_type != CL_PROGRAM_BINARY_TYPE_EXECUTABLE)
    return CL_SUCCESS;

  return cl_program_load_binary_gen_elf(device, prog);
}

LOCAL cl_int
cl_program_get_info_gen(cl_device_id device, cl_program program, cl_uint param_name, void *param_value)
{
  cl_program_gen program_gen;
  DEV_PRIVATE_DATA(program, device, program_gen);

  if (param_name == CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE) {
    if (program_gen->prog_base.binary_type != CL_PROGRAM_BINARY_TYPE_NONE) {
      *(size_t *)param_value = program_gen->rodata_data->d_size;
    } else {
      *(size_t *)param_value = 0;
    }
    return CL_SUCCESS;
  }

  return CL_INVALID_VALUE;
}
