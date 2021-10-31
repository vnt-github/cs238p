# SEGMENTATION AND PAGING
- cpu start in read mode.
- bootloaders starts in read mode for historical reasons.

- BIOS <- reads the boot sector <- which the compiler sells <- loaded at 0x7c00.
- read OS from disk, setups segmentation and paging, then everything jumps to OS kernel and OS starts running.
- how the environment is setup.
- how all the execution protection mechanism by the OS, by the loader, by the BIOS.
- that's why we need to learn about segmentation and paging.
---
# SEGMENTATION
- why we need segmentation?
    - how run two programs one memory?
        - Relocation: load two programs in different non-overlapping spaces in memory.
            - wrong about this solution. lost a couple of properties.
                - we lost the ISOLATION: if we suddenly make a mistake and access some other processes memory then that is bad.
                - isolation can be implemented in software, software forced isolation (SFI): WASM and google NaCL uses this.
                    - compiler adds tiny add before every MOV and JUMP instruction to check is that addresses involved are valid and are in range of allowed current process address space.
                
- what are we aiming for?
    - illusion of private address, so we don't have to reallocated and each program can start at it's private address space 0x00 (called logical address space).

- hardware adds base values to process memory, the base is configured by OS. chose bases st they don't overlap. There is size for each segment and accessing beyond it will terminate the process.

- where do these base addresses come from?
    - Hardware maintains and in-memory data structure to track it and a special register to keep and index into that table.
    - in-memory because we might have 100s of segments.
    - table = Global Descriptor table (GDP): table contains a base and a limit.
    - special register = selector register
    - using this register for each process there is an entry in the GDP that transforms this Logical address space to linear address space by adding base.
    - two different process can have same logical address with different value byt with different linear address.
    - Hardware also implements the check on the limit of the segment for each process to enforce isolation.
    - GDT register points to the physical address where GDT starts.
    - Each process have private GDT: on context switch OS may change the GDT register table register to point to different GDT table OR it can change the index of the code segment.
    - GDT can accidentally overlap with other segments of processes or kernel so we need to be careful how it's done.
    - loading GDT register value is privileged so normal processes can't change it.

- default value of the ds(data segment), ss(stack segment), cs(code segment) is assumed to be of current process segment values.
- only if we jump to different segment then only we use it.
---
- what are limitation os segmentation?
    - we need segments to be continuous in the physical memory.
    - fragmentation: left out chunks may be sufficient to support another process but these chunks are not continuous so are wasted.
        - can be solved by just moving bases in the GDP, essentially moving the process segments.
        - but programs are large, so it's cumbersome, we have to move the entire process segment.
        - there might be no space for swapped out process to come in.
    - can't extend the physical memory by malloc->sbrk if extending memory clashes with another segment.
        - OS can move the other process segment to disk, but the entire process segment and then reload it. so it gets cumbersome.
---
# PAGING
- page sizes: 4096 bytes = 4KB/4MB on 32 bit intel machine, page boundaries are in multiple of 4096.

- paging is a mechanism that splits processes memory into small chunks. we also splits physical memory into equal chunks. and hardware implemented page table is used to translated from virtual address to physical address.
- virtual address uses log2(4096) = 12 bits for offset as we need 12 bits to represent 4096 bits.
- rest of 20 bits left are used for virtual page number, virtual page number are translated to physical page number.
- each process has it's own page table.
---
# PAGE TABLE IMPLEMENTATION
- not hash map: its a little bit involved, we don't have predictable time, some page table are immediate, others will need walk. so no.
- array: better than hash table, keys don't repeat
    - in intel uses register CR3 (control register three), contains the physical page number of the page that contains the array.
    - limitations:
        - even if majority of entries in the array is not used we still allocated size for full array. 
        - large memory footprint = (2^20 entries) * (4 Bytes for each entry)  = 1048576 * 4 Bytes = 4194304 Bytes = 4.194304 MBytes.
- linked list: O(n) lookup time. so not ideal.
- extensible arrays with size in beginning: if for sudden spike workload the size extends to 500,000 then gets don't uses it later then we again have holes.
---
- values in the tables are 4 bytes = 32 bites = 20 bits for page number 12 bits for flags.
- so we use an array of arrays for each chunk.
    - you only allocated table as required.
    - size is 4KB*4 = 16 KB. if we use 4 chunks.
- 1st layer table of regions.
- root 1 page 4 kB = contains 1024 entries
    - contains physical page number of the next level of translation via page table.
- OS maintains CR3 (control register three) to point to physical address corresponding to a particular virtual address.
- each page table is of size 4096 byte = 4 KB, and for each level we add those size.
---
- virtual address space: 1048575, so 20 bits for virtual space number.
```
move EAX, (%EBX)
EAX = 0
EBX = 20983809

>>> x = 20983809
>>> int(bin(x)[:-12], 2)
5123
```
so in above it points to 5123 page number
---
- In hardware the CR3 = 0. os sets it it can be anything.
    - contains value of the page number of the physical page that contains the root table.
- page tables is a radix tree and not a bit tree.
- flags
    A: weather the page was accessed. To allow OS to track it, so OS can micromanage pages.
    D: dirty, there was a write in this page. in data centres when migrating only copy the dirty pages.
---
- what if we had to allocate whole address space then what is the size of radix tree.
    - 1024 pages required each size 4KB so 4MB for pages + extra 4 Bytes for root page table so 4MB + 4 KB.
---
# ISOLATION in PAGE TABLE
- no conflicting page table between processes.
- physical mapping for P1 never overlaps mapping with P2.
---
- 32 bit x86 have two page sizes.
    - 4KB
    - 4MB: larger page table used to decrease the cost of page table lookup.
        - to lookup a trip to memory 200 cycles which is saved here.
        - when running a data centre, if you are operating truly random pages then cost of data access doubles.
        
        - bits 0-21 = 22 bits are offset, bits 22-31 = 10 bits is directory.
        - read only 18 bits of page directory.

---
- 64 bit mode
    - 9bits * 4 + 12 bits
    - 2^9 = 512 entries, each entry 8 bytes.
---
- during translation the values from the PDT ( page directory table ) are only read till 20 most significant bits.
- because we only 20 bits to map the physical address.
- because it's aligned by 4KB pages.

---
PPN: physical page number.
PDE: Pate directory Entry.