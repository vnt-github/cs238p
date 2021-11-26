# what happens when we turn on the power?
- Intel SGX(Software Guard Extensions)
    - its a secure enclave technology
    - shields application from un-trusted OS kernel.
- a sequence of software pieces initialized the platform.
    - management engine: secure technology intel boots before everything else.
    - micro code: pieces of softwares CPU runs to execute complicated instructions.
    - BIOS
    - bootloaders
    - kernel

- hypervisor runs under OS in cloud, hence ring -1 privilege.
---
- secure boot is an idea of chain of trust
- started from some well known state, then measure all other stages like. intel management engine etc.
- secure boot monitors/manages it and takes action if something is wrong.
---
- when press power button.
    - ME starts running
    - reads Firmware OS (probably some version of Minix) from Flash read only memory.
    - starts on of the processor ie. Bootstrap processor (BSP).
---
kernel physical address map from
Physical: 1MB to Virtual: 2GB+1MB.

why 1MB and not map to 2GB and use 1to1 mapping.
- because we might not have 2GB physical memory.
- the QEMU assumes 512MB physical memory size.

- The reason it places the kernel at 0x100000 rather than
0x0 is because the address range 0xa0000:0x100000 contains I/O devices

- the macro V2P_WO (0213) subtracts KERNBASE in order to find the physical address
---
- where is entrypgdir memory comes from?
    - it is inside the kernel data section.
- how big is the first page table pointed by entrypgdir.
    - 1024*4 Bytes = 4096 Bytes
    - 1024 entries and each entry mapping 4 MB regions is of 4 Bytes size.
---
power on softwares chain
- intel management engine
- bios
    - starts system management mode: runs forever, even after boot sequence.
    - initializes firmware and all other devices.
- boot-loader.

---
- why not data section loaded in boot loader.
    - because when we reserve space in text/code section in assembly.
    - we can use this for data section later.
---
- protected mode is enabled when we enable segmentation.

---
- scratch space is added at 0x10000 = 65536.
- so we read 4096 bytes of elf header from dist to this scratch space.
- so that it does not get overridden by kernel binary.
---
- kernel is mapped twice in virtual memory and linear memory.
- **why did we map kernel twice?**
---
```x86asm
1160 # Jump to main(), and switch to executing at
1161 # high addresses. The indirect call is
 needed because
1162 # the assembler produces a PC−relative 
 instruction
1163 # for a direct jump.
1164 mov $main, %eax
1165 jmp *%eax
1166
Jump to main()
xv6/entry.S [kernel]

```

- why direction jump? why can't we just say call main
- cos main is not be a static function and call function is a relative jump and main cannot be called using a relative jump.
- main itself is a symbol in 2GB+1MB+Delta in kernel, so we jump to this address.
- we did not create a return address on the stack, this is breaking the calling convention.
    - we don't care because we are not passing any arguments ie. int main(void)
    - if we had arguments then we should push some dummy return address, to make sure arguments are properly aligned on the stack wrt to the stack pointer after the invocation.
---
after main.c what's next
- setup page tables.
- interrupt handlers/system calls.
- exec().
- processes().
- shell/gui().
---
- what is a process
    - is a collection of resources
        - memory: eg. text, stack, heap
        - in-kernel state: eg. open file descriptors, network sockets, pipes.
    
    - processes are isolated from each other.
        - processes don't trust each other.
        - individual users, some privileged.
        - can't change kernel ( to affect other processes ).

- why not to map kernel in the process virtual address space:
    - not to map the kernel to above addresses is to prevent MELTDOWN attacks.
---
- why paging?
    - because user processes are not trustworthy and they need to be isolated with boundaries.
    - we can use segmentation for these boundaries but paging is much easier and flexible to work with.
---
# memory after boot
- xv6 never uses memory above  (0xe0000000)234MB (PHYSTOP)
    - this is done to avoid complexity of OS to dynamically querying ACPI to the determine physical memory available dynamically.
- I/O space
    - 0-0x100000 and 0xfe000000-0xffffffff
    - used to communicated with IO devices.
    - the instruction goes through PCI devices rather than memory controller.
    - kernel maps the virtual addresses corresponding the these physical addresses so that they are usable in system with paging enabled.
---
- create the kernel address space.
    - create kernel memory allocator
    - allocate memory for page tables
        - page table directory and page table.
    - unlike the previous memory for pages of 4KB from kernel data space we need an allocator because these pages for user processes needs to by dynamic.
- why not to continue with 4MB pages.
    - cos the size of programs will be typically in kbs so 4MB would waste more space.
- a better way is to use a hybrid approach, ie 4MB for kernel and 4KB page sizes for user processes.
---
- xv6 allocator allocator supports
    - page tables.
    - stacks
- NOT data structures like malloc.
---
- how does memory allocator looks like
    - linked list.

```c
struct run {
    struct run *next;
}
char * kalloc(void) {
    struct run *r;
    r = kmem.freelist;
    if (r)
        kmem.freelist = r->next;
    return (char*)r;
}
void kfree(char *v) {
    struct run *r;
    r = (struct run*)v;
    r->next = kmem.freelist;
    kmem.freelist = r;
}
```
- **what is the con with this arrangement for allocator?**

- where does this memory comes from
    - this linked list does not live within kernel, but it lives within page table itself.
    - this memory has to be mapped by page table else we'll get page fault when accessing them.

- it uses the free memory after the kernel memory.
    - create end from 0x801126fc - till the end of 2GB+4MB.

```c
kinit1(end, P2v(4*1024*1024))

kinit1(void *vstart, void *vend) {
    freerange(vstart, vend);
}

freerange(void *vstart, void *vend) {
    char *p;
    // python3: pdroundup = lambda sz: (((sz)+4096-1) & ~(4096-1))
    p = (char*)PGROUNDUP((uint)vstart);
    for(;p+PGSIZE<= (char*)vend; p+=PGSIZE)
        kfree(p);
}
```
what is this end?
- this marks the end of virtual space where kernel ends.
- first address after kernel loaded from ELF file
- extern char end[]; provided by linker.
---
# kernel page table (4KB page table)
```c
kvmalloc(); // allocates first kernel page table memory.
// this allocates physical memory.

kvmalloc(void) {
    kpgdir = setupkvm();
    // switch pages from entrypgdir to kpgdir by setting cr3 register.
    switchkvm();
}

pde_t* setupkvm(void)
{
    pde_t *pgdir;
    struct kmap *k;

    if((pgdir = (pde_t*)kalloc()) == 0)
        return 0;
    memset(pgdir, 0, PGSIZE);

    ...

    for(k = kmap; k < &kmap[NELEM(kmap)]; k++)
        if(mappages(pgdir, k−>virt, k−>phys_end − k−>phys_start, (uint)k−>phys_start, k−>perm) < 0)
            return 0;
    return pgdir;
}
```
---
- kernel map (kmap) maps 4 regions
    - read/write IO region.
    - text + read only memory of kernel text.
    - read write data + free memory.
    - read/write memory of top IO regions.
- k->perm is the read write permissions.

- each process 2 stack one it's own execution stack and one in kernel for that process for handling system calls.
- KERNLINK = KERNBASE 2 GB (0x80000000) +  EXTMEM 1 MB (0x100000).

- **why kinit2 must come after startothers()?**
    - is it because memory mapped till 4MB is enough for 1 processor. 

- mapping the physical address 0 to virtual address of 2GB
    - python3 `int(bin(0x80000000)[2:2+10], 2) = 512` OR python3 `int(bin(2*2**30)[2:2+10], 2) = 512`
    - ie. this is 512th entry in the page table directory.
    - python3 `int(bin(0x80000000)[12:12+10], 2) = 0` OR int(bin(2*2**30)[12:12+10], 2) = 0.
    - ie. this is the 0th entry in the page table level 2.
        - use also set the flags so that user bit is clear and it's readable and writable.