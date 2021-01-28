## 1. Examine the file `in.largefile`, and then run the simulator with flag `-f in.largefile` and `-L 4`. The latter sets the large-file exception to 4 blocks. What will the resulting allocation look like? Run with `-c` to check.

Since largefile allocates a file with 40 blocks, and we have 10 groups of 30 blocks each, the `-L 4` flag will allocate 4 blocks to each of the 10 groups.

## 2. Now run with `-L 30`. What do you expect to see? Once again, turn on `-c` to see if you were right. You can also use `-S` to see exactly which blocks were allocated to the file `/a`.

This should be the same as running with `-L` disabled. Since largefile allocates a file with 40 blocks, and there are 30 data blocks available per group, we'll allocate 29 blocks for `/a` before running out of space in the first group (first block is for the `/` dir). So we don't actually run up again the large file limit.

## 3. Now we will compute some statistics about the file. The first is something we call `filespan`, which is the max distance between any two data blocks of the file or between the inode and any data block. Calculate the filespan of `/a`. Run `ffs.py -f in.largefile -L 4 -T -c` to see what it is. Do the same with `-L 100`. What difference do you expect in filespan as the large-file exception parameter changes from low values to high values?

The spans will decrease as we increase `-L` because the large file won't be spread across so many groups.

## 4. Now let’s look at a new input file, `in.manyfiles`. How do you think the FFS policy will lay these files out across groups? (you can run with `-v` to see what files and directories are created, or just `cat in.manyfiles`). Run the simulator with `-c` to see if you were right.

It will lay the files across 3 groups - one group per directory.

## 5. A metric to evaluate FFS is called `dirspan`. This metric calculates the spread of files within a particular directory, specifically the max distance between the inodes and data blocks of all files in the directory and the inode and datablock of the directory itself. Run with `in.manyfiles` and the `-T` flag, and calculate the dirspan of the three directories. Run with -c` to check. How good of a job does FFS do in minimizing dirspan?

The files don't take up enough space to overflow any particular group, so the dirspan is low. The dirspan is largely affected by the size of the inode region, which makes up the span between the inode block and the data block.

## 6. Now change the size of the inode table per group to 5 (`-I 5`). How do you think this will change the layout of the files? Run with `-c` to see if you were right. How does it affect the dirspan?

Files in the same dir are spread across multiple groups now, increasing dirspan.
Filespan is slightly decreased on the previously more crowded groups, since there are less blocks consumed between the inode table and the file's data block.


## 7. Which group should FFS place inode of a new directory in? The default (simulator) policy looks for the group with the most free inodes. A different policy looks for a set of groups with the most free inodes. For example, if you run with `-A 2`, when allocating a new directory, the simulator will look at groups in pairs and pick the best pair for the allocation. Run `./ffs.py -f in.manyfiles -I 5 -A 2 -c` to see how allocation changes with this strategy. How does it affect dirspan? Why might this policy be good?

The policy lowers the dirspan since it keeps other dirs further away.

Unfortunately it's not as effective as it could be in this example since we end up running out of groups and storing files across multiple dirs in the same group.


## 8. One last policy change we will explore relates to file fragmentation. Run `./ffs.py -f in.fragmented -v` and see if you can predict how the files that remain are allocated. Run with `-c` to confirm your answer. What is interesting about the data layout of file `/i`? Why is it problematic?

The data layout of `/i` is fragmented and non-contiguous, requiring the disk to perform more seeks/rotations when reading/writing the entire file. This will make it slow.


## 9. A new policy, which we call contiguous allocation (`-C`), tries to ensure that each file is allocated contiguously. Specifically, with `-C n`, the file system tries to ensure that `n` contiguous blocks are free within a group before allocating a block. Run`./ffs.py -f in.fragmented -v -C 2 -c` to see the difference. How does layout change as the parameter passed to `-C` increases? Finally, how does `-C` affect filespan and dirspan?

It changes the layout from the last example by ensuring that `/i` is allocated a continuous set of data blocks.

As `-C` increases, files that are less than size `-C` will cause external fragmentation in the data block region.

`-C` increases filespan and dirspan.
