#ifndef _FS_H
#define _FS_H

#include "lib.h"

#define BLOCK_SIZE              4096     // number of bytes per block
#define MAX_DIR_ENTRIES         63       // max num of dentries (including ".")
#define MAX_STR_LENGTH          32
#define OFFSET_4                4

extern uint8_t* fs_addr;

typedef struct dir_entry {
    uint8_t  file_name[MAX_STR_LENGTH];     // 32 bytes wide
    uint32_t file_type;         // 4 bytes wide
    uint32_t inode_num;         // 4 bytes wide
    uint8_t  reserved[24];      // 24 bytes wide
} dentry_t;

typedef struct inode {
    uint32_t length;
    uint32_t data_blocks[(BLOCK_SIZE/OFFSET_4) - 1];
} inode_t;

typedef struct data_block {
    uint8_t data[BLOCK_SIZE];
} data_block_t;

typedef struct boot_block {
    uint32_t num_dir_entries;                    // 4 bytes wide
    uint32_t num_inodes;                         // 4 bytes wide
    uint32_t num_data_blocks;                    // 4 bytes wide
    char     reserved[52];                       // 52 bytes wide
    dentry_t dir_entries[(BLOCK_SIZE/64) - 1];   // 63 bytes wide
} boot_block_t;


typedef struct file_operations_table {
    int32_t (*open)(uint8_t*);
    int32_t (*close)(uint8_t*);
    int32_t (*read)(int32_t, uint8_t*, int32_t);
    int32_t (*write)(int32_t, uint8_t*, int32_t);
} f_op_t;

typedef struct file_descriptor {
    f_op_t* file_op_table_ptr;
    uint32_t inode_num;
    uint32_t file_pos;
    uint32_t flags;
} fd_t;


void file_sys_init(uint8_t* start_addr);
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

#endif
