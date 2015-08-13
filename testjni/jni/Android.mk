LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := testjni
LOCAL_SRC_FILES := testjni.cpp
LOCAL_LDFLAGS := -llog

include $(BUILD_SHARED_LIBRARY)
