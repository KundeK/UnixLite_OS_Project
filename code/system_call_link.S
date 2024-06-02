#define ASM     1

#define SYS_MAX     10

.globl sys_call
sys_call:
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %esp
    pushl %ebp
    pushl %edi
    pushl %esi

    cmpl $SYS_MAX, %eax
    jg INVALID_CMD
    cmpl $1, %eax 
    jl INVALID_CMD
    movl $-4, %esi
    pushl %edx    
    pushl %ecx    
    pushl %ebx    
    call *jump_table(%esi,%eax,4)
    popl %ebx
    popl %ecx
    popl %edx
END:     
    popl %esi
    popl %edi
    popl %ebp
    popl %esp
    popl %edx
    popl %ecx 
    popl %ebx
       
    iret     
INVALID_CMD:
    movl $-1, %eax
    jmp END       

jump_table:                         
    .long sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_sethandler, sys_sigreturn 
