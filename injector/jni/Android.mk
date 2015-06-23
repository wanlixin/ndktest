LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := injector
LOCAL_SRC_FILES := injector.cpp
LOCAL_CFLAGS += -fvisibility=hidden

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE    := injected
LOCAL_SRC_FILES := injected.cpp
LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_LDFLAGS += -llog

include $(BUILD_SHARED_LIBRARY)
