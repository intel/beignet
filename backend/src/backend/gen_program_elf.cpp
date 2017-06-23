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
#include "src/cl_device_data.h"
#include "ocl_common_defines.h"
#include "elfio/elfio.hpp"
#include "backend/gen_program.hpp"
#include "backend/gen_program.hpp"
#include "sys/cvar.hpp"
#include <algorithm>
#include <sstream>
#include <streambuf>
using namespace std;

namespace gbe
{

BVAR(OCL_DUMP_ELF_FILE, false);

/* The elf writer need to make sure seekp function work, so sstream
   can not work, and we do not want the fostream to generate the real
   file. We just want to keep the elf image in the memory. Implement
   a simple streambuf write only class here. */
class wmemstreambuf : public std::streambuf
{
public:
  wmemstreambuf(size_t size) : max_writed(0)
  {
    buf_ = static_cast<char *>(::malloc(size));
    memset(buf_, 0, size);
    buf_size_ = size;
    setbuf(buf_, buf_size_);
  }
  ~wmemstreambuf()
  {
    if (buf_)
      ::free(buf_);
  }

  char *getcontent(size_t &total_sz)
  {
    total_sz = max_writed;
    return buf_;
  }

protected:
  char *buf_;
  std::streamsize buf_size_;
  std::streamsize max_writed;

  virtual std::streambuf *setbuf(char *s, std::streamsize n)
  {
    auto const begin(s);
    auto const end(s + n);
    setp(begin, end);
    return this;
  }

  virtual std::streampos seekpos(std::streampos pos,
                                 std::ios_base::openmode which =
                                   ::std::ios_base::in | ::std::ios_base::out)
  {
    if (which != std::ios_base::out) {
      assert(0);
      return pos_type(off_type(-1));
    }

    if (pos >= epptr() - pbase()) {
      auto old_size = buf_size_;
      while (buf_size_ < pos) {
        buf_size_ *= 2;
      }

      buf_ = static_cast<char *>(::realloc(buf_, buf_size_));
      memset(buf_ + old_size, 0, buf_size_ - old_size);
      setbuf(buf_, buf_size_);
    } else {
      setp(pbase(), epptr());
    }

    pbump(pos);
    return pos;
  }

  virtual int sync() { return 0; }
  virtual int overflow(int c) { return c; };

  virtual std::streamsize xsgetn(const char *s, std::streamsize count)
  {
    assert(0);
    return traits_type::eof();
  }

  virtual std::streamsize xsputn(const char *s, std::streamsize const count)
  {
    if (epptr() - pptr() < count) {
      auto old_pos = pptr() - pbase();
      while (buf_size_ < (pptr() - pbase()) + count) {
        buf_size_ *= 2;
      }
      buf_ = static_cast<char *>(::realloc(buf_, buf_size_));
      memset(buf_ + old_pos, 0, buf_size_ - old_pos);
      setbuf(buf_, buf_size_);
      pbump(old_pos);
    }

    std::memcpy(pptr(), s, count);
    if (pptr() - pbase() + count > max_writed)
      max_writed = pptr() - pbase() + count;

    pbump(count);

    return count;
  }
};

using namespace ELFIO;

/* The format for one Gen Kernel function is following note section format
 --------------------------
 | GEN_NOTE_TYPE_GPU_INFO |
 --------------------------
 | Function Name size:4 |
 ------------------------
 | Desc size:4  |
 ---------------------------
 | The kernel name(strlen) |
 -----------------------------------------------------------------------------------------------
 | SIMD:4 | Local Mem Size:4 | Scratch Size:4 | Stack Size :4 | Barrier/SLM Used:4 | Arg Num:4 |
 -----------------------------------------------------------------------------------------------
   Then the format for each argument is
 --------------------------------------------------------------------------------------------------------------------------
 | Index:4 | Size:4 | Type:4 | Offset:4 | Addr Space:4 | Align(if is ptr) | BTI(if buffer):4 / Index(sampler and image):4 |
 --------------------------------------------------------------------------------------------------------------------------
   Then all sampler info
 -----------------------------------
 | Number:4 | SamperInfo:4 | ......|
 -----------------------------------
   Then all image info
 --------------------------------------------------------------------------------------------
 | Number:4 | BTI:4 | Width:4 | Height:4 | Depth:4 | Data Type:4 | Channel Order:4 | .......|
 --------------------------------------------------------------------------------------------
   Last is the map table of special virtual register and phy register
 --------------------------------------------------------
 | Number:4 | Virt Reg:4 | Phy Reg:4 | Size:4 |.........|
 --------------------------------------------------------  */

/* The format for one Gen Kernel function's OpenCL info is following note section format
 --------------------------
 | GEN_NOTE_TYPE_CL_INFO  |
 ----------------------------------------
 | The kernel function's name: (strlen) |
 ----------------------------------------
 | Function's attribute string: (strlen)|
 ----------------------------------------
 | Work Group size: sizeof(size_t) * 3  |
 ----------------------------------------
 | Argument TypeName: (strlen) |
 ---------------------------------
 | Argument AccessQual: (strlen) |
 ---------------------------------
 | Argument Name: (strlen) |
 ---------------------------  */

/* The format for GPU version is:
 ----------------------------
 | GEN_NOTE_TYPE_GPU_VERSION |
 -----------------------------
 | GEN string (HasWell e.g.) |
 -----------------------------
 | GEN pci id |
 --------------
 | GEN version major:4 |
 -----------------------
 | GEN version minor:4 |
 -----------------------  */

/* The format for CL Device Enqueue info is:
 ----------------------------------------
 | GEN_NOTE_TYPE_CL_DEVICE_ENQUEUE_INFO |
 ---------------------------------------------
 | "Open CL 2.0 Devuce Enqueue Kernel Names" |
 ---------------------------------------------
 | Kernel Index:4 |
 ------------------------------------------
 | Kernel symbol number in symbol table:4 |
 ------------------------------------------   */

/* The format for CL version is:
 ----------------------------
 | GEN_NOTE_TYPE_CL_VERSION |
 ----------------------------------------
 | CL version string (OpenCL 2.0  e.g.) |
 ----------------------------------------
 | CL version major:4 |
 ----------------------
 | CL version minor:4 |
 ----------------------  */

/* The format for Compiler info is:
 -------------------------------
 | GEN_NOTE_TYPE_COMPILER_INFO |
 --------------------------------------
 | Compiler name (GBE_Compiler  e.g.) |
 --------------------------------------
 | LLVM version major:4 |
 ------------------------
 | LLVM version minor:4 |
 ------------------------ */

/* The format for printf is:
 ---------------------------
 | GEN_NOTE_TYPE_CL_PRINTF |
 ---------------------------
 | The Kernel name |
 -------------------------------
 | CL printf bti:4 |
 ----------------------
 | CL printf number:4 |
 -------------------------------------------
 | CL printf id for one printf statement:4 |
 -------------------------------------------
 | printf format string |
 ------------------------
 */

class GenProgramElfContext
{
public:
  enum {
    GEN_NOTE_TYPE_CL_VERSION = 1,
    GEN_NOTE_TYPE_GPU_VERSION = 2,
    GEN_NOTE_TYPE_GPU_INFO = 3,
    GEN_NOTE_TYPE_CL_INFO = 4,
    GEN_NOTE_TYPE_CL_DEVICE_ENQUEUE_INFO = 5,
    GEN_NOTE_TYPE_COMPILER_INFO = 6,
    GEN_NOTE_TYPE_CL_PRINTF = 7,
  };

  struct KernelInfoHelper {
    Elf32_Word simd;
    Elf32_Word local_mem_size;
    Elf32_Word scratch_size;
    Elf32_Word stack_size;
    Elf32_Word barrier_slm_used;
    Elf32_Word arg_num;
  };
  struct ArgInfoHelper {
    Elf32_Word index;
    Elf32_Word size;
    Elf32_Word type;
    Elf32_Word offset;
    Elf32_Word addr_space;
    Elf32_Word align;
    Elf32_Word extra;
  };
  struct ImageInfoHelper {
    Elf32_Word bti;
    Elf32_Word width;
    Elf32_Word height;
    Elf32_Word depth;
    Elf32_Word data_type;
    Elf32_Word channel_order;
  };
  struct VirtRegMapHelper {
    Elf32_Word virt_reg;
    Elf32_Word phy_reg;
    Elf32_Word size;
  };

  GenProgram &genProg;

  elfio writer;
  section *text_sec;
  section *sym_sec;
  section *strtab_sec;
  section *ker_info_sec;
  section *cl_info_sec;
  section *rodata_sec;
  section *reloc_rodata_sec;
  symbol_section_accessor *syma;
  string_section_accessor *stra;
  note_section_accessor *note_writer;
  note_section_accessor *cl_note_writer;
  relocation_section_accessor *rela;
  Elf32_Word sym_num;
  uint64_t bitcode_offset;

  GenProgramElfContext(GenProgram &prog);
  ~GenProgramElfContext(void);

  template <gbe_curbe_type curbe_enum, typename TYPE, int UNIFORM>
  void emitOneCurbeReg(unsigned int &total_num, char *&ptr, GenKernel &kernel);
  Elf_Word emitOneKernel(GenKernel &kernel);
  void emitOneKernelCLInfo(GenKernel &kernel);
};

GenProgramElfContext::GenProgramElfContext(GenProgram &prog)
  : genProg(prog), text_sec(NULL), sym_sec(NULL), strtab_sec(NULL), ker_info_sec(NULL),
    cl_info_sec(NULL), rodata_sec(NULL), reloc_rodata_sec(NULL), syma(NULL), stra(NULL),
    note_writer(NULL), cl_note_writer(NULL), rela(NULL), sym_num(0), bitcode_offset(0)
{
  writer.create(ELFCLASS64, ELFDATA2LSB);
  writer.set_os_abi(ELFOSABI_LINUX);
  writer.set_type(ET_REL);
  writer.set_machine(EM_INTEL205); // TODO: Some value of Intel GPU;

  // Create code section
  text_sec = writer.sections.add(".text");
  text_sec->set_type(SHT_PROGBITS);
  text_sec->set_flags(SHF_ALLOC | SHF_EXECINSTR);
  text_sec->set_addr_align(4);

  // Create string table section
  strtab_sec = writer.sections.add(".strtab");
  strtab_sec->set_type(SHT_STRTAB);
  strtab_sec->set_addr_align(1);

  // Create symbol table section
  sym_sec = writer.sections.add(".symtab");
  sym_sec->set_type(SHT_SYMTAB);
  sym_sec->set_addr_align(0x4);
  sym_sec->set_entry_size(writer.get_default_entry_size(SHT_SYMTAB));
  sym_sec->set_link(strtab_sec->get_index());
  sym_sec->set_info(0x01);

  // Create kernel info section
  ker_info_sec = writer.sections.add(".note.gpu_info");
  ker_info_sec->set_type(SHT_NOTE);
  ker_info_sec->set_flags(SHF_ALLOC);
  ker_info_sec->set_addr_align(0x04);

  // Create cl info section
  cl_info_sec = writer.sections.add(".note.cl_info");
  cl_info_sec->set_type(SHT_NOTE);
  cl_info_sec->set_flags(SHF_ALLOC);
  cl_info_sec->set_addr_align(0x04);

  // Create string table writer
  stra = GBE_NEW(string_section_accessor, strtab_sec);
  // Create symbol table writer
  syma = GBE_NEW(symbol_section_accessor, writer, sym_sec);
  // Create note writer
  note_writer = GBE_NEW(note_section_accessor, writer, ker_info_sec);
  // Create CL note writer
  cl_note_writer = GBE_NEW(note_section_accessor, writer, cl_info_sec);
}

GenProgramElfContext::~GenProgramElfContext(void)
{
  if (syma)
    GBE_DELETE(syma);
  if (stra)
    GBE_DELETE(stra);
  if (note_writer)
    GBE_DELETE(note_writer);
  if (cl_note_writer)
    GBE_DELETE(cl_note_writer);
  if (rela)
    GBE_DELETE(rela);
}

/*Store the special vitrual register map */
template <gbe_curbe_type curbe_enum, typename TYPE, int UNIFORM>
void GenProgramElfContext::emitOneCurbeReg(unsigned int &total_num, char *&ptr, GenKernel &kernel)
{
  int32_t offset = kernel.getCurbeOffset(curbe_enum, 0);
  if (offset >= 0) {
    VirtRegMapHelper *vri = reinterpret_cast<VirtRegMapHelper *>(ptr);
    vri->virt_reg = curbe_enum;
    vri->phy_reg = offset;
    vri->size = UNIFORM ? sizeof(TYPE) : sizeof(TYPE) * kernel.getSIMDWidth();
    ptr += sizeof(VirtRegMapHelper);
    total_num++;
  }
}
template <>
void GenProgramElfContext::emitOneCurbeReg<GBE_CURBE_EXTRA_ARGUMENT, uint64_t, 0>(
  unsigned int &total_num, char *&ptr, GenKernel &kernel)
{
  int32_t offset = kernel.getCurbeOffset(GBE_CURBE_EXTRA_ARGUMENT, GBE_STACK_BUFFER);
  if (offset >= 0) {
    VirtRegMapHelper *vri = reinterpret_cast<VirtRegMapHelper *>(ptr);
    vri->virt_reg = GBE_CURBE_EXTRA_ARGUMENT;
    vri->phy_reg = offset;
    vri->size = sizeof(uint64_t);
    ptr += sizeof(VirtRegMapHelper);
    total_num++;
  }
}

void GenProgramElfContext::emitOneKernelCLInfo(GenKernel &kernel)
{
  uint32_t all_str_len = 0;
  uint32_t attr_size = 0;
  size_t wg_sz[3];
  uint32_t wg_sz_size = 0;
  uint32_t arg_info_size = 0;

  /* Add printf info for this kernel */
  if (kernel.getPrintfNum() != 0) {
    std::map<uint32_t, std::string> all_printf;
    uint32_t printf_n = kernel.collectPrintfStr(all_printf);
    assert(printf_n == kernel.getPrintfNum());
    std::ostringstream oss;
    size_t sz = 0;

    uint32_t bti = kernel.getPrintfBufBTI();
    oss.write((char *)(&bti), sizeof(uint32_t));
    sz += sizeof(uint32_t);
    oss.write((char *)(&printf_n), sizeof(uint32_t));
    sz += sizeof(uint32_t);

    for (auto iter = all_printf.begin(); iter != all_printf.end(); iter++) {
      uint32_t id = iter->first;
      oss.write((char *)(&id), sizeof(uint32_t));
      sz += sizeof(uint32_t);
      oss.write(iter->second.c_str(), strlen(iter->second.c_str()) + 1);
      sz += strlen(iter->second.c_str()) + 1;
    }

    this->cl_note_writer->add_note(GenProgramElfContext::GEN_NOTE_TYPE_CL_PRINTF,
                                   kernel.getName(), oss.str().c_str(), sz);
  }

  if ((kernel.getFunctionAttributes())[0] != 0)
    attr_size = ::strlen(kernel.getFunctionAttributes()) + 1;
  all_str_len = ALIGN(attr_size, 4);

  kernel.getCompileWorkGroupSize(wg_sz);
  if (wg_sz[0] > 0 || wg_sz[1] > 0 || wg_sz[2] > 0) {
    wg_sz_size = sizeof(size_t) * 3;
  }
  all_str_len = all_str_len + wg_sz_size;

  for (unsigned int i = 0; i < kernel.getArgNum(); i++) {
    KernelArgument::ArgInfo *arg_info = kernel.getArgInfo(i);
    if (arg_info == NULL) {
      assert(i == 0); // All have info or none has info
      break;
    }
    arg_info_size += arg_info->typeName.length() + 1;
    arg_info_size += arg_info->accessQual.length() + 1;
    arg_info_size += arg_info->typeQual.length() + 1;
    arg_info_size += arg_info->argName.length() + 1;
    arg_info_size = ALIGN(arg_info_size, 4);
  }
  all_str_len = all_str_len + arg_info_size;

  if (all_str_len == 0)
    return;

  all_str_len += 3 * sizeof(uint32_t); // The length themselves
  char *cl_info = static_cast<char *>(GBE_MALLOC(all_str_len));
  *reinterpret_cast<uint32_t *>(cl_info) = attr_size;
  *reinterpret_cast<uint32_t *>(cl_info + sizeof(uint32_t)) = wg_sz_size;
  *reinterpret_cast<uint32_t *>(cl_info + 2 * sizeof(uint32_t)) = arg_info_size;

  size_t offset = 3 * sizeof(uint32_t);

  if (attr_size > 0) {
    ::memcpy(cl_info + offset, kernel.getFunctionAttributes(),
             ::strlen(kernel.getFunctionAttributes()) + 1);
    offset += attr_size;
    offset = ALIGN(offset, 4);
  }

  if (wg_sz_size > 0) {
    ::memcpy(cl_info + offset, wg_sz, sizeof(size_t) * 3);
    offset += wg_sz_size;
  }

  if (arg_info_size) {
    for (unsigned int i = 0; i < kernel.getArgNum(); i++) {
      KernelArgument::ArgInfo *arg_info = kernel.getArgInfo(i);
      assert(arg_info != NULL);
      if (arg_info->typeName.length() > 0)
        ::memcpy(cl_info + offset, arg_info->typeName.c_str(), arg_info->typeName.length() + 1);
      else
        *(cl_info + offset) = 0;
      offset += (arg_info->typeName.length() + 1);

      if (arg_info->accessQual.length() > 0)
        ::memcpy(cl_info + offset, arg_info->accessQual.c_str(), arg_info->accessQual.length() + 1);
      else
        *(cl_info + offset) = 0;
      offset += (arg_info->accessQual.length() + 1);

      if (arg_info->typeQual.length() > 0)
        ::memcpy(cl_info + offset, arg_info->typeQual.c_str(), arg_info->typeQual.length() + 1);
      else
        *(cl_info + offset) = 0;
      offset += (arg_info->typeQual.length() + 1);

      if (arg_info->argName.length() > 0)
        ::memcpy(cl_info + offset, arg_info->argName.c_str(), arg_info->argName.length() + 1);
      else
        *(cl_info + offset) = 0;
      offset += (arg_info->argName.length() + 1);

      offset = ALIGN(offset, 4);
    }
  }

  assert(offset == all_str_len);

  cl_note_writer->add_note(GEN_NOTE_TYPE_CL_INFO, kernel.getName(), cl_info, all_str_len);
  GBE_FREE(cl_info);
}

Elf_Word GenProgramElfContext::emitOneKernel(GenKernel &kernel)
{
  assert(text_sec != NULL);
  assert(sym_sec != NULL);
  assert(text_sec != NULL);
  assert(syma != NULL);
  assert(stra != NULL);

  sym_num++;

  // Add the kernel's bitcode to .text section
  text_sec->append_data(kernel.getCode(), kernel.getCodeSize());
  // Add the kernel func as a symbol
  Elf_Word symbol_num = syma->add_symbol(*stra, kernel.getName(), bitcode_offset, kernel.getCodeSize(),
                                         STB_GLOBAL, STT_FUNC, 0, text_sec->get_index());
  bitcode_offset += kernel.getCodeSize();

  uint32_t arg_num = kernel.getArgNum();

  size_t sampler_data_sz = kernel.getSamplerSize() * sizeof(uint32_t);
  uint32_t *sampler_data = NULL;
  if (sampler_data_sz) {
    sampler_data = static_cast<uint32_t *>(GBE_MALLOC(sampler_data_sz));
    ::memset(sampler_data, 0, sampler_data_sz);
    kernel.getSamplerData(sampler_data);
  }

  size_t image_data_sz = kernel.getImageSize() * sizeof(ImageInfo);
  ImageInfo *image_data = NULL;
  if (image_data_sz) {
    image_data = static_cast<ImageInfo *>(GBE_MALLOC(image_data_sz));
    ::memset(image_data, 0, image_data_sz);
    kernel.getImageData(image_data);
    std::sort(image_data, image_data + image_data_sz / sizeof(ImageInfo),
              [](ImageInfo &a, ImageInfo &b) { return a.idx < b.idx; });
  }

  void *kernel_info = GBE_MALLOC(4 /* For align */ +
                                 sizeof(KernelInfoHelper) + arg_num * sizeof(ArgInfoHelper) +
                                 sizeof(Elf32_Word) /* For sampler num */ + image_data_sz +
                                 sizeof(Elf32_Word) /* For image num */ +
                                 ((image_data_sz / sizeof(ImageInfo)) * sizeof(ImageInfoHelper)) +
                                 sizeof(Elf32_Word) /* For virt/phy num */ +
                                 GBE_GEN_REG * sizeof(VirtRegMapHelper));
  char *ptr = reinterpret_cast<char *>(ALIGN(reinterpret_cast<long>(kernel_info), 4));
  KernelInfoHelper *ki = reinterpret_cast<KernelInfoHelper *>(ptr);
  ki->simd = kernel.getSIMDWidth();
  ki->local_mem_size = kernel.getSLMSize();
  ki->scratch_size = kernel.getScratchSize();
  ki->stack_size = kernel.getStackSize();
  ki->barrier_slm_used = kernel.getUseSLM();
  ki->arg_num = kernel.getArgNum();
  ptr += sizeof(KernelInfoHelper);

  for (unsigned int i = 0; i < arg_num; i++) {
    ArgInfoHelper *argi = reinterpret_cast<ArgInfoHelper *>(ptr);
    argi->index = i;
    argi->size = kernel.getArgSize(i);
    argi->type = kernel.getArgumentType(i);
    argi->addr_space = kernel.getArgAddressSpace(i);
    argi->align = kernel.getArgAlign(i);

    if (argi->type == GBE_ARG_TYPE_POINTER) {
      if (argi->addr_space == GBE_ADDRESS_SPACE_GLOBAL ||
          (argi->addr_space == GBE_ADDRESS_SPACE_CONSTANT && kernel.getOclVersion() >= 200)) {
        argi->extra = kernel.getArgBTI(i);
      }
    } else if (argi->type == GBE_ARG_TYPE_PIPE) {
      assert(kernel.getOclVersion() >= 200);
      argi->extra = kernel.getArgBTI(i);
    } else if (argi->type == GBE_ARG_TYPE_IMAGE) {
      assert(image_data_sz > 0);
      for (size_t j = 0; j < image_data_sz / sizeof(ImageInfo); j++) {
        if (image_data[j].arg_idx == static_cast<int32_t>(i)) {
          argi->extra = static_cast<Elf32_Word>(j);
          break;
        }
      }
    } else if (argi->type == GBE_ARG_TYPE_SAMPLER) {
      assert(sampler_data_sz > 0);
      for (size_t j = 0; j < sampler_data_sz / sizeof(uint32_t); j++) {
        if (((sampler_data[j] & __CLK_SAMPLER_ARG_MASK) >> __CLK_SAMPLER_ARG_BASE) ==
            static_cast<uint32_t>(i)) {
          argi->extra = static_cast<Elf32_Word>(j);
          break;
        }
      }
    } else {
      argi->extra = 0;
    }

    argi->offset = kernel.getCurbeOffset(GBE_CURBE_KERNEL_ARGUMENT, i);
    ptr += sizeof(ArgInfoHelper);
  }

  /* Store all the sampler info */
  *(reinterpret_cast<Elf32_Word *>(ptr)) =
    static_cast<Elf32_Word>(sampler_data_sz / sizeof(uint32_t)); // Samper number
  ptr = ptr + sizeof(Elf32_Word);
  if (sampler_data_sz > 0) {
    ::memcpy(ptr, sampler_data, sampler_data_sz);
    GBE_FREE(sampler_data);
    ptr = ptr + sampler_data_sz;
  }

  /* Store all the Image info */
  *(reinterpret_cast<Elf32_Word *>(ptr)) =
    static_cast<Elf32_Word>(image_data_sz / sizeof(ImageInfo)); // Image number
  ptr = static_cast<char *>(ptr) + sizeof(Elf32_Word);
  /* Store all the image info by index */
  if (image_data_sz > 0) {
    for (size_t i = 0; i < image_data_sz / sizeof(ImageInfo); i++) {
      ImageInfoHelper *imgi = reinterpret_cast<ImageInfoHelper *>(ptr);
      imgi->bti = image_data[i].idx;
      imgi->width = image_data[i].wSlot;
      imgi->height = image_data[i].hSlot;
      imgi->depth = image_data[i].depthSlot;
      imgi->data_type = image_data[i].dataTypeSlot;
      imgi->channel_order = image_data[i].channelOrderSlot;
      ptr = ptr + sizeof(ImageInfoHelper);
    }

    GBE_FREE(image_data);
  }

  Elf32_Word *p_virt_phy_num = reinterpret_cast<Elf32_Word *>(ptr);
  ptr = static_cast<char *>(ptr) + sizeof(Elf32_Word);
  unsigned int virt_phy_num = 0;

  emitOneCurbeReg<GBE_CURBE_LOCAL_ID_X, Elf32_Word, 0>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_LOCAL_ID_Y, Elf32_Word, 0>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_LOCAL_ID_Z, Elf32_Word, 0>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_LOCAL_SIZE_X, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_LOCAL_SIZE_Y, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_LOCAL_SIZE_Z, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_ENQUEUED_LOCAL_SIZE_X, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_ENQUEUED_LOCAL_SIZE_Y, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_ENQUEUED_LOCAL_SIZE_Z, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_GLOBAL_SIZE_X, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_GLOBAL_SIZE_Y, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_GLOBAL_SIZE_Z, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_GLOBAL_OFFSET_X, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_GLOBAL_OFFSET_Y, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_GLOBAL_OFFSET_Z, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_GROUP_NUM_X, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_GROUP_NUM_Y, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_GROUP_NUM_Z, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_WORK_DIM, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_BLOCK_IP, Elf32_Half, 0>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_DW_BLOCK_IP, Elf32_Word, 0>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_THREAD_NUM, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_THREAD_ID, Elf32_Word, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_CONSTANT_ADDRSPACE, uint64_t, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_STACK_SIZE, uint64_t, 1>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_EXTRA_ARGUMENT, uint64_t, 0>(virt_phy_num, ptr, kernel);
  emitOneCurbeReg<GBE_CURBE_ENQUEUE_BUF_POINTER, uint64_t, 1>(virt_phy_num, ptr, kernel);
  *p_virt_phy_num = virt_phy_num;

  Elf_Word total_sz = static_cast<char *>(ptr) - static_cast<char *>(kernel_info);
  note_writer->add_note(GEN_NOTE_TYPE_GPU_INFO, kernel.getName(), kernel_info, total_sz);

#if 0
  for (int i = 0; i < (int)total_sz; i++) {
    if (i % 16 == 0)
      printf("\n");
    if (i % 2 == 0)
      printf(" ");
    printf("%2.2x", ((unsigned char *)kernel_info)[i]);
  }
  printf("\n");
  for (int i = 0; i < (int)total_sz / 4; i++) {
    printf(" %d", ((unsigned int *)kernel_info)[i]);
  }
#endif

  GBE_FREE(kernel_info);

  emitOneKernelCLInfo(kernel);
  return symbol_num;
}

void *
GenProgram::toBinaryFormat(size_t &ret_size)
{
  ret_size = 0;
  assert(elf_ctx == NULL);
  elf_ctx = GBE_NEW(GenProgramElfContext, *this);

  if (getGlobalConstantSize() > 0) {
    elf_ctx->rodata_sec = elf_ctx->writer.sections.add(".rodata");
    elf_ctx->rodata_sec->set_type(SHT_PROGBITS);
    elf_ctx->rodata_sec->set_flags(SHF_ALLOC);
    elf_ctx->rodata_sec->set_addr_align(1);

    char *const_data = static_cast<char *>(GBE_MALLOC(getGlobalConstantSize()));
    getGlobalConstantData(const_data);
    elf_ctx->rodata_sec->set_data(const_data, getGlobalConstantSize());
    GBE_FREE(const_data);

    if (getGlobalRelocCount() > 0) {
      elf_ctx->reloc_rodata_sec = elf_ctx->writer.sections.add(".rel.rodata");
      elf_ctx->reloc_rodata_sec->set_type(SHT_RELA);
      elf_ctx->reloc_rodata_sec->set_info(elf_ctx->rodata_sec->get_index());
      elf_ctx->reloc_rodata_sec->set_addr_align(0x4);
      elf_ctx->reloc_rodata_sec->set_entry_size(elf_ctx->writer.get_default_entry_size(SHT_RELA));
      elf_ctx->reloc_rodata_sec->set_link(elf_ctx->sym_sec->get_index());
      elf_ctx->rela = GBE_NEW(relocation_section_accessor, elf_ctx->writer, elf_ctx->reloc_rodata_sec);

      char *reloc_data = static_cast<char *>(GBE_MALLOC(getGlobalRelocCount() * sizeof(ir::RelocEntry)));
      getGlobalRelocTable(reloc_data);
      ir::RelocEntry *rel_entry = reinterpret_cast<ir::RelocEntry *>(reloc_data);
      std::sort(rel_entry, rel_entry + getGlobalRelocCount(),
                [](ir::RelocEntry &a, ir::RelocEntry &b) { return a.defOffset < b.defOffset; });

      std::string last_name;
      unsigned int var_defOffset = 0;
      Elf_Word var_symbol = 0;
      for (uint32_t e = 0; e < getGlobalRelocCount(); e++) {
        if (last_name != relocTable->getEntryName(rel_entry[e])) {
          // Add a global symbol
          var_defOffset = rel_entry[e].defOffset;
          last_name = relocTable->getEntryName(rel_entry[e]);
          assert(last_name != ""); // Must have a name
          var_symbol = elf_ctx->syma->add_symbol(*elf_ctx->stra, last_name.c_str(), var_defOffset,
                                                 this->constantSet->getConstant(last_name).getSize(),
                                                 STB_GLOBAL, STT_OBJECT, 0, elf_ctx->rodata_sec->get_index());
        }
        elf_ctx->rela->add_entry(rel_entry[e].refOffset, var_symbol, (unsigned char)R_386_32,
                                 rel_entry[e].defOffset - var_defOffset);
      }

      GBE_FREE(reloc_data);
    }
  }

  /* Add the note about GPU info */
  std::string gpu_name;
  Elf32_Word gpu_version[3]; // pci-id, major and minor
  if (IS_IVYBRIDGE(deviceID)) {
    gpu_name = "IVYBridge";
    gpu_version[0] = 7;
    gpu_version[1] = 0;
  } else if (IS_BAYTRAIL_T(deviceID)) {
    gpu_name = "BayTrail";
    gpu_version[0] = 7;
    gpu_version[1] = 0;
  } else if (IS_HASWELL(deviceID)) {
    gpu_name = "HasWell";
    gpu_version[0] = 7;
    gpu_version[1] = 5;
  } else if (IS_BROADWELL(deviceID)) {
    gpu_name = "BroadWell";
    gpu_version[0] = 8;
    gpu_version[1] = 0;
  } else if (IS_CHERRYVIEW(deviceID)) {
    gpu_name = "CherryView";
    gpu_version[0] = 8;
    gpu_version[1] = 0;
  } else if (IS_SKYLAKE(deviceID)) {
    gpu_name = "SkyLake";
    gpu_version[0] = 9;
    gpu_version[1] = 0;
  } else if (IS_BROXTON(deviceID)) {
    gpu_name = "BroxTon";
    gpu_version[0] = 9;
    gpu_version[1] = 0;
  }
  gpu_version[2] = deviceID;
  elf_ctx->note_writer->add_note(GenProgramElfContext::GEN_NOTE_TYPE_GPU_VERSION,
                                 gpu_name, gpu_version, sizeof(gpu_version));

  /* Add note info about compiler */
  std::string compiler_name("GBE Compiler");
  Elf32_Word compiler_version[2]; // major and minor
  compiler_version[0] = LLVM_VERSION_MAJOR;
  compiler_version[1] = LLVM_VERSION_MINOR;
  elf_ctx->note_writer->add_note(GenProgramElfContext::GEN_NOTE_TYPE_COMPILER_INFO,
                                 compiler_name, compiler_version, sizeof(compiler_version));

  bool write_cl_version = false;
  uint32_t oclVersion = 0;
  std::vector<std::pair<Elf32_Word, Elf32_Word>> device_enqueue_kernel_map;

  for (map<std::string, Kernel *>::const_iterator it = kernels.begin();
       it != kernels.end(); ++it) {
    GenKernel *k = static_cast<GenKernel *>(it->second);

    if (write_cl_version == false) {
      std::string ocl_version_str;
      oclVersion = k->getOclVersion(); // major and minor
      if (oclVersion == 120) {
        ocl_version_str = "OpenCL 1.2";
      } else if (oclVersion == 200) {
        ocl_version_str = "OpenCL 2.0";
      } else
        assert(0);

      elf_ctx->cl_note_writer->add_note(GenProgramElfContext::GEN_NOTE_TYPE_CL_VERSION,
                                        ocl_version_str, &oclVersion, sizeof(oclVersion));
      write_cl_version = true;
    } else {
      assert(oclVersion == k->getOclVersion());
    }

    Elf32_Word kernel_sym_num = elf_ctx->emitOneKernel(*k);

    if (getDeviceEnqueueKernelNameGetSize() > 0) {
      for (size_t k = 0; k < getDeviceEnqueueKernelNameGetSize(); k++) {
        std::string ss = getDeviceEnqueueKernelName(k);
        if (it->first == ss) {
          device_enqueue_kernel_map.push_back(std::pair<Elf32_Word, Elf32_Word>(k, kernel_sym_num));
        }
      }
    }
  }

  if (device_enqueue_kernel_map.size() > 0) {
    Elf32_Word *dek_map =
      (Elf32_Word *)GBE_MALLOC(device_enqueue_kernel_map.size() * sizeof(Elf32_Word) * 2);
    size_t k = 0;
    for (auto &it : device_enqueue_kernel_map) {
      dek_map[k * 2] = it.first;
      dek_map[k * 2 + 1] = it.second;
      k++;
    }

    elf_ctx->cl_note_writer->add_note(GenProgramElfContext::GEN_NOTE_TYPE_CL_DEVICE_ENQUEUE_INFO,
                                      "Open CL 2.0 Devuce Enqueue Kernel Names", dek_map,
                                      (device_enqueue_kernel_map.size() * sizeof(Elf32_Word) * 2));
    GBE_FREE(dek_map);
  }

  wmemstreambuf membuf(4096);
  std::ostream oss(&membuf);
  if (OCL_DUMP_ELF_FILE) {
    elf_ctx->writer.save("gbe_program_elf_dump.o");
  }
  elf_ctx->writer.save(oss);
  GBE_DELETE(elf_ctx);

  size_t elf_size = 0;
  char *elf_mem = membuf.getcontent(elf_size);
  if (elf_size == 0)
    return NULL;

  void *p_elf_ret = ::malloc(elf_size);
  ::memcpy(p_elf_ret, elf_mem, elf_size);
  ret_size = elf_size;
  return p_elf_ret;
}

} /* namespace gbe */
