1. Compute the response time and turnaround time when running three jobs of length 200 with the SJF and FIFO schedulers.

SJF:

job 1:
 * rt: 0
 * tat: 200

job 2:
 * rt: 200
 * tat: 400

job 3:
 * rt: 400
 * tat: 600

FIFO:

job 1:
 * rt: 0
 * tat: 200

job 2:
 * rt: 200
 * tat: 400

job 3:
 * rt: 400
 * tat: 600

2. Now do the same but with jobs of different lengths: 100, 200, and 300.

SJF:

job 1:
 * rt: 0
 * tat: 100

job 2:
 * rt: 100
 * tat: 300

job 3:
 * rt: 300
 * tat: 600

FIFO:

job 1:
 * rt: 0
 * tat: 100

job 2:
 * rt: 100
 * tat: 300

job 3:
 * rt: 300
 * tat: 600


3. Now do the same, but also with the RR scheduler and a time-slice of 1.

### job 1:
 * rt: 0
 * tat: 298

<!-- calculation for TAT: -->
<!-- where T = total time of job -->

<!-- T  TAT -->
<!-- 1: 1 -->
<!-- 2: 4 -->
<!-- 3: 7 -->
<!-- 4: 10 -->
<!-- TAT = (3(T-1) + 1) -->
<!-- ... -->
<!-- 100: 3(99) + 1 = 297 +1 = 298 -->

TAT  T (job1, unless otherwise indicated)
1:   1
4:   2
7:   3
...
97: 33
100: 34
...
298: 100
299: 100 (J2)
300: 100 (J3)


### job 2:
 * rt: 1
 * tat: 499

after job 1 completes:
at time 300, job 1 completes and we start a timeslice on job 2, where job 2 has completed 100 of 200.

TAT  T (job2)
2:   1
5:   2
...
299: 100
301: 101
303: 102
305: 103
...
399: 150
...
499: 200

200: 3(200 - 1) + 2 = 597 + 2 = 599

### job 3:
 * rt: 2
 * tat: 600

TAT T (job3)
3:  1
6:  2
9:  3
...
297: 99
298: job1 completes
300: 100
302: 101
...
498: 199
499: job2 completes
500: 200
501: 201
...
600: 300


4. For what types of workloads does SJF deliver the same turnaroundtimes as FIFO?

When the jobs are already in order of shortest to longest.


5. For what types of workloads and quantum lengths does SJF deliver the same response times as RR?

When workloads are less than the quantum lengths. Otherwise, the response time will take longer on SJF as the RR immediately context switches to the next job.


6. What happens to response time with SJF as job lengths increase? Can you use the simulator to demonstrate the trend?

Response times increase as job lengths increase.

./scheduler.py -p SJF -l 10,20,30 -c

./scheduler.py -p SJF -l 20,40,60 -c

Any increase in the proceeding jobs will cause an increase in the RT of the latter jobs.


7. What happens to response time with RR as quantum lengths increase? Can you write an equation that gives the worst-case response time, given N jobs?

RT increases as quantum lengths increase in RR.

N = number of jobs
Q = quantum length
worst-case response time = f(N) = (N-1) * Q
average response time = g(N) = ((N-1) * Q) / 2



