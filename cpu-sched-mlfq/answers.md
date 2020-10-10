1. Run a few randomly-generated problems with just two jobs andtwo queues; compute the MLFQ execution trace for each. Make your life easier by limiting the length of each job and turning offI/Os.

./mlfq.py --jlist 0,10,0:4,10,0 -q 3 -n 2 -c

./mlfq.py --jlist 0,10,0:4,10,0 -q 5 -n 2 -c


2. How would you run the scheduler to reproduce each of the examples in the chapter?

8.2:
./mlfq.py --jlist 0,200,0 -q 10 -n 3 -c

8.3:
./mlfq.py --jlist 0,180,0:100,20,0 -q 10 -n 3 -c

8.4:
./mlfq.py -S --jlist 0,169,0:49,31,1 -q 10 -n 3 -i 4 -c

8.5:
no boost:
./mlfq.py -l 0,120,0:100,50,5:105,50,5 -q 10 -n 3 -i 5 -S -c
(Job 0 gets starved here)

with boost: (add -B 50)
./mlfq.py -S --jlist 0,120,0:100,50,5:105,50,5 -q 10 -n 3 -i 5 -c -B 50

8.6
with gaming tolerance:
./mlfq.py -l 0,200,0:80,100,9 -q 10 -n 3 -i 1 -c -S
(Job2 issues io just before time slice ends, thus dominating CPU time)

without gaming tolerance (removing the -S arg)
./mlfq.py -l 0,200,0:80,100,9 -q 10 -n 3 -i 1 -c

8.7
./mlfq.py -l 0,150,0:0,150,0 -Q 10,20,40 -n 3 -c -a 2


3. How would you configure the scheduler parameters to behave just like a round-robin scheduler?

Use one queue

(OR having `time slice <= (max job length / jobs number)` ?)


4. Craft a workload with two jobs and scheduler parameters so that one job takes advantage of the older Rules 4a and 4b (turned on with the -S flag) to game the scheduler and obtain 99% of the CPU over a particular time interval.

./mlfq.py -l 0,5,0:3,100,2 -q 3 -n 3 -i 0 -c -S

Job 1 is only 5 ticks, but finishes at 105.
Job 2 starts after job 1, is 100 ticks, and finishes at 103. It dominates the CPU by staying in the highest priority queue and issuing IO just before the quantum length ends.


5. Given a system with a quantum length of 10 ms in its highest queue,how often would you have to boost jobs back to the highest priority level (with the -B flag) in order to guarantee that a single long-running (and potentially-starving) job gets at least 5% of the CPU?

At least every 200 ms.
10ms should be 5% of the boost period => T * .05 = 10 ms


6. One question that arises in scheduling is which end of a queue to add a job that just finished I/O; the -I flag changes this behavior for this scheduling simulator. Play around with some workloadsand see if you can see the effect of this flag

./mlfq.py -n 2 -q 10 -l 0,50,0:0,50,11 -i 1 -S -c
(job 0 finishes at 72, job 1 finishes at 102)

./mlfq.py -n 2 -q 10 -l 0,50,0:0,50,11 -i 1 -S -c -I
(job 0 finishes at 93, job 1 finishes at 101)
