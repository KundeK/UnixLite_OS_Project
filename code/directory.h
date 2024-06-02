#ifndef _DIR_H
#define _DIR_H

#include "file_system.h"

#define MAX_NAME_LENGTH 32

int32_t dir_open(uint8_t* dir_name);
int32_t dir_close(uint8_t* dir_name);
int32_t dir_write(int32_t fd, uint8_t* buf, int32_t nbytes);
int32_t dir_read(int32_t fd, uint8_t* buf, int32_t nbytes);

#endif
