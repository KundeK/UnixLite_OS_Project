#include "file.h"

/* int32_t file_open(uint8_t* file_name)
 * Inputs: file_name - name of file to open
 * Return Value: 0
 * Function: opens file */
dentry_t* dentry_ptr;
dentry_t dentry;

int32_t file_open(uint8_t* file_name) {
    return read_dentry_by_name(file_name, dentry_ptr);
}

/* int32_t file_close()
 * Inputs:
 * Return Value: 0
 * Function: closes file */
int32_t file_close(uint8_t* file_name) {
    return 0;
}

/* int32_t file_write()
 * Inputs:
 * Return Value: -1
 * Function:  */
int32_t file_write(int32_t fd, uint8_t* buf, int32_t nbytes) {
    return -1;
}

/* int32_t file_read(uint32_t count, uint8_t* buf)
 * Inputs:  count - number of bytes to read
            buf - buf to write to
 * Return Value: bytes written
 * Function: reads count bytes from file */
int32_t file_read(int32_t fd, uint8_t* buf, int32_t count) {
    if (count < 0) {return -1;}

    return read_data(((fd_t*)fd)->inode_num, ((fd_t*)fd)->file_pos, buf, count);
}
