#include <sys/stat.h>
#include <time.h> // ctime
//
// Created by greene machine on 1/10/21.
//

#ifndef HOMEWORK_COMMON_H
#define HOMEWORK_COMMON_H

void *print_file_details(struct stat sb) {
    char *file_type;
    switch (sb.st_mode & S_IFMT) {
    case S_IFBLK:
        file_type = "block device";
        break;
    case S_IFCHR:
        file_type = "character device";
        break;
    case S_IFDIR:
        file_type = "directory";
        break;
    case S_IFIFO:
        file_type = "FIFO/pipe";
        break;
    case S_IFLNK:
        file_type = "symbolic link";
        break;
    case S_IFREG:
        file_type = "regular file";
        break;
    case S_IFSOCK:
        file_type = "socket";
        break;
    default:
        file_type = "unknown?";
        break;
    }


    printf("File type:                    %s\n", file_type);
    printf("inode number:                 %ld\n", (long)sb.st_ino);
    printf("links:                        %d\n", sb.st_nlink);
    printf("protection mode:              %lo (octal)\n",
           (unsigned long)sb.st_mode);
    printf("uid:                          %d\n", sb.st_uid);
    printf("gid:                          %d\n", sb.st_gid);
    printf("block size for fs io:         %d\n", sb.st_blksize);
    printf("total size, in bytes:         %lld\n", (long long)sb.st_size);
    printf("number of blocks allocated:   %lld\n", (long long)sb.st_blocks);
    printf("device ID (if special file):  %d\n", sb.st_rdev);
    printf("time of last access:          %s", ctime(&sb.st_atime));
    printf("modified time:                %s", ctime(&sb.st_mtime));
    printf("time of last status change:   %s", ctime(&sb.st_ctime));
}

#endif // HOMEWORK_COMMON_H
