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
        printf("usage: ./deletef  <vdiskname> <filename>\n");
        exit(0);
    }

    strcpy(vdiskname, argv[1]);
    strcpy(filename, argv[2]);

    printf("Deleting File %s\n", filename);
    
    ret = vsfs_mount(vdiskname);
    if (ret != 0)
    {
        printf("could not mount \n");
        exit(1);
    }

    ret = vsfs_delete( filename );

    if( ret == -1 ){
      printf("could not be deleted\n");
    }
    else{
      printf("successfuly deleted\n");
    }

    ret = vsfs_umount();
}
