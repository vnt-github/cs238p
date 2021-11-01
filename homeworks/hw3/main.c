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
    if (argc < 2) {
        fprintf(stderr, "provide elf file name\n");
        exit(EXIT_FAILURE);
    }
    struct elfhdr elf;
    struct proghdr ph;

    int (*sum)(int a, int b);
    void *entry = NULL;
    int ret; 

    int fp = open(argv[1], O_RDONLY);
    off_t lseek_res;
    size_t read_res;
    if (fp) {
        read_res = read(fp, &elf, sizeof(elf));
        if (read_res == -1) {
            fprintf(stderr, "elf fread error\n");
            exit(EXIT_FAILURE);
        }
        int i = 0;
        while (i < elf.phnum) {
            lseek_res = lseek(fp, elf.phoff + i*elf.phentsize, SEEK_SET);

            if (lseek_res == -1) {
                fprintf(stderr, "elf.phoff lseek error\n");
                exit(EXIT_FAILURE);
            }
            read_res = read(fp, &ph, sizeof(ph));
            if (read_res == -1) {
                fprintf(stderr, "ph fread error\n");
            }
            if (ph.type == ELF_PROG_LOAD) {
                void *code_va = mmap(NULL, ph.memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
                lseek_res = lseek(fp, ph.off, SEEK_SET);
                if (lseek_res == -1) {
                    fprintf(stderr, "ph.off lseek error\n");
                    exit(EXIT_FAILURE);
                }
                read_res = read(fp, code_va, ph.filesz);
                if (read_res == -1) {
                    fprintf(stderr, "ph fread error\n");
                    exit(EXIT_FAILURE);
                }
                if (ph.type == ELF_PROG_FLAG_EXEC)
                    entry = code_va;
            }
            i++;
        }
    }

    if (entry != NULL) {
        sum = entry; 
        ret = sum(1, 2);
        printf("sum:%d\n", ret); 
    };
    return 0;
}
