## 1. Run `./lfs.py -n 3`, perhaps varying the seed (-s). Can you figure out which commands were run to generate the final file system contents? Can you tell which order those commands were issued? Finally, can you determine the liveness of each block in the final file system state? Use `-o` to show which commands were run, and `-c` to show the liveness of the final file system state. How much harder does the task become for you as you increase the number of commands issued (i.e., change `-n 3` to `-n 5`)?

> ❯ ./lfs.py -n 3
>
> INITIAL file system contents:
> [   0 ] live checkpoint: 3 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   1 ] live [.,0] [..,0] -- -- -- -- -- --
> [   2 ] live type:dir size:1 refs:2 ptrs: 1 -- -- -- -- -- -- --
> [   3 ] live chunk(imap): 2 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
>
> command?
> command?
> command?
>
> FINAL file system contents:
> [   0 ] L    checkpoint: 14 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   1 ] ?    [.,0] [..,0] -- -- -- -- -- --
> [   2 ] ?    type:dir size:1 refs:2 ptrs: 1 -- -- -- -- -- -- --
> [   3 ] ?    chunk(imap): 2 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   4 ] ?    [.,0] [..,0] [ku3,1] -- -- -- -- --
> [   5 ] ?    type:dir size:1 refs:2 ptrs: 4 -- -- -- -- -- -- --
> [   6 ] ?    type:reg size:0 refs:1 ptrs: -- -- -- -- -- -- -- --
> [   7 ] ?    chunk(imap): 5 6 -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   8 ] L    z0z0z0z0z0z0z0z0z0z0z0z0z0z0z0z0
> [   9 ] L    type:reg size:8 refs:1 ptrs: -- -- -- -- -- -- -- 8
> [  10 ] ?    chunk(imap): 5 9 -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [  11 ] L    [.,0] [..,0] [ku3,1] [qg9,2] -- -- -- --
> [  12 ] L    type:dir size:1 refs:2 ptrs: 11 -- -- -- -- -- -- --
> [  13 ] L    type:reg size:0 refs:1 ptrs: -- -- -- -- -- -- -- --
> [  14 ] L    chunk(imap): 12 9 13 -- -- -- -- -- -- -- -- -- -- -- -- --

1. create file `/ku3`

2. write to the last data block in file `/ku3`
(offset 7, size 4)

3. write file `/qg9`, which is empty.


As we increase the number of commands, it takes linearly more time to see what happened.



## 2. If you find the above painful, you can help yourself a little bit by showing the set of updates caused by each specific command. To do so, run `./lfs.py -n 3 -i`. Now see if it is easier to understand what each command must have been. Change the random seed to get different commands to interpret (e.g.,-s 1,-s 2,-s 3, etc.).

## 3. To further test your ability to figure out what updates are made to disk by each command, run the following: `./lfs.py -o -F -s 100` (and perhaps a few other random seeds). This just shows a set of commands and does NOT show you the final state of the filesystem. Can you reason about what the final state of the file system must be?


> INITIAL file system contents:
> [   0 ] live checkpoint: 3 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   1 ] live [.,0] [..,0] -- -- -- -- -- --
> [   2 ] live type:dir size:1 refs:2 ptrs: 1 -- -- -- -- -- -- --
> [   3 ] live chunk(imap): 2 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
>
> create file /us7
> write file  /us7 offset=4 size=0
> write file  /us7 offset=7 size=7

**create file /us7**
> [   0 ] live checkpoint: 5 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
...
> [   4 ] live [.,0] [..,0] [us7,1] -- -- -- -- --
> [   5 ] live type:dir size:0 refs:2 ptrs: 4 -- -- -- -- -- -- --
> [   6 ] live type:reg size:0 refs:2 ptrs: -- -- -- -- -- -- -- --
> [   7 ] live chunk(imap): 5 6 -- -- -- -- -- -- -- -- -- -- -- -- -- --

**write file  /us7 offset=4 size=0**
> [   0 ] live checkpoint: 9 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
...
> [   8 ] live type:reg size:0 refs:2 ptrs: -- -- -- -- -- -- -- --
> [   9 ] live chunk(imap): 5 8 -- -- -- -- -- -- -- -- -- -- -- -- -- --

**write file  /us7 offset=7 size=7**
> [   0 ] live checkpoint: 12 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
...
> [  10 ] live adasdfasdf
> [  11 ] live type:reg size:8 refs:2 ptrs: -- -- -- -- -- -- -- -- 10
> [  12 ] live chunk(imap): 5 11 -- -- -- -- -- -- -- -- -- -- -- -- -- --


> Final file system contents:
> [   0 ] live checkpoint: 12 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   1 ] live [.,0] [..,0] -- -- -- -- -- --
> [   2 ] live type:dir size:1 refs:2 ptrs: 1 -- -- -- -- -- -- --
> [   3 ] live chunk(imap): 2 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   4 ] live [.,0] [..,0] [us7,1] -- -- -- -- --
> [   5 ] live type:dir size:0 refs:2 ptrs: 4 -- -- -- -- -- -- --
> [   6 ] live type:reg size:0 refs:2 ptrs: -- -- -- -- -- -- -- --
> [   7 ] live chunk(imap): 5 6 -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   8 ] live type:reg size:0 refs:2 ptrs: -- -- -- -- -- -- -- --
> [   9 ] live chunk(imap): 5 8 -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [  10 ] live adasdfasdf
> [  11 ] live type:reg size:8 refs:2 ptrs: -- -- -- -- -- -- -- -- 10
> [  12 ] live chunk(imap): 5 11 -- -- -- -- -- -- -- -- -- -- -- -- -- --

## 4. Now see if you can determine which files and directories are live after a number of file and directory operations. Run `./lfs.py -n 20 -s 1` and then examine the final file system state. Can you figure out which pathnames are valid? Run `./lfs.py -n 20 -s 1 -c -v` to see the results. Run with `-o` to see if your answers match up given the series of random commands. Use different random seeds to get more problems.

> FINAL file system contents:
> [   0 ]      checkpoint: 99 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   1 ]      [.,0] [..,0] -- -- -- -- -- --
> [   2 ]      type:dir size:1 refs:2 ptrs: 1 -- -- -- -- -- -- --
> [   3 ]      chunk(imap): 2 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   4 ]      [.,0] [..,0] [tg4,1] -- -- -- -- --
> [   5 ]      type:dir size:1 refs:2 ptrs: 4 -- -- -- -- -- -- --
> [   6 ]      type:reg size:0 refs:1 ptrs: -- -- -- -- -- -- -- --
> [   7 ]      chunk(imap): 5 6 -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   8 ]      type:reg size:6 refs:1 ptrs: -- -- -- -- -- -- -- --
> [   9 ]      chunk(imap): 5 8 -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [  10 ]      [.,0] [..,0] [tg4,1] [lt0,2] -- -- -- --
> [  11 ]      type:dir size:1 refs:2 ptrs: 10 -- -- -- -- -- -- --
> [  12 ]      type:reg size:0 refs:1 ptrs: -- -- -- -- -- -- -- --
> [  13 ]      chunk(imap): 11 8 12 -- -- -- -- -- -- -- -- -- -- -- -- --
> [  14 ]      n0n0n0n0n0n0n0n0n0n0n0n0n0n0n0n0
> [  15 ]      y1y1y1y1y1y1y1y1y1y1y1y1y1y1y1y1
> [  16 ]      p2p2p2p2p2p2p2p2p2p2p2p2p2p2p2p2
> [  17 ]      l3l3l3l3l3l3l3l3l3l3l3l3l3l3l3l3
> [  18 ]      h4h4h4h4h4h4h4h4h4h4h4h4h4h4h4h4
> [  19 ]      o5o5o5o5o5o5o5o5o5o5o5o5o5o5o5o5
> [  20 ]      y6y6y6y6y6y6y6y6y6y6y6y6y6y6y6y6
> [  21 ]      type:reg size:8 refs:1 ptrs: -- 14 15 16 17 18 19 20
> [  22 ]      chunk(imap): 11 8 21 -- -- -- -- -- -- -- -- -- -- -- -- --
> [  23 ]      [.,0] [..,0] [tg4,1] [lt0,2] [oy3,1] -- -- --
> [  24 ]      type:dir size:1 refs:2 ptrs: 23 -- -- -- -- -- -- --
> [  25 ]      type:reg size:6 refs:2 ptrs: -- -- -- -- -- -- -- --
> [  26 ]      chunk(imap): 24 25 21 -- -- -- -- -- -- -- -- -- -- -- -- --
> [  27 ]      [.,0] [..,0] [tg4,1] [lt0,2] [oy3,1] [af4,3] -- --
> [  28 ]      type:dir size:1 refs:2 ptrs: 27 -- -- -- -- -- -- --
> [  29 ]      type:reg size:0 refs:1 ptrs: -- -- -- -- -- -- -- --
> [  30 ]      chunk(imap): 28 25 21 29 -- -- -- -- -- -- -- -- -- -- -- --
> [  31 ]      a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0
> [  32 ]      type:reg size:6 refs:2 ptrs: -- 31 -- -- -- -- -- --
> [  33 ]      chunk(imap): 28 32 21 29 -- -- -- -- -- -- -- -- -- -- -- --
> [  34 ] L    u0u0u0u0u0u0u0u0u0u0u0u0u0u0u0u0
> [  35 ]      v1v1v1v1v1v1v1v1v1v1v1v1v1v1v1v1
> [  36 ]      x2x2x2x2x2x2x2x2x2x2x2x2x2x2x2x2
> [  37 ]      t3t3t3t3t3t3t3t3t3t3t3t3t3t3t3t3
> [  38 ]      v4v4v4v4v4v4v4v4v4v4v4v4v4v4v4v4
> [  39 ]      n5n5n5n5n5n5n5n5n5n5n5n5n5n5n5n5
> [  40 ]      type:reg size:8 refs:1 ptrs: 34 35 36 37 38 39 19 20
> [  41 ]      chunk(imap): 28 32 40 29 -- -- -- -- -- -- -- -- -- -- -- --
> [  42 ]      o0o0o0o0o0o0o0o0o0o0o0o0o0o0o0o0
> [  43 ]      l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1
> [  44 ]      b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2
> [  45 ]      w3w3w3w3w3w3w3w3w3w3w3w3w3w3w3w3
> [  46 ]      o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4
> [  47 ]      f5f5f5f5f5f5f5f5f5f5f5f5f5f5f5f5
> [  48 ]      n6n6n6n6n6n6n6n6n6n6n6n6n6n6n6n6
> [  49 ]      type:reg size:8 refs:2 ptrs: -- 42 43 44 45 46 47 48
> [  50 ]      chunk(imap): 28 49 40 29 -- -- -- -- -- -- -- -- -- -- -- --
> [  51 ]      [.,0] [..,0] -- [lt0,2] [oy3,1] [af4,3] -- --
> [  52 ]      type:dir size:1 refs:2 ptrs: 51 -- -- -- -- -- -- --
> [  53 ]      type:reg size:8 refs:1 ptrs: -- 42 43 44 45 46 47 48
> [  54 ]      chunk(imap): 52 53 40 29 -- -- -- -- -- -- -- -- -- -- -- --
> [  55 ]      m0m0m0m0m0m0m0m0m0m0m0m0m0m0m0m0
> [  56 ]      j1j1j1j1j1j1j1j1j1j1j1j1j1j1j1j1
> [  57 ]      i2i2i2i2i2i2i2i2i2i2i2i2i2i2i2i2
> [  58 ]      type:reg size:8 refs:1 ptrs: -- -- -- -- -- 55 56 57
> [  59 ]      chunk(imap): 52 53 40 58 -- -- -- -- -- -- -- -- -- -- -- --
> [  60 ] L    a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0
> [  61 ]      f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1
> [  62 ]      type:reg size:8 refs:1 ptrs: -- -- -- -- -- 60 61 57
> [  63 ]      chunk(imap): 52 53 40 62 -- -- -- -- -- -- -- -- -- -- -- --
> [  64 ] L    e0e0e0e0e0e0e0e0e0e0e0e0e0e0e0e0
> [  65 ] L    p1p1p1p1p1p1p1p1p1p1p1p1p1p1p1p1
> [  66 ] L    type:reg size:8 refs:1 ptrs: -- -- -- -- -- 60 64 65
> [  67 ]      chunk(imap): 52 53 40 66 -- -- -- -- -- -- -- -- -- -- -- --
> [  68 ] L    u0u0u0u0u0u0u0u0u0u0u0u0u0u0u0u0
> [  69 ] L    v1v1v1v1v1v1v1v1v1v1v1v1v1v1v1v1
> [  70 ] L    g2g2g2g2g2g2g2g2g2g2g2g2g2g2g2g2
> [  71 ]      v3v3v3v3v3v3v3v3v3v3v3v3v3v3v3v3
> [  72 ]      r4r4r4r4r4r4r4r4r4r4r4r4r4r4r4r4
> [  73 ]      c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5
> [  74 ]      type:reg size:8 refs:1 ptrs: 34 68 69 70 71 72 73 20
> [  75 ]      chunk(imap): 52 53 74 66 -- -- -- -- -- -- -- -- -- -- -- --
> [  76 ] L    a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0
> [  77 ] L    a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1
> [  78 ] L    t2t2t2t2t2t2t2t2t2t2t2t2t2t2t2t2
> [  79 ] L    g3g3g3g3g3g3g3g3g3g3g3g3g3g3g3g3
> [  80 ] L    type:reg size:8 refs:1 ptrs: 34 68 69 70 76 77 78 79
> [  81 ]      chunk(imap): 52 53 80 66 -- -- -- -- -- -- -- -- -- -- -- --
> [  82 ]      [.,0] [..,0] [ln7,4] [lt0,2] [oy3,1] [af4,3] -- --
> [  83 ]      [.,4] [..,0] -- -- -- -- -- --
> [  84 ]      type:dir size:1 refs:3 ptrs: 82 -- -- -- -- -- -- --
> [  85 ]      type:dir size:1 refs:2 ptrs: 83 -- -- -- -- -- -- --
> [  86 ]      chunk(imap): 84 53 80 66 85 -- -- -- -- -- -- -- -- -- -- --
> [  87 ]      type:reg size:8 refs:1 ptrs: -- 42 43 44 45 46 47 48
> [  88 ]      chunk(imap): 84 87 80 66 85 -- -- -- -- -- -- -- -- -- -- --
> [  89 ]      [.,4] [..,0] [zp3,5] -- -- -- -- --
> [  90 ]      type:dir size:1 refs:2 ptrs: 89 -- -- -- -- -- -- --
> [  91 ] L    type:reg size:0 refs:1 ptrs: -- -- -- -- -- -- -- --
> [  92 ]      chunk(imap): 84 87 80 66 90 91 -- -- -- -- -- -- -- -- -- --
> [  93 ] L    [.,4] [..,0] [zp3,5] [zu5,6] -- -- -- --
> [  94 ] L    type:dir size:1 refs:2 ptrs: 93 -- -- -- -- -- -- --
> [  95 ] L    type:reg size:0 refs:1 ptrs: -- -- -- -- -- -- -- --
> [  96 ]      chunk(imap): 84 87 80 66 94 91 95 -- -- -- -- -- -- -- -- --
> [  97 ] L    [.,0] [..,0] [ln7,4] [lt0,2] -- [af4,3] -- --
> [  98 ] L    type:dir size:1 refs:3 ptrs: 97 -- -- -- -- -- -- --
> [  99 ] L    chunk(imap): 98 -- 80 66 94 91 95 -- -- -- -- -- -- -- -- --


## 5. Now let’s issue some specific commands. First, let’s create a file and write to it repeatedly. To do so, use the `-L` flag, which lets you specify specific commands to execute. Let’s create the file ”/foo” and write to it four times: `-L c,/foo:w,/foo,0,1:w,/foo,1,1:w,/foo,2,1:w,/foo,3,1-o`. See if you can determine the liveness of the final file system state; use `-c` to check your answers.

> ❯ ./lfs.py -n 20 -s 1 -L c,/foo:w,/foo,0,1:w,/foo,1,1:w,/foo,2,1:w,/foo,3,1 -o
>
> INITIAL file system contents:
> [   0 ] live checkpoint: 3 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   1 ] live [.,0] [..,0] -- -- -- -- -- --
> [   2 ] live type:dir size:1 refs:2 ptrs: 1 -- -- -- -- -- -- --
> [   3 ] live chunk(imap): 2 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
>
> create file /foo
> write file  /foo offset=0 size=1
> write file  /foo offset=1 size=1
> write file  /foo offset=2 size=1
> write file  /foo offset=3 size=1
>
> FINAL file system contents:
> [   0 ]      checkpoint: 19 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   1 ]      [.,0] [..,0] -- -- -- -- -- --
> [   2 ]      type:dir size:1 refs:2 ptrs: 1 -- -- -- -- -- -- --
> [   3 ]      chunk(imap): 2 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   4 ] L    [.,0] [..,0] [foo,1] -- -- -- -- --
> [   5 ] L    type:dir size:1 refs:2 ptrs: 4 -- -- -- -- -- -- --
> [   6 ]      type:reg size:0 refs:1 ptrs: -- -- -- -- -- -- -- --
> [   7 ]      chunk(imap): 5 6 -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   8 ] L    d0d0d0d0d0d0d0d0d0d0d0d0d0d0d0d0
> [   9 ]      type:reg size:1 refs:1 ptrs: 8 -- -- -- -- -- -- --
> [  10 ]      chunk(imap): 5 9 -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [  11 ] L    w0w0w0w0w0w0w0w0w0w0w0w0w0w0w0w0
> [  12 ]      type:reg size:2 refs:1 ptrs: 8 11 -- -- -- -- -- --
> [  13 ]      chunk(imap): 5 12 -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [  14 ] L    t0t0t0t0t0t0t0t0t0t0t0t0t0t0t0t0
> [  15 ]      type:reg size:3 refs:1 ptrs: 8 11 14 -- -- -- -- --
> [  16 ]      chunk(imap): 5 15 -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [  17 ] L    g0g0g0g0g0g0g0g0g0g0g0g0g0g0g0g0
> [  18 ] L    type:reg size:4 refs:1 ptrs: 8 11 14 17 -- -- -- --
> [  19 ] L    chunk(imap): 5 18 -- -- -- -- -- -- -- -- -- -- -- -- -- --



## 6. Now, let’s do the same thing, but with a single write operation instead of four.  Run `./lfs.py -o -L c,/foo:w,/foo,0,4` to create file ”/foo” and write 4 blocks with a single write operation. Compute the liveness again, and check if you are right with `-c`.What is the main difference between writing a file all at once (as we do here) versus doing it one block at a time (as above)? What does this tell you about the importance of buffering updates in main memory as the real LFS does?

> ❯ ./lfs.py -o -L c,/foo:w,/foo,0,4
>
> INITIAL file system contents:
> [   0 ] live checkpoint: 3 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   1 ] live [.,0] [..,0] -- -- -- -- -- --
> [   2 ] live type:dir size:1 refs:2 ptrs: 1 -- -- -- -- -- -- --
> [   3 ] live chunk(imap): 2 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
>
> create file /foo
> write file  /foo offset=0 size=4
>
> FINAL file system contents:
> [   0 ] L    checkpoint: 13 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   1 ]      [.,0] [..,0] -- -- -- -- -- --
> [   2 ]      type:dir size:1 refs:2 ptrs: 1 -- -- -- -- -- -- --
> [   3 ]      chunk(imap): 2 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   4 ] L    [.,0] [..,0] [foo,1] -- -- -- -- --
> [   5 ] L    type:dir size:1 refs:2 ptrs: 4 -- -- -- -- -- -- --
> [   6 ]      type:reg size:0 refs:1 ptrs: -- -- -- -- -- -- -- --
> [   7 ]      chunk(imap): 5 6 -- -- -- -- -- -- -- -- -- -- -- -- -- --
> [   8 ] L    v0v0v0v0v0v0v0v0v0v0v0v0v0v0v0v0
> [   9 ] L    t1t1t1t1t1t1t1t1t1t1t1t1t1t1t1t1
> [  10 ] L    k2k2k2k2k2k2k2k2k2k2k2k2k2k2k2k2
> [  11 ] L    g3g3g3g3g3g3g3g3g3g3g3g3g3g3g3g3
> [  12 ] L    type:reg size:4 refs:1 ptrs: 8 9 10 11 -- -- -- --
> [  13 ] L    chunk(imap): 5 12 -- -- -- -- -- -- -- -- -- -- -- -- -- --

The log is much shorter because there are fewer I/O's. This also results in less garbage to collect, and more contiguous data blocks.

## 7. Let’s do another specific example. First, run the following: `./lfs.py -L c,/foo:w,/foo,0,1`. What does this set of commands do? Now, run `./lfs.py -L c,/foo:w,/foo,7,1`. What does this set of commands do? How are the two different? What can you tell about the size field in the inode from these two sets of commands?

`./lfs.py -L c,/foo:w,/foo,0,1` creates `/foo`, then writes to `/foo` with offset of 0, size 1.

`./lfs.py -L c,/foo:w,/foo,7,1` creates `/foo`, then writes to `/foo` with offset 7, size 1.

The size of the inode increases from 1 to 8 in the second command due to internal fragmentation. (writing one block in the beginning vs one block at the end)

## 8. Now let’s look explicitly at file creation versus directory creation. Run simulations `./lfs.py -L c,/foo` and `./lfs.py -L d,/foo` to create a file and then a directory. What is similar about these runs,and what is different?

They both assign a new inode to the file/dir, but creating a dir also assigns a data block to store the dirents. The file is empty, so doesn't need a data block.

Creating a dir also increases the reference in the root dir.

## 9. The LFS simulator supports hard links as well. Run the following to study how they work: `./lfs.py -L c,/foo:l,/foo,/bar:l,/foo,/goo -o -i`.What blocks are written out when a hard link is created? How is this similar to just creating a new file, and how is it different? How does the reference count field change as links are created?

Hard links don't create a new inode file - they just add a dirent reference to the parent dir, and increment the ref count in the inode of the existing file.

Blocks changed: checkpoint region, inode of parent dir, inode of file, imap of parent dir.

Same: both update the checkpoint region, parent dir, and imap.
Different: a new file doesn't update the inode of the linked file (because there is no link). A new file creates a new inode.


## 10. LFS makes many different policy decisions.  We do not explore many of them here – perhaps something left for the future – but here is a simple one we do explore: the choice of inode number. First, run `./lfs.py -p c100 -n 10 -o -a s` to show the usual behavior with the ”sequential” allocation policy, which tries to use free inode numbers nearest to zero. Then, change to a ”random” policy by running `./lfs.py -p c100 -n 10 -o -a r` (the `-p c100` flag ensures 100 percent of the random operations are file creations). What on-disk differences does a random policy versus a sequential policy result in? What does this say about the importance of choosing inode numbers in a real LFS?

The inode indexes for new files are chosen randomly, resulting in more imap chunks that are distributed across the log.


## 11. One last thing we've been assuming is that the LFS simulator always updates the checkpoint region after each update. In the real LFS, that isn’t the case: it is updated periodically to avoid long seeks. Run `./lfs.py -N -i -o -s 1000` to see some operations and the intermediate and final states of the file system when the checkpoint region isn’t forced to disk. What would happen if the checkpoint region is never updated? What if it is updated periodically? Could you figure out how to recover the file system to the latest state by rolling forward in the log?

If the CR never updates, then none of our log segments will matter.

It's okay to update the CR periodically.

In this case, the latest log should have the imap, so having the CR point to that should be a reasonable way to recover.
We can also "roll forward" by starting at the latest CR, then read through the next segments and see if there are any valid updates within it. If so, update the FS accordingly to recover any missed data.
