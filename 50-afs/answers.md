### 1. Run a few simple cases to make sure you can predict what values will be read by clients. Vary the random seed flag (-s) and see if you can trace through and predict both intermediate values as well as the final values stored in the files. Also vary the number of files (-f), the number of clients (-C), and the read ratio (-r, from between 0 to 1) to make it a bit more challenging. You might also want to generate slightly longer traces to make for more interesting interactions, e.g., (-n 2 or higher).

> ./afs.py -C 2 -n 1 -s 13 -c

0, 1, 2


> ./afs.py -C 3 -n 2 -s 14

c0 write 0 -> 1
c1 read 0 -> 0
c0 write 1 -> 3
c1 write 1 -> 4
c2 write 0 -> 2
c2 write 2 -> 5


### 2. Now do the same thing and see if you can predict each callback that the AFS server initiates. Try different random seeds, and make sure to use a high level of detailed feedback (e.g.,-d 3) to see when callbacks occur when you have the program compute the answers for you (with -c). Can you guess exactly when each callback occurs? What is the precise condition for one to take place?

> ./afs.py -C 2 -n 1 -s 13 -c -d 3

> ./afs.py -C 3 -n 2 -s 14 -c -d 3

In AFS, with each fetch of a directory or file, the AFS client establishes a callback with the server, thus ensuring that the server would notify the client of a change in its cached state.

If the file is later changed, then the callback occurs.


### 3. Similar to above, run with some different random seeds and see if you can predict the exact cache state at each step. Cache state can be observed by running with -c and -d 7.


> ./afs.py -C 2 -n 1 -s 13 -c -d 7

> ./afs.py -C 3 -n 2 -s 14 -c -d 7




### 4. Now let’s construct some specific workloads. Run the simulation with `-A oa1:w1:c1,oa1:r1:c1` flag. What are different possible values observed by client 1 when it reads the file `a`, when running with the random scheduler? (try different random seeds to see different outcomes)? Of all the possible schedule interleavings of the two clients' operations, how many of them lead to client 1 reading the value 1, and how many reading the value 0?

If client 0 closes the file (after its write) before client 1 opens it (assuming no network latency), then client 1 will read the value of 1.

> ./afs.py -A oa1:w1:c1,oa1:r1:c1 -c -d 7

### 5. Now let’s construct some specific schedules. When running with the `-A oa1:w1:c1,oa1:r1:c1` flag, also run with the following schedules: `-S 01`, `-S 100011`, `-S 011100`, and others of which you can think. What value will client 1 read?

> ./afs.py -A oa1:w1:c1,oa1:r1:c1 -c -d 7 -S 01

0

> ./afs.py -A oa1:w1:c1,oa1:r1:c1 -c -d 7 -S 100011

0

> ./afs.py -A oa1:w1:c1,oa1:r1:c1 -c -d 7 -S 011100

0

### 6. Now run with this workload: `-A oa1:w1:c1,oa1:w1:c1`, and vary the schedules as above. What happens when you run with `-S 011100`? What about when you run with `-S 010011`? What is important in determining the final value of the file

> ./afs.py -A oa1:w1:c1,oa1:w1:c1 -c -d 7 -S 011100

2

> ./afs.py -A oa1:w1:c1,oa1:w1:c1 -c -d 7 -S 010011

2

Final file value is determined by the last client to close after performing a write.
