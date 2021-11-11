#include "console.h"

#define NPDENTRIES      1024    // # directory entries per page directory
#define NPTENTRIES      1024    // # PTEs per page table
#define PGSIZE          4096    // bytes mapped by a page
#define KERNBASE 0x80000000    
#define PTXSHIFT        12      // offset of PTX in a linear address
#define PDXSHIFT        22      // offset of PDX in a linear address

// Page table/directory entry flags.
#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable
#define PTE_U           0x004   // User
#define PTE_PS          0x080   // Page Size

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
    static unsigned int pte1_address[1024] __attribute__((aligned(4096)));
    for (i = 0; i < 1024; i++)
    {
        unsigned int *p = (unsigned  int *)(i * 4096);
        unsigned int addr = (unsigned int)p | PTE_P | PTE_W;
        pte1_address[i] = addr;
    }

    static unsigned int ptd_address[1024] __attribute__((aligned(4096)));
    ptd_address[0] = (unsigned int)pte1_address | PTE_P | PTE_W;

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
