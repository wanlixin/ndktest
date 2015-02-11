LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := thread
LOCAL_SRC_FILES := thread.cpp

LOCAL_CPPFLAGS := --std=c++11

include $(BUILD_EXECUTABLE)
