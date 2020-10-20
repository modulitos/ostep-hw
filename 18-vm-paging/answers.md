## 1. Before doing any translations, let’s use the simulator to study how linear page tables change size given different parameters. Compute the size of linear page tables as different parameters change. Some suggested inputs are below; by using the `-v` flag, you can see how many page-table entries are filled. First, to understand how linear page table size changes as the address space grows, run with these flags:

> ./paging-linear-translate.py -P 1k -a 1m -p 512m -v -n 0
page table should have 1024 entries

> ./paging-linear-translate.py -P 1k -a 2m -p 512m -v -n 0
2048 PTE's

> ./paging-linear-translate.py -P 1k -a 4m -p 512m -v -n 0
4096 PTE's

Then, to understand how linear page table size changes as page size grows:

> ./paging-linear-translate.py -P 1k -a 1m -p 512m -v -n 0
1024 entries

> ./paging-linear-translate.py -P 2k -a 1m -p 512m -v -n 0
512 entries

> ./paging-linear-translate.py -P 4k -a 1m -p 512m -v -n 0

256 entries

Before running any of these, try to think about the expected trends. How should page-table size change as the address space grows? As the page size grows? Why not use big pages in general?

As address space grows, page table size increases.

As the page size grows, page table size decreases.

We don't want pages too big in general or else the address space might not be able to store the offset.

It will also waste space (internal fragmentation?)


## 2. Now let’s do some translations. Start with some small examples, and change the number of pages that are allocated to the address space with the `-u` flag. For example:

ARG seed 0
ARG address space size 16k
ARG phys mem size 32k
ARG page size 1k
ARG verbose True
ARG addresses -1

16 pages are available, but the number of valid pages depends on `-u`

16k address space -> 2^14 (14 bits)

16 pages -> 4 bits for VPN

*So, each address has 4 bits in the PN, and 10 bits in the offset.*

> ./paging-linear-translate.py -P 1k -a 16k -p 32k -v -u 0
16 pages, but none of them are valid.

> ./paging-linear-translate.py -P 1k -a 16k -p 32k -v -u 25
Page Table (from entry 0 down to the max size)
  [       0]  0x80000018
  [       1]  0x00000000
  [       2]  0x00000000
  [       3]  0x00000000
  [       4]  0x00000000
  [       5]  0x80000009
  [       6]  0x00000000
  [       7]  0x00000000
  [       8]  0x80000010
  [       9]  0x00000000
  [      10]  0x80000013
  [      11]  0x00000000
  [      12]  0x8000001f
  [      13]  0x8000001c
  [      14]  0x00000000
  [      15]  0x00000000

Virtual Address Trace
VA 0x00003986 (decimal:    14726) --> PA or invalid address?
0b11_1001_1000_0110

VPN: 0b1110 (decimal: 14)
address invalid


VA 0x00002bc6 (decimal:    11206) --> PA or invalid address?
(0b10 1011 1100 0110)

VPN: 0b1010 (decimal: 10)
VFN: 0x13
address: 0x13 + 0b11_1100_0110
 = 0b0100_1100_0000_0000 + 0b0011_1100_0110
 = 0x4fc6

VA 0x00001e37 (decimal:     7735) --> PA or invalid address?
0b01_1110_0011_0111
VPN: 0b0111 (decimal: 7)
invalid

VA 0x00000671 (decimal:     1649) --> PA or invalid address?
0b0000_0110_0111_0001
VPN: 1
invalid

VA 0x00001bc9 (decimal:     7113) --> PA or invalid address?
0b01_1011_1100_1001
VPN: 0b0110 (decimal: 6)
invalid


> ./paging-linear-translate.py -P 1k -a 16k -p 32k -v -u 50
Page Table (from entry 0 down to the max size)
  [       0]  0x80000018
  [       1]  0x00000000
  [       2]  0x00000000
  [       3]  0x8000000c
  [       4]  0x80000009
  [       5]  0x00000000
  [       6]  0x8000001d
  [       7]  0x80000013
  [       8]  0x00000000
  [       9]  0x8000001f
  [      10]  0x8000001c
  [      11]  0x00000000
  [      12]  0x8000000f
  [      13]  0x00000000
  [      14]  0x00000000
  [      15]  0x80000008


VA 0x00003385 (decimal:    13189) --> PA or invalid address?
0b11_0011_1000_0101

VPN: 0b1100 (decimal: 12)
PFN: 0xf
address = 0xf + 0b11_1000_0101
 = 0b11_1100_0000_0000 + 0b11_1000_0101
 = 0b0011_1111_1000_0101
 = 0x3f85



VA 0x0000231d (decimal:     8989) --> PA or invalid address?
0b10_0011_0001_1101

VPN: 0b1000 (decimal: 8)
invalid

VA 0x000000e6 (decimal:      230) --> PA or invalid address?
0b00_0000_1110_0110

VPN: 0
PFN: 0x18
address: (0x18 << 10) + 0b00_1110_0110
 = (0b0001_1000 << 10) + 0b00_1110_0110
 = (0b0110_0000_0000_0000) + 0b00_1110_0110
 = 0b0110_0000_1110_0110
 = 0x60e6


VA 0x00002e0f (decimal:    11791) --> PA or invalid address?
0b10_1110_0000_1111

VPN: 0b1011 (decimal: 11)
invalid

VA 0x00001986 (decimal:     6534) --> PA or invalid address?
0b01_1001_1000_0110

VPN: 0b0110 (decimal: 6)
PFN: 0x1d (0b0001_1101)

address: (0b0001_1101 << 10) + 0b01_1000_0110
 = 0b0111_0100_0000_0000 + 0b01_1000_0110
 = 0b0111_0101_1000_0110
 = 0x7586

> ./paging-linear-translate.py -P 1k -a 16k -p 32k -v -u 75
etc
> ./paging-linear-translate.py -P 1k -a 16k -p 32k -v -u 100
etc

What happens as you increase the percentage of pages that are allocated in each address space?

We get more valid hits.


## 3. Now let’s try some different random seeds, and some different (and sometimes quite crazy) address-space parameters, for variety:

> ./paging-linear-translate.py -P 8  -a 32   -p 1024 -v -s 1

The memory here is very small - it doesn't make sense to use paging with such a small address space and total memory.

> ./paging-linear-translate.py -P 8k -a 32k  -p 1m   -v -s 2

This makes sense.

> ./paging-linear-translate.py -P 1m -a 256m -p 512m -v -s 3

The addressable memory here is 1/2 of the physical memory.


Which of these parameter combinations are unrealistic? Why?


## 4. Use the program to try out some other problems. Can you find the limits of where the program doesn’t work anymore? For example, what happens if the address-space size is bigger than physical memory?


> ./paging-linear-translate.py -P 8k -a 32m -p 1m -v

Error: physical memory size must be GREATER than address space size (for this simulation)

> ./paging-linear-translate.py -P 8k -a 0 -p 1m -v

Error: must specify a non-zero address-space size.

> ./paging-linear-translate.py -P 0 -a 32m -p 1m -v

Error: physical memory size must be GREATER than address space size (for this simulation)

> ./paging-linear-translate.py -P 16k -a 8k -p 1m -v

Error in argument: address space must be a multiple of the pagesize

> ./paging-linear-translate.py -P 17m -a 32m -p 64m -v

Error in argument: address space must be a multiple of the pagesize
