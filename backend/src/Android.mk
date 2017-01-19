LOCAL_PATH:= $(call my-dir)
include $(LOCAL_PATH)/../../Android.common.mk

include $(CLEAR_VARS)
include $(CLEAR_TBLGEN_VARS)

LLVM_ROOT_PATH := external/llvm
CLANG_ROOT_PATH := external/clang

include $(CLANG_ROOT_PATH)/clang.mk

BACKEND_SRC_FILES:= \
    ${ocl_blob_file} \
    sys/vector.hpp \
    sys/map.hpp \
    sys/set.hpp \
    sys/intrusive_list.hpp \
    sys/intrusive_list.cpp \
    sys/exception.hpp \
    sys/assert.cpp \
    sys/assert.hpp \
    sys/alloc.cpp \
    sys/alloc.hpp \
    sys/mutex.cpp \
    sys/mutex.hpp \
    sys/platform.cpp \
    sys/platform.hpp \
    sys/cvar.cpp \
    sys/cvar.hpp \
    ir/context.cpp \
    ir/context.hpp \
    ir/profile.cpp \
    ir/profile.hpp \
    ir/type.cpp \
    ir/type.hpp \
    ir/unit.cpp \
    ir/unit.hpp \
    ir/constant.cpp \
    ir/constant.hpp \
    ir/sampler.cpp \
    ir/sampler.hpp \
    ir/image.cpp \
    ir/image.hpp \
    ir/half.cpp \
    ir/half.hpp \
    ir/instruction.cpp \
    ir/instruction.hpp \
    ir/liveness.cpp \
    ir/register.cpp \
    ir/register.hpp \
    ir/function.cpp \
    ir/function.hpp \
    ir/profiling.cpp \
    ir/profiling.hpp \
    ir/value.cpp \
    ir/value.hpp \
    ir/lowering.cpp \
    ir/lowering.hpp \
    ir/printf.cpp \
    ir/printf.hpp \
    ir/immediate.hpp \
    ir/immediate.cpp \
    ir/structurizer.hpp \
    ir/structurizer.cpp \
    ir/reloc.hpp \
    ir/reloc.cpp \
    backend/context.cpp \
    backend/context.hpp \
    backend/program.cpp \
    backend/program.hpp \
    backend/program.h \
    llvm/llvm_sampler_fix.cpp \
    llvm/llvm_bitcode_link.cpp \
    llvm/llvm_gen_backend.cpp \
    llvm/llvm_passes.cpp \
    llvm/llvm_scalarize.cpp \
    llvm/llvm_intrinsic_lowering.cpp \
    llvm/llvm_barrier_nodup.cpp \
    llvm/llvm_printf_parser.cpp \
    llvm/ExpandConstantExpr.cpp \
    llvm/ExpandUtils.cpp \
    llvm/PromoteIntegers.cpp \
    llvm/ExpandLargeIntegers.cpp \
    llvm/StripAttributes.cpp \
    llvm/llvm_device_enqueue.cpp \
    llvm/llvm_to_gen.cpp \
    llvm/llvm_loadstore_optimization.cpp \
    llvm/llvm_gen_backend.hpp \
    llvm/llvm_gen_ocl_function.hxx \
    llvm/llvm_unroll.cpp \
    llvm/llvm_to_gen.hpp \
    llvm/llvm_profiling.cpp \
    backend/gen/gen_mesa_disasm.c \
    backend/gen_insn_selection.cpp \
    backend/gen_insn_selection.hpp \
    backend/gen_insn_selection_optimize.cpp \
    backend/gen_insn_scheduling.cpp \
    backend/gen_insn_scheduling.hpp \
    backend/gen_insn_selection_output.cpp \
    backend/gen_insn_selection_output.hpp \
    backend/gen_reg_allocation.cpp \
    backend/gen_reg_allocation.hpp \
    backend/gen_context.cpp \
    backend/gen_context.hpp \
    backend/gen75_context.hpp \
    backend/gen75_context.cpp \
    backend/gen8_context.hpp \
    backend/gen8_context.cpp \
    backend/gen9_context.hpp \
    backend/gen9_context.cpp \
    backend/gen_program.cpp \
    backend/gen_program.hpp \
    backend/gen_program.h \
    backend/gen7_instruction.hpp \
    backend/gen8_instruction.hpp \
    backend/gen_defs.hpp \
    backend/gen_insn_compact.cpp \
    backend/gen_encoder.hpp \
    backend/gen_encoder.cpp \
    backend/gen7_encoder.hpp \
    backend/gen7_encoder.cpp \
    backend/gen75_encoder.hpp \
    backend/gen75_encoder.cpp \
    backend/gen8_encoder.hpp \
    backend/gen8_encoder.cpp \
    backend/gen9_encoder.hpp \
    backend/gen9_encoder.cpp

#Generate GBEConfig for android
LOCAL_MODULE := libgbe
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

generated_path := $(call local-generated-sources-dir)
gbe_config_file = $(LOCAL_PATH)/GBEConfig.h
$(shell echo "// the configured options and settings for LIBGBE" > $(gbe_config_file))
$(shell echo "#define LIBGBE_VERSION_MAJOR 0" >> $(gbe_config_file))
$(shell echo "#define LIBGBE_VERSION_MINOR 2" >> $(gbe_config_file))
$(shell echo "#if defined(__ANDROID__)" >> $(gbe_config_file))
$(shell echo "#if __x86_64__" >> $(gbe_config_file))
$(shell echo "  #define GBE_OBJECT_DIR \"/system/lib64/libgbe.so\"" >> $(gbe_config_file))
$(shell echo "  #define INTERP_OBJECT_DIR \"/system/lib64/libgbeinterp.so\"" >> $(gbe_config_file))
$(shell echo "  #define OCL_BITCODE_BIN \"/system/lib/ocl/beignet.bc\"" >> $(gbe_config_file))
$(shell echo "  #define OCL_HEADER_DIR \"/system/lib/ocl/include\"" >> $(gbe_config_file))
$(shell echo "  #define OCL_PCH_OBJECT \"/system/lib/ocl/beignet.pch\"" >> $(gbe_config_file))
$(shell echo "  #define OCL_BITCODE_BIN_20 \"/system/lib/ocl/beignet_20.bc\"" >> $(gbe_config_file))
$(shell echo "  #define OCL_PCH_OBJECT_20 \"/system/lib/ocl/beigneti_20.pch\"" >> $(gbe_config_file))
$(shell echo "#else /*__x86_64__*/" >> $(gbe_config_file))
$(shell echo "  #define GBE_OBJECT_DIR \"/system/lib/libgbe.so\"" >> $(gbe_config_file))
$(shell echo "  #define INTERP_OBJECT_DIR \"/system/lib/libgbeinterp.so\"" >> $(gbe_config_file))
$(shell echo "  #define OCL_BITCODE_BIN \"/system/lib/ocl/beignet.bc\"" >> $(gbe_config_file))
$(shell echo "  #define OCL_HEADER_DIR \"/system/lib/ocl/include\"" >> $(gbe_config_file))
$(shell echo "  #define OCL_PCH_OBJECT \"/system/lib/ocl/beignet.pch\"" >> $(gbe_config_file))
$(shell echo "  #define OCL_BITCODE_BIN_20 \"/system/lib/ocl/beignet_20.bc\"" >> $(gbe_config_file))
$(shell echo "  #define OCL_PCH_OBJECT_20 \"/system/lib/ocl/beigneti_20.pch\"" >> $(gbe_config_file))
$(shell echo "#endif" >> $(gbe_config_file))
$(shell echo "#else /*__ANDROID__*/" >> $(gbe_config_file))
$(shell echo "  #define GBE_OBJECT_DIR \"\"" >> $(gbe_config_file))
$(shell echo "  #define INTERP_OBJECT_DIR \"\"" >> $(gbe_config_file))
$(shell echo "  #define OCL_BITCODE_BIN \"`pwd $(TOP)`/$(generated_path)\"" >> $(gbe_config_file))
$(shell echo "  #define OCL_HEADER_DIR \"`pwd $(TOP)`/$(generated_path)/libocl/include\"" >> $(gbe_config_file))
$(shell echo "  #define OCL_PCH_OBJECT \"`pwd $(TOP)`/$(generated_path)\"" >> $(gbe_config_file))
$(shell echo "  #define OCL_BITCODE_BIN_20 \"`pwd $(TOP)`/$(generated_path)\"" >> $(gbe_config_file))
$(shell echo "  #define OCL_PCH_OBJECT_20 \"`pwd $(TOP)`/$(generated_path)\"" >> $(gbe_config_file))
$(shell echo "#endif" >> $(gbe_config_file))

#Build HOST libgbe.so
LOCAL_C_INCLUDES := $(TOP_C_INCLUDE) \
                    $(BEIGNET_ROOT_PATH) \
                    $(LOCAL_PATH)/../ \
                    $(LLVM_INCLUDE_DIRS)
LOCAL_CPPFLAGS +=  $(LLVM_CFLAGS) -std=c++11 -fexceptions -DGBE_DEBUG=0 -DGBE_COMPILER_AVAILABLE=1 -DGEN7_SAMPLER_CLAMP_BORDER_WORKAROUND
LOCAL_CFLAGS +=  $(LLVM_CFLAGS) -fexceptions -DGBE_DEBUG=0 -DGBE_COMPILER_AVAILABLE=1 -DGEN7_SAMPLER_CLAMP_BORDER_WORKAROUND
LOCAL_CPPFLAGS += -Wno-extra-semi -Wno-gnu-anonymous-struct -Wno-nested-anon-types
LOCAL_CFLAGS += -Wno-extra-semi -Wno-gnu-anonymous-struct -Wno-nested-anon-types
LOCAL_LDLIBS += -lpthread -lm -ldl -lLLVM -lclang
#LOCAL_STATIC_LIBRARIES := $(CLANG_MODULE_LIBS)
LOCAL_SHARED_LIBRARIES := libclang

TBLGEN_TABLES :=    \
	         AttrList.inc    \
                 Attrs.inc    \
                 CommentCommandList.inc \
                 CommentNodes.inc \
                 DeclNodes.inc    \
                 DiagnosticCommonKinds.inc   \
                 DiagnosticDriverKinds.inc       \
                 DiagnosticFrontendKinds.inc     \
                 DiagnosticSemaKinds.inc

LOCAL_SRC_FILES = $(BACKEND_SRC_FILES)
include $(CLANG_HOST_BUILD_MK)
include $(CLANG_TBLGEN_RULES_MK)
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_HOST_SHARED_LIBRARY)


#Build gbe_bin_generater
include $(CLEAR_VARS)
LOCAL_SRC_FILES := gbe_bin_generater.cpp

LOCAL_C_INCLUDES := $(TOP_C_INCLUDE) \
                    $(BEIGNET_ROOT_PATH) \
                    $(LOCAL_PATH)/ \
                    $(LLVM_INCLUDE_DIRS)

LOCAL_CLANG := true
LOCAL_MODULE := gbe_bin_generater
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS = $(LLVM_CFLAGS) -std=gnu++11 -fexceptions
LOCAL_SHARED_LIBRARIES := libgbe
LOCAL_LDLIBS += -lpthread -lm -ldl

include $(BUILD_HOST_EXECUTABLE)


#Build libgbeinterp.so
include $(CLEAR_VARS)

LLVM_ROOT_PATH := external/llvm
include $(LLVM_ROOT_PATH)/llvm.mk

LOCAL_C_INCLUDES := $(TOP_C_INCLUDE) \
                    $(BEIGNET_ROOT_PATH) \
                    $(LOCAL_PATH)/../ \
                    $(LLVM_INCLUDE_DIRS)

LOCAL_LDFLAGS := -Wl,--no-undefined

LOCAL_CFLAGS += $(SUBDIR_C_CXX_FLAGS)
LOCAL_CPPFLAGS += -Wl,-E -std=c++11 -DGBE_COMPILER_AVAILABLE=1

LOCAL_MODULE := libgbeinterp
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := gbe_bin_interpreter.cpp
LOCAL_SHARED_LIBRARIES := \
libcutils \
$(DRM_INTEL_LIBRARY) \
$(DRM_LIBRARY)

include $(LLVM_DEVICE_BUILD_MK)
include $(BUILD_SHARED_LIBRARY)

#Build targe libgbe.so
include $(CLEAR_VARS)
include $(CLEAR_TBLGEN_VARS)

LOCAL_C_INCLUDES := $(TOP_C_INCLUDE) \
                    $(BEIGNET_ROOT_PATH) \
                    $(LOCAL_PATH)/../ \
                    $(LLVM_INCLUDE_DIRS)

SUBDIR_C_CXX_FLAGS := -fvisibility=hidden
SUBDIR_C_CXX_FLAGS += -funroll-loops -fstrict-aliasing -msse2 -msse3 -mssse3 -msse4.1 -fPIC -Wall
SUBDIR_C_CXX_FLAGS += $(LLVM_CFLAGS)

LOCAL_CPPFLAGS := $(SUBDIR_C_CXX_FLAGS)
LOCAL_CPPFLAGS += -fno-rtti -std=c++11 -DGBE_DEBUG=1 -DGBE_COMPILER_AVAILABLE=1 -DGEN7_SAMPLER_CLAMP_BORDER_WORKAROUND
LOCAL_CPPFLAGS += -Wl,-E

#LOCAL_SDK_VERSION := 19
#LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_CFLAGS := $(SUBDIR_C_CXX_FLAGS)
LOCAL_CFLAGS += -Wl,-E
LOCAL_LDFLAGS := -Wl,--no-undefined
LOCAL_LDLIBS := $(LLVM_LFLAGS)

LOCAL_MODULE := libgbe
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := \
libcutils \
$(DRM_INTEL_LIBRARY) \
$(DRM_LIBRARY) \
libclang libLLVM
#$(THREAD_LIBS_INIT)
#$(DL_LIBS)

#LOCAL_STATIC_LIBRARIES := $(CLANG_MODULE_LIBS)

TBLGEN_TABLES :=    \
	         AttrList.inc    \
                 Attrs.inc    \
                 CommentCommandList.inc \
                 CommentNodes.inc \
                 DeclNodes.inc    \
                 DiagnosticCommonKinds.inc   \
                 DiagnosticDriverKinds.inc       \
                 DiagnosticFrontendKinds.inc     \
                 DiagnosticSemaKinds.inc

LOCAL_SRC_FILES := $(BACKEND_SRC_FILES)

include $(CLANG_DEVICE_BUILD_MK)
include $(CLANG_TBLGEN_RULES_MK)
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_SHARED_LIBRARY)

