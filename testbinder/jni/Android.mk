LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := testbinder
LOCAL_SRC_FILES := testbinder.cpp

LOCAL_LDFLAGS := -landroid_runtime -lutils -lbinder -L$(LOCAL_PATH)/lib

include $(BUILD_EXECUTABLE)
