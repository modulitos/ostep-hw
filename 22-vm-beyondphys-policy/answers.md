## 1. Generate random addresses with the following arguments: `-s 0 -n 10`,`-s 1 -n 10`, and `-s 2 -n 10`. Change the policy from FIFO, to LRU, to OPT. Compute whether each access in said address traces are hits or misses.

default cache size: 3
FIFO:

Access: 8 miss [8]
Access: 7 miss [8,7]
Access: 4 miss [8,7,4]
Access: 2 miss [7, 4, 2]
Access: 5 miss [4, 2, 5]
Access: 4 hit
Access: 7 miss [2, 5, 7]
Access: 3 miss [5, 7, 3]
Access: 4 miss [7, 3, 4]
Access: 5 miss [3, 4, 5]

LRU:

Access: 8 miss [8]
Access: 7 miss [8, 7]
Access: 4 miss [8, 7, 4]
Access: 2 miss [7, 4, 2]
Access: 5 miss [4, 2, 5]
Access: 4 hit  [2, 5, 4]
Access: 7 miss [5, 4, 7]
Access: 3 miss [4, 7, 3]
Access: 4 hit  [7, 3, 4]
Access: 5 miss [3, 4, 5]

OPT:

Access: 8 miss [8]
Access: 7 miss [8, 7]
Access: 4 miss [8, 7, 4]
Access: 2 miss [7, 4, 2]
Access: 5 miss [7, 4, 5]
Access: 4 hit
Access: 7 hit
Access: 3 miss [4, 5]
Access: 4 hit
Access: 5 hit


## 2. For a cache of size 5, generate worst-case address reference streams for each of the following policies: FIFO, LRU, and MRU (worst-case reference streams cause the most misses possible. For the worst case reference streams, how much bigger of a cache is needed to improve performance dramatically and approach OPT?

FIFO:
[1,2,3,4,5,6,1,2,3,4,5,6,1,2,3,4,5,6,...]
(a looping workload)
Increasing cache size by 1 will make all of these cache hits

LRU:
same as FIFO

MRU:
[1,2,3,4,5,6,5,6,5,6,5,6,5,6,5,...]
Increasing cache size by 1 will make all of these cache hits


## 3. Generate a random trace (use python or perl). How would you expect the different policies to perform on such a trace?

All policies will perform equally poorly, on average, because there is no locality to take advantage of.

OPT:
./paging-policy.py -s 0 -n 10 --policy=OPT -c
FINALSTATS hits 4   misses 6   hitrate 40.00

LRU:
./paging-policy.py -s 0 -n 10 --policy=LRU -c
FINALSTATS hits 2   misses 8   hitrate 20.00

FIFO:
./paging-policy.py -s 0 -n 10 --policy=FIFO -c
FINALSTATS hits 1   misses 9   hitrate 10.00

RAND:
./paging-policy.py -s 0 -n 10 --policy=RAND -c
FINALSTATS hits 0   misses 10   hitrate 0.00

CLOCK:
./paging-policy.py -s 0 -n 10 --policy=CLOCK -c
FINALSTATS hits 1   misses 9   hitrate 10.00

## 4. Now generate a trace with some locality. How can you generate such a trace? How does LRU perform on it? How much better than RAND is LRU? How does CLOCK do? How about CLOCK with different numbers of clock bits?

We can generate a trace by introducing temporal locality.

LRU is significantly better than RAND, and CLOCK is better than RAND, but not better than LRU since it only approximates it.


## 5. Use a program like valgrind to instrument a real application and generate a virtual page reference stream.  For example, running `valgrind --tool=lackey --trace-mem=yes ls` will output a nearly-complete reference trace of every instruction and data reference made by the program `ls`. To make this useful for the simulator above, you’ll have to first transform each virtual memory reference into a virtual page-number reference (done by masking off the offset and shifting the resulting bits downward). How big of a cache is needed for your application trace in order to satisfy a large fraction of requests? Plot a graph of its working set as the size of the cache increases.

details about `--trace-mem=yes`:
https://sourceware.org/git/?p=valgrind.git;a=blob;f=lackey/lk_main.c;h=1bcd8ed566edc0fdf68c2efcb797753a4fd8bb39;hb=HEAD#l55

>$ valgrind --tool=lackey --trace-mem=yes ls &> ls-trace.txt
>// Count to 3, ^C
>
>$ ./transform.py    // transform to VPN
>$ ./run.sh          // get some data
>$ # fill in the hit_rates array.
>$ ./plot.py

