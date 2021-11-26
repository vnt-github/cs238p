# https://www.ic s.uci.edu/~aburtsev/238P/2019spring/lectures/final/paper.pdf

(e) (5 points) Ben knows that the scheduler also has “context”, but he is confused again. Can
you explain at what point the “context” of the scheduler is initialized?
Answer: The context of the scheduler is initialized right when the scheduler context
switches into the process for the first time. The context data structure of the scheduler
is allocated on the stack of the scheduler process (i.e., it’s not really allocated, the callee
saved registers are simply pushed on the stack by the swtch() function, and the pointer
to the top of the stack becomes the pointer to the “context” of the scheduler).

- 1 a) i.) solved: No, the questions in the questions i could not have figured by reading the source code. NEED TO REVISIT LECTURES.
ii.) solved: Yes, the answer to the actual question i was able to answer in terms of thinking "what would happen if we don't"
- 1 b) solved: Yes, was able to answer in terms of thinking "what would happen if we don't".
- 1 c) solved: Yes, source code searching and browsing and understanding.
- 1 d) solved: No, missed the information leakage vulnerability. BETTER NOTES OF LECTURES AND MAKE CONNECTIONS TO OTHER QUESTIONS.
- 1 e) Solved: doubt


- 2 a) solved: yes, source code browsing and reasoning.

- 3 a) solved: yes, source code browsing and reasoning.
- 3 b) solved:  yes, source code browsing and reasoning.

- 4 a) solved: pending
- 4 b) solved: pending
- 4 c) solved: pending
- 4 d) solved: pending
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