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
// #include "lib/kernel/console.c"

static void syscall_handler(struct intr_frame *);
void sys_exit(int status);
static int sys_exec(const char* cmd_line);
struct lock fs_lock;


void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&fs_lock);
}

static void
syscall_handler(struct intr_frame *f)
{
  void *esp = f->esp;
  verify_esp(esp);

  int syscall_number = *(int *)esp;

  int args[3];
  int numberOfArgs = get_number_of_args(syscall_number);
  load_args(esp, args, numberOfArgs);

  switch (syscall_number)
  {
    case SYS_HALT:
    shutdown_power_off();
    break;
  case SYS_EXIT:
    sys_exit(args[0]);
    break;
  case SYS_EXEC:
    f->eax = sys_exec((const char*)args[0]);
    return;
    break;
  case SYS_WAIT:
    f->eax = process_wait(args[0]);
    break;
  case SYS_CREATE:
    sys_file_create(args, f);
    break;
  case SYS_REMOVE:
    sys_file_remove(args, f);
    break;
  case SYS_OPEN:
    sys_file_open(args, f);
    break;
  case SYS_FILESIZE:
    sys_file_size(args, f);
    break;
  case SYS_READ:
    // implement read syscall
    break;
  case SYS_WRITE:
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
  thread_exit();
}

void 
verify_esp(void *esp)
{
  // Check if esp is within the valid range
  if (esp < (int *)0x08048000 || esp >= (int *)PHYS_BASE)
  {
    sys_exit(-1);   
  }
  // Check if esp is aligned to a page boundary
  void *ptr = pagedir_get_page(thread_current()->pagedir, esp);
  if (ptr == NULL)
  {
    sys_exit(-1);
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

void 
load_args(void *esp, int *args, int numberOfArgs)
{
  for (int i = 0; i < numberOfArgs; i++)
  {
    verify_esp(esp + (i + 1) * sizeof(int));
    args[i] = *(int *)(esp + (i + 1) * sizeof(int));
  }
}

void
sys_exit(int status)
{
  struct thread *current_thread = thread_current();
  current_thread->exit_status = status;

  printf("%s: exit(%d)\n", thread_current()->name, status);


  thread_exit();
}

struct file* get_file(int fd){
  struct thread* t = thread_current ();
  struct file* cur_file = t->files[fd];
  if(cur_file == NULL)
    sys_exit(-1);
  return cur_file;
}

void
sys_file_open (int* args, struct intr_frame* f){

  char* empty_str = "";
  if(args[0] == NULL || !strcmp((const char*) args[0], empty_str))
    sys_exit(-1);

  lock_acquire (&fs_lock);
  struct file* opened_file = filesys_open ((const char*) args[0]);
  lock_release (&fs_lock);
  if(opened_file != NULL)
  {
    struct thread* t = thread_current ();
    t->files[t->files_cnt] = opened_file;
    f->eax = t->files_cnt++;
  } else 
      f->eax = -1;
}

void sys_file_write (int* args, struct intr_frame* f){
  int fd = args[0], written_bytes = args[2];

  if(fd == 0 || fd == 1 || fd == 2)
    handle_sys_files (fd, (const char*) args[1], (size_t) args[2]);
  
  struct file* cur_file = get_file(fd);

  lock_acquire (&fs_lock);
  written_bytes = file_write (cur_file,(const char *) args[1],(off_t) args[2]);
  lock_release (&fs_lock);

  f->eax = written_bytes;
}

void
handle_sys_files (int fd, const char* buffer, off_t size){
  if(fd == 1)
     putbuf (buffer, size);
  else
    sys_exit(-1);
}


void
sys_file_seek (int* args){
  int fd = args[0];
  if(fd < 3)
    sys_exit(-1);

  struct file* cur_file = get_file(fd);

  lock_acquire(&fs_lock);
  file_seek(cur_file, (off_t) args[1]);
  lock_release(&fs_lock);
}

void
sys_file_tell (int* args, struct intr_frame* f){
  int fd = args[0];
  if(fd < 3)
    sys_exit(-1);

  struct file* cur_file = get_file(fd);

  lock_acquire(&fs_lock);
  off_t off = file_tell(cur_file);
  lock_release(&fs_lock);

  f->eax = off;
}

int
sys_file_size (int* args, struct intr_frame* f){
  int fd = args[0];
  if(fd < 3)
    sys_exit(-1);

  struct file* cur_file = get_file(fd);
  
  lock_acquire(&fs_lock);
  off_t off = file_length(cur_file);
  lock_release(&fs_lock);

  f->eax = off;
}

void
sys_file_remove (int* args, struct intr_frame* f){
  char* empty_str = "";
  if(args[0] == NULL || !strcmp((const char*) args[0], empty_str))
    sys_exit(-1);

  lock_acquire (&fs_lock);
  bool success = filesys_remove ((const char*) args[0]);
  lock_release (&fs_lock);
  f->eax = success;
}

void
sys_file_create (int* args, struct intr_frame* f){
  char* empty_str = "";
  if(args[0] == NULL || !strcmp((const char*) args[0], empty_str))
    sys_exit(-1);

  lock_acquire (&fs_lock);
  bool success = filesys_create ((const char*) args[0], (off_t) args[1]);
  lock_release (&fs_lock);
  f->eax = success;
}


static int
sys_exec(const char* cmd_line)
{
  if (cmd_line == NULL)
    sys_exit(-1);

  for (const char *c = cmd_line; *c != '\0'; c++)
  {
    if (!is_user_vaddr((const void *)c))
      sys_exit(-1);
    verify_esp ((void *)c);
  }  
  
  //!   // Make sure the string is accessible
  //! char *str = pagedir_get_page(thread_current()->pagedir, cmd_line);
  //! if (str == NULL)
  //!   return -1;
  
  tid_t tid = process_execute(cmd_line);
  return tid;
}