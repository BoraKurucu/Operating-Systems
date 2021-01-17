#ifndef __SMEMLIB__
#define __SMEMLIB__

#define _USE_MATH_DEFINES

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <math.h>
#include <semaphore.h>
#include <unistd.h>

#include "mapping.h"
#include "alloc_algorithms.h"
#include "mem_header.h"

#define MIN_SEG_SIZE    32768
#define MAX_SEG_SIZE    4194304
#define MIN_ALLOC       8



/*
inline double log2(double n)
{
    return log(n) * M_LOG2E;
}
*/
char* mapping;

int is_pow_2( int x ){
    while( x%2 == 0 )x = x/2;
    return (x==1);
}


// Define a name for your shared memory; you can give any name that start with a slash character; it will be like a filename.

char memory_name[20] = "/memory";

// Define semaphore(s)

char semaphore_name[20] = "/semaphore";
sem_t* semaphore;

// Define your stuctures and variables.

int seg, segm_size;


int smem_init( int seg_size )
{   
    
    //init_mappings();
    segm_size = seg_size;
    
    //mapping_size = seg_size;
    //check if seg_size in bounds
    if(  seg_size < MIN_SEG_SIZE || seg_size > MAX_SEG_SIZE ){
        printf("requested memory size is out of bounds, should be between [%d,%d] \n" ,MIN_SEG_SIZE, MAX_SEG_SIZE );
        return -1;

    }
        

    //check if seg_size is power of two
    if( !is_pow_2( seg_size ) ){
        printf("Requested memory size should be power of two.\n");
        return -1;
    }
        


    //if( !( ceil( log2( seg_size*1.0 ) ) == floor( log2( seg_size*1.0 ) ) ) )
    //   return -1;

    //create semaphore

    semaphore = sem_open( semaphore_name, O_EXCL | O_CREAT , S_IRUSR | S_IWUSR , 1  );

    if( semaphore == SEM_FAILED ){
        if( errno = EEXIST ){
            
            semaphore = sem_open( semaphore_name, O_EXCL | O_CREAT , S_IRUSR | S_IWUSR , 1 );
            if( semaphore == SEM_FAILED )
                return -1;
        }
        else{
            return -1;
        }
    }

    //initilize semaphore
    //sem_init(&semaphore, 1, 1); 

    //check if shared memory already exist

    seg = shm_open( memory_name, O_RDWR | O_EXCL | O_CREAT , S_IRUSR | S_IWUSR | S_IXUSR );
    
    if( seg == -1 ){
        if( errno = EEXIST ){
            shm_unlink( memory_name );
            seg = shm_open( memory_name, O_RDWR | O_CREAT | O_EXCL , S_IRUSR | S_IWUSR | S_IXUSR );
            if( seg == -1 )
                return -1;
        }
        else{
            return -1;
        }
    }

    //set the size of the shared memory
    //int res;
    //res = ftruncate( seg, seg_size );
    
    if ( ftruncate( seg, seg_size ) < 0)
    {
        printf("error no : %s" , strerror(errno) );
        return -1;
    }

    //Init header of map
    struct Header* addr;
    addr = mmap(0, seg_size , PROT_READ | PROT_WRITE, MAP_SHARED , seg , 0 );
    addr->is_allocated = 0;
    addr->next = 0;
    addr->prev = 0;
    addr->size = seg_size - sizeof( struct Header );
    
    return (0); 
}

int smem_remove()
{
    /*remove semaphore*/
    sem_unlink( semaphore_name );

    /*remove shared memory*/
    shm_unlink( memory_name );

    return (0); 
}

int smem_open()
{
    semaphore = sem_open(semaphore_name, O_RDWR);

    //sem_wait(semaphore);

    seg = shm_open( memory_name, O_RDWR , 0777 );

    struct stat sbuf;
    fstat(seg, &sbuf);

    int* addr;
    int pid;

    addr = (int*) mmap(0, sbuf.st_size , PROT_READ | PROT_WRITE, MAP_SHARED , seg, 0 );
    mapping = (char*)addr;

    //pid = getpid();

    //add process id and its mapping to mappings list
    //add_mapping( addr, pid );
    
    //sem_post(semaphore);

    return (0); 
}


char *smem_alloc(int size)
{
    semaphore = sem_open(semaphore_name, O_RDWR);
    
    if( size <= 0 )return NULL;

    sem_wait(semaphore);
    
    int allocation_size;
    int pid;
    
    struct Header* head;
    head = (struct Header*)mapping;
    
    allocation_size = convert_alloc_size( size );

    struct Header* hole;
    struct Header* allocated;

    hole = find_first_fit( head, allocation_size );

    if( hole == NULL ){
        sem_post(semaphore);
        return NULL;
    }



    allocated = allocate_hole( hole , allocation_size );

    
    sem_post(semaphore);
    if( allocated == NULL ){
        
        printf("No memory space!\n");
        //exit(-1);
        return NULL;
    }
    struct stat sbuf;
    fstat(seg, &sbuf);

    if( ( (char* )allocated + sizeof( struct Header )  + allocated->size ) >=  mapping + sbuf.st_size )return NULL;

    return ( (char* )allocated + sizeof( struct Header ) ); 
}


void smem_free( char *p )
{
    if( p == NULL )return;

    semaphore = sem_open(semaphore_name, O_RDWR);
    sem_wait(semaphore);
    char* ptr = (char*)p;
    
    deallocate( ptr - sizeof( struct Header ) );
    sem_post(semaphore);
}

int smem_close()
{
    sem_close( semaphore );
    return 0;
    //return remove_mapping( pid ); 
}

int convert_alloc_size( int request ){
    if( request == 0 ) return MIN_ALLOC;
    if( request % MIN_ALLOC == 0 )return request;

    return ( ( request/MIN_ALLOC )*( MIN_ALLOC ) + MIN_ALLOC );
}

#endif