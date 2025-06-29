#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "synch.h"
#include "threads/fixedPoint.h"
/* States in a thread's life cycle. */
enum thread_status
{
   THREAD_RUNNING, /* Running thread. */
   THREAD_READY,   /* Not running but ready to run. */
   THREAD_BLOCKED, /* Waiting for an event to trigger. */
   THREAD_DYING    /* About to be destroyed. */
};

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) - 1) /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0      /* Lowest priority. */
#define PRI_DEFAULT 31 /* Default priority. */
#define PRI_MAX 63     /* Highest priority. */
#define MAX_FILES_PER_PROCESS 128
#define SYSTEM_FILES 3
#define PRI_MIN 0      /* Lowest priority. */
#define PRI_DEFAULT 31 /* Default priority. */
#define PRI_MAX 63     /* Highest priority. */
/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */

struct open_file
{
   struct file *file_ptr;
   const char *name;
   struct list_elem elem;
   int fd;
};
struct thread

{
   /* Owned by thread.c. */
   tid_t tid;                 /* Thread identifier. */
   enum thread_status status; /* Thread state. */
   char name[16];             /* Name (for debugging purposes). */
   uint8_t *stack;            /* Saved stack pointer. */
   int priority;              /* Priority. */
   struct list_elem allelem;  /* List element for all threads list. */
   int original_priority;
   struct list donated_priorities; /* List of locks held by the thread. */
   struct lock *waiting_on_lock;

   int nice;               /* Niceness value. */
   fixed_point recent_cpu; /* Recent CPU usage for advanced scheduler. */

   /* Shared between thread.c and synch.c. */
   struct list_elem elem; /* List element. */

   /* Owned by userprog/process.c. */
   uint32_t *pagedir; /* Page directory. */
   /* Huthaifa for userprog*/
   struct thread *parent;
   struct list children;
   struct list_elem child_elem;
   int exit_status;
   int child_exit_status;
   struct file *executable;

   struct semaphore sync_lock;
   tid_t wait_on;

   /* files the process ownes */
   int files_cnt;
   struct list files;
   int next_fd;
   /* Owned by thread.c. */
   unsigned magic; /* Detects stack overflow. */
#ifdef USERPROG
   /* Owned by userprog/process.c. */
   uint32_t *pagedir; /* Page directory. */
#endif

   /* Owned by thread.c. */
   unsigned magic; /* Detects stack overflow. */
};

/* If false (default), use round-robin r.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

static struct list sleep_list;

struct sleeping_thread
{
   struct list_elem elem; /* List element. */
   struct thread *thread;
   int64_t wake_tick;
};

void thread_init(void);
void thread_start(void);

void thread_sleep(int64_t wake_tick);
void thread_wake_sleeping_threads(int64_t current_tick);

void thread_tick(void);
void thread_print_stats(void);

typedef void thread_func(void *aux);
tid_t thread_create(const char *name, int priority, thread_func *, void *, struct file *);

void thread_block(void);
void thread_unblock(struct thread *);

struct thread *thread_current(void);
tid_t thread_tid(void);
const char *thread_name(void);

void thread_exit(void) NO_RETURN;
void thread_yield(void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func(struct thread *t, void *aux);
void thread_foreach(thread_action_func *, void *);

int thread_get_priority(void);
void thread_set_priority(int);
int get_prority_of_a_thread(struct thread *);

int thread_get_nice(void);
void thread_set_nice(int);
int thread_get_recent_cpu(void);
int thread_get_load_avg(void);

void calculatePriority(struct thread *t, void *aux UNUSED);
void calculateLoadAvg(void);
void calculateRecentCpu(struct thread *t);
void incrementRecentCpu(void);
void updateAllPriorities(void);
bool priority_less(const struct list_elem *a, const struct list_elem *b, void *aux);
void repair_file_table(struct thread *);

#endif /* threads/thread.h */
