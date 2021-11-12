#include "console.h"

#define NPDENTRIES      1024    // # directory entries per page directory
#define NPTENTRIES      1024    // # PTEs per page table
#define PGSIZE          4096    // bytes mapped by a page

// Page table/directory entry flags.
#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable
#define PTE_U           0x004   // User

typedef unsigned int   uint;
typedef uint pde_t;

static inline void lcr3(unsigned int val)
{   
  asm volatile("movl %0,%%cr3" : : "r" (val));
}

static inline void halt(void)
{
    asm volatile("hlt" : : );
}

int main(void)
{
    int i; 
    int sum = 0;

    // Initialize the console
    uartinit(); 

    printk("Hello from C\n");
    // Create your page table here
    // __attribute__((__aligned__(PGSIZE)))
    static unsigned int pte1_address[NPTENTRIES] __attribute__((aligned(PGSIZE)));
    for (i = 0; i < NPTENTRIES; i++)
    {
        unsigned int addr = i*PGSIZE | PTE_P | PTE_W;
        pte1_address[i] = addr;
    }
    // __attribute__((__aligned__(PGSIZE)))
    static unsigned int pte2_address[NPTENTRIES] __attribute__((aligned(PGSIZE)));
    for (i = 0; i < NPTENTRIES; i++)
    {
        unsigned int addr = (PGSIZE*NPDENTRIES + i * PGSIZE) | PTE_P | PTE_W;
        pte2_address[i] = addr;
    }

    // __attribute__((__aligned__(PGSIZE)))
    static unsigned int ptd_address[NPDENTRIES] __attribute__((aligned(PGSIZE)));
    ptd_address[0] = (unsigned int)pte1_address | PTE_P | PTE_W;
    ptd_address[1] = (unsigned int)pte2_address | PTE_P | PTE_W;
    unsigned int addr = (unsigned int)ptd_address | PTE_P | PTE_W;
    lcr3(addr);

    for (i = 0; i < 32 /*64*/; i++) {
        int *p = (int *)(i * 4096 * 32);
        sum += *p;

        printk("page\n"); 
    }
    halt(); 
    return sum; 
}
