#include <unistd.h>
#include <jni.h>
#include <android/log.h>

JNIEXPORT jint JNICALL test1(JNIEnv *env, jclass self, jobjectArray para) {
	int argc = env->GetArrayLength(para);
	const char* argv[argc];
	jstring args[argc];

	for (int i = 0; i < argc; i++) {
		args[i] =
				reinterpret_cast<jstring>(env->GetObjectArrayElement(para, i));
		argv[i] = env->GetStringUTFChars(args[i], NULL);
	}

	for (int i = 0; i < argc; i++) {
		__android_log_print(ANDROID_LOG_VERBOSE, "testjni", "%d = %s", i,
				argv[i]);
	}

	for (int i = 0; i < argc; i++) {
		env->ReleaseStringUTFChars(args[i], argv[i]);
	}

	return argc;
}

JNIEXPORT jstring JNICALL test2(JNIEnv *env, jclass self, jobjectArray para) {
	int argc = env->GetArrayLength(para);
	const char* argv[argc];
	jstring args[argc];

	for (int i = 0; i < argc; i++) {
		args[i] =
				reinterpret_cast<jstring>(env->GetObjectArrayElement(para, i));
		argv[i] = env->GetStringUTFChars(args[i], NULL);
	}

	for (int i = 0; i < argc; i++) {
		__android_log_print(ANDROID_LOG_VERBOSE, "testjni", "%d = %s", i,
				argv[i]);
	}

	for (int i = 0; i < argc; i++) {
		env->ReleaseStringUTFChars(args[i], argv[i]);
	}

	return env->NewStringUTF("hello, world");
}

int registerNativeMethods(JNIEnv* env, const char* className,
		JNINativeMethod* gMethods, int numMethods) {
	jclass clazz;

	clazz = env->FindClass(className);
	if (clazz == NULL) {
		return JNI_FALSE;
	}
	if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

static JNINativeMethod methods[] = {

{ "test1", "([Ljava/lang/String;)I", (void*) test1 },

{ "test2", "([Ljava/lang/String;)Ljava/lang/String;", (void*) test2 },

};

jint JNI_OnLoad(JavaVM* vm, void* reserved) {

	JNIEnv* env = NULL;
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		return JNI_ERR;
	}

	if (!registerNativeMethods(env, "com/example/testjni/MainActivity", methods,
			sizeof(methods) / sizeof(methods[0]))) {
		return JNI_ERR;
	}

	return JNI_VERSION_1_4;
}
