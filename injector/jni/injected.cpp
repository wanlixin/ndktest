#include <android/log.h>

#define TRACE(...)   ((void)__android_log_print(ANDROID_LOG_DEBUG, "injected", __VA_ARGS__))

extern "C" __attribute__ ((visibility("default")))
void entry(const char* param) {
	TRACE("injected %s", param);
}
