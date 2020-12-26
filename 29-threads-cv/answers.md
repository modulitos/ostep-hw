## 1. Our first question focuses on `main-two-cvs-while.c` (the working solution). First, study the code. Do you think you have an understanding of what should happen when you run the program?

Yes - this is a working solution because it uses a while loop along with a separate `empty` and `fill` condition variable.

We can configure the number of items each producer thread adds (ie the `loops` option), and the consumers will all run until the items have all been processed (ie until `END_OF_STREAM` is read by each consumer).


## 2. Run with one producer and one consumer, and have the producer produce a few values. Start with a buffer (size 1), and then increase it. How does the behavior of the code change with larger buffers? (or does it?) What would you predict `num_full` to be with different buffer sizes (e.g., `-m 10`) and different numbers of produced items (e.g., `-l 100`), when you change the consumer sleep string from default (no sleep) to `-C 0,0,0,0,0,0,1`?

With larger buffers, there will be fewer switches between our producer and consumer threads, since we'll be able to push/get more items to/from the shared buffer.

I expect `num_full` to be, at most, the lesser of `m` or `l`.

Adding sleep to c6 will add delay into the consumer threads, making the max of `num_fil` more likely to be reached.



## 3. If possible, run the code on different systems (e.g., a Mac andLinux). Do you see different behavior across these systems?

## 4. Let’s look at some timings. How long do you think the following execution, with one producer, three consumers, a single-entry shared buffer, and each consumer pausing at point `c3` for a second, will take? `./main-two-cvs-while.o -p 1 -c 3 -m 1 -C 0,0,0,1,0,0,0:0,0,0,1,0,0,0:0,0,0,1,0,0,0 -l 10 -v -t`

Should take a little over 10 seconds because the delay in `c3` causes us to only process 1 item at a time (ie `num_full` can only be 1 at the most). This prevents items from being parallelized.

(actually takes ~11 seconds since we're adding a second for processing the "end of stream" item.)

## 5. Now change the size of the shared buffer to 3 (-m 3). Will this make any difference in the total time?

Perhaps, but unlikely. `num_full` can be up to 3 if the producer is able to add lots of items to the queue, allowing us to process up to 3 items simultaneously, providing a time as fast as 3-4 seconds. But the delay at `c3` makes this unlikely to happen, because any consumer thread will spend 1 second spinning *while also holding the lock*, so we'll still typically see runtimes ~11 seconds.


## 6. Now change the location of the sleep to `c6` (this models a consumer taking something off the queue and then doing something with it), again using a single-entry buffer. What time do you predict in this case? `./main-two-cvs-while.o -p 1 -c 3 -m 1 -C 0,0,0,0,0,0,1:0,0,0,0,0,0,1:0,0,0,0,0,0,1 -l 10 -v -t`

Since the consumer threads are no longer sleeping while holding the lock, we should be able to process 3 items at a time.

So it should take ~5 seconds, as we're processing 10 items, plus 3 "end of signal" items (1 for each consumer thread), parallelizable by three, plus some extra time for contention ~= (10 + 3) / 3 + contention_time ~= 5 seconds.


## 7. Finally, change the buffer size to 3 again (-m 3). What time do you predict now?

Basically same as before, ~5 seconds - the size of the buffer has little effect because the number of consumer threads is driving the number of threads we can process at the same time.


## 8. Now let’s look at `main-one-cv-while.c`.  Can you configure a sleep string, assuming a single producer, one consumer, and a buffer of size 1, to cause a problem with this code?

With just one producer and one consumer, the implementation is correct.


## 9. Now change the number of consumers to two. Can you construct sleep strings for the producer and the consumers so as to cause a problem in the code?


`./main-one-cv-while.o -c 2 -p 1 -m 1 -v -l 1 -P 0,0,0,1,0,0,0`

(just like figure 30.11)



## 10. Now examine `main-two-cvs-if.c`. Can you cause a problem to happen in this code? Again consider the case where there is only one consumer, and then the case where there is more than one.

Is correct if 1 consumer.

With two consumers, we can hit an invalid assertion, just like in figure 30.9.

Here's an example timing:

`./main-two-cvs-if.o -c 2 -p 1 -m 1 -v -l 2 -C 0,0,1,0,0,0,1:0,0,1,0,0,0,1 -P 1,0,0,0,0,0,0`

giving us this error:

> error: tried to get an empty buffer

This can be avoided by always using `while` instead of if.


**NOTE: Unlike in figure 30.9, we're unable to mimic the case where a thread runs right after another thread is placed in the ready queue. Ideally, we can insert a pause before the `Cond_wait` returns, but after that thread is ready.


## 11. Finally, examine `main-two-cvs-while-extra-unlock.c`. What problem arises when you release the lock before doing a put or a get? Can you reliably cause such a problem to happen, given the sleep strings? What bad thing can happen?

There is a critical section in `do_get` and `do_fill` that can cause an error. I can't reliably make this happen, but here is an example of when this comes up:

> ❯ ./main-two-cvs-while-extra-unlock.o -p 1 -c 2 -m 10 -l 10 -v -C 0,0,0,0,1,0,0:0,0,0,0,0,0,0
>  NF                                                     P0 C0 C1
>   0 [*---  ---  ---  ---  ---  ---  ---  ---  ---  --- ] p0
>   0 [*---  ---  ---  ---  ---  ---  ---  ---  ---  --- ]    c0
>   0 [*---  ---  ---  ---  ---  ---  ---  ---  ---  --- ]       c0
>   0 [*---  ---  ---  ---  ---  ---  ---  ---  ---  --- ] p1
>   1 [u  0 f---  ---  ---  ---  ---  ---  ---  ---  --- ] p4
>   1 [u  0 f---  ---  ---  ---  ---  ---  ---  ---  --- ]    c1
>   1 [u  0 f---  ---  ---  ---  ---  ---  ---  ---  --- ]       c1
>   0 [ --- *---  ---  ---  ---  ---  ---  ---  ---  --- ]    c4
> error: tried to get an empty buffer


