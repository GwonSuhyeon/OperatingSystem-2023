## Assignment4


1. assignment4.c : Simulating Page Replacement Algorithms (Optimal, FIFO, LRU, Second-Chance) for arbitrary virtual addresses.

2. Makefile : For compile assignment4.c.


Receive input for the virtual address length, page size, and physical memory size. Calculate page offset and the total number of frames. Given 5000 random decimal virtual address values, convert them to binary bits, calculate the page number and page offset. Check the page fault/hit status by examining the current frames.

In case of a page fault, search for the frame to replace according to each page replacement algorithm, replace the page, and update the cycle and input order of the frames. In case of a page hit, update the cycle and input order of the frames according to each page replacement algorithm.

After processing the page fault/hit, calculate the physical address for the corresponding virtual address. Repeat these steps to simulate 5000 virtual addresses.
