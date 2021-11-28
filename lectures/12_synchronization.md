- BSP: Bootstrap Processor.
    - BIOS starts BSP which start the Bootloader -> main-> mpmain.
    - startothers(): BSP sends interrupts to other Application Processors (AP) to wake then up.
        - with each interrupt we send a pointer to stack.
        - interrupt itself sends pointer to entry point that each AP needs to start executing when they wake up.

- each AP needs to start running an assembly sequence.
    - this assembly sequence can be anywhere in the memory as long as it's not used.
    - xV6 uses 0x7000, same as used by boot loader, which guarantees that it is not used.

- Inter process interrupt (IPI):
    - interrupt that one cpu can trigger that is processed on another cpu.
    - used to wake up other processors.
    - useful for performing inter-process synchronizations of actions.

- lapicstartap: advances programmable interrupt controller.
    - used to send magical IPI to wake other Application Processors.

---
- cpuid(): this calls the advanced programmable interrupt controller to get the current CPU id.
---
- why do we need a different GDT for each CPU?
    - because of the TSS entry which points to the kernel stack of each process is per CPU.
---
- previously xv6 used gs segment which is the only segment per cpu to maintain Per-CPU variables.
- now xv6 just uses cpus array with cpuid() to get the current processing CPU.
---
- chain of flow for each AP CPU to enter scheduler and start running processes.
    - [on BST CPU]->main()->startothers(void)->[on newly woken up AP CPU]->entryother.S->mpenter()->mpmain()->scheduler()
---
- How CPUs can access the same memory without causing any conflicts.
    - may be 2 or more CPUs are moving to and/or from the same memory locations
    - how is this conflict resolved?
        - using Cache-coherence and memory hierarchy.
---
- lock: this ensures that this is not split into micro operations of load and store. This locks the cacheline and guarantees that cacheline will not be evicted.
---
# preventing deadlock
- general rule:
    - If Process 1 requires locks in order of A, B and C and Process 2 needs locks D, B and C. Then Process 1 and Process 2 both need to acquire the lock in same order.
    - the order of locks can be imposed by sorting locks based on their virtual address.
    - then if Process 1 acquires lock in A, B and C order then Process 2 must acquire lock in B, C, D or D, B, C  or B, D, C depending upon the D's sorted position based on it's address.
- locks and interrupts:
    - If process 1 processes a network interrupt and acquires a lock A and simultaneously another interrupt arrives just after Process 1 acquired lock, then the Process 1 will be interrupted and Process 2 will try to acquire lock A again but will not be able to because Process 1 already acquired it. This will cause deadlock again.
    - to prevent this when before you acquire a lock you disable interrupts.
---
- DOUBT:
```c
201 void*
202 send(struct q *q, void *p)
203 {
204 while(q->ptr != 0)
205 ; // **why we don't use sleep here as well, won't this be wasting CPU cycles?**
206 q->ptr = p;
207 wakeup(q); /*wake recv*/
208 }

- from book: A complete sender/receiver implementation would also sleep in send when waiting for a receiver to consume the value from a previous sen
```
---
# Lost wakeup problem
- the send 204 test is for sending another element.
- to solve holding lock while sleeping.
    - we can pass lock to the sleeping function and release the lock such that we don't break atomicity making sure while we are releasing the lock no one else sends a wakeup and we miss it.

---
- DOUBT:
```c
2836 // Reacquire original lock.
2837 if(lk != &ptable.lock){
2838    release(&ptable.lock);
2839    acquire(lk);
2840 }

- **why line 2838 and 2839 is not flipped**?
    - i read the book and got more confused.
```