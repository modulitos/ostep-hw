## 1. The homework will mostly focus on the log-structured SSD, which is simulated with the “-T log” flag. We’ll use the other types of SSDs for comparison. First, run with flags `-T log -s 1 -n 10 -q`. Can you figure out which operations took place? Use `-c` to check your answers (or just use `-C` instead of -q -c). Use different values of `-s` to generate different random workloads.


> ❯ ./ssd.py -T log -s 1 -n 10 -q
> ARG seed 1
> ARG num_cmds 10
> ARG op_percentages 40/50/10
> ARG skew
> ARG skew_start 0
> ARG read_fail 0
> ARG cmd_list
> ARG ssd_type log
> ARG num_logical_pages 50
> ARG num_blocks 7
> ARG pages_per_block 10
> ARG high_water_mark 10
> ARG low_water_mark 8
> ARG erase_time 1000
> ARG program_time 40
> ARG read_time 10
> ARG show_gc False
> ARG show_state False
> ARG show_cmds False
> ARG quiz_cmds True
> ARG show_stats False
> ARG compute False
>
> FTL   (empty)
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data
> Live
>
> cmd   0:: command(??) -> ??

cmd   0:: write(12, u) -> success

>
> FTL    12:  0
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vEEEEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  u
> Live  +
>
> cmd   1:: command(??) -> ??

cmd   1:: write(32, M) -> success

>
> FTL    12:  0  32:  1
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvEEEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  uM
> Live  ++
>
> cmd   2:: read(32) -> ??

cmd   2:: read(32) -> M

>
> FTL    12:  0  32:  1
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvEEEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  uM
> Live  ++
>
> cmd   3:: command(??) -> ??

cmd   3:: write(38, 0) -> success

>
> FTL    12:  0  32:  1  38:  2
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvEEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  uM0
> Live  +++
>
> cmd   4:: command(??) -> ??

cmd   4:: write(36, e) -> success

>
> FTL    12:  0  32:  1  36:  3  38:  2
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  uM0e
> Live  ++++
>
> cmd 5:: command(??) -> ??
cmd   5:: trim(36, e) -> success

>
> FTL    12:  0  32:  1  38:  2
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  uM0e
> Live  +++
>
> cmd   6:: read(32) -> ??
cmd   6:: read(32) -> M


>
> FTL    12:  0  32:  1  38:  2
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  uM0e
> Live  +++
>
> cmd 7:: command(??) -> ??
cmd   7:: trim(32) -> success


>
> FTL    12:  0  38:  2
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  uM0e
> Live  + +
>
> cmd   8:: read(12) -> ??

cmd   8:: read(12) -> u

>
> FTL    12:  0  38:  2
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  uM0e
> Live  + +
>
> cmd   9:: read(12) -> ??
cmd   9:: read(12) -> u

>
> FTL    12:  0  38:  2
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  uM0e
> Live  + +


## 2. Now just show the commands and see if you can figure out the intermediate states of the Flash. Run with flags `-T log -s 2 -n 10 -C` to show each command. Now, determine the state of theFlash between each command; use `-F` to show the states and see if you were right. Use different random seeds to test your burgeoning expertise.

> ❯ ./ssd.py -T log -s 2 -n 10 -C
> ARG seed 2
> ARG num_cmds 10
> ARG op_percentages 40/50/10
> ARG skew
> ARG skew_start 0
> ARG read_fail 0
> ARG cmd_list
> ARG ssd_type log
> ARG num_logical_pages 50
> ARG num_blocks 7
> ARG pages_per_block 10
> ARG high_water_mark 10
> ARG low_water_mark 8
> ARG erase_time 1000
> ARG program_time 40
> ARG read_time 10
> ARG show_gc False
> ARG show_state False
> ARG show_cmds True
> ARG quiz_cmds False
> ARG show_stats False
> ARG compute False
>
> FTL   (empty)
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data
> Live
>
> cmd   0:: write(36, F) -> success
> cmd   1:: write(29, 9) -> success
> cmd   2:: write(19, I) -> success
> cmd   3:: trim(19) -> success
> cmd   4:: write(22, g) -> success
> cmd   5:: read(29) -> 9
> cmd   6:: read(22) -> g
> cmd   7:: write(28, e) -> success
> cmd   8:: read(36) -> F
> cmd   9:: write(49, F) -> success
>
> FTL    22:  3  28:  4  29:  1  36:  0  49:  5
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvvvEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  F9IgeF
> Live  ++ +++

answer:

> cmd   0:: write(36, F) -> success

> FTL   36: 0
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vEEEEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  F
> Live  +
>

> cmd   1:: write(29, 9) -> success

> FTL   36: 0, 29: 1
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvEEEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  F9
> Live  ++
>

> cmd   2:: write(19, I) -> success

> FTL   36: 0, 29: 1, 19: 2
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvEEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  F9I
> Live  +++

> cmd   3:: trim(19) -> success

> FTL   29: 1, 36: 0
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvEEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  F9I
> Live  ++

> cmd   4:: write(22, g) -> success

> FTL   22: 3, 29: 1, 36: 0
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  F9Ig
> Live  ++ +

> cmd   5:: read(29) -> 9

> FTL   22: 3, 29: 1, 36: 0
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  F9Ig
> Live  ++ +

> cmd   6:: read(22) -> g

> FTL   22: 3, 29: 1, 36: 0
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvEEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  F9Ig
> Live  ++ +

> cmd   7:: write(28, e) -> success

> FTL   28: 4, 22: 3, 29: 1, 36: 0
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvvEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  F9Ige
> Live  ++ ++

> cmd   8:: read(36) -> F

> FTL   28: 4, 22: 3, 29: 1, 36: 0
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvvEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  F9Ige
> Live  ++ ++

> cmd   9:: write(49, F) -> success

> FTL   49: 5, 28: 4, 22: 3, 29: 1, 36: 0
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvvvEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  F9IgeF
> Live  ++ +++


## 3. Let’s make this problem ever so slightly more interesting by adding the `-r 20` flag. What differences does this cause in the commands? Use `-c` again to check your answers.

We now have read commands being issued to non-live addresses. Eg of adding `-r 20` to the previous command:

> ❯ ./ssd.py -T log -s 2 -n 10 -C -r 20
> ARG seed 2
> ARG num_cmds 10
> ARG op_percentages 40/50/10
> ARG skew
> ARG skew_start 0
> ARG read_fail 20
> ARG cmd_list
> ARG ssd_type log
> ARG num_logical_pages 50
> ARG num_blocks 7
> ARG pages_per_block 10
> ARG high_water_mark 10
> ARG low_water_mark 8
> ARG erase_time 1000
> ARG program_time 40
> ARG read_time 10
> ARG show_gc False
> ARG show_state False
> ARG show_cmds True
> ARG quiz_cmds False
> ARG show_stats False
> ARG compute False
>
> FTL   (empty)
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data
> Live
>
> cmd   0:: read(41) -> fail: uninitialized read
> cmd   1:: write(33, j) -> success
> cmd   2:: write(30, A) -> success
> cmd   3:: read(33) -> j
> cmd   4:: write(49, W) -> success
> cmd   5:: write(22, g) -> success
> cmd   6:: read(23) -> fail: uninitialized read
> cmd   7:: read(22) -> g
> cmd   8:: write(28, e) -> success
> cmd   9:: read(33) -> j
>
> FTL    22:  3  28:  4  30:  1  33:  0  49:  2
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvvEEEEE iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data  jAWge
> Live  +++++



## 4. Performance is determined by the number of erases, programs, and reads (we assume here that trims are free). Run the same workload again as above, but without showing any intermediate states (e.g., `-T log -s 1 -n 10`). Can you estimate how long this workload will take to complete? (default erase time is 1000 microseconds, program time is 40, and read time is 10) Use the -S flag to check your answer. You can also change the erase, program, and read times with the -E, -W, -R flags.

> ./ssd.py -T log -s 1 -n 10
>
> cmd   0:: write(12, u) -> success
> cmd   1:: write(32, M) -> success
> cmd   2:: read(32) -> M
> cmd   3:: write(38, 0) -> success
> cmd   4:: write(36, e) -> success
> cmd   5:: trim(36) -> success
> cmd   6:: read(32) -> M
> cmd   7:: trim(32) -> success
> cmd   8:: read(12) -> u
> cmd   9:: read(12) -> u



In this program, there is 1 erase perform on the first block: 1000 us

There are 4 writes: 4 * 40 us

There are 4 reads: 4 * 10 us

for a total of 1000 + 160 + 40 = 1200 us

answer check:

> ./ssd.py -T log -s 1 -n 10 -C -S
>
> Physical Operations Per Block
> Erases   1          0          0          0          0          0          0          Sum: 1
> Writes   4          0          0          0          0          0          0          Sum: 4
> Reads    4          0          0          0          0          0          0          Sum: 4
>
> Logical Operation Sums
>   Write count 4 (0 failed)
>   Read count  4 (0 failed)
>   Trim count  2 (0 failed)
>
> Times
>   Erase time 1000.00
>   Write time 160.00
>   Read time  40.00
>   Total time 1200.00


## 5. Now, compare performance of the log-structured approach and the (very bad) direct approach (`-T direct` instead of `-T log`). First, estimate how you think the direct approach will perform, then check your answer with the -S flag. In general, how much better will the log-structured approach perform than the direct one?

Estimate: The log-structured approach will perform better than the direct approach by a factor of 1000, since it'll require reading each cell, erasing the entire block, then programming each cell plus the new one on each write.

Since erasing is multiple orders of magnitude greater than programs or reads, it'll dominate the duration.


> ./ssd.py -T direct -s 1 -n 10 -C -S
>
>Physical Operations Per Block
>Erases   0          1          0          3          0          0          0          Sum: 4
>Writes   0          1          0          6          0          0          0          Sum: 7
>Reads    0          2          0          5          0          0          0          Sum: 7
>
>Logical Operation Sums
>  Write count 4 (0 failed)
>  Read count  4 (0 failed)
>  Trim count  2 (0 failed)
>
>Times
>  Erase time 4000.00
>  Write time 280.00
>  Read time  70.00
>  Total time 4350.00


## 6. Let us next explore the behavior of the garbage collector. To do so, we have to set the high (-G) and low (-g) watermarks appropriately. First, let’s observe what happens when you run a larger workload to the log-structured SSD but without any garbage collection. To do this, run with flags `-T log -n 1000` (the high watermark default is 10, so the GC won’t run in this configuration). What do you think will happen? Use -C and perhaps -F to see.

> ❯ ./ssd.py -T log -n 1000
> ARG seed 0
> ARG num_cmds 1000
> ARG op_percentages 40/50/10
> ARG skew
> ARG skew_start 0
> ARG read_fail 0
> ARG cmd_list
> ARG ssd_type log
> ARG num_logical_pages 50
> ARG num_blocks 7
> ARG pages_per_block 10
> ARG high_water_mark 10
> ARG low_water_mark 8
> ARG erase_time 1000
> ARG program_time 40
> ARG read_time 10
> ARG show_gc False
> ARG show_state False
> ARG show_cmds False
> ARG quiz_cmds False
> ARG show_stats False
> ARG compute False
>
> FTL   (empty)
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii iiiiiiiiii
> Data
> Live
>
>
> FTL     5: 63   9: 43  27: 52  28: 40  42: 57
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvvvvvvv vvvvvvvvvv vvvvvvvvvv vvvvvvvvvv vvvvvvvvvv vvvvvvvvvv vvvvvvvvvv
> Data  qiUKUGqXg0 Pukzj6xXrz txTzZfdO1z 1g3ljJIppr KjSZIdP7h5 wSWTxCUYJC uqmoUB2I5d
> Live                                              +  +         +    +      +


The disk fills up with garbage data.

## 7. To turn on the garbage collector, use lower values. The high water-mark (`-G N`) tells the system to start collecting once N blocks have been used; the low watermark (`-g M`) tells the system to stop collecting once there are only M blocks in use. What watermark values do you think will make for a working system? Use -C and -F to show the commands and intermediate device states and see.

Setting the high watermark value to the number of blocks (N), and the low watermark to N-1 will have a working system, since it'll be cleaning the fewest blocks possible.

> ./ssd.py -T log -n 1000 -G 7 -g 6 -S


## 8. One other useful flag is -J, which shows what the collector is doing when it runs. Run with flags `-T log -n 1000 -C -J` to see both the commands and the GC behavior. What do you notice about the GC? The final effect of GC, of course, is performance. Use -S to look at final statistics; how many extra reads and writes occur due to garbage collection? Compare this to the ideal SSD (-T ideal); how much extra reading, writing, and erasing is there due to the nature of Flash? Compare it also to the direct approach; in what way (erases, reads, programs) is the log-structured approach superior?

> ./ssd.py -T log -n 1000 -J -G 7 -g 6 -S
>
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvvvEEEE EEEEEEEEEE vvvvvvvvvv vvvvvvvvvv vvvvvvvvvv vvvvvvvvvv vvvvvvvvvv
> Data  fhbAFB                UWrI6h262s SFMOfmV54O xA4SgOvTHf pW4CLHh77E ZDZ4MYCzNM
> Live  ++++++                  ++ +  +  + + ++++++ ++++ +++ + +++ + + ++ +++++++++
>
> Physical Operations Per Block
> Erases  13         13         12         12         12         12         12          Sum: 86
> Writes 126        120        120        120        120        120        120          Sum: 846
> Reads  101        107         92        103         91        108         98          Sum: 700
>
> Logical Operation Sums
>   Write count 846 (0 failed)
>   Read count  374 (0 failed)
>   Trim count  106 (0 failed)
>
> Times
>   Erase time 86000.00
>   Write time 33840.00
>   Read time  7000.00
>   Total time 126840.00


> ❯ ./ssd.py -T direct -n 1000 -J -G 7 -g 6 -S
>
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvvvvvvv vvvvvvvvvv vvvvvvvvvv vvvvvvvvvv vvvvvvvvvv iiiiiiiiii iiiiiiiiii
> Data  CpV4vO7pEO SFN4C2ZOhD AhM57YPmZB zF3IWfSfh4 A4MLxfbMrT
> Live    ++++ +++ + +++++ ++ ++++++ +++ ++ +++++++ +++++++ ++
>
> Physical Operations Per Block
> Erases 117        104         98        110         91          0          0          Sum: 520
> Writes 1062        924        883        1027        828          0          0          Sum: 4724
> Reads  1128        988        944        1093        895          0          0          Sum: 5048
>
> Logical Operation Sums
>   Write count 520 (0 failed)
>   Read count  374 (0 failed)
>   Trim count  106 (0 failed)
>
> Times
>   Erase time 520000.00
>   Write time 188960.00
>   Read time  50480.00
>   Total time 759440.00


> ./ssd.py -T ideal -n 1000 -J -G 7 -g 6 -S
> Block 0          1          2          3          4          5          6
> Page  0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666
>       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
> State vvvvvvvvvv vvvvvvvvvv vvvvvvvvvv vvvvvvvvvv vvvvvvvvvv iiiiiiiiii iiiiiiiiii
> Data  CpV4vO7pEO SFN4C2ZOhD AhM57YPmZB zF3IWfSfh4 A4MLxfbMrT
> Live    ++++ +++ + +++++ ++ ++++++ +++ ++ +++++++ +++++++ ++
>
> Physical Operations Per Block
> Erases   0          0          0          0          0          0          0          Sum: 0
> Writes 117        104         98        110         91          0          0          Sum: 520
> Reads   76         74         71         76         77          0          0          Sum: 374
>
> Logical Operation Sums
>   Write count 520 (0 failed)
>   Read count  374 (0 failed)
>   Trim count  106 (0 failed)
>
> Times
>   Erase time 0.00
>   Write time 20800.00
>   Read time  3740.00
>   Total time 24540.00


## 9. One last aspect to explore is workload skew. Adding skew to the workload changes writes such that more writes occur to some smaller fraction of the logical block space. For example, running with -K 80/20 makes 80% of the writes go to 20% of the blocks. Pick some different skews and perform many randomly-chosen operations (e.g.,-n 1000), using first -T direct to understand the skew, and then -T log to see the impact on a log-structured device. What do you expect will happen? One other small skew control to explore is -k 100; by adding this flag to a skewed workload, the first 100 writes are not skewed. The idea is to first create a lot of data, but then only update some of it. What impact might that have upon a garbage collector?

> ❯ ./ssd.py -T direct -n 1000 -J -G 7 -g 6 -S -K 80/20 -J
>
> Data  JJK36oXJrP g GN NXt V 4rx96WHs D A4C NWpt6E MJ J  DeEK
> Live  +++++++ ++      +++   +   +    +     + +       +  +++
>
> Logical Operation Sums
>   Write count 520 (0 failed)
>   Read count  380 (0 failed)
>   Trim count  100 (0 failed)
>
> Times
>   Erase time 520000.00
>   Write time 189400.00
>   Read time  50730.00
>   Total time 760130.00


> ❯ ./ssd.py -T log -n 1000 -J -G 7 -g 6 -S -K 80/20 -J
>
> Data  Dt16ESJoXJ P64K9r                zCHpINDg3x NeGJ1JEppV ixkpdXWF85 cJ5I3JNpBK
> Live  ++ ++ ++++ ++++ +                      +    ++ +    +       +         + +
>
> Logical Operation Sums
>   Write count 576 (0 failed)
>   Read count  380 (0 failed)
>   Trim count  100 (0 failed)
>
> Times
>   Erase time 59000.00
>   Write time 23040.00
>   Read time  4360.00
>   Total time 86400.00
> gc: 52


> ./ssd.py -T direct -n 1000 -J -G 7 -g 6 -S -K 80/20 -J -k 100
>
> Data  J3i3I1XJJ8 g G KN1txV 5rx96WHsdD z4C9qWpt67 MJNLGCWe0K
> Live  +  ++++ ++      +     +   + +  +       +       +   +
>
> Logical Operation Sums
>   Write count 523 (0 failed)
>   Read count  370 (0 failed)
>   Trim count  107 (0 failed)
>
> Times
>   Erase time 523000.00
>   Write time 190720.00
>   Read time  50900.00
>   Total time 764620.00


> ./ssd.py -T log -n 1000 -J -G 7 -g 6 -S -K 80/20 -J -k 100
>
> Data             JYVfvL5utt MLITc8SZpL 865L53zHpI NC4Dg3xNJ1 JeEppVikpd XWF85cI3J
> Live                   +     +          +     +      +   +++  +  +      +  +  +++
>
> Logical Operation Sums
>   Write count 629 (0 failed)
>   Read count  370 (0 failed)
>   Trim count  107 (0 failed)
>
> Times
>   Erase time 64000.00
>   Write time 25160.00
>   Read time  4760.00
>   Total time 93920.00
> gc: 56

adding `-k 100` increases the erase sum and causes the live data to be more randomly distributed across the disk, which probably causes the gc to run more, having to clean up data in between the live pages.
