#include "system_call.h"
#include "file_system.h"
#include "paging.h"
#include "processes.h"
#include "x86_desc.h"
#include "keyboard.h"

// // Global Variables
int32_t cur_pid = -1;
int32_t prev_pid = -1;
int32_t pid_array[MAX_PID];

/* int32_t sys_halt(uint8_t status)
 * Inputs: status - program halt status
 * Return Value: status
 * Function: halts the current process */
int32_t sys_halt(uint8_t status) {
    halt_helper((uint32_t) status);
    
    return 0;
}

/* int32_t sys_execute(const uint8_t* command)
 * Inputs: command - command to execute
 * Return Value: 0 on success, -1 on failure
 * Function: executes a process */
int32_t sys_execute(const uint8_t* command) {
    cli();

    uint8_t f_cmd[FILE_STR_LEN+1];
    uint8_t cmd_args[CMD_LINE_LEN];
    uint32_t command_len = strlen((const int8_t*)command);

    int active_term = get_active_terminal();

    // parse arg for file cmd
    uint32_t f_idx = 0;
    while(command[f_idx] == ' ' && command[f_idx] != '\0'){
        f_idx++;
    }

    uint32_t i;
    for(i = 0; i < FILE_STR_LEN && f_idx < command_len; i++, f_idx++){
        if(command[f_idx] == ' ' || command[f_idx] == '\n' || command[f_idx] == '\0') {
            break;
        }
        f_cmd[i] = command[f_idx];
    }
    f_cmd[i] = '\0';    // end cmd string in '\0'

    f_idx++;
    // write command arguments to buf
    for (i = 0; i < CMD_LINE_LEN && f_idx < command_len; i++, f_idx++) {
        if (command[f_idx] == '\0') {
            break;
        }
        cmd_args[i] = command[f_idx];
    }
    cmd_args[i] = '\0';

    // check if cmd file exists
    dentry_t dentry;
    if(read_dentry_by_name((const uint8_t*)f_cmd, &dentry) == -1){
        return -1;      // file not found
    }
    // check if cmd file is executable
    uint8_t elf[ELF_SIZE];
    if( read_data(dentry.inode_num, 0, elf, ELF_SIZE) != ELF_SIZE ) {
        return -1;      // couldn't read 4 bytes from the file
    }
    if( !( (elf[0] == ELF0) && (elf[1] == ELF1 ) 
            && (elf[2] == ELF2) && (elf[3] == ELF3)) ) {
        return -1;      // file is not an executable
    }

    prev_pid = cur_pid;
    // find next open pid index
    for(i = 0; i < MAX_PID; i++){
        if(pid_array[i] == 0){
            cur_pid = i;
            pid_array[i] = 1;
            break;
        }
    }
    if (cur_pid <= 2) {     // pid 0-2 are base shells for the 3 terminals
        prev_pid = -1;
    }
    if(i == MAX_PID){
        return FUNNY_NUMBER;    // cannot add another process
    }
    
    // set 128MB virtual to present and map to physical address of cur_pid
    uint32_t pid_phys_addr = (cur_pid+2) << OFFSET_10;
    pd[PROGRAM_IMG_IDX].p = 1;
    pd[PROGRAM_IMG_IDX].addr = pid_phys_addr;

    // set video memory page to present
    pt_user[VID_MEM_INDEX].p = 1;

    // we need to flush TLB before writing to page
    asm volatile("movl %cr3, %eax;"
                 "movl %eax, %cr3");

    uint8_t* img_addr = (uint8_t*) (PROGRAM_VIR_ADDR + PROGRAM_IMG_OFFSET);
    inode_t cur_inode = *((inode_t*)(fs_addr + (dentry.inode_num + 1) * BLOCK_SIZE));
    uint32_t nbytes = cur_inode.length;
    read_data(dentry.inode_num, 0, img_addr, nbytes);

    // initialize PCB for current process
    pcb_t* cur_pcb;
    cur_pcb = (pcb_t*)( CONST_8MB - ((cur_pid+1) * CONST_8KB) );

    // fill PCB with initial valules
    for(i = 0; i < NUM_MAX_FILES; i++){
        fd_t* fd = cur_pcb->file_descriptor_table;
        if(i == 0) {
            fd[i].file_op_table_ptr = &stdin_opt;    // fdt[0] = stdin
            fd[i].inode_num = 0;
            fd[i].file_pos = 0;
            fd[i].flags = 1;   // index in the fdt is filled
        } else
        if(i == 1) {
            fd[i].file_op_table_ptr = &stdout_opt;    // fdt[1] = stdout
            fd[i].inode_num = 0;
            fd[i].file_pos = 0;
            fd[i].flags = 1;   // index in the fdt is filled
        } else {
            fd[i].file_op_table_ptr = &null_opt;    // rest of indexes are empty
            fd[i].inode_num = -1;
            fd[i].file_pos = -1;
            fd[i].flags = 0;   // index in the fdt is empty
        }
    }
    cur_pcb->parent_pid = prev_pid;

    // write cmd_args to cur_pcb cmd_args
    for (i = 0; i < strlen((const int8_t*)cmd_args); i++) {
        cur_pcb->cmd_args[i] = cmd_args[i];
    }
    cur_pcb->cmd_args[i] = '\0';

    // update terminal active pid
    terminals[active_term].active_pid = cur_pid;

    // prepare for context switch by saving 
    uint32_t k_esp;
    asm volatile("movl %%esp, %0" : "=r"(k_esp) : : "memory");
    uint32_t k_ebp;
    asm volatile("movl %%ebp, %0" : "=r"(k_ebp) : : "memory");
    uint32_t k_eflag;
    
    cur_pcb->ebp = k_ebp;
    cur_pcb->esp = k_esp;

    cur_pcb->cur_esp = CONST_8MB - (cur_pid*CONST_8KB) - sizeof(uint32_t);
    cur_pcb->cur_ebp = cur_pcb->cur_esp;

    tss.ss0 = KERNEL_DS;
    tss.esp0 = cur_pcb->cur_esp;

    uint32_t user_eip = *((uint32_t*)(img_addr + OFFSET_24));

    uint32_t user_esp = PROGRAM_VIR_ADDR + CONST_4MB - sizeof(uint32_t);


    asm volatile (
        "pushl %0;"
        "pushl %1;"
        "pushfl;"
        "popl %2;"
        "or $0x0200, %2;"
        "pushl %2;"
        "pushl %3;"
        "pushl %4;"
        :
        : "r"(USER_DS), "g"(user_esp), "r"(k_eflag), "g"(USER_CS), "g"(user_eip)
        : "memory"
    );

    asm volatile("iret;");

    return 0;
}

/* int32_t sys_read(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: fd - file descriptor index
           buf - buf to write to
           nbytes - number of bytes to write
 * Return Value: fd's read call return value
 * Function: calls fd's read function */
int32_t sys_read (int32_t fd, void* buf, int32_t nbytes) {
    int bytes_read;

    if (fd < 0 || fd > NUM_MAX_FILES) {return -1;}

    pcb_t* cur_pcb = (pcb_t*)( CONST_8MB - ((cur_pid+1) * CONST_8KB) );

    fd_t* cur_fd = cur_pcb->file_descriptor_table + fd;

    if (cur_fd->flags == 0) {return -1;}

    bytes_read = cur_fd->file_op_table_ptr->read((int32_t)cur_fd, (uint8_t*)buf, nbytes);

    // update fd file pos
    cur_fd->file_pos += bytes_read;

    return bytes_read;
}

/* int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: fd - file descriptor index
           buf - buf to write to
           nbytes - number of bytes to write
 * Return Value: fd's read call return value
 * Function: calls fd's read function */
int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes) {
    if (fd < 0 || fd > NUM_MAX_FILES) {return -1;}

    pcb_t* cur_pcb = (pcb_t*)( CONST_8MB - ((cur_pid+1) * CONST_8KB) );

    fd_t* cur_fd = cur_pcb->file_descriptor_table + fd;

    if (cur_fd->flags == 0) {return -1;}

    return cur_fd->file_op_table_ptr->write(fd, (uint8_t*)buf, nbytes);
}

/* int32_t sys_open(const uint8_t* filename)
 * Inputs: filename - file name to open
 * Return Value: fd index of opened file, -1 on failure
 * Function: initializes fd in fd array with file to open */
int32_t sys_open (const uint8_t* filename) {
    dentry_t file_dentry;
    int fd;

    // check file name valid
    uint32_t read = read_dentry_by_name(filename, &file_dentry);
    if (read == -1) {return -1;}


    pcb_t* cur_pcb = (pcb_t*)( CONST_8MB - ((cur_pid+1) * CONST_8KB) );

    // find available fd index
    fd = MIN_FD_INDEX;
    while(fd < NUM_MAX_FILES && cur_pcb->file_descriptor_table[fd].flags != 0) {
        fd++;
    }
    if (fd >= NUM_MAX_FILES) {return -1;}    // no open fd


    // set fops table, inode, file position
    switch (file_dentry.file_type) {
        case 0:
            // rtc
            cur_pcb->file_descriptor_table[fd].file_op_table_ptr = &rtc_opt;
            cur_pcb->file_descriptor_table[fd].inode_num = file_dentry.inode_num;
            cur_pcb->file_descriptor_table[fd].file_pos = 0;
            cur_pcb->file_descriptor_table[fd].flags = 1;

            cur_pcb->file_descriptor_table[fd].file_op_table_ptr->open((uint8_t*)filename);
            return fd;
        case 1:
            // dir
            cur_pcb->file_descriptor_table[fd].file_op_table_ptr = &dir_opt;
            cur_pcb->file_descriptor_table[fd].inode_num = file_dentry.inode_num;
            cur_pcb->file_descriptor_table[fd].file_pos = 0;
            cur_pcb->file_descriptor_table[fd].flags = 1;
            return fd;
        case 2:
            // file
            cur_pcb->file_descriptor_table[fd].file_op_table_ptr = &file_opt;
            cur_pcb->file_descriptor_table[fd].inode_num = file_dentry.inode_num;
            cur_pcb->file_descriptor_table[fd].file_pos = 0;
            cur_pcb->file_descriptor_table[fd].flags = 1;
            return fd;
    }

    return fd;
}

/* int32_t sys_close(uint32_t fd)
 * Inputs: fd - file descriptor index to close
 * Return Value: 0 on success, -1 on failure
 * Function: clears file descriptor index of file information */
int32_t sys_close (int32_t fd) {
    if (fd < MIN_FD_INDEX || fd > MAX_FD_INDEX) {return -1;}     // invalid index

    pcb_t* cur_pcb = (pcb_t*)( CONST_8MB - ((cur_pid+1) * CONST_8KB) );
    
    fd_t* cur_fd = cur_pcb->file_descriptor_table + fd;
    
    // return -1 if nothing to close
    if (cur_fd->flags == 0) {return -1;}

    cur_fd->file_op_table_ptr->close((uint8_t*)NULL);

    // invalidate fops table, inode, file position, set flag to 0
    cur_fd->file_op_table_ptr = &null_opt;
    cur_fd->inode_num = -1;
    cur_fd->file_pos = -1;
    cur_fd->flags = 0;

    return 0;
}

/* int32_t sys_getargs(uint8_t* buf, int32_t nbytes)
 * Inputs: buf - buffer to write args to
           nbytes - number of bytes to write
 * Return Value: 0 on success, -1 on failure
 * Function: writes args to buf */
int32_t sys_getargs (uint8_t* buf, int32_t nbytes) {
    int i;
    int arg_strlen;

    pcb_t* cur_pcb = (pcb_t*)( CONST_8MB - ((cur_pid+1) * CONST_8KB) );

    arg_strlen = strlen((const int8_t*)cur_pcb->cmd_args);
    
    // check no args or args too long
    if (arg_strlen == 0 || arg_strlen > nbytes-1) {return -1;}

    // copy args to buf
    for (i = 0; i < arg_strlen; i++) {
        buf[i] = cur_pcb->cmd_args[i];
    }
    buf[i] = '\0';

    return 0;
}

/* int32_t sys_vidmap(uint8_t** screen_start)
 * Inputs: screen_start - pointer to screen_start address
 * Return Value: address of video memory
 * Function: maps text-mode video memory to user space */
int32_t sys_vidmap (uint8_t** screen_start) {
    // check valid input

    if(pt_user[VID_MEM_INDEX].p == 0 || screen_start == NULL) {return -1;}
    if ((uint32_t)screen_start < PROGRAM_VIR_ADDR || 
        (uint32_t)screen_start >= PROGRAM_VIR_ADDR + CONST_8MB) {return -1;}

    *screen_start = (uint8_t*)((PROGRAM_IMG_IDX+1) * PAGE_SIZE_4MB + (VID_MEM_INDEX << OFFSET_12));

    return 0;
}

int32_t sys_sethandler (int32_t signum, void* handler_address) {
    return -1;
}

int32_t sys_sigreturn (void) {
    return -1;
}

/* int32_t halt_helper(uint32_t status)
 * Inputs: status - program halt status
 * Return Value: status
 * Function: halts the current process */
int32_t halt_helper(uint32_t status) {
    cli();

    int active_term = get_active_terminal();
    
    pid_array[cur_pid] = 0;     // remove current process from array

    pcb_t* cur_pcb;
    cur_pcb = (pcb_t*)( CONST_8MB - ((cur_pid+1) * CONST_8KB) );

    if(cur_pcb->parent_pid == -1) {
        cur_pid = -1;
        prev_pid = -1;
        sys_execute((const uint8_t*)"shell");

        return -1;      // cannot halt the base shell
    }
    
    pcb_t* parent_pcb;
    parent_pcb = (pcb_t*)( CONST_8MB - ((cur_pcb->parent_pid+1) * CONST_8KB) );
    prev_pid = parent_pcb->parent_pid;  // move previous pid to parent's parent pid


    // restore paging
    uint32_t pid_phys_addr = (cur_pcb->parent_pid+2) << OFFSET_10;
    pd[PROGRAM_IMG_IDX].p = 1;
    pd[PROGRAM_IMG_IDX].addr = pid_phys_addr;

    // flush TLB
    asm volatile("movl %cr3, %eax;"
                 "movl %eax, %cr3");

    
    // clear stdin/stdout manually
    cur_pcb->file_descriptor_table[0].file_op_table_ptr = NULL;
    cur_pcb->file_descriptor_table[0].inode_num = 0;
    cur_pcb->file_descriptor_table[0].file_pos = 0;
    cur_pcb->file_descriptor_table[0].flags = 0;

    cur_pcb->file_descriptor_table[1].file_op_table_ptr = NULL;
    cur_pcb->file_descriptor_table[1].inode_num = 0;
    cur_pcb->file_descriptor_table[1].file_pos = 0;
    cur_pcb->file_descriptor_table[1].flags = 0;

    int i;
    for (i = MIN_FD_INDEX; i < NUM_MAX_FILES; i++){
        sys_close(i);   // close all files inside the FDT (close and set table entries to NULL)
    }

    cur_pid = cur_pcb->parent_pid;

    // update terminal active pid
    terminals[active_term].active_pid = cur_pid;
    for(i = 0; i < FILE_STR_LEN; i++) {
        terminals[active_term].terminal_buf[i] = '\0';
    }
    if(ts == active_term) {
        clear_keyboard_buf_caller();
    }

    // return to parent process
    tss.ss0 = KERNEL_DS;
    tss.esp0 = CONST_8MB - (cur_pid*CONST_8KB) - sizeof(uint32_t);

    sti();

    asm volatile(
        "movl %0, %%ebp;"
        "movl %%ebp, %%esp;"
        "movl %1, %%eax;"
        "leave;"
        "ret;"
        :
        :"r"(cur_pcb->ebp), "r"((uint32_t)status)
        :"memory"
    );

    return 0;
}

/* void get_cur_pid(void)
 * Inputs: void
 * Return Value: returns the current PID (cur_pid)
 * Function: getter for current PID */
int32_t get_cur_pid() {
    return cur_pid;
}

/* void set_cur_pid(void)
 * Inputs: int pid - new PID to change the current PID
 * Return Value: void
 * Function: setter for current PID */
void set_cur_pid(int pid){
    cur_pid = pid;
}

/* void set_cur_pid(void)
 * Inputs: int pid - new PID to change the previous PID
 * Return Value: void
 * Function: setter for previous PID */
void set_prev_pid(int pid){
    prev_pid = pid;
}
