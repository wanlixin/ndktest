LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := testwait
LOCAL_SRC_FILES := testwait.c

include $(BUILD_EXECUTABLE)
