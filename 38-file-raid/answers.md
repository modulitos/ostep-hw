## 1. Use the simulator to perform some basic RAID mapping tests. Run with different levels (0, 1, 4, 5) and see if you can figure out the mappings of a set of requests. For RAID-5, see if you can figure out the difference between left-symmetric and left-asymmetric layouts. Use some different random seeds to generate different problems than above.

`./raid.py -n 5 -L 0 -R 20`

`./raid.py -n 5 -L 1 -R 20`

`./raid.py -n 5 -L 4 -R 20`
16
disk 1, offset 5

8
disk 2, offset 2

10
disk 1, offset 3

15
disk 0, offset 5

9
disk 0, offset 3

`./raid.py -n 5 -L 5 -R 20 -5 LS`
16
disk 0, offset 5

8
disk 0, offset 2

10
disk 2, offset 3

15
disk 3, offset 5

9
disk 1, offset 3


`./raid.py -n 5 -L 5 -R 20 -5 LA`

16
disk 1, offset 5

8
disk 3, offset 2

10
disk 2, offset 3

15
disk 0, offset 5

9
disk 1, offset 3


Difference between LS and LA is the pattern of the rotated parity blocks:

> left-symmetric    left-asymmetric
> 0 1 2 P           0 1 2 P
> 4 5 P 3           3 4 P 5
> 8 P 6 7           6 P 7 8

## 2. Do the same as the first problem, but this time vary the chunk size with `-C`. How does chunk size change the mappings?

`./raid.py -n 5 -L 0 -R 20 -C 8k`

16
disk 0, offset 4

8
disk 0, offset 2

10
disk 1, offset 2

15
disk 3, offset 3

9
disk 0, offset 3

layout:

> 01   23   45   67
> 89   1011 1213 1415
> 1617 1819 2021 2223

`./raid.py -n 5 -L 5 -5 LS -R 20 -C 8k -c`

layout:
> 01   23   45   P
> 89   1011 P    67
> 1617 P    1213 1415
> P    1819 2021 2223

16
disk 0, offset 4

8
disk 0, offset 2

10
disk 1, offset 2

15
disk 3, offset 5

9
disk 0, offset 3

## 3. Do the same as above, but use the `-r` flag to reverse the nature of each problem.

## 4. Now use the reverse flag but increase the size of each request with the `-S` flag. Try specifying sizes of 8k, 12k, and 16k, while varying the RAID level. What happens to the underlying I/O pattern when the size of the request increases? Make sure to try this with the sequential workload too (`-W sequential`); for what request sizes are RAID-4 and RAID-5 much more I/O efficient?

`./raid.py -n 5 -L 0 -R 20 -C 8k -S 8k -c`

16, 2
disk 0, offset 4
disk 0, offset 5

8, 2
disk 0, offset 2
disk 0, offset 3

10, 2
disk 1 offset 2
disk 1 offset 3

15, 2
disk 3, offset 3
disk 0, offset 4

9,2
disk 0, offset 3
disk 1, offset 2

When size of request increases, the I/O pattern might require reading from another disk.


`./raid.py -L 5 -S 4k -c -W seq -t`
`./raid.py -L 5 -S 8k -c -W seq -t`
`./raid.py -L 5 -S 12k -c -W seq -t`
`./raid.py -L 5 -S 16k -c -W seq -t`

RAID-4 and RAID-5 are more efficient with smaller request sizes.


## 5. Use the timing mode of the simulator (-t) to estimate the performance of 100 random reads to the RAID, while varying the RAID levels, using 4 disks.

`./raid.py -L 0 -t -n 100 -c`
275.7

`./raid.py -L 1 -t -n 100 -c`
278.7

`./raid.py -L 4 -t -n 100 -c`
386.1

`./raid.py -L 5 -t -n 100 -c`
276.7

## 6. Do the same as above, but increase the number of disks. How does the performance of each RAID level scale as the number of disks increases?

`./raid.py -L 0 -t -n 100 -c -D 8`
156
275.7 / 156 = 1.77

`./raid.py -L 1 -t -n 100 -c -D 8`
167.8
278.7 / 167.8 = 1.66

`./raid.py -L 4 -t -n 100 -c -D 8`
165
386.1 / 165 = 2.34

`./raid.py -L 5 -t -n 100 -c -D 8`
158.6
276.7 / 158.6 = 1.76


RAID-4 has the best scaling.

## 7. Do the same as above, but use all writes (`-w 100`) instead of reads. How does the performance of each RAID level scale now? Can you do a rough estimate of the time it will take to complete the workload of 100 random writes?

`./raid.py -L 0 -t -n 100 -c -D 8 -w 100`
156.5

`./raid.py -L 1 -t -n 100 -c -D 8 -w 100`
275.7

`./raid.py -L 4 -t -n 100 -c -D 8 -w 100`
937.8

`./raid.py -L 5 -t -n 100 -c -D 8 -w 100`
290.1


## 8. Run the timing mode one last time, but this time with a sequential workload (`-W sequential`). How does the performance vary with RAID level, and when doing reads versus writes? How about when varying the size of each request? What size should you write to a RAID when using RAID-4 or RAID-5?
