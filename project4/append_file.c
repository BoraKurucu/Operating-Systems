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
        printf("usage: ./appendf  <vdiskname> <filename>\n");
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

    fd = vsfs_open( filename , MODE_APPEND );

    if( fd == -1 ){
      printf("could not open file :(\n");
      exit(1);
    }

    int n;
    printf("Enter number of chars you want to append:");
    scanf("%d", &n );

    char* append = (char*)malloc( 5000 );
    printf("Enter append content: \n");
    scanf( "%s", append );

    printf("append content recieved : \n%s\n" ,  append );

    ret = vsfs_append( fd, append, n );

    if( ret == -1 ){
      printf("could not be appended\n");
    }
    else{
      printf("successfuly appended\n");
    }
    
    free( append );

    vsfs_close(fd);
    ret = vsfs_umount();
}
