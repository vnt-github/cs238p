- first few points from slides.
- why does hardware chose new stack which is configured by the operating system for interrupt handlers beforehand.
    - for security reasons, we can't trust users stack values, like:
        - user reloaded the current stack pointer to point to address 0 or to some unmapped page or point to data structure in kernel address space.
        - as interrupt handler operates in privilege level 0, so using users stack would allow user to accidentally override the data structure in the kernel by whatever interrupt handler will put in that physical address belonging to kernel data structure.

- why do we need an interrupt handler stack in the first place.
    - because at some point the interrupt handler need to return and give control back to the user process.
    - so we need the stack to save the user interrupt pointer so that we can set the current instruction pointer to the stack saved user interrupt pointer before interrupt handler and return control to user process.

- before interrupt handlers 5 of the user register like Stack pointer and instruction pointer gets saved to registers that are not visible to use. Because the interrupt stack is not allocated yet. This is done in the 1st step of the exception handling ie. Write fault data to the exception registers.
- after these the same fetch decode register read and execute cycle continues for interrupt handler.
---
- interrupts: unexpected event from outside the process.
    - they are asynchronous notifications from devices like network cards, timers etc.
- exception: unexpected event from within the processor.
    - they are synchronous
    - accessing unmapped page of memory or address.

- both interrupts and exceptions are handled by the same procedure of.
    - stop execution of current program
    - start execution of a handler
    - processor accesses the handler through and entry in the interrupt descriptor table(IDT).

- Debug instruction:
    - special 0xcc instruction -> 4 hardware debug break points.
    - use special hardware mode of single stepping.

---
Case #1: interrupt path no change in privilege level:
- already in kernel
- no need to setup new interrupt stack cos can already operate on kernel interrupt stack.
- start interrupt steps taken
    - 1. Push the current contents of in this order.
        - ELAGS: saved because you have freedom of choice to disable flags in current cycle so you need to store the previous ELAGS value to restore correctly.
        - CS (code segment): may be you change code segment.
        - EIP (current instruction pointer)
    - push error code if appropriate.
    - load the segment selector in case the interrupt jumps between the code segment.
        - jumping happens when privilege level changes for interrupt handler.
    - If the call is through an interrupt gate, clear the IF flag in the EFLAGS register (disable further interrupts)
    - begin execution of handler.

- return from interrupt using IRET.
    - from slides
    - for interrupt handler to start from previous kernel we don't restore SS and ESP that results in stack switch. 
---
Case #2: interrupt path with change in privilege level
- when we need to switch the code segment we go through IDT -> GDT
- here we can change the privilege level from 3 to 0, because the kernel configures the entry point.
    - like for syscall, kernel will check all the arguments.
    - steps from slides.

---
# TSS: task state segment
- why can't we use the user stack.
    - because user can be malicious, user can change ESP values and map to some unmapped page. the hardware will try to push IP, CS and error code on the stack but the page is not present, so hardware will enter RECURSIVELY into another fault which is a page fault, At this point this page fault will not be handled and will cause triple fault (which is not even delivered which means hardware already rebooted) and system reboots.
    - user can put the stack pointing to some part of the kernel, so then hardware will override kernel Data structures with IP, CS and error code.
    - so OS don't trust the value of ESP register, so it needs a fresh new stack.
- so TSS: task state segment is used.
    - pointed by TR: task register.
    - SS0: stack data or stack segment for kernel
        - at user level the stack segment was at ring level 3.
        - so chose new stack segment with matching ring level 0.
    - ESP0:
        - Location of the stack in that segment 
        - pointer to new fresh stack as required above.

---
# IDT DPL
- why do we also have DPL in each entry in the IDT if we already have it in GDT entries?
    - the reason is to protect interrupt themselves.
    - user code can't invoke page fault interrupt(INT 14).
    - normally this DPL in IDT is set to zero, so user can't can't call the above interrupt 14.
    - other interrupts like division by zero, async interrupts are done by hardware with Privilege level 0 so they still pass through.

---
# IRQ
- fist 32 interrupts are reserved by intel, for like page fault, division by 0 etc.
- next 16 are assigned to IO devices IRQs.
    - IRQ0 is the 32nd interrupt and it's a timer interrupt.
- and rest (208) are software configurable.
    - can be used for system calls.
    - or setup for new device derivers.
---
# IDTR register
- IDS address is kept by hardware in IDTR register which is a part of CPU.
- IDTR register contains base of IDT and its size.
- each entry of IDT has a specific interrupt gate.
- interrupt gate has pointer to interrupt handler
    - it also had DPL.
---
- what are privilege levels and why do we need privilege levels?
 - user process should only be constrained to it's process space.
 - user process should only be allowed to execute instructions that don't violate the integrity of process container.
    - user process not allowed to update/write to cr3, lgdt (GDT), IDTR (IDT), can't switch from ring 3 to ring 0 without a system call.

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


- EXAM QUESTIONS: why do we have GDT for each CPU in place of having a one GDT for entire system.
---
- if GDT are configured for both user and kernel with base 0 and size to entire address space, then how is kernel protected?
    - using the user accessible bit, present in each entry in the page table.
    - if set then privilege level 3 can access else only privilege level 0,1 and 2 can access that page.
    - kernel protected with mapping kernel code with user bit clear in the page table entries.

---
- Disabling interrupts: only possible in ring level 0.

---
- trap frame: is all the bunch of register(including general and segment registers) saved in stack in form of c struct.
---
- why do we have vector64, vector14 etc why can't we have a generic interrupt vector.
    - because we want to distinguish between different interrupt calls
    - pushing the interrupt number
- why can't we save the distinguishing interrupt number in some register?
    - because this is only possible from user calls
    - in case of hardware interrupts like division by zero we do not have that power.

- how do we stop user code from calling INT 14?
    - using the DPL flag in the IDT entry.
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
    - because of possible overflow.
    - because user can set addr to be 0xffffffff so when addr+4 will round to +ve small value and will fail to detect the violation.

---
- what happens when printf("hello world is called");
    - below is the chain of function.
printf->format characters and call system call write(fd, buf, 1) -> INT 0x40 (system call)
-> vector 64 -> alltraps->trap->syscall->sys_write->file_write->writei->devsw[].write->consolewrite->consoleputc->uartputc;