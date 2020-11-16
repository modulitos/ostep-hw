## 1. Examine `flag.s`. This code "implements" locking with a single memory flag. Can you understand the assembly?

It saves the `ax` register to the value of `flag`, and spins until it's free (has a value of 0).
Once the flag is retrieved, it enters the critical section to increment the count.

Then it releases the lock, setting the value of `flag` back to 0.

It loops for `bx` iterations.


## 2. When you run with the defaults, does `flag.s` work? Use the `-M` and `-R` flags to trace variables and registers (and turn on `-c` to see their values). Can you predict what value will end up in `flag`?

Yes, `flag.s` works with the defaults. Final value of `flag` will be 0.

>  flag   ax    bx          Thread 0                Thread 1
>
>     0    0     0
>     0    0     0   1000 mov  flag, %ax
>     0    0     0   1001 test $0, %ax
>     0    0     0   1002 jne  .acquire
>     1    0     0   1003 mov  $1, flag
>     1    0     0   1004 mov  count, %ax
>     1    1     0   1005 add  $1, %ax
>     1    1     0   1006 mov  %ax, count
>     0    1     0   1007 mov  $0, flag
>     0    1    -1   1008 sub  $1, %bx
>     0    1    -1   1009 test $0, %bx
>     0    1    -1   1010 jgt .top
>     0    1    -1   1011 halt
>     0    0     0   ----- Halt;Switch -----  ----- Halt;Switch -----
>     0    0     0                            1000 mov  flag, %ax
>     0    0     0                            1001 test $0, %ax
>     0    0     0                            1002 jne  .acquire
>     1    0     0                            1003 mov  $1, flag
>     1    1     0                            1004 mov  count, %ax
>     1    2     0                            1005 add  $1, %ax
>     1    2     0                            1006 mov  %ax, count
>     0    2     0                            1007 mov  $0, flag
>     0    2    -1                            1008 sub  $1, %bx
>     0    2    -1                            1009 test $0, %bx
>     0    2    -1                            1010 jgt .top
>     0    2    -1                            1011 halt



## 3. Change the value of the register `%bx` with the `-a` flag (e.g., `-a bx=2,bx=2` if you are running just two threads). What does the code do? How does it change your answer for the question above?

That means we'll run each loop twice. Since the default interrupt frequency is 50, I don't think it'll cause an interrupt in the flag...

`flag` will be 0 still, as we haven't encountered the concurrency bug. `count` will end up being 4.


## 4. Set `bx` to a high value for each thread, and then use the `-i` flag to generate different interrupt frequencies; what values lead to a bad outcomes? Which lead to good outcomes?

If we encounter an interrupt after the `jne .acquire` instruction, but before the `mov $1, flag` in T0, while T1 also interrupts, then when both threads resume, they'll both set the value of `flag` to 1.

> ./x86.py -p flag.s -R ax,bx -M flag,count -i 3 -c

## 5. Now let’s look at the program `test-and-set.s`. First, try to understand the code, which uses the `xchg` instruction to build a simple locking primitive. How is the lock acquire written? How about lock release?

The lock acquire is written just like `flag.s`, but we're atomically acquiring the lock, instead of checking its value then setting it in separate steps.

Lock release is the same, since it's atomic in both cases.

## 6. Now run the code, changing the value of the interrupt interval(`-i`) again,and making sure to loop for a number of times. Does the code always work as expected? Does it sometimes lead to an inefficient use of the CPU? How could you quantify that?

The code is correct and always works as expected.

> ./x86.py -p test-and-set.s -R ax,bx -M mutex,count -a bx=3,bx=3 -i 2 -c

It can have inefficient CPU since it's a spin lock. We can quantify that by counting the number of cycles, or full timee slices, that a thread is running while waiting on a lock.

## 7. Use the `-P` flag to generate specific tests of the locking code. For example, run a schedule that grabs the lock in the first thread, but then tries to acquire it in the second. Does the right thing happen? What else should you test?


> ./x86.py -p test-and-set.s -R ax,bx -M mutex,count -P 0011111111111111111 -c

Yes, the "right" thing happens in that the second thread is waiting on the lock. But we should also test fairness and performance. (the ability for a thread to yield while it's waiting on a lock, and not starve other threads)


## 8. Now let’s look at the code in `peterson.s`, which implements Peterson’salgorithm (mentioned in a sidebar in the text). Study the code and see if you can make sense of it.



## 9. Now run the code with different values of `-i`. What kinds of different behavior do you see? Make sure to set the thread IDs appropriately (using `-a bx=0,bx=1` for example) as the code assumes it.

> ./x86.py -p peterson.s -a bx=0,bx=1 -R cx,bx -M flag,turn

The `turn` and the `flag` are being set independently of each other. Each thread starts by acquiring it's thread-specific flag without consulting other threads, and then sets the `turn` to the other thread. Then it starts spinning, exiting if:
 1. the other thread doesn't have its flag set
 2. or if it does, but the running thread has the `turn`


## 10. Can you control the scheduling (with the `-P` flag) to “prove” that the code works? What are the different cases you should show hold? Think about mutual exclusion and deadlock avoidance.

When T0 acquires the T0 flag (after 5 instructions), then switches to T1 (which has already initialized after 6 instructions), T1 gets stuck spinning because T0 hasn't given the turn to T1:

> ./x86.py -p peterson.s -a bx=0,bx=1 -R cx,bx -M flag,turn -P 1111110000011111111111111111111111111111111111111111111111111111111111111111111111111111 -c

When T0 acquires the T0 flag _and_ gives the turn back to T1 (after 6 instructions), T1 is no longer stuck spinning:

> ./x86.py -p peterson.s -a bx=0,bx=1 -R cx,bx -M flag,turn -P 11111100000011111111111111111111111111111111111111111111111111111111111111111111111111111 -c


## 11. Now study the code for the ticket lock in `ticket.s`. Does it match the code in the chapter? Then run with the following flags: `-a bx=1000,bx=1000` (causing each thread to loop through the critical section 1000 times). Watch what happens; do the threads spend much time spin-waiting for the lock?

Yes, it matches to code in the chapter.

> ./x86.py -p ticket.s -a bx=1000,bx=1000 -R ax,bx -M ticket,turn,count -c

Yes, the threads spend lots of time spin waiting on the locks.

It starts out very efficient, but as soon as we encounter an interrupt in a thread that has taken a ticket, but hasn't given up its turn, we get stuck in an inefficient cycle.

Since each thread can only process the ticket that it owns (`turn` value), another thread will have to wait for that ticket to be processed before it can process that are higher than that `turn` value. This basically limits each thread to processing only one ticket per time slice.


## 12. How does the code behave as you add more threads?

As we add more threads, the code gets more inefficient since we have to wait on 2 threads to take their turn before our thread can process its tickets. More spinning in the tryagain loop.


## 13. Now examine `yield.s`, in which a `yield` instruction enables one thread to yield control of the CPU (realistically, this would be an OS primitive, but for the sake of simplicity, we assume an instruction does the task). Find a scenario where `test-and-set.s` wastes cycles spinning, but `yield.s` does not. How many instructions are saved? In what scenarios do these savings arise?

When one thread is waiting on the lock, and the lock won't be given up for some time, then `yield` will save us from spinning for the entire length of the time slice.

> ./x86.py -p test-and-set.s -M count,mutex -R ax,bx -a bx=5,bx=5 -c -i 7
> ./x86.py -p yield.s -M count,mutex -R ax,bx -a bx=5,bx=5 -c -i 7

## 14. Finally, examine `test-and-test-and-set.s`. What does this lock do? What kind of savings does it introduce as compared to `test-and-set.s`?

It changes the mutex to 1 only if the lock is free, which will avoid unnecessary writes.
