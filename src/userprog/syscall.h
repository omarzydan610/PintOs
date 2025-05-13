#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H


/* lock to the file system so that only one process can enter it */
extern struct lock fs_lock;


void syscall_init (void);

#endif /* userprog/syscall.h */
