#include <unistd.h>
#include <sys/mman.h>

#define HOOKER_MAX_METHOD 200

extern "C" void relocateInstruction(uint32_t target_addr,
		void *trampoline_instructions, int length);

struct InlineHookHelper {
	int pageSize;
	int trampoline_size;
	int trampolines[HOOKER_MAX_METHOD][10];

	void* align_addr(void* addr) {
		return reinterpret_cast<void*>(reinterpret_cast<int>(addr)
				& (~(pageSize - 1)));
	}
	long align_size(long size) {
		return (size + pageSize) & (~(pageSize - 1));
	}

	InlineHookHelper() {
		pageSize = sysconf(_SC_PAGE_SIZE);
		trampoline_size = 0;

		mprotect(align_addr(trampolines), align_size(sizeof(trampolines)),
				PROT_WRITE | PROT_EXEC | PROT_READ);
	}

	void* alloc_trampoline() {
		if (trampoline_size >= sizeof(trampolines) / sizeof(trampolines[0])) {
			return 0;
		}
		return trampolines[trampoline_size++];
	}
} inlineHelper;

#define SET_BIT0(addr)		(addr | 1)
#define CLEAR_BIT0(addr)	(addr & 0xFFFFFFFE)
#define TEST_BIT0(addr)		(addr & 1)

void* inlineHook(uint32_t target_addr, uint32_t new_addr) {
	void *trampoline_instructions = inlineHelper.alloc_trampoline();
	if (trampoline_instructions == 0) {
		return 0;
	}

	int length = TEST_BIT0(target_addr) ? 12 : 8;

	relocateInstruction(target_addr, trampoline_instructions, length);

	mprotect(
			inlineHelper.align_addr(
					reinterpret_cast<void*>(CLEAR_BIT0(target_addr))),
			inlineHelper.align_size(length),
			PROT_READ | PROT_WRITE | PROT_EXEC);

	if (TEST_BIT0(target_addr)) {
		int i;

		i = 0;
		if (CLEAR_BIT0(target_addr) % 4 != 0) {
			((uint16_t *) CLEAR_BIT0(target_addr))[i++] = 0xBF00; // NOP
		}
		((uint16_t *) CLEAR_BIT0(target_addr))[i++] = 0xF8DF;
		((uint16_t *) CLEAR_BIT0(target_addr))[i++] = 0xF000; // LDR.W PC, [PC]
		((uint16_t *) CLEAR_BIT0(target_addr))[i++] = new_addr & 0xFFFF;
		((uint16_t *) CLEAR_BIT0(target_addr))[i++] = new_addr >> 16;
	} else {
		((uint32_t *) (target_addr))[0] = 0xe51ff004; // LDR PC, [PC, #-4]
		((uint32_t *) (target_addr))[1] = new_addr;
	}

	mprotect(
			inlineHelper.align_addr(
					reinterpret_cast<void*>(CLEAR_BIT0(target_addr))),
			inlineHelper.align_size(length), PROT_READ | PROT_EXEC);

	cacheflush(CLEAR_BIT0(target_addr), CLEAR_BIT0(target_addr) + length, 0);

	return TEST_BIT0(target_addr) ? (uint32_t *) SET_BIT0(
			(uint32_t) trampoline_instructions) :
									(uint32_t *) trampoline_instructions;
}
