### 1. First, run `fsck.py -D`; this flag turns off any corruption, and thus you can use it to generate a random file system, and see if you can determine which files and directories are in there. So, go ahead and do that! Use the `-p` flag to see if you were right. Try this for a few different randomly-generated file systems by setting the seed (-s) to different values, like 1, 2, and 3.

> ./fsck.py -D
>
> inode bitmap 1000100010000101
> inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:1]
> data bitmap  1000001000001000
> data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []

/g
/g/s
/w (empty dir)
/m (empty file)
/z (empty file, hardlinked to /m)

> ./fsck.py -D -s 1
>
> inode bitmap 1000100110010001
> inodes       [d a:0 r:4] [] [] [] [f a:-1 r:1] [] [] [d a:10 r:2] [d a:15 r:2] [] [] [f a:-1 r:3] [] [] [] [f a:-1 r:1]
> data bitmap  1000000000100001
> data         [(.,0) (..,0) (m,7) (a,8) (g,11)] [] [] [] [] [] [] [] [] [] [(.,7) (..,0) (m,15) (e,11)] [] [] [] [] [(.,8) (..,0) (r,4) (w,11)]

files:
/m/m (empty file)
/m/e (empty file)
/a/r
/a/w (empty file)
/g


dirs:
/
/m
/a


### 2. Now, let’s introduce a corruption. Run `./fsck.py -S 1` to start. Can you see what inconsistency is introduced? How would you fix it in a real file system repair tool? Use `-c` to check if you were right.

> inode bitmap 1000100010000001
> inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:1]
> data bitmap  1000001000001000
> data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []

files:
/g/s (empty file)
/m (i: 13)
/z (i: 13)

dirs:
/g
/w (empty)


Inode Bitmap corrupt bit 13


### 3. Change the seed to `-S 3` or `-S 19`; which inconsistency do you see? Use `-c` to check your answer. What is different in these two cases?

> ❯ ./fsck.py -S 3
> ARG seed 0
> ARG seedCorrupt 3
> ARG numInodes 16
> ARG numData 16
> ARG numRequests 15
> ARG printFinal False
> ARG whichCorrupt -1
> ARG dontCorrupt False
>
> Final state of file system:
>
> inode bitmap 1000100010000101
> inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:2]
> data bitmap  1000001000001000
> data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []

files:
/g/s
/m
/z


dirs:
/
/g
/w (empty dir)

Inode 15 ref count increased
(from 1 to 2)


> ❯ ./fsck.py -S 19 -c
> ARG seed 0
> ARG seedCorrupt 19
> ARG numInodes 16
> ARG numData 16
> ARG numRequests 15
> ARG printFinal False
> ARG whichCorrupt -1
> ARG dontCorrupt False
>
> Final state of file system:
>
> inode bitmap 1000100010000101
> inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:1] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:1]
> data bitmap  1000001000001000
> data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []

files:
/g/s (empty)
/m
/z

dirs:
/
/g
/w (empty dir)

Inode 8 ref count decremented
(from 2 to 1)


The difference in these cases is that one is a file, made incorrect by incrementing the ref count from 1 to 2.
The other is a dir, made incorrect by decrementing the ref count from 2 to 1.

Dirs have a minimum of 2 refs because of:
 * ref from the inode which stores the dir's dir ents - `(., I)`, where I is the inode index
 * ref from the parent inode which stores the dir's filename.

Files have a minimum of 1 ref, because it doesn't need an inode to reference it, outside of the dir which stores its filename.



### 4. Change the seed to `-S 5`; which inconsistency do you see? How hard would it be to fix this problem in an automatic way? Use `-c` to check your answer. Then, introduce a similar inconsistency with `-S 38`; is this harder/possible to detect? Finally, use `-S 642`; is this inconsistency detectable? If so, how would you fix the file system?

> ❯ ./fsck.py -S 5
> ARG seed 0
> ARG seedCorrupt 5
> ARG numInodes 16
> ARG numData 16
> ARG numRequests 15
> ARG printFinal False
> ARG whichCorrupt -1
> ARG dontCorrupt False
>
> Final state of file system:
>
> inode bitmap 1000100010000101
> inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:1]
> data bitmap  1000001000001000
> data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (y,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []

files:
/g/y (empty)
/m
/z

dirs:
/
/g
/w (empty dir)

File name /g/y was renamed. We can detect it by storing the file name in meta data, perhaps on the data block.

> ❯ ./fsck.py -S 38
> ARG seed 0
> ARG seedCorrupt 38
> ARG numInodes 16
> ARG numData 16
> ARG numRequests 15
> ARG printFinal False
> ARG whichCorrupt -1
> ARG dontCorrupt False
>
> Final state of file system:
>
> inode bitmap 1000100010000101
> inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:1]
> data bitmap  1000001000001000
> data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (b,0)] [] [] []

files:
/g/s
/m
/z

dirs:
/
/g
/w

In Inode 12, the dir ent `(.., 0)` was altered to reference a different name: `(b,0)`

We can detect this by ensuring the constraint that each dir contain 1 and only 1 ref to each of its `.` and `..` entries.


> ❯ ./fsck.py -S 642
> ARG seed 0
> ARG seedCorrupt 642
> ARG numInodes 16
> ARG numData 16
> ARG numRequests 15
> ARG printFinal False
> ARG whichCorrupt -1
> ARG dontCorrupt False
>
> Final state of file system:
>
> inode bitmap 1000100010000101
> inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:1]
> data bitmap  1000001000001000
> data         [(.,0) (..,0) (w,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []



dirent `(g,8)` was renamed to `(w,8)`. We can detect this by enforcing a constraint to prevent duplicate dirent names within the same dir.
We can also store the dirent's name in the data block to provide an extra way to verify if the name has been changed.


### 5. Change the seed to `-S 6` or `-S 13`; which inconsistency do you see? Use `-c` to check your answer. What is the difference across these two cases? What should the repair tool do when encountering such a situation?

> ❯ ./fsck.py -S 6
> ARG seed 0
> ARG seedCorrupt 6
> ARG numInodes 16
> ARG numData 16
> ARG numRequests 15
> ARG printFinal False
> ARG whichCorrupt -1
> ARG dontCorrupt False
>
> Final state of file system:
>
> inode bitmap 1000100010000101
> inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [d a:-1 r:1] [f a:-1 r:2] [] [f a:-1 r:1]
> data bitmap  1000001000001000
> data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []

Inode 12 was updated from `[]` to `[d a:-1 r:1]`.

> ❯ ./fsck.py -S 13
> ARG seed 0
> ARG seedCorrupt 13
> ARG numInodes 16
> ARG numData 16
> ARG numRequests 15
> ARG printFinal False
> ARG whichCorrupt -1
> ARG dontCorrupt False
>
> Final state of file system:
>
> inode bitmap 1000100010000101
> inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [f a:-1 r:1] [] [] [f a:-1 r:2] [] [f a:-1 r:1]
> data bitmap  1000001000001000
> data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []


Inode 10 was updated from `[]` to `[f a:-1 r:1]`.


A repair tool can check the references and delete any inodes that aren't referenced by a data block.


### 6. Change the seed to `-S 9`; which inconsistency do you see? Use `-c` to check your answer. Which piece of information should a check-and-repair tool trust in this case?

> ❯ ./fsck.py -S 9
> ARG seed 0
> ARG seedCorrupt 9
> ARG numInodes 16
> ARG numData 16
> ARG numRequests 15
> ARG printFinal False
> ARG whichCorrupt -1
> ARG dontCorrupt False
>
> Final state of file system:
>
> inode bitmap 1000100010000101
> inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [] [d a:-1 r:2] [] [f a:-1 r:1]
> data bitmap  1000001000001000
> data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []

The file type at Inode 13 was changed from `f` to `d`

A dir should always have an associated data block, and if it doesn't, then we can trust that it was either incorrectly added, or was changed to a dir from a file.

If it has refs, then we can trust the latter.


### 7. Change the seed to `-S 15`; which inconsistency do you see? Use `-c` to check your answer. What can a repair tool do in this case? If no repair is possible, how much data is lost?


> ❯ ./fsck.py -S 15
> ARG seed 0
> ARG seedCorrupt 15
> ARG numInodes 16
> ARG numData 16
> ARG numRequests 15
> ARG printFinal False
> ARG whichCorrupt -1
> ARG dontCorrupt False
>
> Final state of file system:
>
> inode bitmap 1000100010000101
> inodes       [f a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:1]
> data bitmap  1000001000001000
> data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []

Inode 0 file type was changed from `f` to `d`.

Repair tool can assume a hard-coded inode for root dir, and ensure it's always a dir.

If no repair is done, all data is lost.


### 8. Change the seed to `-S 10`; which inconsistency do you see? Use `-c` to check your answer. Is there redundancy in the file system structure here that can help a repair?

> ❯ ./fsck.py -S 10
> ARG seed 0
> ARG seedCorrupt 10
> ARG numInodes 16
> ARG numData 16
> ARG numRequests 15
> ARG printFinal False
> ARG whichCorrupt -1
> ARG dontCorrupt False
>
> Final state of file system:
>
> inode bitmap 1000100010000101
> inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:1]
> data bitmap  1000001000001000
> data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,3)] [] [] []

Data block 12's dirent `(..,0)` was changed to `(..,3)`.

There is redundancy in the parent dir in this case, based on the dirent of `(w,4)` in data block 0 for inode 0.


### 9. Change the seed to `-S 16` and `-S 20`; which inconsistency do you see? Use `-c` to check your answer. How should the repair tool fix the problem?

> ❯ ./fsck.py -S 16
> ARG seed 0
> ARG seedCorrupt 16
> ARG numInodes 16
> ARG numData 16
> ARG numRequests 15
> ARG printFinal False
> ARG whichCorrupt -1
> ARG dontCorrupt False
>
> Final state of file system:
>
> inode bitmap 1000100010000101
> inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [] [f a:7 r:2] [] [f a:-1 r:1]
> data bitmap  1000001000001000
> data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []

Inode 13 was changed from `[f a:0 r:2]` to `[f a:7 r:2]`

> ❯ ./fsck.py -S 20
> ARG seed 0
> ARG seedCorrupt 20
> ARG numInodes 16
> ARG numData 16
> ARG numRequests 15
> ARG printFinal False
> ARG whichCorrupt -1
> ARG dontCorrupt False
>
> Final state of file system:
>
> inode bitmap 1000100010000101
> inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:11 r:2] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:1]
> data bitmap  1000001000001000
> data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []

Inode 8 was changed from `[d a:6 r:2]` to `[d a:11 r:2]`.

In these cases, the repair tool can match any blocks that have no references to any inodes that reference empty blocks.

Search the data blocks, find which may be a directory and . is pointed to \g.
