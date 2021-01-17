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
        printf("usage: ./createf  <vdiskname> <filename>\n");
        exit(0);
    }

    strcpy(vdiskname, argv[1]);
    strcpy(filename, argv[2]);

    printf("Creating File %s\n", filename);
    
    ret = vsfs_mount(vdiskname);
    if (ret != 0)
    {
        printf("could not mount \n");
        exit(1);
    }

    ret = vsfs_create( filename );

    if( ret == -1 ){
      printf("could not be created\n");
    }
    else{
      printf("successfuly created\n");
    }

    ret = vsfs_umount();
}
