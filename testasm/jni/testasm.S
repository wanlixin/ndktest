#include "sys/linux-syscalls.h"

.data

sleeptime:
	.long   5, 0

.text

#ifdef __i386__

.global _start
.type _start, @function

_start:
	movl    $__NR_nanosleep, %eax
	movl    $sleeptime,     %ebx
	int     $0x80

	movl    $__NR_exit,      %eax
	movl    $0,             %ebx
	int     $0x80

#else

.globl _start
.type _start, #function

_start:
	ldr     r0, =sleeptime
	ldr     r1, =sleeptime
    mov     r7, $__NR_nanosleep
    swi     #0

    mov     r0, $0
    mov     r7, $__NR_exit
    swi     #0

#endif
