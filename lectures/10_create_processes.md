- exec("/bin/ls", argv);
    - argv last element 0 by convention.
    - argv 1st element is the program itself
        - for busy box like systems.

- before anything we need to map pages for the new process.
    - with the kernel space and user space.

- why can't we use the existing page table for new exec().
    - because exec might fail halfway or crash.
        - because we may run out of memory.
    - in that case we need to return an error to process that called exec.
    - but if we destroy the old address space then there is no place to return the error because it's already gone. 


- exec explanation assumes the very first init process is already created and running.
    - the existing process clones itself with fork() and then for new process calls exec().

- **EXAM QUESTION: how is fork implemented: read about it.**
    - it's just copy from memory to other place in memory.
---
- allocuvm(): allocate enough pages in the user part of the address space.
    - mapped pages will be located in the range of available physical memory, and we mapped all these pages into the list of free pages for kalloc() with 2 GB shift.

---
# loaduvm:
-  where does char *addr come from?
    - the call to loaduvm sets it to (char*)ph.vaddr it's from ELF file.

- why can't we just simply use readi(ip, addr, offset+i,n ) in line 1933.
    - currently CR3 is pointing to old page table that has something in the addr.
    - loading into address space controlled by old page table may override by new process.

- why do we have to use walkpgdir to get the page table entry and get pa in place of addr. why is pa != addr?
    - we have one process invoking exec.
    - exec goes to kernel and we are creating a new address space(page table)
    - we are loading into this address space.
    - which page table is currently active? which one has the address addr translated by the hardware?
        - old page table (page table of the process calling exec like init for eg.)
    - because we don't want to use the virtual address from the old page table in the new page table created by the exec.
    - if exec worked correctly then we'll eventually cr3 to point to new page table directory, but the cr3 still points to old page table
    - so we need to walkpgdir to get the physical address corresponding to new vir virtual address.
    - how can you now access this new virtual address in our old page table from the new physical address you got by walkpgdir?
        - P2V(pa):  2GB + pa (physical address).

- what could go wrong if we did'nt do walkpgdir?
    - the readi will use virtual address new to put data, and hardware will use old physical address space. this will override the exec calling functions address, and that'll destroy the old address space.
    - somewhere in between the exec if it fails then we need to return and error to the function calling exec, and if we override then we don't have a function to return exec failed error to return to.

---
# Double mapping
- each new page is mapped twice once for user and once for kernel.
- the moment you map a page in user part of address space will get mapped to kernel part of the address space.
    - the allocuvm mapped the user address space from ph.vaddr in the user address space.
    - the loaduvm loaded the content and mapped to the kernel space using readi(ip, P2V(pa), offset+i, n) != n).
        - this is still using the old pages of the process that called exec. 

---
# Allocate program stack
- we need to allocate the guard space to catch the stack overflow
- 4096 bytes from one page.
- guard page is not bulletproof.
    - you can allocated more that 4096 like 8000 bytes then it'll happily override the process data section until you again hit the guard page.
    - it is a best effort service.

- sz parameter that track the size of the address space.
- how to make guard page throw exception on user access?
    - remove the user bit on that page?
- why do we allocate the guard page altogether and just don't set the present bit to 1. that means that the page is not mapped at all.
    - remember we were using the proc->sz boundary check.
    - what if user passes us a pointer that is inside the guard page.
        - if page is not present then user fools us. it passes the proc->sz check and kernel tries to access this page which is not set present and faults with page fault and crashes.
    - here xv6 sets the present bit to 1 and allows the user to write data in the guard page, without faulting.

- **what changes need to be done to not waste extra guard page space?**
    - inside the process keep track of proc->guard page that keeps the position of guard page and check it during system calls validation that user does not access any address inside the proc->guard page, and kill the user if that the case.
    - check if page fault is for the the address between the proc->guard and proc->guard + PGSIZE then just kill the user.
---
- why pass program name in 0th argv
    - you can implement different behavior based on what the arv[0] is. 
    - some programs like busybox do it.
        - it implements everything (like wc, grep, ls etc) with a single binary.

---
- arguments to main stack
- foo hello world


|
|hello                                                                     | <- initial ESP
|world                                                                     |
|0|ptr to hello |  ptr to world| ptr to argv | argc:2 |dummy return address| <- argv array <- ESP

- have to put 4 bytes with some dummy return address.
- may be put address above the 2GB range and be sure that it's user unmapped.
- so in xv6 we use exit() instead of return here because of this.
---
- deallocate old page table.
    - walk the old page table entry by entry and deallocate non empty entries.
        - because we don't know which physical address are pointed by those entires.
    - Deallocate pages that contain Level 2 of the page-table
    - free them back to memory allocator.
---
- why use deallocuvm(pde_t *pgdir, unint oldsz, unint newsz) uses oldsz and newsz instead of using 0 to proc->sz()?
    - because this functions can be used to deallocate in other scenarios.
---
- userinit()
    - p = allocproc(); // this allocates a new process data structure.
    - Configure trap frame for "iret".
    - why do we need a trap frame in order to run?
        - we need to pretend the kernel stack of the process has trap frame present.
        - we need to pretend the very first process was already running in the past.
        - for kernel the only legit situation is when process is parked in a kernel. that it was preempted by and interrupt. it already had a user stack and then pretend some interrupt in past preempted the process and created the trap frame in the kernel stack.
        - so that the process can be exited back to the user level for "iret".

    - **EXAM QUESTION: p−>tf−>eflags = FL_IF**
        - FL_IF=1 that means that the interrupts are enabled.
        - we initialize the interrupt flags as enabled so when we exit to user level the interrupts will actually be re-enabled.
        - otherwise when you exit into user it'll never get and interrupt back.

---
- we have P1 shell and creating P2 wc with exec().
    - we use allocuvm to create a new page table space for P2.
    - we use loaduvm to load the P2 processes's (wc's) text and data in to the pages from the disk.
    - we configure the user stack for arguments for P2s wc.
    - we configure the kernel stack with the trap frame and CS: code segment with privilege level 3, and EIP: with pointer to wc.
    - when iret will be called the kernel stack will be popped. CS will enable privilege level 3 for user and EIP for the correct start address of program.
    - we then make this P2 new process's state == RUNNABLE, which puts P2 into the Queue of ready to run processes.
    - from this queue the schedular will pick the process and context switch.
    - during context switch P2 will first run in kernel space then eventually exit in user space.
    - then user execution will start from main function in the user space.

- inituser
    - setups the console.
        - this is a special file in a file system which is a device with a minor and a major number.
        - When you write in this files system you instead communicate to write on this device.
        - These devices are present in the /dev/sda
        - if initialing for the first time then create the console device with mknod("console", 1, 1);
    - you cannot exit from sh in init.

---
- code segments for kernel and user which all span the all 0-4GB are required by the hardware to differentiate between the privilege level 3 of user segment and privilege level 0 of the kernel.
    - segments are the only way to encode the privilege levels.