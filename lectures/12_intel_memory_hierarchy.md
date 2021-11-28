- everything fetched from the instruction pointer will be cached in the L1 instruction pointer.
- cacheline: The block of memory that is transferred to a memory cache.
- what happens if one core happens to load a cacheline.
- all units of transactions between memory and core, between caches and core is in units of cacheline.
- in intel and AMD one cacheline is 64 bits.

---
- to load something from registers: 0 cycles.
- cycles to access 1st level cache: 4 cycles, size of L1: 2*32 KB.
    - if missing L1 cache, L1 cache forwards to L2, L2 cache forwards to L3.
    - L3 is running a directory: it knows which memory is responsible for containing this cacheline.
    - this is forwarded to memory, which then replies with cacheline to L3 to L2 to L1.
    - caches were inclusive so L3, L2 and L1 all have the memory reply.
- cycles to access L2 cache: 12 cycles, size: 256 KB.
- cycles to access L3 cache: 53 cycles, size: 30 MB.
- cycles to access local memory: 200-300 cycles, size: TBs, 96 DIMM slots

latency = sqrt(capacity)

- Core 1 L1 to Core 2 L2 latency: 4 + 12 + 53 + 53 cycles = 122 cycles.

- accessing remote memory: 365 cycles.
- QPI link takes: 150 cycles.
- cycles to access PCI-e: 1500 cycles, corresponding to 600 nano seconds.

---
- simultaneous load of cache line by different processors is allowed.
- if one of the cores tries to store (write) to the same cacheline.
    - we use cache-coherence protocol.
    - before storing (writing) the L1 cache checks if this cache line is in shared state.
        - if in shared state then we need to evict this cache line from other Cores caches.
        - storing core cache sends requests to invalidate the shared cacheline to other cores.
        - This eviction requests goes through L1->L2->L3->L2(other process)->L1(other process).
        - this makes sure that no further reads is allowed.
        - now cacheline is only allowed in the exclusive state to the Core 0 cache.
        - Core 0 can write and read to this cacheline multiple times.
    - now when Core 1 tries to load (read) the shared cacheline, it forwards to request to Core 0 so that Core 0 can write the cacheline to the memory to make sure that most recent values is in the memory.
    - now cacheline is again in the shared state between Core 0 and Core 1.