- static linking:
    - all the symbols will be already be resolved with the correct addresses before putting in the binary executable file.
    - all the data and the text segments of the different programs are merged by the linker during compile time in the binary executable file.
    - debug information and relocation symbol are also placed in the same binary executable.
- dynamic linking:
    - the object file still be merged and dynamic linking will produce a much smaller binary executable elf file as it only merge the data section and text section of every other .c file except **system libraries**.
    - when you invoke the exec system call the OS will create an address space
    - for all unresolved symbols from the **static libraries/dynamically linked or loaded library** like printf, OS will load those libraries into memory once but will map multiple times.
    - now OS will resolve the symbols. two main strategies
        - at the moment of loading, not how it's done in modern OS.
        - libraries use Position Independent Code (PIC) and use PLT (procedure linkage table) and GOT (Global offset table).
    
- during linking we can specify that load address where text sections is supposed to be loaded in memory for it to run. 

- ELF Files have following section.
    - data
    - text
    - relocation symbol
    - debug info.: so that on crash the debug info is not overloaded and is maintained.
    - static linking ELF have library dynamic does not.

- loaders is injected into the memory of the process.
- A.OUT is the ELF
    int a_magic; // magic number
        - confirm OS that it's an a.out file
- A.out have a fixed order of all the parts.
---
- the addresses are the ELF can be linear, virtual or physical.
- they need to be aligned with what is the current state of segmentation and pagination in the kernel is.
- depending on how the OS configures the executable mode it can be any type of address.
- bt 99% of the time it's virtual address.
---
- A.out didn't allow handling all other files formals for shared libraries(.so), static libraries(.a), executables etc.
---
- linker: Merging (linking) code and data sections together, moves and relocates to the desired address entry point.
- loader: Take all “segments” marked as LOAD and load them into memory and start running it.

---
- program header table
    - segment header table
    - used by loaders.
    - contains info required for loading, read from memory and run.

- section header table ( can be place at end but not required )
    - pointers to sections
    - used by linker.

- ELF file format is used by both linker and loader.
- 0x7c00 is convention to load boot-loader by machines.
- entry point has to be close to zero address.

- .init is used to initialize the process.
- GOT is actually loaded as well.

---
- eh_frame: elf headers frame.