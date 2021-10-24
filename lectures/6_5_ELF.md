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
- A.out didn't allow handling all other files formals for shared libraries(.so), static libraries(.a), executables etc.

- program header table
    - segment header table
    - used by loaders.
    - think contains info required for loading, read from memory and run.

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