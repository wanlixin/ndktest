LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := testsignal
LOCAL_SRC_FILES := testsignal.c

include $(BUILD_EXECUTABLE)
