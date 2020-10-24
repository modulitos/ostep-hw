## 1. For timing, you’ll need to use a timer (e.g., gettimeofday()). How precise is such a timer? How long does an operation have to take in order for you to time it precisely? (this will help determine how many times, in a loop, you’ll have to repeat a page access in order to time it successfully)

gettimeofday mutates a timeval struct, which measures time in ms granularity. So timing an operation for a few seconds should be sufficient for us to measure it in ns.


## 2. Write the program, called `tlb.c`, that can roughly measure the cost of accessing each page. Inputs to the program should be: the number of pages to touch and the number of trials.

> ./tlb.o 2 900000000
3 ns

> ./tlb.o 3 900000000
2 ns

> ./tlb.o 4 900000000
2 ns

> ./tlb.o 5 900000000

2 ns

> ./tlb.o 6 900000000
2 ns

> ./tlb.o 7 900000000
2 ns

> ./tlb.o 8 900000000
2 ns

> ./tlb.o 16 900000000
5 ns

> ./tlb.o 32 900000000
9 ns

> ./tlb.o 64 900000000
11 ns

> ./tlb.o 128 900000000
10 ns

> ./tlb.o 256 50000
11 ns

> ./tlb.o 512 50000
10 ns

> ./tlb.o 1024 50000
10 ns

> ./tlb.o 2048 50000
10 ns

> ./tlb.o 4096 50000
10 ns

> ./tlb.o 8192 50000
12 ns

> ./tlb.o 16384 50000
15 ns

> ./tlb.o 32768 50000
18 ns

> ./tlb.o 65536 5000
22 ns

> ./tlb.o 131072 1000
21 ns

## 3. Now write a script in your favorite scripting language (bash?) to run this program, while varying the number of pages accessed from1 up to a few thousand, perhaps incrementing by a factor of two per iteration. Run the script on different machines and gathersomedata. How many trials are needed to get reliable measurements?


## 4. Next, graph the results, making a graph that looks similar to the one above. Use a good tool like ploticus or even zplot. Visualization usually makes the data much easier to digest; why do you think that is?

Done using ploticus (see attached png)


## 5. One thing to watch out for is compiler optimization. Compilers do all sorts of clever things, including removing loops which increment values that no other part of the program subsequently uses. How can you ensure the compiler does not remove the main loopabove from your TLB size estimator?

Using gcc's optimize option gcc -O0 to disable optimization. This is the default setting.

## 6. Another thing to watch out for is the fact that most systems today ship with multiple CPUs, and each CPU, of course, has its own TLB hierarchy. To really get good measurements, you have to run yourcode on just one CPU, instead of letting the scheduler bounce it from one CPU to the next. How can you do that? (hint: look up“pinning a thread” on Google for some clues) What will happen ifyou don’t do this, and the code moves from one CPU to the other?

On Linux:
> pthread_setaffinity_np()

but unfortunately, it's not possible to set processor affinity in Macos.

## 7. Another issue that might arise relates to initialization. If you don’t initialize the arraya above before accessing it, the first time you access it will be very expensive, due to initial access costs such as demand zeroing. Will this affect your code and its timing? What can you do to counterbalance these potential costs?

Use calloc, and make sure to allocate and initialize the array before the timer starts.
