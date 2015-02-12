#include <stdio.h>

/*
 * Describes a single frame of a backtrace.
 */
typedef struct {
    uintptr_t absolute_pc;     /* absolute PC offset */
    uintptr_t stack_top;       /* top of stack for this frame */
    size_t stack_size;         /* size of this stack frame */
} backtrace_frame_t;

/*
 * Describes the symbols associated with a backtrace frame.
 */
typedef struct {
    uintptr_t relative_pc;       /* relative frame PC offset from the start of the library,
                                    or the absolute PC if the library is unknown */
    uintptr_t relative_symbol_addr; /* relative offset of the symbol from the start of the
                                    library or 0 if the library is unknown */
    char* map_name;              /* executable or library name, or NULL if unknown */
    char* symbol_name;           /* symbol name, or NULL if unknown */
    char* demangled_name;        /* demangled symbol name, or NULL if unknown */
} backtrace_symbol_t;

void dumpNativeStack() {
	const size_t MAX_DEPTH = 32;
	const size_t MAX_BACKTRACE_LINE_LENGTH = 800;
	backtrace_frame_t backtrace[MAX_DEPTH];
	ssize_t frames = unwind_backtrace(backtrace, 1, MAX_DEPTH);
	size_t i;
	printf("================== frames:%d\n", frames);
	if (frames > 0) {
		backtrace_symbol_t backtrace_symbols[MAX_DEPTH];
		get_backtrace_symbols(backtrace, frames, backtrace_symbols);

		for (i = 0; i < frames; i++) {
			char line[MAX_BACKTRACE_LINE_LENGTH];
			format_backtrace_line(i, &backtrace[i], &backtrace_symbols[i], line,
					MAX_BACKTRACE_LINE_LENGTH);
			printf("================== %s || %x\n", line,
					backtrace[i].absolute_pc);
		}

		free_backtrace_symbols(backtrace_symbols, frames);
	}
}

int main() {
	dumpNativeStack();
	return 0;
}
