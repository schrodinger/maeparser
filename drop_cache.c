#define _XOPEN_SOURCE 600
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/**
 * This program will advise the Linux kernel that we don't need a file in
 * the near future, in the hope that it will flush it from its disk cache.
 * This can be helpful in getting more realistic performance estimates on a
 * cold cache.
 *
 * usage: drop_cache <filename>
 */
int main(int argc, char* argv[])
{
    int fd = -1;

    fd = open(argv[1], O_RDONLY);

    posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);

    close(fd);

    return 0;
}

