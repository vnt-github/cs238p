#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <sys/mman.h>


#include <sys/types.h>
#include <unistd.h>

// Format of an ELF executable file

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

// File header
struct elfhdr {
  unsigned int magic;  // must equal ELF_MAGIC
  unsigned char elf[12];
  unsigned short type;
  unsigned short machine;
  unsigned int version;
  unsigned int entry;
  unsigned int phoff;
  unsigned int shoff;
  unsigned int flags;
  unsigned short ehsize;
  unsigned short phentsize;
  unsigned short phnum;
  unsigned short shentsize;
  unsigned short shnum;
  unsigned short shstrndx;
};

// Program section header
struct proghdr {
  unsigned int type;
  unsigned int off;
  unsigned int vaddr;
  unsigned int paddr;
  unsigned int filesz;
  unsigned int memsz;
  unsigned int flags;
  unsigned int align;
};

// Values for Proghdr type
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4

int main(int argc, char* argv[]) {
    struct elfhdr elf;
    struct proghdr ph;

    int (*sum)(int a, int b);
    void *entry = NULL;
    int ret; 


    FILE *fp = fopen("elf", "r");
    if (fp) {
        fread(&elf, sizeof(elf), 1, fp);
        // printf("%d %ld\n", elf.magic, ftell(fp));
        int i = 0;
        while (i < elf.phnum) {
            lseek((int)fp, elf.phoff + i*elf.phentsize, SEEK_SET);
            fread(&ph, sizeof(ph), 1, fp);
            // printf("%d %ld\n", ph.off, ftell(fp));
            if (ph.type == ELF_PROG_LOAD) {
                void *code_va = mmap(NULL, ph.memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
                // entry = (int *)code_va(int a, int b);
                entry = code_va;
                int (*entry)(int, int) = entry;
                lseek((int)fp, ph.off, SEEK_SET);
                fread(code_va, ph.filesz, 1, fp);
            }
            i++;
        }
    }

    /* Add your ELF loading code here */

    if (entry != NULL) {
        sum = entry; 
        ret = sum(1, 2);
        printf("sum:%d\n", ret); 
    };
    return 0;
}


// https://github.com/lirongyuan/ELF-Reader-and-Loader/blob/master/loader.c