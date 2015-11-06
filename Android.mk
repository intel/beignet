LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

BEIGNET_ROOT_PATH := $(LOCAL_PATH)

#subdirs := backend/src/libocl


subdirs := backend/src/libocl \
           backend/src \
	   src \
           utests \

include $(addprefix $(LOCAL_PATH)/,$(addsuffix /Android.mk, $(subdirs)))
