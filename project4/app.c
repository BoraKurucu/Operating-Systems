#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "vsimplefs.h"

int main(int argc, char **argv)
{
    int ret;
    int fd1, fd2, fd;
    int i;
    char c;
    char buffer[1024];
    char buffer2[8] = {50, 50, 50, 50, 50, 50, 50, 50};
    int size;
    char vdiskname[200];

    printf("started\n");

    if (argc != 2)
    {
        printf("usage: app  <vdiskname>\n");
        exit(0);
    }
    strcpy(vdiskname, argv[1]);

    ret = vsfs_mount(vdiskname);
    if (ret != 0)
    {
        printf("could not mount \n");
        exit(1);
    }

    printf("creating files\n");
    vsfs_create("file1.bin");
    vsfs_create("file2.bin");
    vsfs_create("file3.bin");

    fd1 = vsfs_open("file1.bin", MODE_APPEND);
    fd2 = vsfs_open("file2.bin", MODE_APPEND);

    for (i = 0; i < 10000; ++i)
    {
        buffer[0] = (char)65;
        vsfs_append(fd1, (void *)buffer, 1);
    }

    for (i = 0; i < 10000; ++i)
    {
        buffer[0] = (char)65;
        buffer[1] = (char)66;
        buffer[2] = (char)67;
        buffer[3] = (char)68;
        vsfs_append(fd2, (void *)buffer, 4);
    }

    vsfs_close(fd1);
    vsfs_close(fd2);

    fd = vsfs_open("file3.bin", MODE_APPEND);
    for (i = 0; i < 100; ++i)
    {
        memcpy(buffer, buffer2, 8); // just to show memcpy
        vsfs_append(fd, (void *)buffer, 8);
    }
    vsfs_close(fd);

    fd = vsfs_open("file3.bin", MODE_READ);
    size = vsfs_getsize(fd);
    printf("file size : %d\n" , size );
    
    for (i = 0; i < size; ++i)
    {
        vsfs_read(fd, (void *)buffer, 1);
        c = (char)buffer[0];
        printf("%d, ", c );

        c = c + 1;
    }
    vsfs_close(fd);

    ret = vsfs_umount();
}
