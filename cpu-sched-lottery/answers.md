1. Compute the solutions for simulations with 3 jobs and random seeds of 1, 2, and 3.

./lottery.py -j 3 -s 1

./lottery.py -j 3 -s 2

./lottery.py -j 3 -s 3

2. Now run with two specific jobs: each of length 10, but one (job 0) with just 1 ticket and the other (job 1) with 100 (e.g., -l 10:1,10:100).What happens when the number of tickets is so imbalanced? Will job 0 ever run before job 1 completes? How often? In general, what does such a ticket imbalance do to the behavior of lottery scheduling?

Job 0 CAN run before job 1 completes, but it's unlikely. There is a ~10% chance that job 0 will run before job 1 completes. I believe the exact odds are (1 - (99/100)^10) = 9.56%.

Such a ticket imbalance basically guarantees that job 0 won't start running, or at least won't make significant progress, until Job 1 finishes.


3. When running with two jobs of length 100 and equal ticket allocations of 100 (-l 100:100,100:100), how unfair is the scheduler? Run with some different random seeds to determine the (probabilistic) answer; let unfairness be determined by how much earlier one job finishes than the other.

./lottery.py -l 100:100,100:100 -c -s 0

job 0: 192
job 1: 200

./lottery.py -l 100:100,100:100 -c -s 1

job 0: 200
job 1: 196

./lottery.py -l 100:100,100:100 -c -s 2

job 0: 200
job 1: 190

The unfairness factor is inversely proportional to the job length.


4. How does your answer to the previous question change as the quantum size (-q) gets larger?

As the quantum length increases, the unfairness factor also increases, since there will be fewer samples to "even out" the disparity.


5. Can you make a version of the graph that is found in the chapter? What else would be worth exploring? How would the graph look with a stride scheduler?


