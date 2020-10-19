## 1. First run with the flags `-n 10 -H 0 -p BEST -s 0` to generate a few random allocations and frees. Can you predict what alloc()/free() will return? Can you guess the state of the free list after each request? What do you notice about the free list over time?

`./malloc.py -n 10 -H 0 -p BEST -s 0`

seed 0
size 100
baseAddr 1000
headerSize 0
alignment -1
policy BEST
listOrder ADDRSORT
coalesce False
numOps 10
range 10
percentAlloc 50
allocList
compute False

ptr[0] = Alloc(3) returned ?
returned 1003
List?
[size: 97, addr: 1003]


Free(ptr[0])
returned 0
List?
[addr: 1000, size: 3], [size: 97, addr: 1003]


ptr[1] = Alloc(5) returned ?
1003
List?
[addr: 1000, size: 3], [addr: 1008, size: 92]

Free(ptr[1])
returned 0
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1008, size: 92]

ptr[2] = Alloc(8) returned ?
1008
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1016, size: 84]


Free(ptr[2]) returned ?
0
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1008, size: 8], [addr: 1016, size: 84]

ptr[3] = Alloc(8) returned ?
1008
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1016, size: 84]

Free(ptr[3]) returned ?
0
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1008, size: 8], [addr: 1016, size: 84]

ptr[4] = Alloc(2) returned ?
1000
List?
[addr: 1002, size: 1], [addr: 1003, size 5], [addr: 1008, size: 8], [addr: 1016, size: 84]

ptr[5] = Alloc(7) returned ?
1008
List?
[addr: 1002, size: 1], [addr: 1003, size 5], [addr: 1015, size: 1], [addr: 1016, size: 84]




## 2. How are the results different when using a WORST fit policy tosearch the free list (-p WORST)? What changes?

Results below. We end up with more fragments, but those fragments tend to have a larger minimum space.

ptr[0] = Alloc(3) returned ?
returned 1003
List?
[size: 97, addr: 1003]


Free(ptr[0])
returned 0
List?
[addr: 1000, size: 3], [size: 97, addr: 1003]


ptr[1] = Alloc(5) returned ?
1003
List?
[addr: 1000, size: 3], [addr: 1008, size: 92]

Free(ptr[1])
returned 0
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1008, size: 92]

ptr[2] = Alloc(8) returned ?
1008
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1016, size: 84]


Free(ptr[2]) returned ?
0
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1008, size: 8], [addr: 1016, size: 84]

ptr[3] = Alloc(8) returned ?
1016
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1008, size: 8], [addr: 1024, size: 76]

Free(ptr[3]) returned ?
0
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1008, size: 8], [addr: 1016, size: 8], [addr: 1024, size: 84]

ptr[4] = Alloc(2) returned ?
1024
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1008, size: 8], [addr: 1016, size: 8], [addr: 1026, size: 84]

ptr[5] = Alloc(7) returned ?
1026
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1008, size: 8], [addr: 1016, size: 8], [addr: 1033, size: 84]

## 3. What about when using FIRST fit (-p FIRST)? What speeds up when you use first fit?

Faster search times with FIRST fit. In this case, it's the same results as BEST fit, but no exhaustive search was needed.

Results:

ptr[0] = Alloc(3) returned ?
returned 1003
List?
[size: 97, addr: 1003]


Free(ptr[0])
returned 0
List?
[addr: 1000, size: 3], [size: 97, addr: 1003]


ptr[1] = Alloc(5) returned ?
1003
List?
[addr: 1000, size: 3], [addr: 1008, size: 92]

Free(ptr[1])
returned 0
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1008, size: 92]

ptr[2] = Alloc(8) returned ?
1008
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1016, size: 84]


Free(ptr[2]) returned ?
0
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1008, size: 8], [addr: 1016, size: 84]

ptr[3] = Alloc(8) returned ?
1016
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1016, size: 84]

Free(ptr[3]) returned ?
0
List?
[addr: 1000, size: 3], [addr: 1003, size 5], [addr: 1008, size: 8], [addr: 1016, size: 84]

ptr[4] = Alloc(2) returned ?
1000
List?
[addr: 1002, size: 1], [addr: 1003, size 5], [addr: 1008, size: 8], [addr: 1016, size: 84]

ptr[5] = Alloc(7) returned ?
1008
List?
[addr: 1002, size: 1], [addr: 1003, size 5], [addr: 1015, size: 1], [addr: 1016, size: 84]



## 4. For the above questions, how the list is kept ordered can affect thetime it takes to find a free location for some of the policies. Use the different free list orderings (-l ADDRSORT,-l SIZESORT+,-l SIZESORT-) to see how the policies and the list orderings interact.


Using SIZESORT- with FIRST has a significant change.


## 5. Coalescing of a free list can be quite important. Increase the number of random allocations (say to -n 1000). What happens to larger allocation requests over time? Run with and without coalescing (i.e., without and with the -C flag). What differences in outcome do you see? How big is the free list over time in each case? Does the ordering of the list matter in this case?

With n of 1000, and FIRST, size is 51.
With coalescing, size is reduced to 1!

Using SIZESORT- with FIRST prevents the coalescing from happening, hardly at all!

## 6. What happens when you change the percent allocated fraction -P to higher than 50? What happens to allocations as it nears 100? What about as the percent nears 0?

The size of the list seems to peak with P around 40-60%. It drops at both ends.


## 7. What kind of specific requests can you make to generate a highly-fragmented free space? Use the -A flag to create fragmented free lists, and see how different policies and options change the organization of the free list.


Anything with SIZESORT- and FIRST will likely be highly fragmented.

>./malloc.py -c -A +10,-0,+20,-1,+30,-2,+40,-3 -p FIRST -l SIZESORT+
>./malloc.py -c -A +10,-0,+20,-1,+30,-2,+40,-3 -p FIRST -l SIZESORT+ -C
>./malloc.py -c -A +10,-0,+20,-1,+30,-2,+40,-3 -p FIRST -l SIZESORT-
>./malloc.py -c -A +10,-0,+20,-1,+30,-2,+40,-3 -p FIRST -l SIZESORT- -C
>./malloc.py -c -A +10,-0,+20,-1,+30,-2,+40,-3 -p WORST -l SIZESORT+
>./malloc.py -c -A +10,-0,+20,-1,+30,-2,+40,-3 -p WORST -l SIZESORT+ -C
>./malloc.py -c -A +10,-0,+20,-1,+30,-2,+40,-3 -p WORST -l SIZESORT-
>./malloc.py -c -A +10,-0,+20,-1,+30,-2,+40,-3 -p WORST -l SIZESORT- -C
