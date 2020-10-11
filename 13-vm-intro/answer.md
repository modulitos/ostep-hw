1. The first Linux tool you should check out is the very simple toolfree. First, typeman freeand read its entire manual page; it’s short, don’t worry!

(using vm_stat on macos, which shows memory in KB)
this script helps:
https://gist.github.com/vigo/1690045


2. Now, runfree, perhaps using some of the arguments that mightbe useful (e.g.,-m, to display memory totals in megabytes). Howmuch memory is in your system? How much is free? Do thesenumbers match your intuition?

Installed memory: 16.38G
Free memory: 2.28G


3. Next, create a little program that uses a certain amount of memory, called `memory-user.c`. This program should take one commandline argument: the number of megabytes of memory it will use. When run, it should allocate an array, and constantly stream through the array, touching each entry. The program should do this indefinitely, or, perhaps, for a certain amount of time also specified at the command line.


4. Now, while running your memory-user program, also (in a different terminal window, but on the same machine) run the `free` tool. How do the memory usage totals change when your program is running? How about when you kill the memory-user program? Do the numbers match your expectations?  Try this for different amounts of memory usage. What happens when you use really large amounts of memory?

When the program runs, the amount of free memory is reduced, presumably because it is being allocated to the process by the OS. When the program is stopped, the memory is free and added to the free memory in the OS. This matches my expectations.

For very large amounts of memory, the program allocates more memory that was not previously free. Eg:

starting memory:
> ❯ free_macos
> Installed memory: 16.38G
> Free memory: 3.78G

run the program, consuming 3GB for 5 seconds:
> ❯ ./memory-user.o 3000 5

while running:
> ❯ free_macos
> Installed memory: 16.38G
> Free memory: 1.68G

(note how there is still free memory left!)

Once the program has stopped:
> ❯ free_macos
> Installed memory: 16.38G
> Free memory: 4.86G

It seems like an extra GB was free by the OS to accommodate our process!


Also, when the program is run with more memory than exists on the system, we get:

> segmentation fault  ./memory-user.o 20000 5


5. Let’s try one more tool, known as `pmap`. Spend some time, and readthepmapmanual page in detail.

(on macos, it seems like the `pmap` equivalent is `vmmap`)


6. To use `pmap`, you have to know theprocess IDof the process you’reinterested in. Thus, first runps auxwto see a list of all processes;then, pick an interesting one, such as a browser. You can also use your memory-user program in this case (indeed, you can even have that program call getpid() and print out its PID for your`convenience).

vmmap <pid>



7. Now run `pmap` on some of these processes, using various flags (like-X) to reveal many details about the process. What do you see? How many different entities make up a modern address space, as opposed to our simple conception of code/stack/heap?

There are LOTS of regions, like the memory maps of the process, and other C libraries. Here's a quick summary of a `vmmap` call on the pid of `./memeory-user.o 200 10` (200MB, for 10 seconds):

> ==== Summary for process 43037
> ReadOnly portion of Libraries: Total=393.4M resident=100.0M(25%) swapped_out_or_unallocated=293.4M(75%)
> Writable regions: Total=217.3M written=200.1M(92%) resident=200.1M(92%) swapped_out=0K(0%) unallocated=17.1M(8%)
>
>                                 VIRTUAL RESIDENT    DIRTY  SWAPPED VOLATILE   NONVOL    EMPTY   REGION
> REGION TYPE                        SIZE     SIZE     SIZE     SIZE     SIZE     SIZE     SIZE    COUNT (non-coalesced)
> ===========                     ======= ========    =====  ======= ========   ======    =====  =======
> Kernel Alloc Once                    8K       4K       4K       0K       0K       0K       0K        1
> MALLOC guard page                   16K       0K       0K       0K       0K       0K       0K        4
> MALLOC metadata                     44K      36K      36K       0K       0K       0K       0K        5
> MALLOC_LARGE                     200.0M   200.0M   200.0M       0K       0K       0K       0K        2         see MALLOC ZONE table below
> MALLOC_LARGE metadata                4K       4K       4K       0K       0K       0K       0K        1         see MALLOC ZONE table below
> MALLOC_SMALL                      8192K       8K       8K       0K       0K       0K       0K        1         see MALLOC ZONE table below
> MALLOC_TINY                       1024K      16K      16K       0K       0K       0K       0K        1         see MALLOC ZONE table below
> STACK GUARD                       56.0M       0K       0K       0K       0K       0K       0K        1
> Stack                             8192K      20K      20K       0K       0K       0K       0K        1
> __DATA                             614K     362K     166K       0K       0K       0K       0K       41
> __DATA_CONST                        36K      36K      24K       0K       0K       0K       0K        2
> __LINKEDIT                       388.8M    96.0M       0K       0K       0K       0K       0K        4
> __OBJC_RO                         32.3M    19.2M       0K       0K       0K       0K       0K        1
> __OBJC_RW                         1908K    1048K       4K       0K       0K       0K       0K        2
> __TEXT                            4764K    4072K       0K       0K       0K       0K       0K       40
> unused but dirty shlib __DATA      2502     2502     2502       0K       0K       0K       0K       11
> ===========                     ======= ========    =====  ======= ========   ======    =====  =======
> TOTAL                            701.3M   320.8M   200.3M       0K       0K       0K       0K      118
>
>                                  VIRTUAL   RESIDENT      DIRTY    SWAPPED ALLOCATION      BYTES DIRTY+SWAP          REGION
> MALLOC ZONE                         SIZE       SIZE       SIZE       SIZE      COUNT  ALLOCATED  FRAG SIZE  % FRAG   COUNT
> ===========                      =======  =========  =========  =========  =========  =========  =========  ======  ======
> DefaultMallocZone_0x104b20000     209.0M     200.0M     200.0M         0K        156     200.0M        16K      1%       5



8. Finally, let’s runpmapon your `memory-user` program, with different amounts of used memory. What do you see here? Does the output from `pmap` match your expectations?

for 500 MB, here's the output:

> ==== Summary for process 43141
> ReadOnly portion of Libraries: Total=393.4M resident=100.0M(25%) swapped_out_or_unallocated=293.4M(75%)
> Writable regions: Total=517.3M written=500.1M(97%) resident=500.1M(97%) swapped_out=0K(0%) unallocated=17.1M(3%)
>
>                                 VIRTUAL RESIDENT    DIRTY  SWAPPED VOLATILE   NONVOL    EMPTY   REGION
> REGION TYPE                        SIZE     SIZE     SIZE     SIZE     SIZE     SIZE     SIZE    COUNT (non-coalesced)
> ===========                     ======= ========    =====  ======= ========   ======    =====  =======
> Kernel Alloc Once                    8K       4K       4K       0K       0K       0K       0K        1
> MALLOC guard page                   16K       0K       0K       0K       0K       0K       0K        4
> MALLOC metadata                     44K      36K      36K       0K       0K       0K       0K        5
> MALLOC_LARGE                     500.0M   500.0M   500.0M       0K       0K       0K       0K        4         see MALLOC ZONE table below
> MALLOC_LARGE metadata                4K       4K       4K       0K       0K       0K       0K        1         see MALLOC ZONE table below
> MALLOC_SMALL                      8192K       8K       8K       0K       0K       0K       0K        1         see MALLOC ZONE table below
> MALLOC_TINY                       1024K      16K      16K       0K       0K       0K       0K        1         see MALLOC ZONE table below
> STACK GUARD                       56.0M       0K       0K       0K       0K       0K       0K        1
> Stack                             8192K      20K      20K       0K       0K       0K       0K        1
> __DATA                             614K     362K     166K       0K       0K       0K       0K       41
> __DATA_CONST                        36K      36K      24K       0K       0K       0K       0K        2
> __LINKEDIT                       388.8M    96.0M       0K       0K       0K       0K       0K        4
> __OBJC_RO                         32.3M    19.2M       0K       0K       0K       0K       0K        1
> __OBJC_RW                         1908K    1048K       4K       0K       0K       0K       0K        2
> __TEXT                            4764K    4072K       0K       0K       0K       0K       0K       40
> unused but dirty shlib __DATA      2502     2502     2502       0K       0K       0K       0K       11
> ===========                     ======= ========    =====  ======= ========   ======    =====  =======
> TOTAL                              1.0G   620.8M   500.3M       0K       0K       0K       0K      120
>
>                                  VIRTUAL   RESIDENT      DIRTY    SWAPPED ALLOCATION      BYTES DIRTY+SWAP          REGION
> MALLOC ZONE                         SIZE       SIZE       SIZE       SIZE      COUNT  ALLOCATED  FRAG SIZE  % FRAG   COUNT
> ===========                      =======  =========  =========  =========  =========  =========  =========  ======  ======
> DefaultMallocZone_0x10f692000     509.0M     500.0M     500.0M         0K        156     500.0M        16K      1%       7

It looks like all of the regions are roughly the same as when calling our program with 200MB of memory, except for `MALLOC_LARGE`, which allocates 500MB instead of 200MB.
