#LOCAL_PATH:= $(call my-dir)

#include $(CLEAR_VARS)
TOP_C_INCLUDE := bionic $(BEIGNET_ROOT_PATH)/include
TOP_CPPFLAGS := -Wall -Wno-invalid-offsetof -mfpmath=sse -fno-rtti -Wcast-align -std=c++11 -msse2 -msse3 -mssse3 -msse4.1 -D__ANDROID__
TOP_CFLAGS := -Wall -mfpmath=sse -msse2 -Wcast-align -msse2 -msse3 -mssse3 -msse4.1 -D__ANDROID__

LLVM_INCLUDE_DIRS := external/llvm/device/include\
                     external/llvm/include \
                     external/clang/include \

LLVM_CFLAGS := -DNDEBUG -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
LLVM_LFLAGS := -ldl -lm

LLVM_FOUND := true

DRM_INCLUDE_PATH := external/drm/intel external/drm/include/drm external/drm
DRM_LIBRARY := libdrm
DRM_FOUND := true

THREAD_LIBS_INIT := libpthread

DRM_INTEL_LIBRARY := libdrm_intel
DRM_INTEL_FOUND := true

GBE_LIBRARY := libgbe
GBE_FOUND := false

OCLIcd_FOUND := false

