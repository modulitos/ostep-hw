1. First build `main-race.c`. Examine the code so you can see the (hopefully obvious) data race in the code. Now run `helgrind` (by typing `valgrind --tool=helgrind main-race`) to see how it reports the race. Does it point to the right lines of code? What other information does it give to you?

Yes, it outputs a warning and points to the right lines of code:

>==28384== Possible data race during read of size 4 at 0x100008020 by thread #2
>==28384== Locks held: none
>==28384==    at 0x100003E1A: worker (main-race.c:8)
>==28384==    by 0x10062F108: _pthread_start (in /usr/lib/system/libsystem_pthread.dylib)
>==28384==    by 0x10062AB8A: thread_start (in /usr/lib/system/libsystem_pthread.dylib)
>==28384==
>==28384== This conflicts with a previous write of size 4 by thread #1
>==28384== Locks held: none
>==28384==    at 0x100003EAE: main (main-race.c:15)
>==28384==  Address 0x100008020 is in a rw- mapped file ./main-race.o segment
>==28384==
>==28384== ----------------------------------------------------------------
>==28384==
>==28384== Possible data race during write of size 4 at 0x100008020 by thread #2
>==28384== Locks held: none
>==28384==    at 0x100003E23: worker (main-race.c:8)
>==28384==    by 0x10062F108: _pthread_start (in /usr/lib/system/libsystem_pthread.dylib)
>==28384==    by 0x10062AB8A: thread_start (in /usr/lib/system/libsystem_pthread.dylib)
>==28384==
>==28384== This conflicts with a previous write of size 4 by thread #1
>==28384== Locks held: none
>==28384==    at 0x100003EAE: main (main-race.c:15)
>==28384==  Address 0x100008020 is in a rw- mapped file ./main-race.o segment
>==28384==


2. What happens when you remove one of the offending lines of code? Now add a lock around one of the updates to the shared variable, and then around both. What does `helgrind` report in each of these cases?

The detected error is gone when one of the lines of code is removed.

When a global `pthread_mutex_t` is added, and the locks are added around it's usages, the error is gone (`helgrind` with threads only works on Linux)


3. Now let’s look at `main-deadlock.c`. Examine the code. This code has a problem known as `deadlock` (which we discuss in much more depth in a forthcoming chapter). Can you see what problem it might have?

This program can suffer from deadlock because Thread 0 might have a lock on m2, and Thread 1 might have a lock on m1 at the same time. While Thread 0 waits for a lock on m1, and while Thread 1 waits for a lock on m2, the program is stuck indefinitely.


4. Now run `helgrind` on this code. What does `helgrind` report?


5. Now run `helgrind` on `main-deadlock-global.c`. Examine the code; does it have the same problem that `main-deadlock.c` has? Should `helgrind` be reporting the same error? What does this tell you about tools like `helgrind`?


6. Let’s next look at `main-signal.c`. This code uses a variable (done) to signal that the child is done and that the parent can now continue. Why is this code inefficient? (what does the parent end up spending its time doing, particularly if the child thread takes a long time to complete?)


7. Now run `helgrind` on this program. What does it report? Is the code correct?


8. Now look at a slightly modified version of the code, which is found in `main-signal-cv.c`. This version uses a condition variable to do the signaling (and associated lock). Why is this code preferred to the previous version? Is it correctness, or performance, or both?


9. Once again run `helgrind` on `main-signal-cv`. Does it report any errors?
