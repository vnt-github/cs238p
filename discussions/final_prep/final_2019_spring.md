# https://www.ic s.uci.edu/~aburtsev/238P/2019spring/lectures/final/paper.pdf

(e) (5 points) Ben knows that the scheduler also has “context”, but he is confused again. Can
you explain at what point the “context” of the scheduler is initialized?
Answer: The context of the scheduler is initialized right when the scheduler context
switches into the process for the first time. The context data structure of the scheduler
is allocated on the stack of the scheduler process (i.e., it’s not really allocated, the callee
saved registers are simply pushed on the stack by the swtch() function, and the pointer
to the top of the stack becomes the pointer to the “context” of the scheduler).

- what does being "EBX is callee saved register" have to do with the alltraps. if interrupted alltraps will push EBX regardless of calle or caller saved.
- create an ask with and example of P1 and P2 with scheduler and init with scheduler.
- why? won't the alltraps be called in this case.
- Is there a possiblity for swtch to be reached without reaching the alltraps from any other path flow? like from scheduler function?

- 1 a) i.) solved: No, the questions in the questions i could not have figured by reading the source code. NEED TO REVISIT LECTURES.
ii.) solved: Yes, the answer to the actual question i was able to answer in terms of thinking "what would happen if we don't"
- 1 b) solved: Yes, was able to answer in terms of thinking "what would happen if we don't".
- 1 c) solved: Yes, source code searching and browsing and understanding.
- 1 d) solved: No, missed the information leakage vulnerability. BETTER NOTES OF LECTURES AND MAKE CONNECTIONS TO OTHER QUESTIONS.
- 1 e) Solved: No, missed the context definition for switching.


- 2 a) solved: yes, source code browsing and reasoning.

- 3 a) solved: yes, source code browsing and reasoning.
- 3 b) solved:  yes, source code browsing and reasoning.

- 4 a) solved: partially.
- mine:
    - the log block(2-31) is written twice per transaction. once for data write and ~~then with special commit indicating a complete operation~~
- ans:
    - the second is after transaction is installed the log header is updated with zeros.
- 4 b) solved: yes
- 4 c) solved: yes
- 4 d) solved: partially, did not provide the code for modifications.
- to double the maximum file size we need to introduce more indirections as follows
    - 1st inode already have 12 addresses and 13th points to next indirect address block
    - 2nd level provide addresses for 13 blocks and 14th entry redirect to next level
    - 3rd level provides addresses to more 127 blocks and 128th points to the last level
    - 4th and last level will have 128 blocks for addresses
- to accommodate these all the source code handling redirection should be modified specifically bmap and iappend.
- so in total we will have 12+13+127+128 = 280 address blocks addressed which is double the 140 blocks.
    - 
- 5 a) yes: yes, source code browsing and reasoning.

---
# conclusions:
- more critical lecture with diagrams to remember better and source code references.
- BETTER NOTES OF LECTURES AND MAKE CONNECTIONS TO OTHER QUESTIONS
---
# content
- Context switch
- Synchronization
- File system
- Processes