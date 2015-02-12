LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := liblibev
LOCAL_SRC_FILES := ev.c event.c libev.c

include $(BUILD_EXECUTABLE)
