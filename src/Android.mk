LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

include $(LOCAL_PATH)/../Android.common.mk

ocl_config_file = $(LOCAL_PATH)/OCLConfig.h
$(shell echo "// the configured options and settings for LIBCL" > $(ocl_config_file))
$(shell echo "#define LIBCL_DRIVER_VERSION_MAJOR 1" >> $(ocl_config_file))
$(shell echo "#define LIBCL_DRIVER_VERSION_MINOR 2" >> $(ocl_config_file))
$(shell echo "#define LIBCL_C_VERSION_MAJOR 1" >> $(ocl_config_file))
$(shell echo "#define LIBCL_C_VERSION_MINOR 2" >> $(ocl_config_file))

LOCAL_C_INCLUDES := $(TOP_C_INCLUDE) $(BEIGNET_ROOT_PATH)/backend/src/backend/ $(BEIGNET_ROOT_PATH)
LOCAL_C_INCLUDES += $(DRM_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(LLVM_INCLUDE_DIRS)
LOCAL_C_INCLUDES += hardware/drm_gralloc
LOCAL_CPPFLAGS := $(TOP_CPPFLAGS) -std=c++11 -DHAS_USERPTR
LOCAL_CFLAGS := $(TOP_CFLAGS) -DHAS_USERPTR
OPTIONAL_EGL_LIBRARY :=
LOCAL_LDFLAGS := -Wl,-Bsymbolic

LOCAL_LDLIBS := -lm -ldl
LOCAL_SHARED_LIBRARIES += liblog libcutils
LOCAL_ADDITIONAL_DEPENDENCIES := $(GBE_BIN_GENERATER)
LOCAL_MODULE := libcl

LOCAL_REQUIRED_MODULES := $(HOST_OUT_EXECUTABLES)/gbe_bin_generater
LOCAL_ADDITIONAL_DEPENDENCIES := $(BEIGNET_ROOT_PATH)/backend/src/Android.mk

KERNEL_PATH := $(BEIGNET_ROOT_PATH)/src/kernels
KERNEL_NAMES := cl_internal_copy_buf_align4 \
                cl_internal_copy_buf_align16 \
                cl_internal_copy_buf_unalign_same_offset \
                cl_internal_copy_buf_unalign_dst_offset \
                cl_internal_copy_buf_unalign_src_offset \
                cl_internal_copy_buf_rect \
                cl_internal_copy_buf_rect_align4 \
                cl_internal_copy_image_1d_to_1d \
                cl_internal_copy_image_2d_to_2d \
                cl_internal_copy_image_3d_to_2d \
                cl_internal_copy_image_2d_to_3d \
                cl_internal_copy_image_3d_to_3d \
                cl_internal_copy_image_2d_to_2d_array \
                cl_internal_copy_image_1d_array_to_1d_array \
                cl_internal_copy_image_2d_array_to_2d_array \
                cl_internal_copy_image_2d_array_to_2d \
                cl_internal_copy_image_2d_array_to_3d \
                cl_internal_copy_image_3d_to_2d_array \
                cl_internal_copy_image_2d_to_buffer \
                cl_internal_copy_image_2d_to_buffer_align16 \
                cl_internal_copy_image_3d_to_buffer \
                cl_internal_copy_buffer_to_image_2d \
                cl_internal_copy_buffer_to_image_2d_align16 \
                cl_internal_copy_buffer_to_image_3d \
                cl_internal_fill_buf_align8 \
                cl_internal_fill_buf_align4 \
                cl_internal_fill_buf_align2 \
                cl_internal_fill_buf_unalign \
                cl_internal_fill_buf_align128 \
                cl_internal_fill_image_1d \
                cl_internal_fill_image_1d_array \
                cl_internal_fill_image_2d \
                cl_internal_fill_image_2d_array \
                cl_internal_fill_image_3d
BUILT_IN_NAME := cl_internal_built_in_kernel

GBE_BIN_GENERATER := $(HOST_OUT_EXECUTABLES)/gbe_bin_generater

$(shell rm $(KERNEL_PATH)/$(BUILT_IN_NAME).cl)
define GEN_INTERNAL_KER
    # Use the python script to generate the header files.
    $(shell $(GBE_BIN_GENERATER) -s $(KERNEL_PATH)/$(1).cl -o $(KERNEL_PATH)/$(1)_str.c)
    $(shell cat $(KERNEL_PATH)/$(1).cl >> $(KERNEL_PATH)/$(BUILT_IN_NAME).cl)
endef
$(foreach KERNEL_NAME, ${KERNEL_NAMES}, $(eval $(call GEN_INTERNAL_KER,$(KERNEL_NAME))))

$(shell $(GBE_BIN_GENERATER) -s $(KERNEL_PATH)/$(BUILT_IN_NAME).cl -o $(KERNEL_PATH)/$(BUILT_IN_NAME)_str.c)

GIT_SHA1 = git_sha1.h
$(shell chmod +x $(LOCAL_PATH)/git_sha1.sh)
$(shell $(LOCAL_PATH)/git_sha1.sh $(LOCAL_PATH) ${GIT_SHA1})

LOCAL_SRC_FILES:= \
    $(addprefix kernels/,$(addsuffix _str.c, $(KERNEL_NAMES))) \
    $(addprefix kernels/,$(addsuffix _str.c, $(BUILT_IN_NAME))) \
    cl_base_object.c \
    cl_api.c \
    cl_api_platform_id.c \
    cl_api_device_id.c \
    cl_api_mem.c \
    cl_api_kernel.c \
    cl_api_command_queue.c \
    cl_api_event.c \
    cl_api_context.c \
    cl_api_sampler.c \
    cl_api_program.c \
    cl_alloc.c \
    cl_kernel.c \
    cl_program.c \
    cl_gbe_loader.cpp \
    cl_sampler.c \
    cl_accelerator_intel.c \
    cl_event.c \
    cl_enqueue.c \
    cl_image.c \
    cl_mem.c \
    cl_platform_id.c \
    cl_extensions.c \
    cl_device_id.c \
    cl_context.c \
    cl_command_queue.c \
    cl_command_queue.h \
    cl_command_queue_gen7.c \
    cl_command_queue_enqueue.c \
    cl_device_enqueue.c \
    cl_utils.c \
    cl_driver.h \
    cl_driver.cpp \
    cl_driver_defs.c \
    intel/intel_gpgpu.c \
    intel/intel_batchbuffer.c \
    intel/intel_driver.c \
    performance.c

LOCAL_SHARED_LIBRARIES := \
libgbe \
libdl \
$(DRM_INTEL_LIBRARY) \
$(DRM_LIBRARY) \
$(OPTIONAL_EGL_LIBRARY) \
libhardware

#LOCAL_CLANG := true
include external/libcxx/libcxx.mk
include $(BUILD_SHARED_LIBRARY)
