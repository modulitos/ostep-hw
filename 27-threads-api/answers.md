NOTE: `helgrind` only works on Linux! (the Macos version of valgrind is undefined for threads)

1. First build `main-race.c`. Examine the code so you can see the (hopefully obvious) data race in the code. Now run `helgrind` (by typing `valgrind --tool=helgrind main-race`) to see how it reports the race. Does it point to the right lines of code? What other information does it give to you?

Yes, it outputs 2 errors:
>==13484== ERROR SUMMARY: 2 errors from 2 contexts (suppressed: 0 from 0)

pointing to the right lines of code:

>==13484== Possible data race during read of size 4 at 0x10C04C by thread #1
>==13484== Locks held: none
>==13484==    at 0x1091E5: main (main-race.c:19)
>==13484==
>==13484== This conflicts with a previous write of size 4 by thread #2
>==13484== Locks held: none
>==13484==    at 0x10917A: worker (main-race.c:10)
>==13484==    by 0x4841886: mythread_wrapper (hg_intercepts.c:387)
>==13484==    by 0x48863E8: start_thread (in /usr/lib/libpthread-2.32.so)
>==13484==    by 0x499F292: clone (in /usr/lib/libc-2.32.so)
>==13484==  Address 0x10c04c is 0 bytes inside data symbol "balance"
>==13484==
>==13484== ----------------------------------------------------------------
>==13484==
>==13484== Possible data race during write of size 4 at 0x10C04C by thread #1
>==13484== Locks held: none
>==13484==    at 0x1091EE: main (main-race.c:19)
>==13484==
>==13484== This conflicts with a previous write of size 4 by thread #2
>==13484== Locks held: none
>==13484==    at 0x10917A: worker (main-race.c:10)
>==13484==    by 0x4841886: mythread_wrapper (hg_intercepts.c:387)
>==13484==    by 0x48863E8: start_thread (in /usr/lib/libpthread-2.32.so)
>==13484==    by 0x499F292: clone (in /usr/lib/libc-2.32.so)
>==13484==  Address 0x10c04c is 0 bytes inside data symbol "balance"

Line 19 is the second instance of `balance++`, and line 10 is the first `balance++` in the thread.


2. What happens when you remove one of the offending lines of code? Now add a lock around one of the updates to the shared variable, and then around both. What does `helgrind` report in each of these cases?

The detected errors are gone when one of the offending lines is removed.

When a global `pthread_mutex_t` is added, and the locks are added around it's usages, the error is gone


3. Now let’s look at `main-deadlock.c`. Examine the code. This code has a problem known as `deadlock` (which we discuss in much more depth in a forthcoming chapter). Can you see what problem it might have?

This program can suffer from deadlock because Thread 0 might have a lock on m2, and Thread 1 might have a lock on m1 at the same time. While Thread 0 waits for a lock on m1, and while Thread 1 waits for a lock on m2, the program is stuck indefinitely.


4. Now run `helgrind` on this code. What does `helgrind` report?

`helgrind` reports:

>==14074== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 7 from 7)

and

>==15727== Thread #3: lock order "0x10C080 before 0x10C0C0" violated

along with details about how the second forked thread (#3) is accessing the locks in the wrong order.

5. Now run `helgrind` on `main-deadlock-global.c`. Examine the code; does it have the same problem that `main-deadlock.c` has? Should `helgrind` be reporting the same error? What does this tell you about tools like `helgrind`?

We get the same error, but it **shouldn't** be reporting it because the outer lock, `g`, is preventing a deadlock from occurring. This means that `helgrind` is only erroring on specific anti-patterns, but doesn't prove when some deadlocks are unreachable.

6. Let’s next look at `main-signal.c`. This code uses a variable (done) to signal that the child is done and that the parent can now continue. Why is this code inefficient? (what does the parent end up spending its time doing, particularly if the child thread takes a long time to complete?)

It's not efficient because the main thread is spinning while waiting for `done` to update. This is a resource burden, and prevents the OS from overlapping the process with other processes/threads.

7. Now run `helgrind` on this program. What does it report? Is the code correct?

Lots of errors reported:
>==16621== ERROR SUMMARY: 24 errors from 3 contexts (suppressed: 46 from 46)

The code is correct, but `helgrind` is showing an error because we are mutating data between threads without having a lock.

8. Now look at a slightly modified version of the code, which is found in `main-signal-cv.c`. This version uses a condition variable to do the signaling (and associated lock). Why is this code preferred to the previous version? Is it correctness, or performance, or both?

This is preferred because of performance reasons since the condition `Pthread_cond_wait` puts the calling thread to sleep, waiting for the condition to signal it to wake up. When the condition is updated with `Pthread_cond_signal`, the thread is woken up. (note we're still using the while loop because some impls of pthread can cause the thread to spuriously wake up)

It's also preferred since it's less error prone, since we're using conditions, surrounded by locks, to synchronize the threads. Without the locks, it's much more difficult to keep the impl free of race conditions.

9. Once again run `helgrind` on `main-signal-cv`. Does it report any errors?

No errors!
