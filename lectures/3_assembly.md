- mov eip, eax is invalid
- we need both conditional and unconditional jumps. for ex. need unconditional jump for below to skip else if 'if' block is executed.
    ```
    if (cmp)
        {
            ...
            ...


            ...
            jump: skip_else
        }
    else
        {
            ...
            ...
        }
    skip_else: print()
    ```
- when value greater that 32 bits occur the overflow flag is set in EFLAGS.
- the bits that overflow will be lost.
---
STACK: function calls

- we need stack to call functions and **be able to return to the calling function**

- stack grows up towards 0 address space.
- we push into stack (ESP is decreased) the address of next line to the function call that is calculated as EIP + sizeof(current call instruction)
    ```
    push eip + sizeof(CALL) # this is MOV [ESP], EAX; SUB ESP, 4
    jmmp _my_function
    ```    
- on ret the stack is popped (ESP is increased) and gets the return address as popped value and EIP now points to the return address.
    ```
    pop EAX # this is MOV EAX, [ESP]; ADD ESP, 4
    ```
    - NOTE: the memory of at previous location of ESP is still there, we just incremented the ESP for it to be overwritten later. Unless there is a security concern, then you wipe the values.

- other uses:
    - local data storage
    - parameter passing
    - evaluation stack
        - register spill: when you are running out of registers you can push into stack.
        ```
        ADD EAX, ... # but already some old value in EAX that you don't wanna loose.
                     # so push the old value into stack and use EAX and pop to reset EAX to old value. 
        ```
- xv6 allocated only **1 page for stack** & places a **guard page** at the end of stack to prevent infinite recursive calls by detect running out of stack space. This works as GUARD page is not mapped so OS trows page fault and kills the process.
- stack space in contigous by default, but it can be configured to be run with chained stack.
---

