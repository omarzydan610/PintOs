#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "threads/synch.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "string.h"
// #include "lib/kernel/console.c"

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

static void syscall_handler(struct intr_frame *);
void sys_exit(int status);
static int sys_exec(const char *cmd_line);
struct lock fs_lock;

void verify_esp(void *esp);

const int MAX_FILE_NAME_LENGTH = 14;

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&fs_lock);
}

static void
syscall_handler(struct intr_frame *f)
{
  verify_esp(f->esp);
  void *esp = f->esp;

  int syscall_number = *(int *)esp;

  int args[3];
  int numberOfArgs = get_number_of_args(syscall_number);
  load_args((int *)f->esp, args, numberOfArgs);

  switch (syscall_number)
  {
  case SYS_HALT:
    shutdown_power_off();
    break;
  case SYS_EXIT:
    sys_exit(args[0]);
    break;
  case SYS_EXEC:
    verify_string(args[0]);
    f->eax = process_execute((const char *)args[0]);
    return;
    break;
  case SYS_WAIT:
    f->eax = process_wait(args[0]);
    break;
  case SYS_CREATE:
    verify_string(args[0]);
    sys_file_create(args, f);
    break;
  case SYS_REMOVE:
    verify_string(args[0]);
    sys_file_remove(args, f);
    break;
  case SYS_OPEN:
    verify_string(args[0]);
    sys_file_open(args, f);
    break;
  case SYS_FILESIZE:
    sys_file_size(args, f);
    break;
  case SYS_READ:
    verify_buffer(args[1], args[2]);
    sys_file_read(args, f);
    break;
  case SYS_WRITE:
    verify_buffer(args[1], args[2]);
    sys_file_write(args, f);
    break;
  case SYS_SEEK:
    sys_file_seek(args);
    break;
  case SYS_TELL:
    sys_file_tell(args, f);
    break;
  case SYS_CLOSE:
    // implement close syscall
    break;
  default:
    sys_exit(-1);
  }
  // printf("syscall handler finished : %d", syscall_number);
}


void verify_esp(void *esp)
{
  // Check if esp is within the valid range
  if (esp == NULL || !is_user_vaddr(esp) || pagedir_get_page(thread_current()->pagedir, esp) == NULL)
  {
    sys_exit(-1);
  }
}

void verify_string(const void *str)
{
  verify_esp((void *)str);

  const char *s = (const char *)str;
  while (true)
  {
    verify_esp((void *)s);
    if (*s == '\0')
    {
      break;
    }
    s++;
  }
}

void verify_buffer(void *buffer, unsigned size)
{
  verify_esp((void *)buffer);

  int cnt = 0;
  char *curr = (char *)buffer;
  while (cnt < size)
  {
    verify_esp((void *)curr);
    curr++;
    cnt++;
  }
}

int get_number_of_args(int syscall_number)
{
  switch (syscall_number)
  {
  case SYS_HALT:
    return 0;
  case SYS_EXIT:
    return 1;
  case SYS_EXEC:
    return 1;
  case SYS_WAIT:
    return 1;
  case SYS_CREATE:
    return 2;
  case SYS_REMOVE:
    return 1;
  case SYS_OPEN:
    return 1;
  case SYS_FILESIZE:
    return 1;
  case SYS_READ:
    return 3;
  case SYS_WRITE:
    return 3;
  case SYS_SEEK:
    return 2;
  case SYS_TELL:
    return 1;
  case SYS_CLOSE:
    return 1;
  default:
    sys_exit(-1);
  }
}

void load_args(int *esp, int *args, int numberOfArgs)
{
  for (int i = 0; i < numberOfArgs; i++)
  {
    verify_esp(esp + (i + 1));
    args[i] = *(int *)(esp + (i + 1));
  }
}

void sys_exit(int status)
{
  struct thread *current_thread = thread_current();
  current_thread->exit_status = status;
  if (current_thread->executable != NULL)
  {
    file_allow_write(current_thread->executable);
    file_close(current_thread->executable);
  }

  for (int i = SYSTEM_FILES; i < MAX_FILES_PER_PROCESS; i++)
  {
    if (current_thread->files[i] != NULL)
    {
      lock_acquire(&fs_lock);
      file_close(current_thread->files[i]->file_ptr);
      lock_release(&fs_lock);
      free(current_thread->files[i]);
      current_thread->files[i] = NULL;
    }
  }

  printf("%s: exit(%d)\n", current_thread->name, status);
  thread_exit();
}

struct file *my_get_file(int fd, const char *caller_name)
{

  // printf("my get file and fd is : %d caller is : %s\n", fd, caller_name);
  struct thread *t = thread_current();
  struct open_file *cur_file = t->files[fd];
  if (cur_file == NULL)
    sys_exit(-1);
  return cur_file->file_ptr;
}

void sys_file_open(int *args, struct intr_frame *f)
{

  const char *empty_str = "", *file_name = (const char *)args[0];
  // printf("file open is called file name is : %s\n", file_name);
  if (file_name == NULL)
    sys_exit(-1);
  else if (!strcmp(file_name, empty_str))
  {
    f->eax = -1;
    return;
  }

  // printf("file name is empty\n file name : %s\n", file_name);
  // printf("reached here in open -> 1 file name is : %s\n", file_name);
  lock_acquire(&fs_lock);
  struct file *opened_file = filesys_open(file_name);
  lock_release(&fs_lock);
  // printf("reached here in open -> 2 file name is : %s and there's a file opened = %d\n", file_name, opened_file != NULL);

  if (opened_file != NULL)
  {
    struct thread *t = thread_current();
    struct open_file *file = (struct open_file *)malloc(sizeof(struct open_file));
    file->file_ptr = opened_file;
    file->name = file_name;
    t->files[t->files_cnt] = file;
    f->eax = t->files_cnt++;
    // printf("reached here in open -> 2 file name is : %s returned fd is : %d\n", file_name, t->files_cnt - 1);
  }
  else
  {
    f->eax = -1;
  }
}

void sys_file_write(int *args, struct intr_frame *f)
{
  int fd = args[0], written_bytes = args[2];

  if (fd < 0 || fd >= MAX_FILES_PER_PROCESS)
  {
    f->eax = 0;
    return;
  }

  if (fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO)
  {
    handle_sys_files(fd, (const char *)args[1], (size_t)args[2]);
    f->eax = written_bytes;
    // printf("write finish\n");
    return;
  }

  const char *name = "sys write";
  struct file *cur_file = my_get_file(fd, name);

  lock_acquire(&fs_lock);
  f->eax = file_write(cur_file, (const void *)args[1], (off_t)args[2]);
  lock_release(&fs_lock);
}

void handle_sys_files(int fd, const char *buffer, off_t size)
{
  if (fd == STDOUT_FILENO)
  {
    // printf("putuf is called\n");
    putbuf(buffer, size);
    // printf("putuf returned\n");
  }
  else
    sys_exit(-1);
}

void sys_file_seek(int *args)
{
  int fd = args[0];
  if (fd < SYSTEM_FILES)
    sys_exit(-1);

  const char *name = "sys seek";
  struct file *cur_file = my_get_file(fd, name);

  lock_acquire(&fs_lock);
  file_seek(cur_file, (off_t)args[1]);
  lock_release(&fs_lock);
}

void sys_file_tell(int *args, struct intr_frame *f)
{
  int fd = args[0];
  if (fd < SYSTEM_FILES)
    sys_exit(-252);

  const char *name = "sys tell";
  struct file *cur_file = my_get_file(fd, name);

  lock_acquire(&fs_lock);
  off_t off = file_tell(cur_file);
  lock_release(&fs_lock);

  f->eax = off;
}

int sys_file_size(int *args, struct intr_frame *f)
{
  int fd = args[0];
  if (fd < SYSTEM_FILES)
    sys_exit(-1);

  const char *name = "sys size";
  struct file *cur_file = my_get_file(fd, name);

  lock_acquire(&fs_lock);
  off_t off = file_length(cur_file);
  lock_release(&fs_lock);

  f->eax = off;
}

void sys_file_remove(int *args, struct intr_frame *f)
{
  char *empty_str = "";
  if (args[0] == NULL || !strcmp((const char *)args[0], empty_str))
    sys_exit(-1);

  lock_acquire(&fs_lock);
  bool success = filesys_remove((const char *)args[0]);
  lock_release(&fs_lock);

  if (success)
    remove_file_from_table((const char *)args[0]);
  f->eax = success;
}

void remove_file_from_table(const char *name)
{
  struct thread *t = thread_current();
  for (int i = SYSTEM_FILES; i < MAX_FILES_PER_PROCESS; ++i)
  {
    if (t->files[i]->name == name)
    {
      free(t->files[i]);
      t->files[i] = NULL;
      t->files_cnt--;
      break;
    }
  }
}

void sys_file_create(int *args, struct intr_frame *f)
{
  char *empty_str = "";
  if (args[0] == NULL || !strcmp((const char *)args[0], empty_str))
    sys_exit(-1);

  if (strlen((char *)args[0]) > MAX_FILE_NAME_LENGTH)
  {
    f->eax = 0;
    return;
  }

  lock_acquire(&fs_lock);
  bool success = filesys_create((const char *)args[0], (off_t)args[1]);
  lock_release(&fs_lock);
  f->eax = success;
}

void sys_file_read(int *args, struct intr_frame *f)
{
  int fd = args[0];
  const void *buffer = (const void *)args[1];
  unsigned length = args[2];
  if (fd == NULL || buffer == NULL || length == NULL)
  {
    f->eax = 0;
    return;
  }

  if (fd < 0 || fd >= MAX_FILES_PER_PROCESS)
  {
    f->eax = 0;
    return;
  }

  if (fd < SYSTEM_FILES)
    handle_read_from_system_files(fd, buffer, length);
  const char *name = "sys read";
  struct file *file_to_read_from = my_get_file(fd, name);
  int actual_read = file_read(file_to_read_from, buffer, length);
  f->eax = actual_read;
}

int handle_read_from_system_files(int fd, void *buffer, unsigned length)
{
  if (fd == STDIN_FILENO)
  {
    unsigned char *buf = (unsigned char *)buffer;
    for (int i = 0; i < length; ++i)
    {
      unsigned char c = input_getc();
      buf[i] = c;
    }
  }
  else
    sys_exit(-1);
  return length;
}