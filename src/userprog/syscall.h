#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "filesys/off_t.h"

/* lock to the file system so that only one process can enter it */


void syscall_init (void);
// System call handler and initialization
void syscall_init(void);

// Memory verification functions
void verify_esp(void *esp);
void verify_string(const void *str);
void verify_buffer(void *buffer, unsigned size);

// Argument handling
int get_number_of_args(int syscall_number);
void load_args(int *esp, int *args, int numberOfArgs);

// File operations
struct file *my_get_file(int fd);
int sys_file_open(const char* file_name);
int sys_file_write(int fd, void* buffer, unsigned size);
void handle_sys_files(int fd, const char *buffer, unsigned size);
_Bool sys_file_create(const char* file_name, unsigned size);
_Bool sys_file_remove(const char* file_name);
int sys_file_size(int fd);
void sys_file_seek(int fd, unsigned new_pos);
int sys_file_tell(int fd);
int sys_file_read(int fd, void* buffer, unsigned length);
void sys_file_close(int fd);

// Process management
void sys_exit(int status);
static int sys_exec(const char *cmd_line);
#endif /* userprog/syscall.h */
