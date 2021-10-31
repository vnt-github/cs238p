WHY?
- call by reference/value: is a convention at a language level so they are not a calling convention. It is not at the level of assembly or application binary interface.
- we need calling conventions to specify:
    - How to pass arguments:
        - on the stack?
        - on the registers?
    - How to return values?
        - on th stack?
        - on the registers?

- conventions differ from compiler.
    - 99% of compilers using the below.
        - cdecl on 32 bit.
        - system V(roman 5) on 64 bit.
---
STACK FRAME
- A dedicated base pointer (generally EBP, but can be any other general register as well): extended frame base pointer is used to.
- Each function maintains the frame using below on entry and exit to function

- compiler can function without stack frame. **BUT WE MAINTAIN STACK FRAME FOR HUMANS DEBUGGING**. without stack frame it's hard to unwind the stack in case of a crash.
    - as it's easier to check the arguments and local variables wrt EBP, instead of ESP.
    - -fomit-frame-pointer or on -O, -O1, -O2.. any level of optimization: disable frame pointers. you save 5-7 cycles by doing this by saving push EBP, and MOV EBP, ESP instructions.
    - it also makes EBP available as general register, so chances of register spill decreases. also saves 2 cycles for push and 2 cycles for pop for each register spill.

```x86asm
my_function:        ; on entry:
    push ebp        ; save original EBP value on stack
    mov ebp, esp    ; new EBP=ESP
___________________________________________________
    ...             ; function body
    ...
    ...             ;
___________________________________________________
    pop ebp         ; on exit: restore original EBP value
    ret
```
# VARIABLES

GLOBAL VARIABLES: which are in global scope. 
    - initialized: maintained in data section. maintained in disk, pointers in code are patched by linker/loader.
    - uninitialized: maintained in BSS (text section). uninitialized are set to 0s.

```c
#include <stdio.h>
char hello[] = "Hello"; // GLOBAL: initialized so goes to data section.

int main(int ac, char **av) {
    static char world[] = "word!"; // GLOBAL (because of static keyword): initialized so goes to data section.
    ...
}
```

DYNAMIC VARIABLES
    - maintained in stack.
    - sbrk system call allocate chunk of memory. On top of sbrk runs malloc which has metadata tracking regions of memory that is allocated and which is unallocated, free and empty. It only services regions in power of 2, so request to allocate 65 will get 128 but will give back 65 maintaining rest as unallocated.

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char hello[] = "Hello";
int main(int ac, char *av) {
    char world[] = "word!";
    char *str = malloc(64);         // DYNAMIC: memory allocated on heap (pointed by address stored in *str), but str pointer variable itself is in stack.
    memcpy(str, "beautiful", 64);
    ...
}
```

LOCAL VARIABLES
- variables whose scope is valid only during the duration of the function.
- each function has a private instance of local variable, as functions can be called recursively it's convenient to allocated local variables on stack.

```c
#include <stdio.h>

char hello[] = "Hello";
int main(int ac, char **av) {
    char world[] = "word!"; // LOCAL: maintained in stack.
    ...
}

```

```c
void my_function()
{
    int a, b, c;
    ...
    a = 10;
    b = 5;
    c = 2;
}
```
- to allocate local variables we preserve some space on stack.
- you push into the stack in the order of initialization.
    - for int a,b,c; push into the stack from left to right. ie. a first, b second and then c.
```x86asm
my_function:                    ; on entry:
    push ebp                    ; save original EBP value on stack
    mov ebp, esp                ; new EBP=ESP
    sub esp, LOCAL_BYTES        ; #bytes needed by locals 3 int variables each 4 bytes so LOCAL_BYTES=12 bytes
___________________________________________________
    ...
    mov [ebp-4], 10 ;           ; a = 10; location of variable a
    mov [ebp-8], 5              ; b = 5; location of variable b
    mov [ebp-12], 2             ; c = 2; location of variable c
    ...             
___________________________________________________
    mov esp, ebp                ; deallocate locals
    pop ebp                     ; on exit: restore original EBP value
    ret
```
---
- HOW TO PASS ARGUMENTS?
    - x86 32 bit: pass arguments on stack, return value is in EAX, EDX.
    - x86 64 bit: pass first 6 arguments in registers (RDI, RSI, RDX, RCX, R8, R9) to save later instruction cycles to populate them with arguments, rest on stack , return values is in RAX and RDX.
    - using two return register is because it is possible for two 32-bit/64-bit values multiplied together to overflow a 32-bit/64-bit value. so we use EDX:EAX to retain larger return values without overflows. Usually only EAX or RAX is used for not overflows.

```
void my_function(int x, int y, int z) { ... } // signature
my_function(2, 5, 10);  // invoking
```
- push into stack from right to left in argument list, ie. last argument first.
```x86asm
push 10
push 5
push 2
call _my_function
```

---
NOTE: below is for int argument and return values
```x86asm
address       stack
0xb4         |                       |
0xb0         |last argument: 10      | [ebp + 16 ](3rd function argument)
0xac         |2nd last argument:  5  | [ebp + 12] (2nd function argument)
0xa8         |3rd last argument:  2  | [ebp + 8] (1st function argument)
0xa4         |calling fn return addr.| [ebp + 4] (return address) <- ESP right after CALL system call
0xa0         |  FP                   | [ebp] (old ebp values) **<- EBP points here** which function EBP?
0x9c         |1st assigned local var.| [ebp-4] (1st local variable)
0x98         |                       |
0x94         |                       |
0x90         |xth assigned local var.| [ebp-x*4] **<- ESP points here**
0x8c         |                       | 
```
- caller function pushes and pops the argument.
- local variable optimization: if uninitialized variable is just present to return from function then compiler can skip allocating stack space to that return local variable as it uses EAX for that.

- x86 has special instruction leave: to deallocate space for local variables and popping the stack.
    ```
    mov esp, ebp
    pop ebp
    ```

# WHO SAVES AND RESTORES THE REGISTERS?
- processors doesn't save registers.
- needs convention: what gets saved by the callee and caller.
    - caller saved: EAX, ECX & EDX so callee function is free to use them.
    - rest(like EBX) are calee saved: ie. if calee function uses it then it need push into stack and pop later to restore their original values.