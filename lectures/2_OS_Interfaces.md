# what is an OS?
- share hardware across multiple processes: illutions of private cpu and private memory.
- abstracts hardware: hides details of specific hardware devices.
- provides services: serves as a library for applications.
- security: isolation of processes, controlled ways to communicate (in a secure manner).

- isolation of processes: one process can't access memory of another process, and process can't access memory of kernel.

- a piece of code that boots into hardware becomes a OS, aka kernel.
- this kernel provides some interface to the processes, which kernel runs on it's top.
---
# typical UNIX OS
- kernel: a piece of code that boots into hardware provides operating system services.
    - system call interface.
    - file system.
    - scheduler.
    - network stack.
    - virtual memory.
    - device drivers.

    - encoded in yellow.
    - access level: ring 0.

- any application
    - runs on top of kernal.
    - uses system libraries.

    - access level: ring 3.

---
- APPLICATION open('/foo.txt', ..flags..).
- system library: lib.c in linux,  makes syscall to kernel.
- kernel invokes the file system handler like sys_open.
---
# system call
- why can't we just create the function open('/foo.txt', ..flags..) inside the kernel.
    - because we want security by process isolation of kernel memory from user level application memory.
    - so to access a memory belonging to a kernel the modern os triggers a page fault.

- user level library for eg. lib.c invokes an int instruction of 0x80
- this instruction triggers an exception and the control goes to IDT(interrupt descriptor table)
- IDT is looked for this particular exception handler and invokes it.
- this int instruction exception changes privilege from ring 3 to ring 0.
---
# What system call do we need for a basic OS?
- file IO: open, read, write, close, chdir, mkdir, fstate, link, unlink.
- memory access: alloc, dealloc
    - sbrk(size): extends memory of a process, or decrese my allocation.
- interprocess communication.
    - dup(): create a duplicate of file descriptor.
    - pipe(): allow to create a channel that connects two processes.
- fork(), exec(): system call required to start new processes.
- network: sockets()

---
# what is shell?
- it's just another program that starts on hardware.
- it's one of the first program the OS starts. like init process -> shell process.
- it's a program with a command prompt interface.
- it invokes the command or tools in a file systems and returns back with prompt.
- frame buffer is written with ascii character values which is read by graphical memory and connects to a digital panel to show corresponding character by lighting specific cells on the display.
- shell uses same read(), write() system calls, with specific file descriptor that implements a console driver which connects with a vga driver.
- it can read data from keyborad for input.
---
# Console and file I/O.

console read
- read 0 interrupt -> invoke sys read
- make sure buff have memory access that is a user memory not in kernel.

- what is file descriptor?
    - for each process OS maintains a table called File Descriptor Table.
    - FDT is a collection of file or file like objects.
        - file, device( like console ) or pipe.
    - file descriptor is just a pointer stating the number into this FDT to use.
    - when opening a new file (via open, **OR VIA dup(p[0])**) we get the smallest open file descriptor ie. lowest empty unoccupied number in the file descriptor table.

- two processes can read and write to same file, they have to synchronize somehow to make sure they are not overiding writes of each other.

---
# Console
- read(0, buf, size); here 0 is the standard input by convention.
- it returns the file descriptor for console file object.
- console interacts with the vga drive, this vga driver write to the frame buffer.
- this frame buffer is scanned by monitor hardware to display content.
- kernel -> vga driver -> visual frame buffer -> graphical card

---
- `wc -l ls.c`
    - shell starts the process(exec()) wc and waits for it's response. 
    - after wc is done shell reads it's response and outputs it.

---
# Creating new process.
- fork(): creates the identical copy of fork invoking process, every bit of memory os the duplicate/clone child process is same, except its process id: 0.
    - it even copies the file descriptors of parent into child.
- exec(): replaces the memory of current process with a completely new binary file.
    - it leaves the file descriptor table untouched, ie. it remains the way it was before it's invoked.

- # why dont we just have another system call pid = create("/bin/wc")
    - because we want to implement I/O redirection.

# PIPES
- pipes are system call that exposes a kernel cicle ring buffer as a pair of file descriptors, one for reading and one for writing.
- allows to connect output for one program to the input of another program.

- it's implemented as below
```
for command: cat < ls.c
shell (child of)
0 = fork() # this shows this current is the child process
close(0) #  this frees up the stdin FDT entry to be read for redirected file
0 = open(ls.c) # this opens ls.c and sets first open FDT entry i.e. 0
exec('cat') # note that cat works with 0 as input which is previously set to point to ls.c.
```
- the above works because after fork the file descriptors are changed in child and it's reflected in the parent(piazza discussion for same)

- for pipes we can read and write from any file descriptor even same for input and output.

combo of fork()/exec()
- without this combo if we create our own createProcess(...) then this function need to support a large number of combination of various configuration or arguments for handling various IO and IO redirections.

- for forked() process the OS cleans the file descriptors after the process finishes.

- file descriptor tables are not shared between processes.

---
# Reading: The C Programming Language
- *px++ = *(px++) = *(px+1) because the uniary operators are calculated from left to right.
- array name is a constant so a = pa, a++ or p=&a are illegal. but when array name is passed to a function it's passed as a variable, because arguments are passed by value.
- C guarantees that no pointer that validly points at data will contian zero, so a return value of zero is used to signal an abnormal event, generally no space for pointer available.
- apart from adding or subtracting a pointer and an integer; subtracting or comparing two pointers, all other pointer arithmetic is illegal. 
- It is not permitted to add two pointers, or to multiply or divide or shift or mask them, or to add float or double to them.