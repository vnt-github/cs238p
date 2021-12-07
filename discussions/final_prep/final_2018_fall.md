- 1 a) solved: partially, from source code and lectures. Better Notes of lectures and revision
- 1 b) solved: yes, source code and lecture slides. 
- 1 c) solved: no, i could not have figured by reading the source code. NEED TO REVISIT LECTURES.
- 1 d) solved: yes.

- 2 a) solved: partially
- 2 b) solved: yes

- 3 a) solved: yes, lectures and notes and source code.
- 4 a) solved: yes, lectures and notes.
- 4 b) solved: yes, lectures and notes.
- 5 a) solved: yes, lectures and intuition.
- 5 b) solved: yes, lectures and intuition.
- 5 c) solved: yes, partially
    - is hw_encrypt async? YES
```c
struct {
    int encrypting;
    struct spinlock lock;
} cdev;

encrypt_block(cdev c, buffer b, key k) {
    while (1) {
        acquire(&cd.lock);
        if (!cd.encrypting) {
            cd.encrypting = 1;
            // NOTE: can we have release below? Yes
            hw_encrypt(b, k);
            break;
        } else {
            sleep(cd, &cd.lock);
        }
    }
    // NOTE: just like in solution you only release once done sending hw_encrypt call.
    release(&cd.lock);
}

// trap.c
case T_IRQO + IRQ_CDEV:
    acquire(&cd.lock);
    cd.encrypting = 0;
    wakeup(cd);
    release(&cd.lock);
```

- 6 a) solved: yes, lectures and notes.
- 6 b) solved: yes, lectures and notes.
---
# Conclusion:
- Better Notes of lectures and revision