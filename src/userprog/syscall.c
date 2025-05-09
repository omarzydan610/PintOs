#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"

static void syscall_handler(struct intr_frame *);

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
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
    // implement exit syscall
    break;
  case SYS_EXEC:
    // implement exec syscall
    break;
  case SYS_WAIT:
    // implement wait syscall
    break;
  case SYS_CREATE:
    // implement create syscall
    break;
  case SYS_REMOVE:
    // implement remove syscall
    break;
  case SYS_OPEN:
    // implement open syscall
    break;
  case SYS_FILESIZE:
    // implement filesize syscall
    break;
  case SYS_READ:
    // implement read syscall
    break;
  case SYS_WRITE:
    // implement write syscall
    break;
  case SYS_SEEK:
    // implement seek syscall
    break;
  case SYS_TELL:
    // implement tell syscall
    break;
  case SYS_CLOSE:
    // implement close syscall
    break;
  //! default:
    //! exit(-1);
  }
  thread_exit();
}

void verify_esp(void *esp)
{
  // Check if esp is within the valid range
  if (esp < (int *)0x08048000 || esp >= (int *)PHYS_BASE)
  {
    //! exit(-1);   
  }
  // Check if esp is aligned to a page boundary
  void *ptr = pagedir_get_page(thread_current()->pagedir, esp);
  if (ptr == NULL)
  {
    //! exit(-1);
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
  //! default:
    //! exit(-1);
  }
}

void load_args(void *esp, int *args, int numberOfArgs)
{
  for (int i = 0; i < numberOfArgs; i++)
  {
    verify_esp(esp + (i + 1) * sizeof(int));
    args[i] = *(int *)(esp + (i + 1) * sizeof(int));
  }
}