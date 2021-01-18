## 1. Run the simulator with some different random seeds (say 17, 18, 19,20), and see if you can figure out which operations must have taken place between each state change.

> ./vsfs.py -s 17
> mkdir("/u")
> creat("/a")
> unlink("/a")
> mkdir("/z")
> mkdir("/s")
> creat("/z/x")
> link("/z/x", "u/b")
> unlink("/u/b")
> fd=open("/z/x", O_WRONLY|O_APPEND); write(fd, buf, BLOCKSIZE); close(fd);
> create("/u/b")


## 2. Now do the same, using different random seeds (say 21, 22, 23,24), except run with the `-r` flag, thus making you guess the state change while being shown the operation. What can you conclude about the inode and data-block allocation algorithms, in terms of which blocks they prefer to allocate?

> ./vsfs.py -s 21

Initial state

inode bitmap  10000000
inodes       [d a:0 r:2][][][][][][][]
data bitmap   10000000
data         [(.,0) (..,0)][][][][][][][]

mkdir("/o");

  State of file system (inode bitmap, inodes, data bitmap, data)?


inode bitmap  11000000
inodes       [d a:0 r:3][d a:1 r:2][][][][][][]
data bitmap   11000000
data         [(.,0) (..,0) (o, 1)][(.,1) (..,0)][][][][][][]


creat("/b");

  State of file system (inode bitmap, inodes, data bitmap, data)?

inode bitmap  11100000
inodes       [d a:0 r:3][d a:1 r:2][f a:-1 r:1][][][][][]
data bitmap   11000000
data         [(.,0) (..,0) (o, 1) (b, 2)][(.,1) (..,0)][][][][][][]

creat("/o/q");

  State of file system (inode bitmap, inodes, data bitmap, data)?

inode bitmap  11110000
inodes       [d a:0 r:3][d a:1 r:2][f a:-1 r:1][f a:-1 r:1][][][][]
data bitmap   11000000
data         [(.,0) (..,0) (o, 1) (b, 2)][(.,1) (..,0) (q, 3)][][][][][][]


fd=open("/b", O_WRONLY|O_APPEND); write(fd, buf, BLOCKSIZE); close(fd);

  State of file system (inode bitmap, inodes, data bitmap, data)?

inode bitmap  11110000
inodes       [d a:0 r:3][d a:1 r:2][f a:2 r:1][f a:-1 r:1][][][][]
data bitmap   11100000
data         [(.,0) (..,0) (o, 1) (b, 2)][(.,1) (..,0) (q, 3)][m][][][][][]



fd=open("/o/q", O_WRONLY|O_APPEND); write(fd, buf, BLOCKSIZE); close(fd);

  State of file system (inode bitmap, inodes, data bitmap, data)?

inode bitmap  11110000
inodes       [d a:0 r:3][d a:1 r:2][f a:2 r:1][f a:3 r:1][][][][]
data bitmap   11110000
data         [(.,0) (..,0) (o, 1) (b, 2)][(.,1) (..,0) (q, 3)][m][j][][][][]


creat("/o/j");

  State of file system (inode bitmap, inodes, data bitmap, data)?

inode bitmap  11111000
inodes       [d a:0 r:3][d a:1 r:2][f a:2 r:1][f a:3 r:1][f a:-1 r:1][][][]
data bitmap   11110000
data         [(.,0) (..,0) (o, 1) (b, 2)][(.,1) (..,0) (q, 3) (j, 4)][m][j][][][][]

unlink("/b");

  State of file system (inode bitmap, inodes, data bitmap, data)?

inode bitmap  11011000
inodes       [d a:0 r:3][d a:1 r:2][][f a:3 r:1][f a:-1 r:1][][][]
data bitmap   11010000
data         [(.,0) (..,0) (o, 1)][(.,1) (..,0) (q, 3) (j, 4)][][j][][][][]


fd=open("/o/j", O_WRONLY|O_APPEND); write(fd, buf, BLOCKSIZE); close(fd);

  State of file system (inode bitmap, inodes, data bitmap, data)?

inode bitmap  11011000
inodes       [d a:0 r:3][d a:1 r:2][][f a:3 r:1][f a:2 r:1][][][]
data bitmap   11110000
data         [(.,0) (..,0) (o, 1)][(.,1) (..,0) (q, 3) (j, 4)][k][j][][][][]


creat("/o/x");

  State of file system (inode bitmap, inodes, data bitmap, data)?

inode bitmap  11111000
inodes       [d a:0 r:3][d a:1 r:2][f a:-1 r:1][f a:3 r:1][f a:2 r:1][][][]
data bitmap   11110000
data         [(.,0) (..,0) (o, 1)][(.,1) (..,0) (q, 3) (j, 4) (x, 2)][k][j][][][][]


mkdir("/o/t");

  State of file system (inode bitmap, inodes, data bitmap, data)?


inode bitmap  11111100
inodes       [d a:0 r:3][d a:1 r:3][f a:-1 r:1][f a:3 r:1][f a:2 r:1][d a: r:2][][]
data bitmap   11111000
data         [(.,0) (..,0) (o, 1)][(.,1) (..,0) (q, 3) (j, 4) (x, 2) (t, 5)][k][j][(.,5) (..,1)][][][]



## 3. Now reduce the number of data blocks in the file system, to very low numbers (say two), and run the simulator for a hundred or so requests. What types of files end up in the file system in this highly-constrained layout? What types of operations would fail?

> ./vsfs.py -s 21 -d 2 -n 100 -p

Any operation that adds a data block will fail, since running out of memory will cause the operation to fail. These failing operations are: writing to a file or creating a dir.



## 4. Now do the same, but with inodes. With very few inodes, what types of operations can succeed? Which will usually fail? What is the final state of the file system likely to be?

> ./vsfs.py -i 2 -n 100 -p -c -s 21

Any operation creating an inode will fail:

mkdir
create new file

Also hardlinking/unlinking/writing will fail since there are no files to link/unlink/write
