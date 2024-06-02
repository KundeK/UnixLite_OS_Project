#include "paging.h"

/* void paging_init(void)
 * Inputs: void
 * Return Value: void
 * Function: Initializing more than one virtual address space mapped into the physical address space */
void paging_init(void) {
    int i;

    // set up pd[0] 0-4MB section
    // set Present, Read/Write, Addr
    pd[0].p = 1;
    pd[0].rw = 1;
    pd[0].us = 0;
    pd[0].pwt = 0;
    pd[0].pcd = 0;
    pd[0].a = 0;
    pd[0].d = 0;
    pd[0].ps = 0;
    pd[0].g = 0;
    pd[0].avl = 0;
    pd[0].addr = ((uint32_t) pt) >> OFFSET_12;

    // set up rest of pd[1] 4-8MB section for kernel
    // set Present, Read/Write, Page Select, Global, Addr
    pd[1].p = 1;
    pd[1].rw = 1;
    pd[1].us = 0;
    pd[1].pwt = 0;
    pd[1].pcd = 0;
    pd[1].a = 0;
    pd[1].d = 0;
    pd[1].ps = 1;
    pd[1].g = 1;
    pd[1].avl = 0;
    pd[1].addr = 1 << OFFSET_10;

    // set up video memory entry of pt
    pt[VID_MEM_INDEX].p = 1;
    pt[VID_MEM_INDEX].rw = 1;
    pt[VID_MEM_INDEX].us = 0;
    pt[VID_MEM_INDEX].pwt = 0;
    pt[VID_MEM_INDEX].pcd = 0;
    pt[VID_MEM_INDEX].a = 0;
    pt[VID_MEM_INDEX].d = 0;
    pt[VID_MEM_INDEX].pat = 0;
    pt[VID_MEM_INDEX].g = 0;
    pt[VID_MEM_INDEX].avl_9_11 = 0;
    pt[VID_MEM_INDEX].addr_31_12 = (int)VID_MEM_INDEX;

    // set up terminal vid mem entries
    pt[TERMINAL_ONE_VIDMEM_IDX].p = 1;
    pt[TERMINAL_ONE_VIDMEM_IDX].rw = 1;
    pt[TERMINAL_ONE_VIDMEM_IDX].us = 0;
    pt[TERMINAL_ONE_VIDMEM_IDX].pwt = 0;
    pt[TERMINAL_ONE_VIDMEM_IDX].pcd = 0;
    pt[TERMINAL_ONE_VIDMEM_IDX].a = 0;
    pt[TERMINAL_ONE_VIDMEM_IDX].d = 0;
    pt[TERMINAL_ONE_VIDMEM_IDX].pat = 0;
    pt[TERMINAL_ONE_VIDMEM_IDX].g = 0;
    pt[TERMINAL_ONE_VIDMEM_IDX].avl_9_11 = 0;
    pt[TERMINAL_ONE_VIDMEM_IDX].addr_31_12 = (int)TERMINAL_ONE_VIDMEM_IDX;

    pt[TERMINAL_TWO_VIDMEM_IDX].p = 1;
    pt[TERMINAL_TWO_VIDMEM_IDX].rw = 1;
    pt[TERMINAL_TWO_VIDMEM_IDX].us = 0;
    pt[TERMINAL_TWO_VIDMEM_IDX].pwt = 0;
    pt[TERMINAL_TWO_VIDMEM_IDX].pcd = 0;
    pt[TERMINAL_TWO_VIDMEM_IDX].a = 0;
    pt[TERMINAL_TWO_VIDMEM_IDX].d = 0;
    pt[TERMINAL_TWO_VIDMEM_IDX].pat = 0;
    pt[TERMINAL_TWO_VIDMEM_IDX].g = 0;
    pt[TERMINAL_TWO_VIDMEM_IDX].avl_9_11 = 0;
    pt[TERMINAL_TWO_VIDMEM_IDX].addr_31_12 = (int)TERMINAL_TWO_VIDMEM_IDX;

    pt[TERMINAL_THREE_VIDMEM_IDX].p = 1;
    pt[TERMINAL_THREE_VIDMEM_IDX].rw = 1;
    pt[TERMINAL_THREE_VIDMEM_IDX].us = 0;
    pt[TERMINAL_THREE_VIDMEM_IDX].pwt = 0;
    pt[TERMINAL_THREE_VIDMEM_IDX].pcd = 0;
    pt[TERMINAL_THREE_VIDMEM_IDX].a = 0;
    pt[TERMINAL_THREE_VIDMEM_IDX].d = 0;
    pt[TERMINAL_THREE_VIDMEM_IDX].pat = 0;
    pt[TERMINAL_THREE_VIDMEM_IDX].g = 0;
    pt[TERMINAL_THREE_VIDMEM_IDX].avl_9_11 = 0;
    pt[TERMINAL_THREE_VIDMEM_IDX].addr_31_12 = (int)TERMINAL_THREE_VIDMEM_IDX;

    // initialize rest of page directory
    for (i = 2; i < NUM_PD; i++) {
        pd[i].p = 0;
        pd[i].rw = 1;
        pd[i].us = 0;
        pd[i].pwt = 0;
        pd[i].pcd = 0;
        pd[i].a = 0;
        pd[i].d = 0;
        pd[i].ps = 1;
        pd[i].g = 0;
        pd[i].avl = 0;
        pd[i].addr = i << OFFSET_10;
    }

    // set 128MB page to present
    pd[PROGRAM_IMG_IDX].p = 1;
    pd[PROGRAM_IMG_IDX].ps = 1;
    pd[PROGRAM_IMG_IDX].us = 1;

    // // set user video memory at page after 128MB
    pd[PROGRAM_IMG_IDX+1].p = 1;
    pd[PROGRAM_IMG_IDX+1].rw = 1;
    pd[PROGRAM_IMG_IDX+1].us = 1;
    pd[PROGRAM_IMG_IDX+1].pwt = 0;
    pd[PROGRAM_IMG_IDX+1].pcd = 0;
    pd[PROGRAM_IMG_IDX+1].a = 0;
    pd[PROGRAM_IMG_IDX+1].d = 0;
    pd[PROGRAM_IMG_IDX+1].ps = 0;
    pd[PROGRAM_IMG_IDX+1].g = 0;
    pd[PROGRAM_IMG_IDX+1].avl = 0;
    pd[PROGRAM_IMG_IDX+1].addr = ((uint32_t) pt_user) >> OFFSET_12;

    // set up video memory entry of pt_user
    pt_user[VID_MEM_INDEX].p = 1;
    pt_user[VID_MEM_INDEX].rw = 1;
    pt_user[VID_MEM_INDEX].us = 1;
    pt_user[VID_MEM_INDEX].pwt = 0;
    pt_user[VID_MEM_INDEX].pcd = 0;
    pt_user[VID_MEM_INDEX].a = 0;
    pt_user[VID_MEM_INDEX].d = 0;
    pt_user[VID_MEM_INDEX].pat = 0;
    pt_user[VID_MEM_INDEX].g = 0;
    pt_user[VID_MEM_INDEX].avl_9_11 = 0;
    pt_user[VID_MEM_INDEX].addr_31_12 = (int)(VID_MEM_INDEX);


    // this loop will stop at index VID_MEM_INDEX = 0xB8
    for (i = 0; i < NUM_PT && i != VID_MEM_INDEX; i++) {
        pt[i].p = 0;
        pt[i].rw = 1;
        pt[i].us = 0;
        pt[i].pwt = 0;
        pt[i].pcd = 0;
        pt[i].a = 0;
        pt[i].d = 0;
        pt[i].pat = 0;
        pt[i].g = 0;
        pt[i].avl_9_11 = 0;
        pt[i].addr_31_12 = i;
    }

    // x86 code to enable paging
    asm volatile(
                 "movl %%cr4, %%eax;"
                 "or $0x00000010, %%eax;"   // enable PSE in cr4
                 "movl %%eax, %%cr4;"
                 "movl %0, %%eax;"
                 "movl %%eax, %%cr3;"       // load pd into cr3
                 "movl %%cr0, %%eax;"
                 "or $0x80000000, %%eax;"   // enable PG in cr0
                 "mov %%eax, %%cr0;"
                 :
                 : "g" (pd)
                 : "eax", "memory", "cc");

    return;
}

/* uint32_t init_user_page(uint32_t pid)
 * Inputs: uint32_t pid - PID for user page
 * Return Value: 0 for success, -1 for failure
 * Function: Initializes user page for current PID */
uint32_t init_user_page(uint32_t pid) {
    if (pid > 1 || pid < 0) {return -1;}        // check valid pid
    if (pd[pid+2].p == 1) {return -1;}          // check already present

    pd[pid+2].p = 1;
    pd[pid+2].rw = 1;
    pd[pid+2].us = 1;
    pd[pid+2].pwt = 0;
    pd[pid+2].pcd = 0;
    pd[pid+2].a = 0;
    pd[pid+2].d = 0;
    pd[pid+2].ps = 1;
    pd[pid+2].g = 0;
    pd[pid+2].avl = 0;
    pd[pid+2].addr = (pid + 2) << OFFSET_10;

    return 0;
}

/* uint32_t close_user_page(uint32_t pid)
 * Inputs: uint32_t pid - PID for user page
 * Return Value: 0 for success, -1 for failure
 * Function: closes user page for current PID */
uint32_t close_user_page(uint32_t pid) {
    if (pid > 1 || pid < 0) {return -1;}        // check valid pid
    // if (pd[pid+2].p == 1) {return -1;}          // check already present

    pd[pid+2].p = 0;
    pd[pid+2].rw = 1;
    pd[pid+2].us = 1;
    pd[pid+2].pwt = 0;
    pd[pid+2].pcd = 0;
    pd[pid+2].a = 0;
    pd[pid+2].d = 0;
    pd[pid+2].ps = 1;
    pd[pid+2].g = 0;
    pd[pid+2].avl = 0;
    pd[pid+2].addr = (pid + 2) << OFFSET_10;

    return 0;
}
