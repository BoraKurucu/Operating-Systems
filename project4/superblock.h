#ifndef __SUPERBLOCK__
#define __SUPERBLOCK__

#include "vsimplefs.h"
#include <stdio.h>
#include <stdbool.h> 
#include <stdint.h>

#define MAX_FILES 112

struct Superblock{

  int num_blocks;
  unsigned int free_fcb[ MAX_FILES ];
  
  char dummy[ BLOCKSIZE - 452 ];
};

/*
void set_fcb( struct Superblock*  superblock, int n, int v ){
  if( v != 0 || v != 1)
	  return;

  int arr_block = n / sizeof( uint8_t );
  int offset = n - arr_block * sizeof( uint8_t );

  
  uint8_t mask = ( 1 << ( sizeof( uint8_t ) - offset - 1 ) );
  
  if( v == 1)
  {
	 superblock->free_fcb[arr_block] = mask | superblock->free_fcb[arr_block];
  }
  else
  {
     superblock->free_fcb[arr_block] = (~mask) & superblock->free_fcb[arr_block];
  }
}


uint8_t get_fcb(struct Superblock*  superblock,int n)
{
  uint8_t arr_block = n / sizeof( uint8_t );
  uint8_t offset = n - arr_block * sizeof( uint8_t );

  uint8_t block = superblock->free_fcb[ arr_block ];
  block = (block >> ( sizeof( uint8_t )  - offset - 1 ) );

  return block % 2;
}
*/


void init_superblock( int disk_size ){

  struct Superblock* sb = (struct Superblock *) malloc( sizeof( struct Superblock ) );

  sb -> num_blocks = disk_size / BLOCKSIZE;
  int i;
  for( i = 0 ; i < MAX_FILES ; ++i){
    sb -> free_fcb[i] = 0;
  } 
  
  write_block( sb, 0 );
  free(sb);
}

int get_empty_dir( ){

  struct Superblock* sb;
  //printf("size of superblock : %d\n" , sizeof( struct Superblock ) );
  sb = (struct Superblock*)malloc( sizeof( struct Superblock ) );
  read_block ( sb, 0 );
  
  int i;

  for( i = 0 ; i < MAX_FILES ; ++i ){
    if( sb->free_fcb[i] == 0){
      free( sb );
      return i;
    }
  } 

  free(sb);
  return -1;

}

void set_sb_dir( int k ){
  struct Superblock* sb;
  sb = (struct Superblock*)malloc( sizeof( struct Superblock ) );
  read_block ( sb, 0 );
  sb->free_fcb[ k ] = 1;
  write_block ( sb, 0 );
  free( sb );
}

#endif
