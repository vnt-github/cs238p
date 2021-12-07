- what parts do we need to run.
    - data section
        - global initialized variables
    - bss: **Block Starting Symbol**
        - global uninitialized variables
    - text
        - instructions
    - stack
        - function calls
        - local variables
        - register overflows
        - it's started even before main is called as main itself accepts arguments.
    - heap
        - dynamic variables.

- where is global initialized variables saved: for eg int a = 5;
    - data section
    - it is loaded from disk

- where is global uninitialized(or initialized to zero) variables saved: for eg int b = 0;
    - bss section
    - it not not kept on disk, it's kept in header of the elf file there is a size information about the variables, with implied 0 values.
    - for example: just set 4kb to zero and memset to zero.

- where is text/code section kept
    - it's kept in disk.

- where are dynamic variables kept.
    - heap.
- where are functions and local variables kept.
    - stack.

- how to allocate dynamic memory from heap
    - malloc is a library that maintains metadata about the heap.
    - it tracks regions that are free and occupied.
    - when allocating memory it scans the available space and allocates the next bigger free memory block available.
    - this metadata may be maintained as linked list of element elements.
    - there is a list for each size ie. 16 bit, 32 bit, 64 bit.
    - it allocates pages.

- each thread needs a heap and stack per thread.

- the virtual memory for 32 bit architecture is 4 GB.
- DLL
    - dynamically loaded library are loaded in the heap.
    - library have it's own text and data section.
    - libraries are passive so it does not have a stack section.
    - if later library creates a thread of execution then it has stack.


- kernel is mapped in the top half space of the virtual memory.
    - xv86 uses strict split between kernel and user memory.
    - kernel also has it's own text and data section.
    - each process also has a process stack on which is executes.
    - the reason for separate kernel memory is for performance.

- kernel memory in the virtual memory is the same physical kernel memory, mapped multiple times for each process.
- page table looks for ring bit before allowing accessing in a memory, so if a ring 3 (user) tries to access a virtual memory that has ring 1 bit set (kernel) that the page table throws a page fault. 
- so at the page table directory level the users's invalid kernel memory access is blocked.

---
# How are these regions loaded.
-  text/code section
    - compiler and linker
- data section
    - compiler and linker
- heap
    - OS kernel
- stack
    - OS kernel


objdump -sd a.out
s: show all sections.
d: disassemble the sections.

- because of stack alignment gcc something will allocate 16 bytes instead of required 8 bytes for 2 local variables.
- xchg %ax, %ax 2 byte alignment
- nop 1 byte code alignment
---
- object files:
    - elf file
        - header section: where is data, code sections.
- normal libraries:
    - static libraries bundled in ar format.
- linking: merging object files combining symbols that are in referenced from other files. This is done by linker by resolving the missing references.
---
# what needs to be done to merge (or move) code in memory
- no change:
    - no change in relative addresses.
    - no change in constant or register operations
    
- change:
    - need to change absolute addresses.
    - need to change global variable instructions like.
        - mov eax, [var] where [var] is address of global variable that is unresolved.
    - call to label b where label b is not in current data section then we need to resolve it/ie. it is undefined yet.

    - **SO ADDRESSES OF GLOBAL VARIABLES AND FUNCTION WHICH ARE NOT KNOWN RELATIVE TO THE CURRENT CODE** needs to be changed.

---
- x86 is a little endian system.
    - highest address are placed in furthest away positions from the opcode.
        - for example: mov EAX, a; where a is at 0x1234
        - instruction: A1 34 12 00 00; where A1 is the op code.

- unresolved addresses
    - for unresolved addresses of global variables, functions, the compiler puts 0s.

- There is a special section(GOT-Global Offset Table) in the .elf file that tracks that a is defined and whenever a in data section changes we need to patch wherever it is used.
- It also tracks b that is undefined and that puts the value of b address when resolved.

---
- linker also can tell to re allocate the binary to start running at specific address like 0x7cc.
- linking by linker is a separate part, sometimes compilers come with linker.

---
- why we need to define signature of the functions.
    - we will break calling convention otherwise of passing arguments via stack.
    - because we want to verify the type and numbers of arguments and return type.
    - if we infer from function call then we let go of type checking and we also wont know what to expect in return value.
---
- for global function that are defined elsewhere
    - compiler puts -ve address(e8 f3 ff ff ff) to call system call.
    - this points to previous instruction.
    - and records that it's not found yet.
- for global variables defined in current file.
    - compiler pushes into data section and marks it in the disassembled object file.
    - and records the fact that its a global variable that needs to be patched on relocation.
---