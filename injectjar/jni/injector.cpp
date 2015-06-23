#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <dlfcn.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <jni.h>
#include <android/log.h>

#define ERESTARTSYS		512
#define ERESTARTNOINTR		513
#define ERESTARTNOHAND  	514
#define ERESTART_RESTARTBLOCK	516

#define CPSR_T_MASK			0x20

#define TRACE(...)   ((void)__android_log_print(ANDROID_LOG_DEBUG, "injectjar", __VA_ARGS__))

class CPtrace {
public:
	CPtrace(pid_t pid) :
			m_pid(pid) {
	}

	~CPtrace() {
		restart_syscall();

		if (ptrace(PTRACE_SETREGS, m_pid, 0, &m_regs_bak) < 0) {
			TRACE("setregs %s", strerror(errno));
		}

		if (ptrace(PTRACE_DETACH, m_pid, NULL, NULL) < 0) {
			TRACE("detach %s", strerror(errno));
		}
	}

	bool attach() {
		if (ptrace(PTRACE_ATTACH, m_pid, NULL, NULL) < 0) {
			TRACE("attach %s", strerror(errno));
			return false;
		}

		int status;
		do {
			pid_t w = waitpid(m_pid, &status, WUNTRACED | WCONTINUED);
			if (w == -1) {
				TRACE("waitpid %s", strerror(errno));
				return false;
			}

			if (WIFEXITED(status)) {
				TRACE("exited, status=%d", WEXITSTATUS(status));
				return false;
			} else if (WIFSIGNALED(status)) {
				TRACE("killed by signal %d", WTERMSIG(status));
				return false;
			} else if (WIFSTOPPED(status)) {
				if (WSTOPSIG(status) != SIGSTOP) {
					TRACE("stopped by signal %d", WSTOPSIG(status));
				}
				break;
			} else {
				TRACE("waitpid %08x", status);
			}
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));

		if (ptrace(PTRACE_GETREGS, m_pid, 0, &m_regs_bak) < 0) {
			TRACE("getregs %s", strerror(errno));
			return false;
		}

		m_regs = m_regs_bak;
		return true;
	}

	bool call(void* addr, long* params, int num_params) {
		int i;
		for (i = 0; i < num_params && i < 4; i++) {
			m_regs.uregs[i] = params[i];
		}

		if (i < num_params) {
			int len = (num_params - i) * sizeof(size_t);
			m_regs.ARM_sp -= len;
			if (!write_data((void*) m_regs.ARM_sp, (void*) &params[i], len)) {
				return false;
			}
		}

		m_regs.ARM_pc = (long) addr;
		if (m_regs.ARM_pc & 1) {
			m_regs.ARM_pc &= (~1u);
			m_regs.ARM_cpsr |= CPSR_T_MASK;
		} else {
			m_regs.ARM_cpsr &= ~CPSR_T_MASK;
		}
		m_regs.ARM_lr = 0;

		if (ptrace(PTRACE_SETREGS, m_pid, 0, &m_regs) < 0) {
			TRACE("setregs %s", strerror(errno));
			return false;
		}

		if (ptrace(PTRACE_CONT, m_pid, NULL, 0) < 0) {
			TRACE("cont %s", strerror(errno));
			return false;
		}

		int status;
		do {
			pid_t w = waitpid(m_pid, &status, WUNTRACED | WCONTINUED);
			if (w == -1) {
				TRACE("waitpid %s", strerror(errno));
				return false;
			}

			if (WIFEXITED(status)) {
				TRACE("exited, status=%d", WEXITSTATUS(status));
				return false;
			} else if (WIFSIGNALED(status)) {
				TRACE("killed by signal %d", WTERMSIG(status));
				return false;
			} else if (WIFSTOPPED(status)) {
				if (WSTOPSIG(status) != SIGSEGV) {
					TRACE("stopped by signal %d", WSTOPSIG(status));
				}
				break;
			} else {
				TRACE("waitpid %08x", status);
			}
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));

		return true;
	}

	long retval() {
		if (ptrace(PTRACE_GETREGS, m_pid, 0, &m_regs) < 0) {
			TRACE("getregs %s", strerror(errno));
			return 0;
		}

		return m_regs.ARM_r0;
	}

	void* dlopen(void* remote_dlopen, const char* filename) {
		m_regs = m_regs_bak;
		int len = strlen(filename) + 1;
		m_regs.ARM_sp -= len;
		m_regs.ARM_sp &= ~3;
		write_data((void*) m_regs.ARM_sp, filename, len);

		long params[2] = { m_regs.ARM_sp, (RTLD_NOW | RTLD_GLOBAL) };
		call(remote_dlopen, params, 2);
		return (void*) retval();
	}

	void* dlsym(void* remote_dlsym, void* handle, const char* symbol) {
		m_regs = m_regs_bak;
		int len = strlen(symbol) + 1;
		m_regs.ARM_sp -= len;
		m_regs.ARM_sp &= ~3;
		write_data((void*) m_regs.ARM_sp, symbol, len);

		long params[2] = { (long) handle, m_regs.ARM_sp };
		call(remote_dlsym, params, 2);
		return (void*) retval();
	}

	void dlclose(void* remote_dlclose, void* handle) {
		long params[1] = { (long) handle };
		call(remote_dlclose, params, 1);
	}

	void call_entry(void* entry, const char* jar_path, const char* entryName) {
		m_regs = m_regs_bak;

		int len = strlen(jar_path) + 1;
		m_regs.ARM_sp -= len;
		m_regs.ARM_sp &= ~3;
		write_data((void*) m_regs.ARM_sp, jar_path, len);
		long param1 = m_regs.ARM_sp;

		len = strlen(entryName) + 1;
		m_regs.ARM_sp -= len;
		m_regs.ARM_sp &= ~3;
		write_data((void*) m_regs.ARM_sp, entryName, len);
		long param2 = m_regs.ARM_sp;

		long params[2] = { param1, param2 };
		call(entry, params, 2);
	}

	void* get_module_base(pid_t pid, const char* module_name) {
		FILE* fp;
		long addr = 0;
		char* pch;
		char filename[32];
		char line[1024];

		if (pid < 0) {
			snprintf(filename, sizeof(filename), "/proc/self/maps");
		} else {
			snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
		}

		fp = fopen(filename, "r");

		if (fp != NULL) {
			while (fgets(line, sizeof(line), fp)) {
				if (strstr(line, module_name)) {
					pch = strtok(line, "-");
					addr = strtoul(pch, NULL, 16);

					if (addr == 0x8000)
						addr = 0;

					break;
				}
			}

			fclose(fp);
		}

		return (void*) addr;
	}

protected:
	void read_data(void* target, void* buf, size_t size) {
		long* dst = (long*) buf;
		long* src = (long*) target;

		int len = size / 4;
		for (int i = 0; i < len; i++, src++) {
			*dst++ = ptrace(PTRACE_PEEKTEXT, m_pid, src, 0);
		}

		int remain = size % 4;
		if (remain > 0) {
			long val = ptrace(PTRACE_PEEKTEXT, m_pid, src, 0);
			uint8_t* psrc = (uint8_t*) &val;
			uint8_t* pdst = (uint8_t*) dst;
			for (int i = 0; i < remain; i++) {
				*pdst++ = *psrc++;
			}
		}
	}

	bool write_data(void* dest, const void* data, size_t size) {
		long* dst = (long*) dest;
		long* src = (long*) data;

		int len = size / 4;
		for (int i = 0; i < len; i++, src++, dst++) {
			if (ptrace(PTRACE_POKETEXT, m_pid, dst, *src) < 0) {
				TRACE("poketext %s", strerror(errno));
				return false;
			}
		}

		int remain = size % 4;
		if (remain > 0) {
			long val = ptrace(PTRACE_PEEKTEXT, m_pid, dst, 0);
			uint8_t* psrc = (uint8_t*) src;
			uint8_t* pdst = (uint8_t*) &val;
			for (int i = 0; i < remain; i++) {
				*pdst++ = *psrc++;
			}

			if (ptrace(PTRACE_POKETEXT, m_pid, dst, val) < 0) {
				TRACE("poketext %s", strerror(errno));
				return false;
			}
		}

		return true;
	}

	void restart_syscall() {
		switch (m_regs_bak.ARM_r0) {
		case -ERESTART_RESTARTBLOCK: {
			TRACE("ERESTART_RESTARTBLOCK");
			m_regs_bak.ARM_r0 = -EAGAIN;
			m_regs_bak.ARM_r7 = 0;
			if (m_regs_bak.ARM_cpsr & CPSR_T_MASK)
				m_regs_bak.ARM_pc -= 2;
			else
				m_regs_bak.ARM_pc -= 4;
			break;
		}
		case -ERESTARTSYS:
		case -ERESTARTNOINTR:
		case -ERESTARTNOHAND: {
			TRACE("ERESTART_OTHER %ld", m_regs_bak.ARM_r0);
			if (-ERESTARTSYS == m_regs_bak.ARM_ORIG_r0
					|| -ERESTARTNOINTR == m_regs_bak.ARM_ORIG_r0
					|| -ERESTARTNOHAND == m_regs_bak.ARM_ORIG_r0
					|| -ERESTART_RESTARTBLOCK == m_regs_bak.ARM_ORIG_r0) {
				m_regs_bak.ARM_r0 = -EINTR;
			} else {
				m_regs_bak.ARM_r0 = m_regs_bak.ARM_ORIG_r0;
				if (m_regs_bak.ARM_cpsr & CPSR_T_MASK)
					m_regs_bak.ARM_pc -= 2;
				else
					m_regs_bak.ARM_pc -= 4;
			}
			break;
		}
		}
	}

	pid_t m_pid;
	pt_regs m_regs;
	pt_regs m_regs_bak;
};

int main(int argc, char* argv[]) {
	if (argc < 7) {
		TRACE("usage: injector pid so_path jar_path");
		return EXIT_FAILURE;
	}
	TRACE("pid: %s", argv[3]);
	TRACE("so_path: %s", argv[4]);
	TRACE("jar_path: %s", argv[5]);
	TRACE("entry: %s", argv[6]);

	pid_t pid = atoi(argv[3]);

	CPtrace tracer(pid);
	if (!tracer.attach()) {
		return EXIT_FAILURE;
	}

	uint8_t* local_linker = (uint8_t*) tracer.get_module_base(-1, "/linker");
	uint8_t* local_dlopen = (uint8_t*) ::dlopen;
	uint8_t* local_dlsym = (uint8_t*) ::dlsym;
	uint8_t* local_dlclose = (uint8_t*) ::dlclose;

	uint8_t* remote_linker = (uint8_t*) tracer.get_module_base(pid, "/linker");
	void* remote_dlopen = local_dlopen + (remote_linker - local_linker);
	void* remote_dlsym = local_dlsym + (remote_linker - local_linker);
	void* remote_dlclose = local_dlclose + (remote_linker - local_linker);

	void* handle = tracer.dlopen(remote_dlopen, argv[4]);
	if (handle == NULL) {
		TRACE("dlopen failed %s", argv[2]);
		return EXIT_FAILURE;
	}

	void* entry = tracer.dlsym(remote_dlsym, handle, "entry");
	if (entry == NULL) {
		TRACE("dlsym failed");
		return EXIT_FAILURE;
	}

	tracer.call_entry(entry, argv[5], argv[6]);

//	tracer.dlclose(remote_dlclose, handle);

	return EXIT_SUCCESS;
}

extern "C" __attribute__ ((visibility("default")))
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	int fd = open("/proc/self/cmdline", O_RDONLY);
	if (fd >= 0) {
		char cmdline[4096];
		int count = read(fd, cmdline, sizeof(cmdline));
		int argc = 0;
		char* argv[10];
		argv[argc] = cmdline;
		for (int i = 0; i < count; i++) {
			if (cmdline[i] == 0) {
				argc++;
				argv[argc] = &cmdline[i + 1];
			}
		}
		close(fd);

		main(argc, argv);

	}
	return JNI_VERSION_1_4;
}

namespace android {
class AndroidRuntime {
public:
	static JNIEnv* getJNIEnv();
};
}

jclass g_entryClass = 0;

extern "C" __attribute__ ((visibility("default")))
void entry(const char* jar_path, const char* entry) {
	TRACE("jar_path:%s entry: %s", jar_path, entry);

	JNIEnv* env = android::AndroidRuntime::getJNIEnv();
	if (env == 0) {
		TRACE("env is null");
		return;
	}

	if (g_entryClass == 0) {
		jclass classLoader = env->FindClass("java/lang/ClassLoader");
		jmethodID getSystemClassLoaderMethod = env->GetStaticMethodID(
				classLoader, "getSystemClassLoader",
				"()Ljava/lang/ClassLoader;");
		jobject systemClassLoader = env->CallStaticObjectMethod(classLoader,
				getSystemClassLoaderMethod);

		if (systemClassLoader == 0) {
			TRACE("systemClassLoader is null");
			return;
		}

		jclass pathClassLoader = env->FindClass(
				"dalvik/system/PathClassLoader");
		jmethodID initMethod =
				env->GetMethodID(pathClassLoader, "<init>",
						"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");

		jobject loaded = env->NewObject(pathClassLoader, initMethod,
				env->NewStringUTF(jar_path), 0, systemClassLoader);

		if (loaded == 0) {
			TRACE("loaded is null");
			return;
		}
		if (env->ExceptionCheck()) {
			env->ExceptionDescribe();
			env->ExceptionClear();
			return;
		}

		jmethodID loadClassMethod = env->GetMethodID(pathClassLoader,
				"loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

		if (loadClassMethod == 0) {
			TRACE("loadClassMethod is null");
			return;
		}
		jclass entryClass = (jclass) env->CallObjectMethod(loaded,
				loadClassMethod, env->NewStringUTF(entry));

		if (entryClass == 0) {
			TRACE("entryClass is null");
			return;
		}

		g_entryClass = (jclass) env->NewGlobalRef(entryClass);
	}

	jmethodID mainMethod = env->GetStaticMethodID(g_entryClass, "main", "()V");
	if (mainMethod == 0) {
		TRACE("mainMethod is null");
		return;
	}

	env->CallStaticVoidMethod(g_entryClass, mainMethod);
}
