## 1. First let’s make sure you understand how the programs generally work, and some of the key options. Study the code in `vector-deadlock.c`, as well as in `main-common.c` and related files. Now, run `./vector-deadlock -n 2 -l 1 -v`, which instantiates two threads (-n 2), each of which does one vector add (-l 1), and does so in verbose mode (-v). Make sure you understand the output. How does the output change from run to run?

This ` -n 2 -l 1 -v` configuration means that we'll never hit a deadlock, because there is no circular wait.

## 2. Now add the `-d` flag, and change the number of loops (-l) from 1 to higher numbers. What happens? Does the code (always) deadlock?

It makes deadlock possible because there is no longer a total ordering in the locks that are acquired between the src and dest vectors.

## 3. How does changing the number of threads (-n) change the outcome of the program? Are there any values of -n that ensure no deadlock occurs?

Increasing the number of threads will increase the odds of hitting deadlock, since there will be more opportunities for circular waits.

If -n is 1, then we can never get deadlock because there is only 1 thread, thus no circular waits.


## 4. Now examine the code in `vector-global-order.c`. First, make sure you understand what the code is trying to do; do you understand why the code avoids deadlock? Also, why is there a special case in this `vector_add()` routine when the source and destination vectors are the same?

It avoids deadlock by ensuring total ordering when the locks are acquired, thus no chances of a circular wait.

When the src/dest vecs are the same, then we must special case to avoid waiting on a mutex twice in a row, which would result in automatic deadlock.

## 5. Now run the code with the following flags: `-t -n 2 -l 100000 -d`.How long does the code take to complete? How does the total time change when you increase the number of loops, or the number of threads?

> Time: 0.07 seconds

Increasing the number of threads increases the time much more than increasing the number of loops. This is likely due to lock contention.

10xing the threads:
> ❯ ./cmake-build-debug/32_vector-global-order -t -n 10 -l 100000 -d
> Time: 1.33 seconds

10xing the loops:
>❯ ./cmake-build-debug/32_vector-global-order -t -n 2 -l 1000000 -d
>Time: 0.73 seconds


## 6. What happens if you turn on the parallelism flag (-p)? How much would you expect performance to change when each thread is working on adding different vectors (which is what `-p` enables) versus working on the same ones?

This should remove the lock contention between threads, allowing us to add threads with little/no cost.

10xing the threads (with `-p`):
> ❯ ./cmake-build-debug/32_vector-global-order -t -n 10 -l 100000 -d -p
> Time: 0.12 seconds

10xing the loops (with `-p`):
>❯ ./cmake-build-debug/32_vector-global-order -t -n 2 -l 1000000 -d -p
>Time: 0.32 seconds

Now 10xing the threads is _faster_ than 10xing the loops!


## 7. Now let’s study `vector-try-wait.c`. First make sure you understand the code. Is the first call to `pthread_mutex_trylock()` really needed? Now run the code. How fast does it run compared to the global order approach? How does the number of retries, as counted by the code, change as the number of threads increases?

The first call is not needed.

Increasing the number of threads will take longer because we'll hit more retries, and will take longer than the global order equivalent. Also opens us up to livelock issues (but unlikely to actually happen).

Number of retries increases as threads go up. Adding `-p` makes number of retries go to 0.

10xing the threads:
>❯ ./cmake-build-debug/32_vector-try-wait -t -n 10 -l 100000 -d
>Retries: 52637740
>Time: 6.00 seconds

10xing the loops:
>❯ ./cmake-build-debug/32_vector-try-wait -t -n 2 -l 1000000 -d
>Retries: 9739729
>Time: 1.41 seconds



## 8. Now let’s look at `vector-avoid-hold-and-wait.c`. What is the main problem with this approach? How does its performance compare to the other versions, when running both with `-p` and without it?

Perf will be worse than previous versions on parallel workflows, due to the global lock:

Without parallelism:
>❯ ./cmake-build-debug/32_vector-avoid-hold-and-wait -t -n 2 -l 1000000 -d
>Time: 1.14 seconds

With parallelism (this is the slowest impl):
> ❯ ./cmake-build-debug/32_vector-avoid-hold-and-wait -t -n 2 -l 1000000 -d -p
> Time: 0.61 seconds


global order, no parallelism:
>❯ ./cmake-build-debug/32_vector-global-order -t -n 2 -l 1000000 -d
>Time: 0.73 seconds


global order with `-p`:
>❯ ./cmake-build-debug/32_vector-global-order -t -n 2 -l 1000000 -d -p
>Time: 0.32 seconds

try wait, no parallelism:
>❯ ./cmake-build-debug/32_vector-try-wait -t -n 2 -l 1000000 -d
>Retries: 16624239
>Time: 1.82 seconds

try wait, with `-p`:
>❯ ./cmake-build-debug/32_vector-try-wait -t -n 2 -l 1000000 -d -p
>Retries: 0
>Time: 0.31 seconds



## 9. Finally, let’s look at `vector-nolock.c`. This version doesn't use locks at all; does it provide the exact same semantics as the other versions? Why or why not?

It's the same semantics/behavior, but relies on assembly to lock the addresses in memory instead of a mutex.



## 10. Now compare its performance to the other versions, both when threads are working on the same two vectors (no `-p`) and when each thread is working on separate vectors (-p). How does this no-lock version perform?


no parallel:
>❯ ./cmake-build-debug/32_vector-nolock -t -n 2 -l 1000000 -d
>Time: 2.59 seconds

parallel:
>❯ ./cmake-build-debug/32_vector-nolock -t -n 2 -l 1000000 -d -p
>Time: 0.89 seconds


I'm not sure why it's so much slower, but could be due to either hw optimization or some bottleneck in the assembly code.
