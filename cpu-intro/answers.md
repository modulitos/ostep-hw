1.
`./process-run.py -l 5:100,5:100`
cpu utilization?
> 100%

2.
`./process-run.py -l 4:100,1:0`
how many ticks to complete both processes?
> 10
(note: it takes 1 extra tick to return from the program if in the IO state)

3.
`./process-run.py -l 1:0,4:100`
What happens now? Does switching the order matter?
> yes - the second process can now start while the first is waiting on IO.
> Ticks: 6

4.
`./process-run.py -l 1:0,4:100 -c -S SWITCH_ON_END`
What happens?
(does not switch when another process is doing IO)
> 9 ticks (5 + 4)

5.
`./process-run.py -l 1:0,4:100 -c -S SWITCH_ON_IO`
What happens now?
> 6 ticks (5 + 1(check io)), with the 4 cpu ticks done during the IO delay.

6.
`./process-run.py -l 3:0,5:100,5:100,5:100 -S SWITCH_ON_IO -I IO_RUN_LATER -c -p`
What happens? Is this efficient use of resources?
> 27  ticks (1 + 5 (&4 ios) + 5 + 5 + 1 + 4(ios) + 1 + 4(ios) + 1 (check io)).
> No, not efficient, because the IO ticks are not being run concurrently.

7.
`./process-run.py -l 3:0,5:100,5:100,5:100 -S SWITCH_ON_IO -I IO_RUN_IMMEDIATE -c -p`
What happens now? (note that RUN_IMMEDIATE is greedy!)
> 18 (1 + 4 (&4 ios), + 1 + 1 (&1 ios, P2 done) + 3 (&3 ios) + 1 + 2 (&2 ios, P3 done) + 4 (&4 ios) + 1 (P4 done))
> Efficient, as we're prioritizing IO ticks to keep them running concurrently with other processes.

8.
`./process-run.py -s 1 -l 3:50,3:50`
`./process-run.py -s 2 -l 3:50,3:50`
`./process-run.py -s 3 -l 3:50,3:50`
what happens?
What if we use `-I IO_RUN_IMMEDIATE` vs `-I IO_RUN_LATER`?
What if we use `-S SWITCH_ON_IO` vs `-S SWITCH_ON_END`?


for: `./process-run.py -s 1 -l 3:50,3:50` seed gives 2 procs: (c, io, io), (c,c,c):
> 12 ticks (1(c) + 1 + 1(c,io) + 1(c,io) + 1(c,io) + 1(io) + 1(io-start) + 4(io) + 1 (io check))
with `IO_RUN_IMMEDIATE` (`./process-run.py -s 1 -l 3:50,3:50 -I IO_RUN_IMMEDIATE -c -p`):
> 12 ticks (not enough cpu ticks to block the io's)
with `SWITCH_ON_END` (`./process-run.py -s 1 -l 3:50,3:50 -S SWITCH_ON_END -c -p`):
> 14 ticks (1 + 5 + 5 + 1 + 1 + 1)
