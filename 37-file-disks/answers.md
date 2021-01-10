NOTE: use python2 here for the graphical library to work!

## 1. Compute the seek, rotation, and transfer times for the following sets of requests:

`-a 0`:
seek: 0
rotation: 165
xfer: 30

`-a 6`:
seek: 0
rotation: 345
xfer: 30

`-a 30`:
seek: 80
rotation: 345 - 80 = 265
xfer: 30

`-a 7,30,8`:
seek: 80 + 80 = 160
rotation: 15 + (300 -80) + (360 + 30 - 80) = 545
xfer: 30 + 30 + 30

`-a 10,11,12,13`:
seek: 0 + 0 + 40
rotation: 105 + 0 + (360 - 40)
xfer: 30 + 30 + 30 + 30



## 2. Do the same requests above, but change the seek rate to different values:

`-S 2`
same (no seek)

`-S 4`
same (no seek)

`-S 8`
seek: 10

=> 335 + 10 + 30 = 375

`-S 10`

Much faster now since we don't have to loop again!

`-S 40`

`-S 0.1`


How do the times change?

The times are much faster when seek rate speeds up, especially if we're able to save an entire loop.

## 3. Do the same requests above, but change the rotation rate:-R 0.1,-R 0.5,-R 0.01. How do the times change?

Default rotational value is 1, so the times are much longer as the rotational speed slows down. In some cases they might be faster since we can save extra loops, but it doesn't speed up any of the examples above.

## 4. FIFO is not always best, e.g., with the request stream `-a 7,30,8`, what order should the requests be processed in? Run the shortest seek-time first(SSTF) scheduler (`-p SSTF`) on this workload; how long should it take(seek, rotation, transfer) for each request to be served?


`python2 disk.py -a 7,30,8 -p SSTF -c`
=> 375
(Default FIFO => 795)

It'll run 7, then 8, then 30, instead of 7, 30, 8 on FIFO. Saves a lot of time.



## 5. Now use the shortest access-time first (SATF) scheduler (`-p SATF`). Does it make any difference for `-a 7,30,8` workload? Find a set of requests where SATF outperforms SSTF; more generally, when is SATF better than SSTF?


`python2 disk.py -a 34,13 -p SATF -G`
=> 225
`python2 disk.py -a 34,13 -p SSTF -G`
=> 495


SATF does better than SSTF when the seek time is shorter than the rotational time. It's even more pronounced when `-S` seek speed is increased.



## 6. Here is a request stream to try: `-a 10,11,12,13`. What goes poorly when it runs? Try adding track skew to address this problem (`-o skew`). Given the default seek rate, what should the skew be to maximize performance? What about for different seek rates (e.g.,`-S 2`,`-S 4`)? In general, could you write a formula to figure out the skew?

`python2 disk.py  -a 10,11,12,13 -G -o 2`
=> 285

a skew of 2 is the lowest value to catch it in time.

skew = track-distance(40) / seek-speed / (rotational-space-degrees(360 / 12) * rotation-speed) = 40 / 1 / (30 * 1) ≈ 2

-S 2: 40 / 2 / 30 ≈ 1

-S 4: 40 / 4 / 30 ≈ 1

## 7. Specify a disk with different density per zone, e.g.,`-z 10,20,30`, which specifies the angular difference between blocks on the outer, middle, and inner tracks. Run some random requests (e.g., `-a -1`` -A 5,-1,0`, which specifies that random requests should be used via the `-a -1` flag and that five requests ranging from 0 to the max be generated), and compute the seek, rotation, and transfer times. Use different random seeds. What is the bandwidth (in sectors per unit time) on the outer, middle, and inner tracks?

## 8. A scheduling window determines how many requests the disk can examineat once. Generate random workloads (e.g.,-A 1000,-1,0, with differentseeds) and see how long the SATF scheduler takes when the scheduling win-dow is changed from 1 up to the number of requests. How big of a windowis needed to maximize performance? Hint: use the-cflag and don’t turnon graphics (-G) to run these quickly. When the scheduling window is setto 1, does it matter which policy you are using?

## 9. Create a series of requests to starve a particular request, assuming an SATFpolicy. Given that sequence, how does it perform if you use aboundedSATF(BSATF) scheduling approach?  In this approach, you specify thescheduling window (e.g.,-w 4); the scheduler only moves onto the nextwindow of requests whenallrequests in the current window have been ser-viced. Does this solve starvation? How does it perform, as compared toSATF? In general, how should a disk make this trade-off betweenperfor-mance and starvation avoidance?

## 10. All the scheduling policies we have looked at thus far aregreedy; they pickthe next best option instead of looking for an optimal schedule.Can youfind a set of requests in which greedy is not optimal?
