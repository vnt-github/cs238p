- Boot -> BIOS (present on Flash ROM) -> Bootable device (first few bytes is bootloader: locate kernel and load it).
- BIOS finds "bootable drive" by looking in certain pre-determined places. 
- a drive is 'bootable' if it contains softwared that can finish the booting process.
- The BIOS knows it's found a bootable drive by looking at the first few kilobytes of the drive and looking for some magical numbers set in that drive's memory. 
- When the BIOS has found its bootable drive, it loads part of the drive called bootloader into memory and transfers execution to it.
- The bootloader's job is to take our kernel, put it into memory, and then transition control to it.
- bootloader: GRUB.

---
multiboot_header.asm
- dd -> define double word: it declares that we're going to stick some 32-bit data at this location
- Labels also refer to the memory occupied by the data and code which directly follows it

- The first, global start, says "I'm going to define a label start, and I want it to be available outside of this file." If we don't say this, GRUB won't know where to find its definition. You can kind of think of it like a 'public' annotation in other languages.\
- 0xb8000: the upper-left corner of the screen.
- You might be wondering, "If we're in 32 bit mode, isn't a word 32 bits?" since sometimes 'word' is used to talk about native CPU register size. Well, the 'word' keyword in the context of x86_64 assembly specifically refers to 2 bytes, or 16 bits of data. This is for reasons of backwards compatibility.
---
linker.ld
- . = 0x100000; This line means that we will start putting sections at the one megabyte mark, Below one megabyte is all kinds of memory-mapped stuff. Remember the VGA stuff? It wouldn't work if we mapped our kernel's code to that part of memory... garbage on the screen!