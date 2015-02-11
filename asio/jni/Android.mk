LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := asio
LOCAL_SRC_FILES := test.cpp

LOCAL_C_INCLUDES += $(LOCAL_PATH)/asio

LOCAL_CPPFLAGS := --std=c++11 -fexceptions -frtti -DASIO_STANDALONE

include $(BUILD_EXECUTABLE)
