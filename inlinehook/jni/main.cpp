#include <unistd.h>
#include <stdio.h>

void* inlineHook(uint32_t target_addr, uint32_t new_addr);

template<typename T>
T inlineHookT(T target_addr, T new_addr) {
	return reinterpret_cast<T>(inlineHook(
			reinterpret_cast<uint32_t>(target_addr),
			reinterpret_cast<uint32_t>(new_addr)));
}

int test_thumb(int i) {
	printf("test_thumb %d\n", i);
	return i + 1;
}

int test_arm(int i);

int (*orig_thumb)(int i);
int (*orig_arm)(int i);

int onHook_thumb(int i) {
	printf("onHook_thumb begin %d\n", i);
	orig_arm(i);
	printf("onHook_thumb over %d\n", i);
	return 6;
}

int onHook_arm(int i) {
	printf("onHook_arm begin %d\n", i);
	orig_arm(i);
	printf("onHook_arm over %d\n", i);
	return 6;
}

int main(int argc, char *argv[]) {
	test_thumb(1);
	test_arm(1);

	orig_thumb = inlineHookT(test_thumb, onHook_thumb);
	orig_arm = inlineHookT(test_arm, onHook_arm);

	test_thumb(1);
	test_arm(1);

	printf(
			"inlinehook pid:%d test_thumb:%p test_arm:%p orig_thumb:%p orig_arm:%p\n",
			getpid(), test_thumb, test_arm, orig_thumb, orig_arm);
}
