LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libev
LOCAL_SRC_FILES := ev.c event.c libev.c

include $(BUILD_EXECUTABLE)
