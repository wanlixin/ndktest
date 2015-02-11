LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := testasm
LOCAL_SRC_FILES := testasm.S
LOCAL_LDFLAGS += -nostdlib -static

include $(BUILD_EXECUTABLE)
