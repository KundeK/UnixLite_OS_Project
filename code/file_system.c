#include "file_system.h"

// file system already initialized and written, can extract data from boot block
uint8_t* fs_addr;
boot_block_t* boot_block;

/* void file_sys_init(uint8_t* start_addr)
 * Inputs: start_addr - start address of file system
 * Return Value: none
 * Function: initializes file system struct/boot block */
void file_sys_init(uint8_t* start_addr) {
    fs_addr = start_addr;
    boot_block = (boot_block_t*) start_addr;       // cast start of file system as boot_block*
}

/* int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
 * Inputs: fname - file name
           dentry - data entry pointer to write to
 * Return Value: 0 on success, -1 on fail
 * Function: populates dentry paramater with dentry in file system with same name */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry) {
    if (strlen((const int8_t*)fname) > MAX_STR_LENGTH
            || strlen((const int8_t*)fname) <= 0) {
            return -1;
    }

    int i;

    // int str_len = strlen((const int8_t*)fname) > MAX_STR_LENGTH ? MAX_STR_LENGTH : strlen((const int8_t*)fname);
    int str_len;
    int search_str_len;

    for (i = 0; i < boot_block->num_dir_entries; i++) {
        if (MAX_STR_LENGTH < (search_str_len = strlen((const int8_t*)boot_block->dir_entries[i].file_name))) {
            search_str_len = MAX_STR_LENGTH;
        }
        
        str_len = strlen((const int8_t*)fname) < search_str_len ? search_str_len : strlen((const int8_t*)fname);

        if ( !(strncmp((const int8_t*)fname, (const int8_t*)boot_block->dir_entries[i].file_name, (uint32_t)str_len)) ) {
            read_dentry_by_index(i, dentry);
            return 0;
        }
    }


    return -1;  // name not found
}

/* int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
 * Inputs: index - dentry index
           dentry - data entry pointer to write to
 * Return Value: 0 on success, -1 on fail
 * Function: populates dentry paramater with dentry in file system with index */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry) {
    if (index >= (uint32_t)boot_block->num_dir_entries || index < 0) {return -1;}  // index out of range

    *dentry = *(boot_block->dir_entries + index);

    return 0;
}

/* int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
 * Inputs: inode - inode num to read
           offset - byte offset to begin read
           buf - buffer to write read data to
           length - number of bytes to read
 * Return Value: number of bytes read
 * Function: populates buf with data */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    int i;
    inode_t* cur_inode;
    data_block_t* cur_dblock;
    uint32_t file_length;
    uint32_t cur_dblock_index;

    // from inode, compare length with number of data blocks,
    // use inode to get data block addresses and read?
    if (inode >= boot_block->num_inodes) {return -1;}    // inode out of range

    if (length < 0) {return -1;}        // check valid length
    
    // get inode block  
    cur_inode = (inode_t*)(fs_addr + (inode + 1) * BLOCK_SIZE);
    file_length = cur_inode->length;

    if (offset > file_length) {return -1;}        // invalid offset

    // cur_dblock_index = fs_addr + (1 + boot_block->num_inodes + offset/BLOCK_SIZE)
    cur_dblock_index = cur_inode->data_blocks[offset/BLOCK_SIZE];
    cur_dblock = (data_block_t*)(fs_addr + (1 + boot_block->num_inodes + cur_dblock_index) * BLOCK_SIZE);


    for (i = 0; i < length && i + offset < file_length; i++) {
        if ((i + offset) % BLOCK_SIZE == 0) {
            cur_dblock_index = cur_inode->data_blocks[(i + offset)/BLOCK_SIZE];
            cur_dblock = (data_block_t*)(fs_addr + (1 + boot_block->num_inodes + cur_dblock_index) * BLOCK_SIZE);
        }

        buf[i] = cur_dblock->data[(i+offset) % BLOCK_SIZE];
    }

    return i;
}
