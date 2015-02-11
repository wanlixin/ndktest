.data
sleeptime:
.long   10,0

.text

.global _start
	.type _start, @function

_start:
	movl    0xa2, %eax
	movl    $sleeptime,     %ebx
	int     $0x80
	
	movl    1,      %eax
	movl    $0,             %ebx
	int     $0x80
