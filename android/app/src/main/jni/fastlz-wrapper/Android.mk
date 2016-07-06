LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := fastlz-wrapper

LOCAL_CFLAGS := -std=gnu++11
LOCAL_CFLAGS += -funwind-tables -Wl,--no-merge-exidx-entries
LOCAL_CPPFLAGS += -fexceptions

LOCAL_C_INCLUDES += $NDK/sources/cxx-stl/gnu-libstdc++/4.8/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_SRC_FILES  := 

#fastlz

FASTLZ_PATH := ../fastlz

LOCAL_SRC_FILES += 

LOCAL_LDLIBS    := -llog

include $(BUILD_SHARED_LIBRARY)
