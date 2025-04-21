# PROJECT 1: THREADS - DESIGN DOCUMENT

## GROUP

Fill in the names and email addresses of your group members.

- Huthaifa Omar
- FirstName LastName <email@domain.example>
- FirstName LastName <email@domain.example>

## PRELIMINARIES

If you have any preliminary comments on your submission, notes for the TAs, or extra credit, please give them here.

Please cite any offline or online sources you consulted while preparing your submission, other than the Pintos documentation, course text, lecture notes, and course staff.

## ALARM CLOCK

### DATA STRUCTURES

**A1:** Copy here the declaration of each new or changed `struct` or `struct` member, global or static variable, `typedef`, or enumeration. Identify the purpose of each in 25 words or less.

in `thread.c` file there is ```c struct thread``` I will use a sorted data structure `sorted list` that contains wake_tick for each sleeping thread. This data structure should get the minimum wake_tick value in O(1) so that it is fast and doesn't consume time in the `timer_interrupt` function.

```c
struct sleeping_thread {
    struct list_elem elem; //used to link the linked list with this struct
    struct thread *thread;
    int64_t wake_tick;
}
```

### ALGORITHMS

**A2:** Briefly describe what happens in a call to timer_sleep(), including the effects of the timer interrupt handler.

- In timer_sleep()

    1. disable interrupts
    2. wake_tick will be calculated by adding timer_ticks() + ticks
    3. add current thread to sorted sleeping list, with threads ordered by their wake_tick
    4. call thread_block()
    5. restore interrupts

- In timer_interrupt()
    1. increment ticks
    2. check if any sleeping thread have reached their wake_tick
    3. remove all threads that have reached their wake_tick
    4. unblock these threads using thread_unblock
    5. call thread_tick()

**A3:** What steps are taken to minimize the amount of time spent in the timer interrupt handler?
    - Will use a sorted list to make it efficient to get the threads that should be removed from the sleeping list in O(1)

### SYNCHRONIZATION

**A4:** How are race conditions avoided when multiple threads call timer_sleep() simultaneously?
    - by disabling interrupts.
**A5:** How are race conditions avoided when a timer interrupt occurs during a call to timer_sleep()?
    - by disabling interrupts.

### RATIONALE

**A6:** Why did you choose this design? In what ways is it superior to another design you considered?
    - It is the most intuitive and easy and effective design in my opinion and I have not thought of any other approach.

## PRIORITY SCHEDULING

### DATA STRUCTURES

**B1:** Copy here the declaration of each new or changed `struct` or `struct` member, global or static variable, `typedef`, or enumeration. Identify the purpose of each in 25 words or less.

**B2:** Explain the data structure used to track priority donation. Use ASCII art to diagram a nested donation. (Alternately, submit a .png file.)

### ALGORITHMS

**B3:** How do you ensure that the highest priority thread waiting for a lock, semaphore, or condition variable wakes up first?

**B4:** Describe the sequence of events when a call to lock_acquire() causes a priority donation. How is nested donation handled?

**B5:** Describe the sequence of events when lock_release() is called on a lock that a higher-priority thread is waiting for.

### SYNCHRONIZATION

**B6:** Describe a potential race in thread_set_priority() and explain how your implementation avoids it. Can you use a lock to avoid this race?

### RATIONALE

**B7:** Why did you choose this design? In what ways is it superior to another design you considered?

## ADVANCED SCHEDULER

### DATA STRUCTURES

**C1:** Copy here the declaration of each new or changed `struct` or `struct` member, global or static variable, `typedef`, or enumeration. Identify the purpose of each in 25 words or less.

### ALGORITHMS

**C2:** Suppose threads A, B, and C have nice values 0, 1, and 2. Each has a recent_cpu value of 0. Fill in the table below showing the scheduling decision and the priority and recent_cpu values for each thread after each given number of timer ticks:

| timer ticks | recent_cpu |     | priority |     |     | thread |
| ----------- | ---------- | --- | -------- | --- | --- | ------ |
|             | A          | B   | C        | A   | B   | C      | to run |
| 0           |            |     |          |     |     |        |        |
| 4           |            |     |          |     |     |        |        |
| 8           |            |     |          |     |     |        |        |
| 12          |            |     |          |     |     |        |        |
| 16          |            |     |          |     |     |        |        |
| 20          |            |     |          |     |     |        |        |
| 24          |            |     |          |     |     |        |        |
| 28          |            |     |          |     |     |        |        |
| 32          |            |     |          |     |     |        |        |
| 36          |            |     |          |     |     |        |        |

**C3:** Did any ambiguities in the scheduler specification make values in the table uncertain? If so, what rule did you use to resolve them? Does this match the behavior of your scheduler?

**C4:** How is the way you divided the cost of scheduling between code inside and outside interrupt context likely to affect performance?

### RATIONALE

**C5:** Briefly critique your design, pointing out advantages and disadvantages in your design choices. If you were to have extra time to work on this part of the project, how might you choose to refine or improve your design?

**C6:** The assignment explains arithmetic for fixed-point math in detail, but it leaves it open to you to implement it. Why did you decide to implement it the way you did? If you created an abstraction layer for fixed-point math, that is, an abstract data type and/or a set of functions or macros to manipulate fixed-point numbers, why did you do so? If not, why not?

## SURVEY QUESTIONS

Answering these questions is optional, but it will help us improve the course in future quarters. Feel free to tell us anything you want--these questions are just to spur your thoughts. You may also choose to respond anonymously in the course evaluations at the end of the quarter.

1. In your opinion, was this assignment, or any one of the three problems in it, too easy or too hard? Did it take too long or too little time?

2. Did you find that working on a particular part of the assignment gave you greater insight into some aspect of OS design?

3. Is there some particular fact or hint we should give students in future quarters to help them solve the problems? Conversely, did you find any of our guidance to be misleading?

4. Do you have any suggestions for the TAs to more effectively assist students, either for future quarters or the remaining projects?

5. Any other comments?
