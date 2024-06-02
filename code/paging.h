/* 
 * paging.h - Defines paging functions for the code
 */

#ifndef _PAGING_H
#define _PAGING_H

#include "lib.h"
#include "x86_desc.h"
// #include "processes.h"

#define OFFSET_12 12
#define OFFSET_10 10
#define OFFSET_22 22
#define KERNEL_ADDR 0x400000
#define USER_ADDR   0x8000000
#define PROGRAM_IMG_IDX     32
#define PAGE_SIZE_4MB   0x400000
#define TERMINAL_ONE_VIDMEM_IDX    0xB9
#define TERMINAL_TWO_VIDMEM_IDX    0xBA
#define TERMINAL_THREE_VIDMEM_IDX  0xBB

/* Initializing paging mechanism for the OS */
void paging_init(void);

uint32_t init_user_page(uint32_t pid);
uint32_t close_user_page(uint32_t pid);

#endif
