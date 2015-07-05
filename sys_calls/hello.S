#gcc hello.S -nostdlib
.data
msg:
	.ascii "Hello World\n"
	len = . -msg
.text
.globl _start

_start:
#write string to stdout
	movl $len, %edx
	movl $msg, %ecx	#message
###	movl $1000, %ecx	###pass wrong pointer to message
	movl $1, %ebx	#handle stdout
	movl $4, %eax	#sys_write
#	movl $4000, %eax	###pass wrong syscall
	int $0x80	#call kernel
#exit
	movl $0, %ebx	#exit code
###	movl $1, %ebx	###exit with 1
	movl $1, %eax	#sys_exit
	int $0x80	#call kernel

