#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <libelf.h>
#include <gelf.h>
#include <string.h>
#include <assert.h>

typedef int (*compile_program_func)(int device_id, const char *source, size_t src_length, const char **headers,
                                    size_t *header_lengths, const char **header_names, int header_num,
                                    const char *options, size_t err_buf_size, char *err, size_t *err_ret_size,
                                    char **binary, size_t *binary_size);
typedef int (*build_program_func)(int device_id, const char *source, size_t src_length,
                                  const char *options, size_t err_buf_size, char *err,
                                  size_t *err_ret_size, char **binary, size_t *binary_size);
static compile_program_func compile_program = NULL;
static build_program_func build_program = NULL;

static char *output_file_name = NULL;
static char *input_file_name = NULL;
static int pci_id = 0;
static char build_log[1024];

int load_compiler(void)
{
  const char *gbePath = NULL;
  void *dlhCompiler = NULL;
  void *genCompileProgram = NULL;
  void *genBuildProgram = NULL;

  gbePath = COMPILER_BACKEND_OBJECT;

  dlhCompiler = dlopen(gbePath, RTLD_LAZY | RTLD_LOCAL);
  if (dlhCompiler == NULL)
    return -1;

  genCompileProgram = dlsym(dlhCompiler, "GenCompileProgram");
  if (genCompileProgram == NULL) {
    dlclose(dlhCompiler);
    return -1;
  }

  genBuildProgram = dlsym(dlhCompiler, "GenBuildProgram");
  if (genBuildProgram == NULL) {
    dlclose(dlhCompiler);
    return -1;
  }

  compile_program = genCompileProgram;
  build_program = genBuildProgram;
  return 0;
}

const char *file_map_open(size_t *file_len)
{
  void *address;

  /* Open the file */
  int fd = open(input_file_name, O_RDONLY);
  if (fd < 0)
    return NULL;

  *file_len = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);

  /* Map it */
  address = mmap(0, *file_len, PROT_READ, MAP_SHARED, fd, 0);
  if (address == NULL) {
    return NULL;
  }

  return address;
}

static void write_out_kernel_binary(char *obj_bin, size_t obj_size, char *elf_bin, size_t elf_size)
{
  Elf_Kind ek;
  Elf *elf_p = NULL;
  int ret;
  size_t val = 0;
  size_t sec_num;
  Elf_Scn *sh_strtab = NULL;
  Elf_Data *sh_strtab_data = NULL;
  Elf_Scn *elf_sec = NULL;
  GElf_Shdr sec_header;
  GElf_Shdr *p_sec_header = NULL;
  int i, j;
  Elf_Scn *symtab = NULL;
  Elf_Data *symtab_data = NULL;
  size_t symtab_entry_num;
  Elf_Scn *strtab = NULL;
  Elf_Data *strtab_data = NULL;
  GElf_Sym *p_sym_entry = NULL;
  GElf_Sym sym_entry;
  char *name;
  FILE *fp;
  char *p = NULL;
  char *q = NULL;

  elf_p = elf_memory(elf_bin, elf_size);
  if (elf_p == NULL) {
    printf("Can not parse elf file\n");
    exit(-1);
  }

  ek = elf_kind(elf_p);
  if (ek != ELF_K_ELF) {
    elf_end(elf_p);
    printf("Can not parse elf file, not a valid elf file\n");
    exit(-1);
  }

  ret = elf_getphdrnum(elf_p, &val);
  if (ret < 0) {
    elf_end(elf_p);
    printf("Can not parse elf file, not a valid elf file\n");
    exit(-1);
  }

  /* Should always have sections. */
  ret = elf_getshdrnum(elf_p, &val);
  if (ret < 0 || val <= 0) {
    printf("Can not parse elf file, not a valid elf file\n");
    exit(-1);
  }
  sec_num = val;

  /* Should always have a .shstrtab section. */
  ret = elf_getshdrstrndx(elf_p, &val);
  if (ret < 0) {
    elf_end(elf_p);
    printf("Can not parse elf file, not a valid elf file\n");
    exit(-1);
  }
  /* Get the section name string buffer. */
  sh_strtab = elf_getscn(elf_p, val);
  assert(sh_strtab);

  sh_strtab_data = elf_getdata(sh_strtab, NULL);
  if (sh_strtab_data == NULL) {
    elf_end(elf_p);
    printf("Can not parse elf file, not a valid elf file\n");
    exit(-1);
  }

  /* Find all the special sections. */
  for (i = 0; i < (int)sec_num; i++) {
    elf_sec = elf_getscn(elf_p, i);
    assert(elf_sec);
    p_sec_header = gelf_getshdr(elf_sec, &sec_header);
    assert(p_sec_header == &sec_header);
    if (strcmp(sh_strtab_data->d_buf + p_sec_header->sh_name, ".symtab") == 0) {
      symtab = elf_sec;
    } else if (strcmp(sh_strtab_data->d_buf + p_sec_header->sh_name, ".strtab") == 0) {
      strtab = elf_sec;
    }
  }

  if (symtab == NULL || strtab == NULL) {
    printf("Can not parse elf file, not a valid elf file\n");
    exit(-1);
  }

  strtab_data = elf_getdata(strtab, NULL);
  assert(strtab_data);
  symtab_data = elf_getdata(symtab, NULL);
  assert(symtab_data);
  p_sec_header = gelf_getshdr(symtab, &sec_header);
  assert(p_sec_header == &sec_header);
  symtab_entry_num = p_sec_header->sh_size / p_sec_header->sh_entsize;
  assert(p_sec_header->sh_size % p_sec_header->sh_entsize == 0);

  fp = fopen(output_file_name, "w");
  if (fp == NULL) {
    printf("Can not open file to output\n");
    exit(-1);
  }

  fprintf(fp, "%s", "#include <stddef.h>\n");

  p = strrchr(output_file_name, '/');
  if (p == NULL) {
    printf("Output path %s is invalid\n", output_file_name);
    exit(-1);
  }
  while (*p == '/')
    p++;

  q = p;
  while (*q != '.')
    q++;
  *q = 0;

  fprintf(fp, "char * %s_kernels = \n\"", p);

  /* Add all kernel names */
  j = 0;
  for (i = 0; i < (int)(symtab_entry_num); i++) {
    if (j)
      fprintf(fp, ";");

    p_sym_entry = gelf_getsym(symtab_data, i, &sym_entry);
    assert(p_sym_entry == &sym_entry);
    if (ELF32_ST_TYPE(p_sym_entry->st_info) != STT_FUNC)
      continue;
    if (ELF32_ST_BIND(p_sym_entry->st_info) != STB_GLOBAL)
      continue;

    name = p_sym_entry->st_name + strtab_data->d_buf;
    assert(name);
    fprintf(fp, "%s", name);
    j++;
  }
  fprintf(fp, "\";\n");

  /* Output the binary */
  fprintf(fp, "char %s[] = {\n", p);
  for (i = 0; i < (int)obj_size; i++) {
    unsigned char c = obj_bin[i];
    fprintf(fp, "0x%2.2x", c);
    if (i != (int)obj_size - 1)
      fprintf(fp, ", ");
  }
  fprintf(fp, "};\n");

  fprintf(fp, "size_t %s_size = %ld;\n", p, obj_size);
}

int main(int argc, const char **argv)
{
  int oc;
  size_t file_sz = 0;
  const char *file_content = NULL;
  int ret;
  size_t err_log_sz = 0;
  char *obj_bin = NULL;
  size_t obj_size = 0;
  char *elf_bin = NULL;
  size_t elf_size = 0;

  while ((oc = getopt(argc, (char *const *)argv, "i:o:p:s")) != -1) {
    switch (oc) {
    case 'p':
      pci_id = atoi(optarg);
      break;

    case 'i':
      input_file_name = optarg;
      break;

    case 'o':
      output_file_name = optarg;
      break;

    default:
      printf("unknown opt!!!\n");
      exit(-1);
    }
  }

  if (input_file_name == NULL) {
    printf("unknown input_file_name file name\n");
    exit(-1);
  }
  if (output_file_name == NULL) {
    printf("unknown output_file_name file name\n");
    exit(-1);
  }

  if (load_compiler() < 0) {
    printf("can not load compiler\n");
    exit(-1);
  }

  file_content = file_map_open(&file_sz);
  if (file_content == NULL) {
    printf("can not open file %s\n", input_file_name);
    exit(-1);
  }

  if (pci_id == 0)
    pci_id = 0x5A84; // Set a common one.

  ret = (*compile_program)(pci_id, file_content, file_sz + 1, NULL, NULL, NULL, 0, NULL,
                           sizeof(build_log), build_log, &err_log_sz, &obj_bin, &obj_size);
  if (ret == 0) {
    printf("Compiler program error, error msg is \n%s\n", build_log);
    exit(-1);
  }

  ret = (*build_program)(pci_id, file_content, file_sz + 1, NULL, sizeof(build_log), build_log, &err_log_sz, &elf_bin, &elf_size);
  if (ret == 0) {
    printf("Build program error, error msg is \n%s\n", build_log);
    exit(-1);
  }

  write_out_kernel_binary(obj_bin, obj_size, elf_bin, elf_size);
  return 0;
}
