#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "vsimplefs.h"

int main(int argc, char **argv)
{
    int ret;
    char vdiskname[200];
    int m; 

    if (argc != 3) {
	printf ("usage: create_format <vdiskname> <m>\n"); 
	exit(1); 
    }

    strcpy (vdiskname, argv[1]); 
    m = atoi(argv[2]); 
    
    printf ("started\n"); 
    
    ret  = create_format_vdisk (vdiskname, m); 
    if (ret != 0) {
	printf ("there was an error in creating the disk\n");
	exit(1); 
    }

    printf ("disk created and formatted. %s %d\n", vdiskname, m); 
}
