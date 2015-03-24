LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := testdgram
LOCAL_SRC_FILES := testdgram.c

include $(BUILD_EXECUTABLE)
