- first few points from slides.
- what is interrupt descriptor table:
    - OS maintains special table in memory.
    - it's entries are configured by the OS.
    - depending of interrupt it'll choose the entry in the descriptor entry table.
    - each entry will be the instruction pointer of the first instruction of the interrupt handler.
        - the second entry may choses a different code segment from the GDT to allow for changing privilege from user to kernel on system calls. 
            - without this user cannot change to kernel level.
                - case 1: users uses CALL address_in_kernel_space
                    - this faults during the page translation as user bit for kernel pages is not set.
                - case 2: long jump: ljump kernel_segment, label_in_kernel,
                    - this faults because GDT privilege level of kernel_segment is 0 and CPL is 3 and GDT does not allows this. 
- why does hardware chose new stack which is configured by the operating system for interrupt handlers beforehand.
    - for security reasons, we can't trust users stack values, like:
        - user reloaded the current stack pointer to point to address 0 or to some unmapped page or point to data structure in kernel address space.
        - as interrupt handler operates in privilege level 0, so using users stack would allow user to accidentally override the data structure in the kernel by whatever interrupt handler will put in that physical address belonging to kernel data structure.
- why do we need an interrupt handler stack in the first place.
    - because hardware needs to save five registers that are immediately overridden during the interrupt transition.
    - because at some point the interrupt handler need to return and give control back to the user process.
    - so we need the stack to save the user interrupt pointer so that we can set the current instruction pointer to the stack saved user interrupt pointer before interrupt handler and return control to user process.

- before interrupt handlers, 5 of the user register like Stack pointer and instruction pointer gets saved to registers that are not visible to use. Because the interrupt stack is not allocated yet. This is done in the 1st step of the exception handling ie. Write fault data to the exception registers.
- after these the same fetch decode register read and execute cycle continues for interrupt handler.
---
- interrupts: event from outside the process.
    - they are asynchronous notifications from devices like network cards, timers etc.
- exception: event from within the process.
    - they are synchronous
    - accessing unmapped page of memory or address.

- both interrupts and exceptions are handled by the same procedure of.
    - stop execution of current program
    - start execution of a handler
    - processor accesses the handler through and entry in the interrupt descriptor table(IDT).

- Debug instruction:
    - special 0xcc instruction inserted in binary as breakpoint -> 4 hardware debug breakpoints which are faster.
    - use special hardware mode for single stepping.

---
Case #1: interrupt path no change in privilege level:
- already in kernel
- no need to setup new interrupt stack cos can already operate on kernel interrupt stack.
- start interrupt steps taken
    - 1. Push the current contents of in this order.
        - ELAGS: saved because you have freedom of choice to disable flags in current cycle so you need to store the previous ELAGS value to restore correctly.
        - CS (code segment): may be even without chaning the privilege level you change code segment.
        - EIP (current instruction pointer)
    - push error code if appropriate.
    - load the segment selector in case the interrupt jumps between the code segment.
        - jumping happens when privilege level changes for interrupt handler.
    - If the call is through an interrupt gate, clear the IF flag in the EFLAGS register (disable further interrupts)
    - begin execution of handler.

- return from interrupt using IRET.
    - from slides
    - for interrupt handler to start from previous kernel process we don't restore SS and ESP that results in stack switch. 
---
Case #2: interrupt path with change in privilege level
- when we need to switch the code segment we go through IDT -> GDT
- here we can change the privilege level from 3 to 0, because the kernel configures the entry point.
    - like for syscall, kernel will check all the arguments.
    - since the stack is also changing hence we also push user SS and ESP into the kernel stack.
---
# TSS: task state segment
- why can't we use the user stack.
    - because user can be malicious, user can change ESP values and map to some unmapped page. the hardware will try to push IP, CS and error code on the stack but the page is not present, so hardware will enter RECURSIVELY into another fault which is a page fault, At this point this page fault will not be handled and will cause triple fault (which is not even delivered which means hardware already rebooted) and system reboots.
    - user can put the stack pointing to some part of the kernel, so then hardware will override kernel Data structures with IP, CS and error code.
    - so OS don't trust the value of ESP register, so it needs a fresh new stack.
- so TSS: task state segment is used.
    - pointed by TR: task register.
    - SS0: chooses data or stack segment for kernel
        - at user level the stack segment was at ring level 3.
        - so chose new stack segment with matching ring level 0.
    - ESP0:
        - Location of the stack in that segment 
        - pointer to new fresh stack as required above.

---
# IDTR register: maintained by hardware as part of CPU.
- IDT Base address: staring address(base) of the IDT table.
- IDT limit: the size of IDT table.
    - used to check the boundary of interrupts.

---
# IRQ
- fist 32 interrupts are reserved by intel, for like page fault, division by 0 etc.
    - interrupt 14 is always for the page fault.
- next 16 are assigned to IO devices IRQs.
    - IRQ0 is the 32nd(0 indexed) interrupt and it's a timer interrupt.
- and rest (208) are software configurable.
    - can be used for system calls.
        - interrupt 0x40 issues as system call in Xv6.
        - interrupt 0x80 issued as system call in linus
    - or setup for new device derivers.
---
# IDTR register
- IDS address is kept by hardware in IDTR register which is a part of CPU.
- IDTR register contains base of IDT and its size.
- each entry of IDT has a specific interrupt gate.
- interrupt gate has pointer to interrupt handler
    - it also had DPL which checks the invokers CPL against the interrupts DPL so that user is limited to calling system call interrupt only.

---
# Interrupt gate
- **offset 31...16 - offset 15...0: pointer to the interrupt handler.**
---
- what are privilege levels and why do we need privilege levels?
 - user process should only be constrained to it's process space.
 - user process should only be allowed to execute instructions that don't violate the integrity of process container.
    - user process not allowed to update/write to cr3, lgdt (GDT), IDTR (IDT), can't switch from ring 3 to ring 0 without a system call.
- privilege rings enforces the integrity of the process.

- Every entry in the global descriptor table, each segment will have a privilege 2 bit field ranging from 0-3.
- for example: when doing long jump
```x86asm
ljump 1<<3, start32 ; the 1<<3 points to a entry in the GDT for code segment. 
```
- CPU will check the code segments DPL(descriptor privilege level) and stop invalid access like from ring level 3 accessing ring level 0.
- current privilege level saved in %cs register, loaded from GDT during jumping translation.
- iret: return to user only possible from current privilege level of ring level 0.

- limited ways to change code segment.
    - ljump
    - iret

- **EXAM QUESTIONS: why do we have GDT for each CPU in place of having a one GDT for entire system.**
    - because the GDT also contains the TSS which points to per process kernel stack and hence can't be shared. 
---
- transition from lower privilege level like ring 0 to higher privilege level like ring 3 is allowed but the other way around is not allowed.
    - for example: we cannot use IRET to return from privilege level 3 to privilege level 0.

- we will prepare user stack and call iret to execute the first user process.

---
- if GDT are configured for both user and kernel with base 0 and size to entire address space, then how is kernel protected?
    - using the user accessible bit, present in each entry in the page table.
    - if set then privilege level 3 can access else only privilege level 0,1 and 2 can access that page.
    - kernel protected with mapping kernel code with user bit clear in the page table entries.

---
- Disabling interrupts: only possible in ring level 0.
    - this is done by setting FL_IF to 0 in EFLAGS, This is the 10th least significant bit in the EFLAGS.
    - this will only disable asynchronous interrupts except for NMI (Non Maskable Interrupts).
    - if user process given process then he can go into infinite loops and run forever or execute the value of pi without timer interrupt.
    - OS will never get the timer interrupt delivered and won't be able to make a switch to another process to ensure fair allocation of CPU resources.
    - in code subsequent interrupts are also set in the SETGATE macro for the IDT table.
        - except syscall all interrupts are disabled.
        - for syscall, we might encounter further interrupts so DO NOT DISABLE INTERRUPTS.
---
# IDT DPL
- why do we also have DPL in each entry in the IDT if we already have it in GDT entries?
    - the reason is to protect interrupt themselves.
    - user code can't invoke page fault interrupt(INT 14).
        - user trying to call any interrupt besides system call will get **GENERAL PROTECTION FAULT** exception which is **interrupt vector 13**. 
    - normally this DPL in IDT is set to zero, so user can't can't call the above interrupt 14.
    - other interrupts like division by zero, async interrupts are done by hardware with Privilege level 0 so they still pass through.

- how do we stop user code from calling INT 14?
    - using the DPL flag in the IDT entry.
---
# NMI: Non-maskable interrupts
- it's the only asynchronous interrupt not disabled by clearing the IF flag is the EFLAGS register.
- it is used for very critical, low-level condition that require immediate intervention by the Operating System.
---
- IDT entries:
    - CS: code segment and pointer to the interrupt handler.
    - the code segment points to the entry in the GDT with the DPL.
- direct use of CALL or long jump (ljump) to the kernel space by the user process is blocked during the page translation pass by matching the CPL and the USER ACCESSABLE BIT in the page.
- but transition through an interrupts to kernel space is allowed because the kernel manages this translation.
---
- trap frame: is all the bunch of register(including general and segment registers) saved in stack in form of c struct.
---
# first thing to do in interrupts kernel code
    - need to save general registers of the current process.
    - hardware do not save the general registers.
    - because even executing instruction might change those instructions.
---
- why do we have vector64, vector14 etc why can't we have a generic interrupt vector.
    - because we want to distinguish between different interrupt calls
    - pushing the interrupt number
- why can't we save the distinguishing interrupt number in some register?
    - because this is only possible from user calls
    - in case of hardware interrupts like division by zero we do not have that power because in that case we immediately land in the interrupt handler.

---
- what is proc->sz
    - it's the pointer to the inclusive end of user address space.
    - heap growth moves the proc-sz.
    - it is inclusive of the user address space.

- why is addr+4 > proc->sz check used
    - maybe user address space ends in between like (2/3rd or 1/2) of the integer.
    - maybe not entire user fits into the user address space.
    - maybe user tries to fools us and if kernel tries to access it'll end with page fault which will panic the kernel and make it crash.

- why not addr+4 >= proc->sz:
    - because proc-sz is inclusive user valid user space, and if address+4 == proc->sz then it's a valid integer and we can't return -1.


- why can't we just check addr+4 > proc->sz and need another addr >= proc->sz?
    - because of possible overflow. see the code block below.
    - because user can set addr to be 0x7fffffff so when addr+4 will round to -ve value (of -2147483645) and will fail to detect the violation.

```c
    int a = 0x7fffffff;
    if (a+1 > 20) {
        printf("True a: %d a+1: %d\n", a, a+1);
    } else {
        // this is logged as:
        // False got a: 2147483647 a+4: -2147483645
        printf("False got a: %d a+1: %d\n", a, a+10);
    }
```

---
- what happens when printf("hello world is called");
    - below is the chain of function.
-> printf->format characters and call system call write(fd, buf, 1) -> INT 0x40 (system call)
- then below is the flow for any interrupt and system call.
-> vector 64 -> alltraps->trap->syscall->sys_write->file_write->writei->devsw[].write->consolewrite->consoleputc->uartputc;

---
- there cannot be three level of nesting.
    - 1st level system call
    - 2nd level for enabled interrupts like device wake up, division by zero etc.
        - as this disables further interrupts we can't have any more interrupts unless it's a Non Maskable interrupts which Xv6 does not handle.