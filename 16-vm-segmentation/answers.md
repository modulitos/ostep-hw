## 1. First let’s use a tiny address space to translate some addresses. Here’s a simple set of parameters with a few different random seeds; can you translate the addresses?

`./segmentation.py -a 128 -p 512 -b 0 -l 20 -B 512 -L 20 -s 0`

ARG seed 0
ARG address space size 128
ARG phys mem size 512

Segment register information:

  Segment 0 base  (grows positive) : 0x00000000 (decimal 0)
  Segment 0 limit                  : 20

  Segment 1 base  (grows negative) : 0x00000200 (decimal 512)
  Segment 1 limit                  : 20

seg0 (0, 19)
seg1 (493, 512)

Virtual Address Trace

VA  0: 0x0000006c (decimal:  108) --> PA or segmentation violation?
PA=
     ff
   0x200 (s1 base)
 - 0x014 (offset)
   0x1ec

VA  1: 0x00000061 (decimal:   97) --> PA or segmentation violation?

seg violation

VA  2: 0x00000035 (decimal:   53) --> PA or segmentation violation?

seg violation

VA  3: 0x00000021 (decimal:   33) --> PA or segmentation violation?

seg violation

VA  4: 0x00000041 (decimal:   65) --> PA or segmentation violation?

seg violation


`./segmentation.py -a 128 -p 512 -b 0 -l 20 -B 512 -L 20 -s 1`
ARG seed 1
ARG address space size 128
ARG phys mem size 512

Segment register information:

  Segment 0 base  (grows positive) : 0x00000000 (decimal 0)
  Segment 0 limit                  : 20

  Segment 1 base  (grows negative) : 0x00000200 (decimal 512)
  Segment 1 limit                  : 20

seg0: [0, 19]
seg1: [493, 512]

Virtual Address Trace
VA  0: 0x00000011 (decimal:   17) --> PA or segmentation violation?
seg 0
PA=

    0x00 (base)
   +0x11
    0x11

VA  1: 0x0000006c (decimal:  108) --> PA or segmentation violation?

Seg1

PA=

     ff
   0x200
-  0x014
    0x1ec


VA  2: 0x00000061 (decimal:   97) --> PA or segmentation violation?

Seg violation

VA  3: 0x00000020 (decimal:   32) --> PA or segmentation violation?

Seg violation

VA  4: 0x0000003f (decimal:   63) --> PA or segmentation violation?

Seg violation



`./segmentation.py -a 128 -p 512 -b 0 -l 20 -B 512 -L 20 -s 2`

ARG seed 2
ARG address space size 128
ARG phys mem size 512

Segment register information:

  Segment 0 base  (grows positive) : 0x00000000 (decimal 0)
  Segment 0 limit                  : 20

  Segment 1 base  (grows negative) : 0x00000200 (decimal 512)
  Segment 1 limit                  : 20

seg0: (0, 19)
seg1: (493, 512)

Virtual Address Trace
VA  0: 0x0000007a (decimal:  122) --> PA or segmentation violation?
PA=

      1f
   0x0200 (base)
-  0x0006 (offset)
   0x01fa

VA  1: 0x00000079 (decimal:  121) --> PA or segmentation violation?

PA= 0x01f9
(subtract 1 from the above, as the offset is 0x07 now)

VA  2: 0x00000007 (decimal:    7) --> PA or segmentation violation?

PA=
0x00000007

VA  3: 0x0000000a (decimal:   10) --> PA or segmentation violation?

PA=
0x0000000a

VA  4: 0x0000006a (decimal:  106) --> PA or segmentation violation?
seg violation




## 2. Now, let’s see if we understand this tiny address space we’ve constructed (using the parameters from the question above). What is the highest legal virtual address in segment 0? What about the lowest legal virtual address in segment 1? What are the lowest and highest illegal addresses in this entire address space? Finally, how would you run segmentation.py with the -a flag to test if you are right?

Highest legal virtual address in seg 0 is 19.

Lowest legal virtual address in seg 1 is 108.

Lowest illegal virtual address in space is 20.
Highest illegal virtual address in space is 107.


To test these answers, we can run with -a set to 129, and then 108 will be the highest illegal v address, and 109 the lowest legal v address in seg 1.

And if we set -a to 127, 106 will be highest illegal v address, with 107 the lowest legal v address in seg 1



`./segmentation.py -a 129 -p 512 -b 0 -l 20 -B 512 -L 20 -s 1`
`./segmentation.py -a 127 -p 512 -b 0 -l 20 -B 512 -L 20 -s 1`

`./segmentation.py -a 128 -p 512 -b 0 -l 20 -B 512 -L 20 -s 1 -A 19,108,20,107 -c`


## 3. Let’s say we have a tiny 16-byte address space in a 128-byte physical memory.  What base and bounds would you set up so as to get the simulator to generate the following translation results for the specified address stream: valid, valid, violation, ..., violation, valid,valid? Assume the following parameters:

`./segmentation.py -a 16 -p 128 -A 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15`

--b0 ?
--l0 ?
--b1 ?
--l1 ?

`./segmentation.py -a 16 -p 128 -A 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 --b0 0 --l0 2 --b1 15 --l1 2 -c`


## 4. Assume we want to generate a problem where roughly 90% of the randomly-generated virtual addresses are valid (not segmentation violations).  How should you configure the simulator to do so? Which parameters are important to getting this outcome?

We want 10% of the address space to be inaccessible by either segment. So it'll be important to set the -a, --b0, --l0, --b1, --l1 params accordingly.


here's an example:
`./segmentation.py -a 10 -p 128 --b0 0 --l0 4 --b1 9 --l1 5`

We have 10 bytes in the address space, but the byte at v address 4 is not available to either segment.


## 5. Can you run the simulator such that no virtual addresses are valid? How?

setting both limits to 0 prevents all v addresses from being valid.

`./segmentation.py -a 10 -p 128 --b0 0 --l0 0 --b1 9 --l1 0`

