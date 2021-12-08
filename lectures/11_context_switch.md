```c
//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};
```

- But what do you think needs to happen inside  switch()?
    - proc data structure is always available in a global space.
        - because we agreed the process (proc) data structures remains on some global array/list of processes and it'll not go anywhere.
        - scheduler will fetch the process (proc) data structure from global list and everything will be restored.
        - so we do not need to do anything with proc properties like page table. ptr to pgdir is already there. Next time i'll come back i'll be able to get previous pgdir using the ptr pde_t*. this is already in the box.


    - swtch: /home/vbharot/cs238p/xv6-public/swtch.S
        - use switching from P1 to process 2 we load the context of P2.
    
    - we will run for some time in kernel for P2.
    - IRET will move us into the user space for P2.
    - when we are processing interrupts we already push all user level general registers using pushal during alltraps.
    - need to preserve general registers in the stack in the switch.
        - Now the only thing left is to save the **states of the general registers**. **WHY AGAIN?**
            - **THESE REGISTERS WERE UPDATED/MODIFIED DURING THE PERIOD OF EXECUTION BETWEEN ALLTRAPS AND SCHED CALL INSIDE THE KERNEL**.
            - **AND AS THESE REGISTERS ARE CALEE SAVED WE NEED TO PRESERVE THEM BEFORE SWTCH CALL**
        - but for context switch using scheduler to context switch from kernel scheduler to a new process?
            - only thing left to push is the state of the calee saved general registers.
            - so sched also pushes them on the stack and sealing the box.

- we pack all the states of the current process from which we are switching into a execution stack of the scheduler.
- stack of the scheduler contains all the information needed to restore the operation of the scheduler function.
- cpu->scheduler: contains the pointer to top of some other stack of the scheduler.

---
- why do we only push 4, ebp, ebx, esi and edi in the swtch: label. and not push eip as included in the struct context?
    - because the call to the swtch will push the EIP as the return address.

---
- scheduler is not a process. its a context of execution that we left in the kernel.
- EIP: 2479: is switchkvm(): inside scheduler(void).
    - This switches to the kernel address space. Because we maybe destroyed the user space, maybe this process is being destroyed or exited and we don't want to run on address space that will be deallocated.

- scheduler is per CPU data structure.
- cpus are on global array, and we use cpu ids to access each CPU.
---
- swtch(FROM, TO)
- Where does the proc->context point?
    - the kernel stack of the new process.

- context is always on the top of some stack.
    - ESP starts pointing to new stack top which is the context top and start popping.
    - you recursively return till you enter into user space by executing IRET instruction in the alltraps: label.

---
- DOUBT: missed switchuvm look
- i understand that it restores to p->pgdir ie. user page table and p->kstack ie. process kernel stack;
```c
      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
```