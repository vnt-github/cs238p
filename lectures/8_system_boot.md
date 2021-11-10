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
- why did we map kernel twice?
---
- why direction jump
- cos main may not be a static function and call function is a relative jump and main cannot be called using a relative jump.
- main itself is a symbol in 2GB+1MB+Delta in kernel, so we jump to this address.
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

- not to map the kernel to above addresses is to prevent MELTDOWN attacks.
---
# memory after boot
- xv6 never uses memory above  (0xe0000000)234MB (PHYSTOP)
- I/O space
    - 0-0x100000 and 0xfe000000-0xffffffff
    - used to communicated with IO devices.
    - the instruction goes through PCI devices rather than memory controller.
---
- create the kernel address space.
    - create kernel memory allocator
    - allocate memory for page tables
        - page table directory and page table.
- why not to continue with 4MB pages.
    - cos the size of programs will be typically in kbs so 4MB would waste more space.
- a better way is to use a hybrid approach, ie 4mb for kernel and 4kb page sizes for user processes.
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
- where does this memory comes from
    - this linked list does not live within kernel, but it lives within page table itself.
    - this memory has to be mapped by page table.

- it uses the free memory after the kernel memory.
    - create end from 0x801126fc - till the end of 2GB+4MB.

```
kinit1(end, P2v(4*1024*1024))

kinit1(void *vstart, void *vend) {
    freerange(vstart, vend);
}

freerange(void *vstart, void *vend) {
    char *p;
    p = (char*)PGROUNDUP((uint)vstart);
    for(;p+PGSIZE<= (char*)vend; p+=PGSIZE)
        kfree(p);
}
```
what is this end?
- extern char end[]; provided by linker.
---
# kernel page table (4KB page table)
 
kvmalloc(); // allocates first kernel page table memory.


kvmalloc(void) {
    kpgdir = setupkvm();
    switchkvm();
}

pde_t* setupkvm(void) {

}
---
- kernel map (kmap) maps 4 regions
    - read/write IO region.
    - text + read only memory of kernel text.
    - read write data + free memory.
    - read/write memory of top IO regions.
- k->perm is the read write permissions.

- each process 2 stack one it's own execution stack and one in kernel for that process for handling system calls.
- KERNLINK = KERNBASE 2 GB (0x80000000) +  EXTMEM 1 MB (0x100000).