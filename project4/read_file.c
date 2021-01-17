#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "vsimplefs.h"

int main(int argc, char **argv)
{
    int ret;
    int fd;
    char buffer[1024];
    int size;
    char vdiskname[200];
    char filename[200];
    int i;
    char c;

    if (argc != 3)
    {
        printf("usage: ./readf  <vdiskname> <filename>\n");
        exit(0);
    }

    strcpy(vdiskname, argv[1]);
    strcpy(filename, argv[2]);

    printf("Reading File %s\n", filename);
    
    ret = vsfs_mount(vdiskname);
    if (ret != 0)
    {
        printf("could not mount \n");
        exit(1);
    }

    fd = vsfs_open( filename , MODE_READ );

    if( fd == -1 ){
      printf("could not open file :(\n");
      exit(1);
    }

    size = vsfs_getsize(fd);
    printf("file size : %d\n" , size );
    printf("File Content : \n");
    for (i = 0; i < size; ++i)
    {
        vsfs_read(fd, (void *)buffer, 1);
        c = (char)buffer[0];
        printf("%c", c );
        c = c + 1;
    }
    printf("\n");

    vsfs_close(fd);
    ret = vsfs_umount();
}
