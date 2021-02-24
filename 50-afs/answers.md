### 1. Run a few simple cases to make sure you can predict what values will be read by clients. Vary the random seed flag (-s) and see if you can trace through and predict both intermediate values as well as the final values stored in the files. Also vary the number of files (-f), the number of clients (-C), and the read ratio (-r, from between 0 to 1) to make it a bit more challenging. You might also want to generate slightly longer traces to make for more interesting interactions, e.g., (-n 2 or higher).

### 2. Now do the same thing and see if you can predict each callback thatthe AFS server initiates. Try different random seeds, and make sureto use a high level of detailed feedback (e.g.,-d 3) to see when call-backs occur when you have the program compute the answers foryou (with-c). Can you guess exactly when each callback occurs?What is the precise condition for one to take place?


### 3. Similar to above, run with some different random seeds and see ifyou can predict the exact cache state at each step. Cache statecanbe observed by running with-cand-d 7.


### 4. Now let’s construct some specific workloads. Run the simulationwith-A oa1:w1:c1,oa1:r1:c1flag. What are different possi-ble values observed by client 1 when it reads the filea, when run-ning with the random scheduler? (try different random seeds tosee different outcomes)? Of all the possible schedule interleavingsof the two clients’ operations, how many of them lead to client 1reading the value 1, and how many reading the value 0?


### 5. Now let’s construct some specific schedules. When running withthe-A oa1:w1:c1,oa1:r1:c1flag, also run with the followingschedules:-S 01,-S 100011,-S 011100, and others of whichyou can think. What value will client 1 read?


### 6. Now run with this workload:-A oa1:w1:c1,oa1:w1:c1, andvary the schedules as above. What happens when you run with-S011100? What about when you run with-S 010011? What isimportant in determining the final value of the file
