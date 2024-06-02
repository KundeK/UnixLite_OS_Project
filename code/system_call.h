/* 
 * system_call.h - Defines system call handlers
 */

#ifndef _SYSCALL_MACRO
#define _SYSCALL_MACRO

#include "lib.h"

#define FILE_STR_LEN    128
#define MAX_PID         6

#define CONST_4KB       0x1000
#define CONST_8KB       0x2000
#define CONST_8MB       0x800000
#define CONST_4MB       0x400000

#define OFFSET_24       24
#define OFFSET_16       16
#define OFFSET_8        8

#define ELF_SIZE    4
#define ELF0    0x7F
#define ELF1    0x45
#define ELF2    0x4C
#define ELF3    0x46

#define FUNNY_NUMBER    69

#define MIN_FD_INDEX    2
#define MAX_FD_INDEX    7

/* We create the necessary system handler function */
extern void sys_call(void);

int32_t sys_halt(uint8_t status);
int32_t sys_execute(const uint8_t* command);
int32_t sys_read (int32_t fd, void* buf, int32_t nbytes);
int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t sys_open (const uint8_t* filename);
int32_t sys_close (int32_t fd);
int32_t sys_getargs (uint8_t* buf, int32_t nbytes);
int32_t sys_vidmap (uint8_t** screen_start);
int32_t sys_sethandler (int32_t signum, void* handler_address);
int32_t sys_sigreturn (void);

int32_t halt_helper(uint32_t status);

int32_t get_cur_pid();
void set_cur_pid(int pid);
void set_prev_pid(int pid);

#endif
