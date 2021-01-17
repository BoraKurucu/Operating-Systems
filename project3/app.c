#include <unistd.h>
#include <stdlib.h>

#include "smemlib.h"

int main()
{
    int i, ret; 
    char *p;  	

    ret = smem_open(); 
    if (ret == -1)
	exit (1); 
    
    p = smem_alloc (1024); // allocate space to hold 1024 characters
    for (i = 0; i < 1024; ++i)
	p[i] = 'a'; // init all chars to ‘a’
    smem_free(p);

    smem_close(); 
    
    return (0); 
}
