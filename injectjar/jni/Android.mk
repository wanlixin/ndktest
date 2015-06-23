LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := injector
LOCAL_SRC_FILES := injector.cpp
LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_LDFLAGS += -llog -landroid_runtime -L$(LOCAL_PATH)

include $(BUILD_SHARED_LIBRARY)
