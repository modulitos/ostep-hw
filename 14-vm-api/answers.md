1. First, write a simple program called `null.c` that creates a pointer to an integer, sets it to NULL, and then tries to dereference it. Compile this into an executable called null. What happens when you run this program?

> segmentation fault  ./null.o


2. Next, compile this program with symbol information included (with the -g flag).  Doing so let’s put more information into the executable, enabling the debugger to access more useful information about variable names and the like. Run the program under the debugger by typing `gdb null` and then, once gdb is running, typing run. What does gdb show you?

(used `lldb` because this was done on macos)

> * thread #1, queue = 'com.apple.main-thread', stop reason = EXC_BAD_ACCESS (code=1, address=0x0)


3. Finally, use the valgrind tool on this program. We’ll use the `memcheck` tool that is a part of valgrind to analyze what happens.  Run this by typing in the following: `valgrind --leak-check=yes null`. What happens when you run this? Can you interpret the output from the tool?

> valgrind --leak-check=yes null.o

running this statement:

```c
p = NULL;
```

causes valgrind to report the following error:

>=22905== 4 bytes in 1 blocks are definitely lost in loss record 1 of 38
>==22905==    at 0x100123545: malloc (in /usr/local/Cellar/valgrind/HEAD-6049595/lib/valgrind/vgpreload_memcheck-amd64-darwin.so)
>==22905==    by 0x100003F18: main (null.c:6)


dereferencing the null pointer results in the following error messages:

> ==23458== Invalid read of size 4
> ==23458==    at 0x100003F0C: main (null.c:11)
> ==23458==  Address 0x0 is not stack'd, malloc'd or (recently) free'd
> ==23458==
> ==23458==
> ==23458== Process terminating with default action of signal 11 (SIGSEGV)
> ==23458==  Access not within mapped region at address 0x0
> ==23458==    at 0x100003F0C: main (null.c:11)
> ==23458==  If you believe this happened as a result of a stack
> ==23458==  overflow in your program's main thread (unlikely but
> ==23458==  possible), you can try to increase the size of the
> ==23458==  main thread stack using the --main-stacksize= flag.
> ==23458==  The main thread stack size used in this run was 8388608.
> ==23458==


4. Write a simple program that allocates memory using malloc() but forgets to free it before exiting. What happens when this program runs? Can you use gdb to find any problems with it? How about valgrind (again with the `--leak-check=yes` flag)?

gdb (or lldb) doesn't catch the missing free.

But valgrind catches it:

>4 bytes in 1 blocks are definitely lost in loss record 1 of 38


5. Write a program that creates an array of integers called data of size 100 using malloc; then, set data[100] to zero. What happens when you run this program? What happens when you run this program using valgrind? Is the program correct?

The program seems to be working, but is not correct, because it is setting a value outside the bounds of the array. This results in *undefined behavior*. This means that almost anything can happen, and it won't always run correctly.

But valgrind reports:

>Invalid write of size 4
>Invalid read of size 4


6. Create a program that allocates an array of integers (as above), frees them, and then tries to print the value of one of the elements of the array. Does the program run? What happens when you use valgrind on it?

The program sometimes works, but usually returns gibberish when reading the freed array item.

valgrind reports:

> ==25372== Invalid read of size 4
> ==25372==    at 0x100003F26: main (array-of-ints.c:20)
> ==25372==  Address 0x10082492c is 396 bytes inside a block of size 400 free'd
> ==25372==    at 0x10012391D: free (in /usr/local/Cellar/valgrind/HEAD-6049595/lib/valgrind/vgpreload_memcheck-amd64-darwin.so)
> ==25372==    by 0x100003F21: main (array-of-ints.c:17)
> ==25372==  Block was alloc'd at
> ==25372==    at 0x100123545: malloc (in /usr/local/Cellar/valgrind/HEAD-6049595/lib/valgrind/vgpreload_memcheck-amd64-darwin.so)
> ==25372==    by 0x100003EE8: main (array-of-ints.c:6)



7. Now pass a funny value to free (e.g., a pointer in the middle of the array you allocated above). What happens? Do you need tools to find this type of problem?

When adding this:

>free((void *) &ints[50]);

We get a runtime error:

> ./array-of-ints.o
> malloc: *** error for object 0x7f89ef405b78: pointer being freed was not allocated

This is what lldb reports:

>thread #1, queue = 'com.apple.main-thread', stop reason = signal SIGABRT


8. Try out some of the other interfaces to memory allocation. For example, create a simple vector-like data structure and related routines that use `realloc()` to manage the vector. Use an array to store the vectors elements; when a user adds an entry to the vector, use `realloc()` to allocate more space for it. How well does such a vector perform? How does it compare to a linked list? Use valgrind to help you find bugs.


see `vector.h` and `vector.c`.


9. Spend more time and read about using gdb and valgrind. Knowing your tools is critical; spend the time and learn how to become an expert debugger in the UNIX and C environment.
