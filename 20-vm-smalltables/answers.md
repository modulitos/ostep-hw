## 1. With a linear page table, you need a single register to locate the page table, assuming that hardware does the lookup upon a TLB miss. How many registers do you need to locate a two-level page table? A three-level table?

**2 registers** for a two-level page table: one for the upper lever, and one for the lower level page directory.

**3 registers** for a three-level page table: one for the upper level, one for the mid level, and one for the lowest level page directory.

## 2. Use the simulator to perform translations given random seeds 0,1, and 2, and check your answers using the `-c` flag. How many memory references are needed to perform each lookup?

assumptions:
 * each page has size: 32 bytes
 * address space: 32KB => 1024 pages
 * physical memory: 128 pages

thus:
virtual address needs 15 bits (5 offset, 10 VPN)
 - 5 offset comes from 32 byte page

physical address needs 12 bits (5 offset, 7 PFN)
 - 5 offset comes from 32 byte page

Each page table page holds 32 PTE's (given)
thus the PD has 32 PDE's total (1024 PTE's in PD * (1 PDE / 32 PTE's))

VPN is the upper 10 bits of a VA
PDE index is the upper 5 bits of the VPN.
PTE index is the lower 5 bits of the VPN.

PDE address = PDBR + (PDE index + sizeof(PDE))
sizeof(PTE) = 1 byte (32 bytes/page * (1 page/32 PTE's))
sizeof(PDE) = 1 byte (given)
PDE = accessMemory(PDE Address)

PTE and PDE formats are identical - first bit is valid bit.
PTE: last 7 bits are the PFN
PDE: last 7 bits are the PTE

phys addr = (PTE.PFN << shift) + offset


NOTES:
A linear page table would have 1024 pages => 32 KB in total size
(1024 pages * 32 bytes per page).
It would have 2^10 entries => 1 entry per page

PD value (page 108):
83fee0da7fd47febbe9ed5ade4ac90d692d8c1f89fe1ede9a1e8c7c2a9d1dbff


### Virtual Address 0x611c
0b 110 0001 0001 1100
VPN: 110 0001 000
Offset: 1 1100 (16 + 8 + 4 = 28)

PDE index: 110 00 (24 decimal)
PTE index: 01 000 (8 decimal)

PDE contents = (page 108) + (24 bytes) = 0xa1 = 0b1010 0001
PT = 0b010 0001 (33 decimal)

page 33:
7f7f7f7f7f7f7f7fb57f9d7f7f7f7f7f7f7f7f7f7f7f7f7f7f7ff6b17f7f7f7f

PTE address = (page 33) + (8 bytes) = 0xb5 = 0b1011 0101
PFN: 0b011 0101 = 0x35 (32 + 16 + 5 = 53 decimal)
phys addr = (0x35 << 5) + 28 = 53 + 28
= numpy.base_repr((0x35 << 5) + 28, 16) = 0x6bc


page 53:
0f0c18090e121c0f081713071c1e191b09161b150e030d121c1d0e1a08181100

<!-- phys addr = (0x35 << 10) + 28 = (0b 0 1101 0100 0000 0000) + 0b 1 1010 -->
<!-- = 0b 0 1101 0100 0001 1010 = 0x 1c -->

= 0x08

  --> pde index:0x18 [decimal 24] pde contents:0xa1 (valid 1, pfn 0x21 [decimal 33])
    --> pte index:0x8 [decimal 8] pte contents:0xb5 (valid 1, pfn 0x35 [decimal 53])
      --> Translates to Physical Address 0x6bc --> Value: 08

### 0x3da8
0b 011 1101 1010 1000
VPN: 011 1101 101
PDE index: 0 1111= 0x0f = 15
PTE index: 0 1101 = 0xd = 13

pde contents = page 108, byte 15: 0xd6
=> 0b1101 0110
pfn = 0b101 0110 => 0x56 = (64 + 16 + 6) = 86


page 86:
7f7f7f7f7f7f7fc57f7f7f7f7f7f7f7f7f7f7f7fca7f7fee7f7f7f7f7f7f7f7f

pte contents = page 86, byte 13: 0x7f
=> 0b0111 1111
pfn = 0b0111 1111 = 64 + 32 + 16 + 15 = 127
not valid!

  --> pde index:0x0f [decimal 15] pde contents:0xd6 (valid 1, pfn 0x56 [decimal 86])
    --> pte index:0xd [decimal 13] pte contents:0x7f (valid 0, pfn 0x7f [decimal 127])
    Invalid!

### 0x17f5
=> 0b001 0111 1111 0101
VPN: 001 0111 111
PDE index = 0b 0 0101 = 0x5 = 5
PTE index = 0b 1 1111 = 0x1f = (16 + 15) = 31
offset: 0b 1 0101 = 0x15 = 16 + 5 = 21

PDBR = 108 (page 108):
83fee0da7fd47febbe9ed5ade4ac90d692d8c1f89fe1ede9a1e8c7c2a9d1dbff

pde contents = page 108, byte 5: 0xd4 = 0b 1101 0100
pfn for PT = 0b 101 0100 = 0x54 = (64 + 16 + 4) = 84

page 84:
7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f947f7f7f7f7fce

pte contents = page 84, byte 31: 0xce = 0b 1100 1110
pfn = 0b 100 1110 = 0x4e = (64 + 14) = 78


page 78:
0e02171b1c1a1b1c100c1508191a1b121d110d141e1c1802120f131a07160306

<!-- phys addr contents = page 78, byte 21 (offset) = 0x1c = (16 + 12) = 28 -->

phys addr = numpy.base_repr((0x4e << 5) + 21, 16)
= 9d5


  --> pde index:0x5 [decimal 5] pde contents:0xd4 (valid 1, pfn 0x54 [decimal 84])
    --> pte index: 0x1f [decimal 31] pte contents:0xce (valid 1, pfn 0x4e [decimal 78])
      --> Translates to Physical Address 0x9d5 --> Value 0x1c

## 3. Given your understanding of how cache memory works, how do you think memory references to the page table will behave in the cache? Will they lead to lots of cache hits (and thus fast accesses?) Or lots of misses (and thus slow accesses)?

Lots of misses, due to lack of temporal and spatial locality.
