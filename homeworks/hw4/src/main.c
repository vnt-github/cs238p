#include "console.h"

#define NPDENTRIES      1024    // # directory entries per page directory
#define NPTENTRIES      1024    // # PTEs per page table
#define PGSIZE          4096    // bytes mapped by a page

// Page table/directory entry flags.
#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable

#define NPTE          64

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
    int i, j; 
    int sum = 0;

    // Initialize the console
    uartinit(); 

    printk("Hello from C\n");


    __attribute__((__aligned__(PGSIZE)))
    unsigned int ptd_address[NPDENTRIES];
    __attribute__((__aligned__(PGSIZE)))
    unsigned int pte_address[NPTE][NPTENTRIES];
    for (j = 0; j < NPTE; j++)
    {
        for (i = 0; i < NPTENTRIES; i++)
        {
            unsigned int addr = (PGSIZE*NPDENTRIES*j + i * PGSIZE) | PTE_P | PTE_W;
            pte_address[j][i] = addr;
        }
        ptd_address[j] = (unsigned int)&pte_address[j][0] | PTE_P | PTE_W; 
    }
    
    // NOTE: we do not need to set PTE_P | PTE_W flags for cr3
    unsigned int addr = (unsigned int)ptd_address;
    lcr3(addr);

    for (i = 0; i < 32 /*64*/; i++) {
        int *p = (int *)(i * 4096 * 32);
        sum += *p;

        printk("page\n"); 
    }
    halt(); 
    return sum; 
}
