#include "directory.h"
#include "terminal.h"

uint32_t read_index = 0;
dentry_t* dir_dentry_ptr;
dentry_t dir_dentry;

/* uint32_t dir_open(uint8_t* dir_name)
 * Inputs: dir_name - name of directory to open
 * Return Value: 0
 * Function: opens dentry */
int32_t dir_open(uint8_t* dir_name) {
    dir_dentry_ptr = &dir_dentry;
    read_index = 0;
    return read_dentry_by_name(dir_name, dir_dentry_ptr);
}

/* uint32_t dir_close()
 * Inputs: 
 * Return Value: 0
 * Function: closes dentry */
int32_t dir_close(uint8_t* dir_name) {
    read_index = 0;
    return 0;
}

/* uint32_t dir_write()
 * Inputs: 
 * Return Value: -1
 * Function:  */
int32_t dir_write(int32_t fd, uint8_t* buf, int32_t nbytes) {
    return -1;
}

/* uint32_t dir_read(uint8_t* buf)
 * Inputs: buf - buf to write to
 * Return Value: bytes written
 * Function: writes file name to buf */
int32_t dir_read(int32_t fd, uint8_t* buf, int32_t nbytes) {
    int i;
    dentry_t dentry;
    dentry_t* dentry_ptr = &dentry;

    if (read_dentry_by_index(read_index, dentry_ptr) == -1) {return 0;}

    for (i = 0; i < MAX_NAME_LENGTH; i++) {
        buf[i] = dentry.file_name[i];
    }
    buf[MAX_NAME_LENGTH] = NULL;

    read_index++;

    return strlen((const int8_t*) buf);
}
