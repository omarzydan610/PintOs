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
#include "threads/palloc.h"
// #include "lib/kernel/console.c"

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

struct lock fs_lock;

void verify_esp(void *esp);
static void syscall_handler(struct intr_frame *f);

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
    f->eax = sys_file_create((const char *)args[0], (unsigned)args[1]);
    break;
  case SYS_REMOVE:
    verify_string(args[0]);
    f->eax = sys_file_remove((const char *)args[0]);
    break;
  case SYS_OPEN:
    verify_string(args[0]);
    f->eax = sys_file_open((const char *)args[0]);
    break;
  case SYS_FILESIZE:
    f->eax = sys_file_size(args[0]);
    break;
  case SYS_READ:
    verify_buffer(args[1], args[2]);
    f->eax = sys_file_read(args[0], (void *)args[1], (unsigned)args[2]);
    break;
  case SYS_WRITE:
    verify_buffer(args[1], args[2]);
    f->eax = sys_file_write(args[0], (void *)args[1], (unsigned)args[2]);
    break;
  case SYS_SEEK:
    sys_file_seek(args[0], (off_t)args[1]);
    break;
  case SYS_TELL:
    f->eax = sys_file_tell(args[0]);
    break;
  case SYS_CLOSE:
    // verify_buffer(args[0], 1);
    sys_file_close(args[0]);
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

  printf("%s: exit(%d)\n", current_thread->name, status);
  thread_exit();
}

struct file *my_get_file(int fd)
{
  if (fd < 0 || fd < SYSTEM_FILES)
    return NULL;

  // printf("my get file and fd is : %d caller is : %s\n", fd, caller_name);
  struct thread *t = thread_current();
  struct list_elem *e;
  for (e = list_begin(&t->files); e != list_end(&t->files); e = list_next(e))
  {
    struct open_file *of = list_entry(e, struct open_file, elem);
    if (of->fd == fd)
      return of->file_ptr;
  }
  return NULL;
}

int sys_file_open(const char *file_name)
{

  const char *empty_str = "";
  // printf("file open is called file name is : %s\n", file_name);
  if (file_name == NULL)
    sys_exit(-1);
  else if (!strcmp(file_name, empty_str))
    return -1;

  // printf("file name is empty\n file name : %s\n", file_name);
  // printf("reached here in open -> 1 file name is : %s\n", file_name);
  lock_acquire(&fs_lock);
  struct file *opened_file = filesys_open(file_name);
  lock_release(&fs_lock);
  // printf("reached here in open -> 2 file name is : %s and there's a file opened = %d\n", file_name, opened_file != NULL);

  if (opened_file != NULL)
  {
    struct thread *t = thread_current();
    struct open_file *file = palloc_get_page(PAL_ZERO);
    if (file == NULL)
      return -1;

    file->file_ptr = opened_file;
    file->name = file_name;
    file->fd = t->next_fd++;

    list_push_back(&t->files, &file->elem);
    return file->fd;
    // printf("reached here in open -> 2 file name is : %s returned fd is : %d\n", file_name, t->files_cnt - 1);
  }
  else
    return -1;
}

int sys_file_write(int fd, void *buffer, unsigned size)
{

  if (fd < 0 || fd >= MAX_FILES_PER_PROCESS)
    return 0;

  if (fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO)
  {
    handle_sys_files(fd, buffer, size);
    return size;
    // printf("write finish\n");
  }

  struct file *cur_file = my_get_file(fd);
  if (cur_file == NULL)
    return -1;

  lock_acquire(&fs_lock);
  size = file_write(cur_file, buffer, (off_t)size);
  lock_release(&fs_lock);

  return size;
}

void handle_sys_files(int fd, const char *buffer, unsigned size)
{
  if (fd == STDOUT_FILENO)
  {
    // printf("putuf is called\n");
    putbuf(buffer, (off_t)size);
    // printf("putuf returned\n");
  }
  else
    sys_exit(-1);
}

void sys_file_seek(int fd, unsigned new_pos)
{
  if (fd < SYSTEM_FILES)
    sys_exit(-1);

  struct file *cur_file = my_get_file(fd);

  lock_acquire(&fs_lock);
  file_seek(cur_file, (off_t)new_pos);
  lock_release(&fs_lock);
}

int sys_file_tell(int fd)
{
  if (fd < SYSTEM_FILES)
    sys_exit(-1);

  struct file *cur_file = my_get_file(fd);

  lock_acquire(&fs_lock);
  off_t off = file_tell(cur_file);
  lock_release(&fs_lock);

  return off;
}

int sys_file_size(int fd)
{
  if (fd < SYSTEM_FILES)
    sys_exit(-1);

  struct file *cur_file = my_get_file(fd);

  lock_acquire(&fs_lock);
  off_t off = file_length(cur_file);
  lock_release(&fs_lock);

  return off;
}

bool sys_file_remove(const char *file_name)
{
  char *empty_str = "";
  if (file_name == NULL || !strcmp(file_name, empty_str))
    sys_exit(-1);

  lock_acquire(&fs_lock);
  bool success = filesys_remove(file_name);
  lock_release(&fs_lock);

  if (success)
    remove_file_from_table(file_name);
  return success;
}

void remove_file_from_table(const char *name)
{
  struct thread *t = thread_current();
  struct list_elem *e;

  for (e = list_begin(&t->files); e != list_end(&t->files); e = list_next(e))
  {
    struct open_file *of = list_entry(e, struct open_file, elem);
    if (of->name == name)
    {
      list_remove(&of->elem);
      palloc_free_page(of);
      t->files_cnt--;
      return;
    }
  }
}

bool sys_file_create(const char *file_name, unsigned size)
{
  char *empty_str = "";
  if (file_name == NULL || !strcmp(file_name, empty_str))
    sys_exit(-1);

  if (strlen(file_name) > MAX_FILE_NAME_LENGTH)
    return 0;

  lock_acquire(&fs_lock);
  bool success = filesys_create(file_name, (off_t)size);
  lock_release(&fs_lock);
  return success;
}

int sys_file_read(int fd, void *buffer, unsigned length)
{
  if (fd == NULL || buffer == NULL || length == NULL || fd < 0 || fd >= MAX_FILES_PER_PROCESS)
    return 0;

  if (fd < SYSTEM_FILES)
    handle_read_from_system_files(fd, buffer, length);

  struct file *file_to_read_from = my_get_file(fd);
  if (file_to_read_from == NULL)
    return -1;
  int actual_read = file_read(file_to_read_from, buffer, length);
  return actual_read;
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

void sys_file_close(int fd)
{
  if (fd < SYSTEM_FILES || fd > MAX_FILES_PER_PROCESS)
    sys_exit(-1);

  struct thread *t = thread_current();
  struct list_elem *e;

  for (e = list_begin(&t->files); e != list_end(&t->files); e = list_next(e))
  {
    struct open_file *of = list_entry(e, struct open_file, elem);
    if (of != NULL && of->fd == fd)
    {
      file_close(of->file_ptr);
      list_remove(&of->elem);
      palloc_free_page(of);
      t->files_cnt--;
      return;
    }
  }
}

void remove_file_from_table_by_fd(int fd)
{
  struct thread *t = thread_current();
  struct list_elem *e;

  for (e = list_begin(&t->files); e != list_end(&t->files); e = list_next(e))
  {
    struct open_file *of = list_entry(e, struct open_file, elem);
    if (of->fd == fd)
    {
      list_remove(&of->elem);
      palloc_free_page(of);
      t->files_cnt--;
      return;
    }
  }
}