LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libgbe
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

generated_sources := $(call local-generated-sources-dir)/libocl

$(shell mkdir -p ${generated_sources}/include/)
$(shell mkdir -p ${generated_sources}/src/)
#$(shell echo "cat $(LOCAL_PATH)/tmpl/ocl_defines.tmpl.h \\> ${LIBOCL_BINARY_DIR}/include/ocl_defines.h")
$(shell cat $(LOCAL_PATH)/tmpl/ocl_defines.tmpl.h > ${generated_sources}/include/ocl_defines.h)
#$(shell echo "cat $(LOCAL_PATH)/../ocl_common_defines.h \\>\\> ${LIBOCL_BINARY_DIR}/include/ocl_defines.h")
$(shell cat ${LOCAL_PATH}/../ocl_common_defines.h >> ${generated_sources}/include/ocl_defines.h)
$(shell echo "Generate the header: ${generated_sources}/include/ocl_defines.h")

define COPY_THE_HEADER
    # Use the python script to generate the header files.
    $(shell cp ${LOCAL_PATH}/include/$(1).h ${generated_sources}/include/$(1).h)
endef
define COPY_THE_SOURCE
    # Use the python script to generate the header files.
    $(shell cp ${LOCAL_PATH}/src/$(1).cl ${generated_sources}/src/$(1).cl)
endef

OCL_COPY_MODULES := ocl ocl_types ocl_float ocl_printf
$(foreach _M_, ${OCL_COPY_MODULES}, $(eval $(call COPY_THE_HEADER,$(_M_))))

OCL_COPY_MODULES := ocl_workitem ocl_atom ocl_async ocl_sync ocl_memcpy ocl_memset ocl_misc ocl_vload ocl_geometric ocl_image ocl_work_group
OCL_SOURCE_FILES := $(OCL_COPY_MODULES)
$(foreach _M_, ${OCL_COPY_MODULES}, $(eval $(call COPY_THE_HEADER,$(_M_))))
$(foreach _M_, ${OCL_COPY_MODULES}, $(eval $(call COPY_THE_SOURCE,$(_M_))))

define GENERATE_HEADER_PY
    # Use the python script to generate the header files.
    $(shell cat ${LOCAL_PATH}/tmpl/$(1).tmpl.h > ${generated_sources}/include/$(1).h)
    $(shell /usr/bin/python ${LOCAL_PATH}/script/gen_vector.py ${LOCAL_PATH}/script/$(1).def ${generated_sources}/include/$(1).h 1)
    $(shell echo "#endif" >> ${generated_sources}/include/$(1).h)
endef
define GENERATE_SOURCE_PY
    # Use the python script to generate the header files.
    $(shell cat ${LOCAL_PATH}/tmpl/$(1).tmpl.cl > ${generated_sources}/src/$(1).cl)
    $(shell /usr/bin/python ${LOCAL_PATH}/script/gen_vector.py ${LOCAL_PATH}/script/$(1).def ${generated_sources}/src/$(1).cl 0)
endef

OCL_COPY_MODULES_PY := ocl_common ocl_relational ocl_integer ocl_math ocl_simd
OCL_SOURCE_FILES += $(OCL_COPY_MODULES_PY)
$(foreach _M_, ${OCL_COPY_MODULES_PY}, $(eval $(call GENERATE_HEADER_PY,$(_M_))))
$(foreach _M_, ${OCL_COPY_MODULES_PY}, $(eval $(call GENERATE_SOURCE_PY,$(_M_))))

define GENERATE_HEADER_BASH
    # Use the python script to generate the header files.\
    $(shell ${LOCAL_PATH}/script/$(1).sh -p > ${generated_sources}/include/$(1).h)
endef
define GENERATE_SOURCE_BASH
    # Use the python script to generate the header files.
    $(shell ${LOCAL_PATH}/script/$(1).sh > ${generated_sources}/src/$(1).cl)
endef
OCL_COPY_MODULES_SH := ocl_as ocl_convert
OCL_SOURCE_FILES += $(OCL_COPY_MODULES_SH)
$(foreach _M_, ${OCL_COPY_MODULES_SH}, $(eval $(call GENERATE_HEADER_BASH,$(_M_))))
$(foreach _M_, ${OCL_COPY_MODULES_SH}, $(eval $(call GENERATE_SOURCE_BASH,$(_M_))))

CLANG_OCL_FLAGS := -fno-builtin -ffp-contract=off -cl-kernel-arg-info -DGEN7_SAMPLER_CLAMP_BORDER_WORKAROUND "-cl-std=CL1.2"
define ADD_CL_TO_BC_TARGET
    # Use the python script to generate the header files.
    $(shell $(HOST_OUT)/bin/clang -cc1 ${CLANG_OCL_FLAGS} -I ${generated_sources}/include/ -emit-llvm-bc -triple spir -o ${generated_sources}/$(1).bc -x cl ${generated_sources}/src/$(1).cl)
endef
$(foreach _M_, ${OCL_SOURCE_FILES}, $(eval $(call ADD_CL_TO_BC_TARGET,$(_M_))))

define COPY_THE_LL
    # Use the python script to generate the header files.
    $(shell cp ${LOCAL_PATH}/src/$(1).ll ${generated_sources}/src/$(1).ll)
endef
define ADD_LL_TO_BC_TARGET
    # Use the python script to generate the header files.
    $(shell $(HOST_OUT)/bin/llvm-as -o ${generated_sources}/$(1).bc ${generated_sources}/src/$(1).ll)
endef
OCL_LL_MODULES := ocl_barrier ocl_clz
OCL_SOURCE_FILES += $(OCL_LL_MODULES)
$(foreach _M_, ${OCL_LL_MODULES}, $(eval $(call COPY_THE_LL,$(_M_))))
$(foreach _M_, ${OCL_LL_MODULES}, $(eval $(call ADD_LL_TO_BC_TARGET,$(_M_))))

$(shell $(HOST_OUT)/bin/llvm-link -o ${generated_sources}/../beignet.bc $(addprefix ${generated_sources}/, $(addsuffix .bc, ${OCL_SOURCE_FILES})))

$(shell $(HOST_OUT)/bin/clang -cc1 ${CLANG_OCL_FLAGS} -triple spir -I ${generated_sources}/include/ --relocatable-pch -emit-pch -isysroot ${generated_sources} -x cl ${generated_sources}/include/ocl.h -o ${generated_sources}/../beignet.pch)

