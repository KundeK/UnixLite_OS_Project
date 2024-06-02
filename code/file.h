#ifndef _FILE_H
#define _FILE_H

#include "directory.h"
#include "file_system.h"

int32_t file_open(uint8_t* file_name);
int32_t file_close(uint8_t* file_name);
int32_t file_write(int32_t fd, uint8_t* buf, int32_t nbytes);
int32_t file_read(int32_t inode, uint8_t* buf, int32_t count);

#endif
