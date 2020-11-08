1. Let’s examine a simple program, “loop.s”. First, just read and understand it. Then, run it with these arguments (`./x86.py -p loop.s -t 1 -i 100 -R dx`) This specifies a single thread, an interrupt every 100 instructions, and tracing of register `%dx`. What will `%dx` be during the run? Use the `-c` flag to check your answers; the answers, on the left, show the value of the register (or memory value) after the instruction on the right has run.

>   dx          Thread 0
>    0
>    -1   1000 sub  $1,%dx
>    -1   1001 test $0,%dx
>    -1   1002 jgte .top
>    -1   1003 halt

2. Same code, different flags: (`./x86.py -p loop.s -t 2 -i 100 -a dx=3,dx=3 -R dx`) This specifies two threads, and initializes each `%dx` to 3. What values will `%dx` see? Run with `-c` to check. Does the presence of multiple threads affect your calculations? Is there a race in this code?

>   dx          Thread 0                Thread 1
>    3
>    2   1000 sub  $1,%dx
>    2   1001 test $0,%dx
>    2   1002 jgte .top
>    1   1000 sub  $1,%dx
>    1   1001 test $0,%dx
>    1   1002 jgte .top
>    0   1000 sub  $1,%dx
>    0   1001 test $0,%dx
>    0   1002 jgte .top
>   -1   1000 sub  $1,%dx
>   -1   1001 test $0,%dx
>   -1   1002 jgte .top
>   -1   1003 halt
>    3   ----- Halt;Switch -----  ----- Halt;Switch -----
>    2                            1000 sub  $1,%dx
>    2                            1001 test $0,%dx
>    2                            1002 jgte .top
>    1                            1000 sub  $1,%dx
>    1                            1001 test $0,%dx
>    1                            1002 jgte .top
>    0                            1000 sub  $1,%dx
>    0                            1001 test $0,%dx
>    0                            1002 jgte .top
>   -1                            1000 sub  $1,%dx
>   -1                            1001 test $0,%dx
>   -1                            1002 jgte .top
>   -1                            1003 halt

<!-- Multiple threads does not affect calculations in this case because the interrupt is too long. But there is a race condition still, as the sub-test-jgte statements contain a critical section. -->

Multiple threads do not affect the calculations because there is no shared state, nor critical section; thus no data race.



3. Run this: `./x86.py -p loop.s -t 2 -i 3 -r -a dx=3,dx=3 -R dx` This makes the interrupt interval small/random; use different seeds (-s) to see different interleavings. Does the interrupt frequency change anything?

>   dx          Thread 0                Thread 1
>    3
>    2   1000 sub  $1,%dx
>    2   1001 test $0,%dx
>    2   1002 jgte .top
>    3   ------ Interrupt ------  ------ Interrupt ------
>    2                            1000 sub  $1,%dx
>    2                            1001 test $0,%dx
>    2                            1002 jgte .top
>    2   ------ Interrupt ------  ------ Interrupt ------
>    1   1000 sub  $1,%dx
>    1   1001 test $0,%dx
>    2   ------ Interrupt ------  ------ Interrupt ------
>    1                            1000 sub  $1,%dx
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1   1002 jgte .top
>    0   1000 sub  $1,%dx
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1                            1001 test $0,%dx
>    1                            1002 jgte .top
>    0   ------ Interrupt ------  ------ Interrupt ------
>    0   1001 test $0,%dx
>    0   1002 jgte .top
>   -1   1000 sub  $1,%dx
>    1   ------ Interrupt ------  ------ Interrupt ------
>    0                            1000 sub  $1,%dx
>   -1   ------ Interrupt ------  ------ Interrupt ------
>   -1   1001 test $0,%dx
>   -1   1002 jgte .top
>    0   ------ Interrupt ------  ------ Interrupt ------
>    0                            1001 test $0,%dx
>    0                            1002 jgte .top
>   -1   ------ Interrupt ------  ------ Interrupt ------
>   -1   1003 halt
>    0   ----- Halt;Switch -----  ----- Halt;Switch -----
>   -1                            1000 sub  $1,%dx
>   -1                            1001 test $0,%dx
>   -1   ------ Interrupt ------  ------ Interrupt ------
>   -1                          1002 jgte .top
>   -1                            1003 halt


The interrupt frequency does not change any of the results.


4. Now, a different program, `looping-race-nolock.s`, which accesses a shared variable located at address 2000; we’ll call this variable `value`. Run it with a single thread to confirm your understanding: `./x86.py -plooping-race-nolock.s -t 1 -M 2000` What is `value` (i.e.,at memory address 2000) throughout the run? Use `-c` to check.


> 2000          Thread 0
>    0
>    0   1000 mov 2000, %ax
>    0   1001 add $1, %ax
>    1   1002 mov %ax, 2000
>    1   1003 sub  $1, %bx
>    1   1004 test $0, %bx
>    1   1005 jgt .top
>    1   1006 halt



5. Run with multiple iterations/threads: `./x86.py -plooping-race-nolock.s -t 2 -a bx=3 -M 2000` Why does each thread loop three times? What is final value of `value`?

> 2000          Thread 0                Thread 1
>    0
>    0   1000 mov 2000, %ax
>    0   1001 add $1, %ax
>    1   1002 mov %ax, 2000
>    1   1003 sub  $1, %bx
>    1   1004 test $0, %bx
>    ?   1005 jgt .top
>    ?   1000 mov 2000, %ax
>    ?   1001 add $1, %ax
>    2   1002 mov %ax, 2000
>    ?   1003 sub  $1, %bx
>    ?   1004 test $0, %bx
>    ?   1005 jgt .top
>    3   1000 mov 2000, %ax
>    ?   1001 add $1, %ax
>    ?   1002 mov %ax, 2000
>    ?   1003 sub  $1, %bx
>    ?   1004 test $0, %bx
>    ?   1005 jgt .top
>    ?   1006 halt
>    ?   ----- Halt;Switch -----  ----- Halt;Switch -----
>    3                            1000 mov 2000, %ax
>    ?                            1001 add $1, %ax
>    4                            1002 mov %ax, 2000
>    ?                            1003 sub  $1, %bx
>    ?                            1004 test $0, %bx
>    ?                            1005 jgt .top
>    5                            1000 mov 2000, %ax
>    ?                            1001 add $1, %ax
>    ?                            1002 mov %ax, 2000
>    ?                            1003 sub  $1, %bx
>    ?                            1004 test $0, %bx
>    ?                            1005 jgt .top
>    6                            1000 mov 2000, %ax
>    ?                            1001 add $1, %ax
>    ?                            1002 mov %ax, 2000
>    ?                            1003 sub  $1, %bx
>    ?                            1004 test $0, %bx
>    ?                            1005 jgt .top
>    6                            1006 halt


Each thread loops 3 times because `bx` starts with a value of 3.


6. Run with random interrupt intervals: `./x86.py -p looping-race-nolock.s -t 2 -M 2000 -i 4 -r -s 0` with different seeds (-s 1,-s 2, etc.) Can you tell by looking at the thread interleaving what the final value of `value` will be? Does the timing of the interrupt matter? Where can it safely occur? Where not? In other words, where is the critical section exactly?


> 2000          Thread 0                Thread 1
>    0
>    0   1000 mov 2000, %ax
>    0   1001 add $1, %ax
>    1  1002 mov %ax, 2000
>    1   1003 sub  $1, %bx
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1                            1000 mov 2000, %ax
>    1                            1001 add $1, %ax
>    2                            1002 mov %ax, 2000
>    2                            1003 sub  $1, %bx
>    2   ------ Interrupt ------  ------ Interrupt ------
>    2   1004 test $0, %bx
>    2   1005 jgt .top
>    2   ------ Interrupt ------  ------ Interrupt ------
>    2                            1004 test $0, %bx
>    2                            1005 jgt .top
>    2   ------ Interrupt ------  ------ Interrupt ------
>    2   1006 halt
>    2   ----- Halt;Switch -----  ----- Halt;Switch -----
>    2                            1006 halt

Timing of the interrupt matters due to the data race. The critical section is between mov-add-mov.


7. Now examine fixed interrupt intervals: `./x86.py -p looping-race-nolock.s -a bx=1 -t 2 -M 2000 -i 1` What will the final value of the shared variable `value` be? What about when you change `-i 2`, `-i 3`, etc.? For which interrupt intervals does the program give the “correct” answer?


i = 1
> 2000          Thread 0                Thread 1
>    0
>    0   1000 mov 2000, %ax
>    0   ------ Interrupt ------  ------ Interrupt ------
>    0                            1000 mov 2000, %ax
>    0   ------ Interrupt ------  ------ Interrupt ------
>    0   1001 add $1, %ax
>    0   ------ Interrupt ------  ------ Interrupt ------
>    0                            1001 add $1, %ax
>    0   ------ Interrupt ------  ------ Interrupt ------
>    1   1002 mov %ax, 2000
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1                            1002 mov %ax, 2000
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1   1003 sub  $1, %bx
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1                            1003 sub  $1, %bx
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1   1004 test $0, %bx
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1                            1004 test $0, %bx
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1   1005 jgt .top
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1                            1005 jgt .top
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1   1006 halt
>    1   ----- Halt;Switch -----  ----- Halt;Switch -----
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1                            1006 halt

i = 2
> 2000          Thread 0                Thread 1
>    0
>    0   1000 mov 2000, %ax
>    0   1001 add $1, %ax
>    0   ------ Interrupt ------  ------ Interrupt ------
>    0                            1000 mov 2000, %ax
>    0                            1001 add $1, %ax
>    0   ------ Interrupt ------  ------ Interrupt ------
>    1   1002 mov %ax, 2000
>    1   1003 sub  $1, %bx
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1                            1002 mov %ax, 2000
>    1                            1003 sub  $1, %bx
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1   1004 test $0, %bx
>    1   1005 jgt .top
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1                            1004 test $0, %bx
>    1                            1005 jgt .top
>    1   ------ Interrupt ------  ------ Interrupt ------
>    1   1006 halt
>    1   ----- Halt;Switch -----  ----- Halt;Switch -----
>    1                            1006 halt

i = 3
> 2000          Thread 0                Thread 1
>    0
>    ?   1000 mov 2000, %ax
>    ?   1001 add $1, %ax
>    1   1002 mov %ax, 2000
>    ?   ------ Interrupt ------  ------ Interrupt ------
>    ?                            1000 mov 2000, %ax
>    ?                            1001 add $1, %ax
>    2                            1002 mov %ax, 2000
>    ?   ------ Interrupt ------  ------ Interrupt ------
>    ?   1003 sub  $1, %bx
>    ?   1004 test $0, %bx
>    ?   1005 jgt .top
>    ?   ------ Interrupt ------  ------ Interrupt ------
>    ?                            1003 sub  $1, %bx
>    ?                            1004 test $0, %bx
>    ?                            1005 jgt .top
>    ?   ------ Interrupt ------  ------ Interrupt ------
>    ?   1006 halt
>    ?   ----- Halt;Switch -----  ----- Halt;Switch -----
>    ?                            1006 halt

Only when i is 3 do we get the "correct" answer, because then our critical section will be executed atomically.


8. Run the same for more loops (e.g., set `-a bx=100`). What interrupt intervals (-i) lead to a correct outcome? Which intervals are surprising?

When i is 4, we get surprising results since we no longer get the "correct" answer of 200, but 150. This is because when i is a multiple of 3, we are ensured that the critical section is executed atomically, since there are 6 instructions total.


9. One last program: `wait-for-me.s`. Run: `./x86.py -p wait-for-me.s -a ax=1,ax=0 -R ax -M 2000` This sets the `%ax` register to 1 for thread 0, and 0 for thread 1, and watches `%ax` and memory location 2000. How should the code behave? How is the value at location 2000 being used by the threads? What will its final value be?

> 2000      ax          Thread 0                Thread 1
>    0       1
>    0       1   1000 test $1, %ax
>    0       1   1001 je .signaller
>    1       1   1006 mov  $1, 2000
>    1       1   1007 halt
>    1       0   ----- Halt;Switch -----  ----- Halt;Switch -----
>    1       0                            1000 test $1, %ax
>    1       0                            1001 je .signaller
>    1       0                            1002 mov  2000, %cx
>    1       0                            1003 test $1, %cx
>    1       0                            1004 jne .waiter
>    1       0                            1005 halt

10. Now switch the inputs: `./x86.py -p wait-for-me.s -aax=0,ax=1 -R ax -M 2000` How do the threads behave? What is thread 0 doing? How would changing the interrupt interval (e.g.,-i 1000, or perhaps to use random intervals) change the trace out-come? Is the program efficiently using the CPU?

Thread 0 is now looping, and will not stop until `value` is equal to 1.

After a timer interrupt, Thread 1 starts, then sets the `value` to 1, then exits. When we switch back to Thread 0, it's now able to exit.

eg:
`./x86.py -p wait-for-me.s -aax=0,ax=1 -R ax -M 2000 -i 10`
