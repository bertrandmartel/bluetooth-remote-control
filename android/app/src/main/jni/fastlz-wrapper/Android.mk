LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := bleremote

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../fastlz
LOCAL_SRC_FILES  := bleremote_wrapper.c ../fastlz/fastlz.c

LOCAL_LDLIBS    := -llog

include $(BUILD_SHARED_LIBRARY)
