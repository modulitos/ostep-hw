## 1. First, open two separate terminal connections to thesamemachine, so thatyou can easily run something in one window and the other. Now, in one window, run `vmstat 1`, which shows statistics about machine usage every second. Read the man page, the associated README, and any other information you need so that you can understand its output. Leave this window running vmstat for the rest of the exercises below. Now, we will run the program `mem.c` but with very little memory usage.This can be accomplished by typing `./mem 1` (which uses only 1 MB of memory). How do the CPU usage statistics change when running `mem`? Do the numbers in the `user time` column make sense? How does this change when running more than one instance of `mem` at once?

When we start the `mem` program, the number of copy-on-write pages jumps up ~300-800. It also jumps up when the program is stopped.

(on Macos, using `iostat` to get "user time" and "idle time")

| running instances of `mem` |  us | sy |  id |
|----------------------------+-----+----+-----|
| 0 (start)                  |  6% | 6% | 92% |
| 1 (1 mem process)          | 32% | 4% | 68% |
| 2 (2 mem processes)        | 52% | 4% | 42% |



## 2. Let’s now start looking at some of the memory statistics while running mem.We’ll focus on two columns: `swpd` (the amount of virtual memory used) and `free` (the amount of idle memory). Run./mem 1024 (which allocates 1024 MB) and watch how these values change. Then kill the running program (by typing control-c) and watch again how the values change. What do you notice about the values? In particular, how does the free column change when the program exits? Does the amount of free memory increase by the expected amount when `mem` exits?

before:
free: 279 K
swpd: 1.2 M

during:
free: 8 K
swpd: 1.4 M

after:
free: 300 K
swpd: 1.2 M

The amount of free memory increases a bit more than what was allocated to the `mem` process.


## 3. We’ll next look at the swap columns (`si` and `so`), which indicate how much swapping is taking place to and from the disk. Of course, to activate these, you’ll need to run `mem` with large amounts of memory. First, examine how much free memory is on your Linux system (for example, by typing `cat/proc/meminfo`; type `man proc` for details on the `/proc` file system and the types of information you can find there).  One of the first entries in/proc/meminfois the total amount of memory in your system. Let’s as-sume it’s something like 8 GB of memory; if so, start by runningmem 4000(about 4 GB) and watching the swap in/out columns. Do they ever give non-zero values? Then, try with 5000,6000, etc. What happens to these values as the program enters the second loop (and beyond), as compared to the first loop? How much data (total) are swapped in and out during the second, third, and subsequent loops? (do the numbers make sense?)

installed memory: ~16 GB
total consumed memory: ~13.5 GB (wired down + active + inactive)
=> Free memory: ~2.5 GB

./mem 1000
./mem 2000
=> no change in swap

./mem 4000
swpouts: 123 K
swpins: none

This makes sense, as the size of swpout of 123K ~= 123K pages * 4KB/pg = 600MB
Note that swpouts are *compressed* pages, so the are likely smaller than 4KB each.

./mem 6000
swpout: 88K, then 58K

Only happens in the first loop, then drops to 0.


## 4. Do the same experiments as above, but now watch the other statistics (such as CPU utilization, and block I/O statistics). How do they change when `mem` is running?


User time and system time % usage spikes, and % idle drops.

tps (transfers per second), KB/t (KB per transfer), and MB/s spike as well.

Drops back down after the first loop or so.


## 5. Now let’s examine performance. Pick an input for `mem` that comfortably fits in memory (say4000 if the amount of memory on the system is 8 GB). How long does loop 0 take (and subsequent loops 1, 2, etc.)? Now pick a size comfortably beyond the size of memory (say 12000 again assuming 8 GB of memory). How long do the loops take here? How do the bandwidth numbers compare? How different is performance when constantly swapping versus fitting everything comfortably in memory? Can you make a graph, with the size of memory used by mem on the x-axis, and the bandwidth of accessing said memory on the y-axis? Finally, how does the performance of the first loop compare to that of subsequent loops, for both the case where everything fits in memory and where it doesn't?

Within our memory limits:
> ❯ ./mem.o 8000
> allocating 8388608000 bytes (8000.00 MB)
>   number of integers in array: 2097152000
> loop 0 in 5471.90 ms (bandwidth: 1462.02 MB/s)
> loop 1 in 6276.57 ms (bandwidth: 1274.58 MB/s)
> loop 2 in 5920.45 ms (bandwidth: 1351.25 MB/s)
> loop 3 in 1732.44 ms (bandwidth: 4617.75 MB/s)
> loop 4 in 1459.00 ms (bandwidth: 5483.21 MB/s)
> loop 5 in 1445.47 ms (bandwidth: 5534.54 MB/s)

Beyond our memory limits:

> ❯ ./mem.o 20000
> allocating 20971520000 bytes (20000.00 MB)
>   number of integers in array: 5242880000
> loop 0 in 14605.34 ms (bandwidth: 1369.36 MB/s)
> loop 1 in 17605.82 ms (bandwidth: 1135.99 MB/s)
> loop 2 in 16839.42 ms (bandwidth: 1187.69 MB/s)
> loop 3 in 18345.81 ms (bandwidth: 1090.17 MB/s)
> loop 4 in 17399.74 ms (bandwidth: 1149.44 MB/s)
> loop 5 in 18981.12 ms (bandwidth: 1053.68 MB/s)
> loop 6 in 17481.26 ms (bandwidth: 1144.08 MB/s)


When it all fits in memory, performance starts to increase once the entire array is loaded into memory.

If it doesn't fit in memory, then the performance never improves, because it's constantly swapping.


## 6. Swap space isn't infinite. You can use the tool `swapon` with the `-s` flag to see how much swap space is available. What happens if you try to run `mem` with increasingly large values, beyond what seems to be available in swap? At what point does the memory allocation fail?

on Macos, to get swap usage:
> sysctl vm.swapusage
> vm.swapusage: total = 5120.00M  used = 4168.50M  free = 951.50M  (encrypted)
(about 5GB)

Instead of failing, allocation seem to hand forever...
(eg: `./mem 60000`)

Perhaps Mac just uses extra hard drive space when the swap is exceeded?

## 7. Finally, if you’re advanced, you can configure your system to use different swap devices using `swapon` and `swapoff`. Read the man pages for details. If you have access to different hardware, see how the performance of swapping changes when swapping to a classic hard drive, a flash-based SSD, and even a RAID array. How much can swapping performance be improved via newer devices? How close can you get to in-memory performance?
