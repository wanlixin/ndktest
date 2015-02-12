LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := callstack
LOCAL_SRC_FILES := callstack.c

LOCAL_LDFLAGS := -lcorkscrew -L.

include $(BUILD_EXECUTABLE)
