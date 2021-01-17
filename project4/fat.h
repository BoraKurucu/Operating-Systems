#ifndef __FAT__
#define __FAT__

#include "vsimplefs.h"
#include "superblock.h"
#include "root_block.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <stdint.h>

#define FATBLOCK_COUNT 256

struct FatEntry{
  int is_full; //1 if not empty, 0 otherwise
  int value;
};

//copy fat entry
void copy_fat_entry( struct FatEntry* dest,  struct FatEntry* source ){
  dest->is_full = source->is_full;
  dest->value = source->value;
}

unsigned int get_fat_entry_value( struct FatEntry* entry ){
  unsigned int  ret = entry->value;
  return ret;
}

/*
void set_fat_entry( struct FatEntry* entry, unsigned int n ){
    entry->value = n;
}
*/


struct FatBlock{
  struct FatEntry array[ BLOCKSIZE / sizeof( struct FatEntry) ];
};

void init_fat_block()
{
 struct FatBlock* cur_fat_block;
  cur_fat_block = (struct FatBlock*)malloc( sizeof( struct FatBlock ) );

  //printf(" Size of FAT bLOCK : %d\n",  sizeof( struct FatBlock ) );
  //printf("BLOCK SIIZE : %d\n" , BLOCKSIZE );

  int i,j;

  //Init first FAT
  //printf("init first fat block \n");

  for( j = 0 ; j < BLOCKSIZE/sizeof( struct FatEntry) ; ++j ){

    if( j < FATBLOCK_COUNT + ROOT_BLOCKS + 1 )
      cur_fat_block->array[j].is_full = 1;
    else
    {
      cur_fat_block->array[j].is_full = 0;
    }
    cur_fat_block->array[j].value = 0;
  }

  //printf("writing first fat to disk\n");
  //printf("is full = %d\n" , cur_fat_block->array[0].is_full );
  
  write_block( cur_fat_block , 1 + ROOT_BLOCKS );
  //printf("first fat written to dsik\n");

  for( j = 0 ; j < BLOCKSIZE/sizeof( struct FatEntry ) ; ++j ){
    cur_fat_block->array[j].is_full = 0;  
    cur_fat_block->array[j].value = 0;
  }

  //printf("writing other blocks to disk\n");
  for( i = 1; i < FATBLOCK_COUNT ; i++ ){
    write_block( cur_fat_block, i + 1 + ROOT_BLOCKS );
  }

  //printf("all fat blocks written to disk\n");

  free( cur_fat_block );
}

void get_fat_entry( int n , struct FatEntry* entry ){
  int fat_block_num = n / ( BLOCKSIZE/ sizeof( struct FatEntry) );
  int offset = n - fat_block_num * ( BLOCKSIZE/ sizeof( struct FatEntry) );
  
  struct FatBlock* cur_fat_block;
  cur_fat_block = (struct FatBlock*)malloc( sizeof( struct FatBlock ) );

  read_block ( cur_fat_block, fat_block_num + 1 + MAX_FILES * sizeof( struct DirEntry ) / BLOCKSIZE );

  copy_fat_entry( entry, cur_fat_block->array + offset );

  free( cur_fat_block );
  
}

int write_fat_entry( int n, struct FatEntry* entry ){
  int fat_block_num = n / ( BLOCKSIZE/ sizeof( struct FatEntry) );
  int offset = n - fat_block_num * ( BLOCKSIZE/ sizeof( struct FatEntry) );
  
  struct FatBlock* cur_fat_block;
  cur_fat_block = (struct FatBlock*)malloc( sizeof( struct FatBlock ) );

  read_block ( cur_fat_block, fat_block_num + 1 + MAX_FILES * sizeof( struct DirEntry ) / BLOCKSIZE );

  copy_fat_entry( cur_fat_block->array + offset , entry );

  write_block( cur_fat_block, fat_block_num + 1 + MAX_FILES * sizeof( struct DirEntry ) / BLOCKSIZE );

  free( cur_fat_block );
}

int get_block_from_pos( struct DirEntry* fbc, int pos ){

  //printf("inside get bfp, pos = %d\n", pos);
  int i;
  struct FatEntry* fe;
  fe = (struct FatEntry*)malloc( sizeof( struct FatEntry ) );

  int cur_block = fbc->first_block;
  
  for( i = 0 ; i < pos/BLOCKSIZE ; i++ ){
    get_fat_entry( cur_block , fe );
    cur_block = get_fat_entry_value( fe );
    //printf("cb : %d\n",cur_block );
  }

  free( fe );
  return cur_block;
}

int get_next_fat( int cur_block ){
  
  struct FatEntry* fe;
  fe = (struct FatEntry*)malloc( sizeof( struct FatEntry ) );
  get_fat_entry( cur_block , fe );
  cur_block = get_fat_entry_value( fe );

  free( fe );
  return cur_block;
}



int check_enough_blocks( int req_blocks ){

  if( req_blocks <= 0 )return 1;

  int i,j;
  int n_empty = 0;

  struct FatBlock* fb;
  fb = (struct FatBlock*)malloc( sizeof( struct FatBlock ) );

  for( i = 0 ; i < FATBLOCK_COUNT ; i++ ){
    read_block( fb , i + 1 + ROOT_BLOCKS  );

    for( j = 0 ; j < BLOCKSIZE / sizeof( struct FatEntry ) ; ++j ){
        
      if( !fb->array[j].is_full  ){
        n_empty++;

        if( n_empty == req_blocks ){
          free( fb );
          return 1;
        }
      }     
    }
  }

  free( fb );
  return 0;

}

void write_blocks( struct DirEntry* fbc, void* buf, int size, int req_blocks ){

  int i,j;

  if( size <= 0 )return;

  int first_block = fbc->first_block;

  int n_empty = 0;

  struct FatEntry* fe;
  fe = (struct FatEntry*)malloc( sizeof( struct FatEntry ) );

  int cur_block,empty_block;

  if( fbc->file_size % BLOCKSIZE == 0 ){

    //find empty block
    struct FatBlock* fb;
    fb = (struct FatBlock*)malloc( sizeof( struct FatBlock ) );
    int done = 0;

    for( i = 0 ; i < FATBLOCK_COUNT && !done ; i++ ){
      read_block( fb , i + 1 + ROOT_BLOCKS  );

      for( j = 0 ; j < BLOCKSIZE / sizeof( struct FatEntry ) && !done ; ++j ){
          
        if( ! fb->array[j].is_full  ){
          empty_block = i * ( BLOCKSIZE / sizeof( struct FatEntry ) ) + j;
          done = 1;
        }     
      }
    }
    free( fb );
    req_blocks--;

    //printf("allocating new block, empty block = %d\n", empty_block );

    if( fbc->file_size == 0 ){
      fbc->first_block = empty_block;
      cur_block = fbc->first_block;
    }
    else{
      //find last block
      //printf("finding last block, fs : %d \n", fbc->file_size );
      int last_block = get_block_from_pos( fbc, fbc->file_size - 1 );
      
      struct FatEntry* fe;
      fe = (struct FatEntry*)malloc( sizeof( struct FatEntry ) );

      //printf("writing empty block to FAT, last_block = %d, empty block = %d\n", last_block, empty_block );
      get_fat_entry( last_block , fe );
      fe->value = empty_block;
      write_fat_entry( last_block , fe );
      cur_block = empty_block;

      free( fe );
    }

    get_fat_entry( empty_block , fe );
    fe->is_full = 1;
    write_fat_entry( empty_block , fe );
    
  }else{
    cur_block = get_block_from_pos( fbc, fbc->file_size );
  }
  
  /*
  for( i = 0 ; i < fbc->file_size / BLOCKSIZE ; i++ ){
    get_fat_entry( cur_block , fe );
    cur_block = get_fat_entry_value( fe );
  }
  */

  int offset = fbc->file_size % BLOCKSIZE;

  char* content = (char*)malloc( BLOCKSIZE );

  if( req_blocks == 0 ){
    read_block( content, cur_block );
    //printf("wrriting to block, size = %d\n", size );
    //printf("writing : \n %s" , (char*)buf );
    strncpy( content + offset , (char*)buf , size );
    write_block( content , cur_block );
  }
  else{
    
    int* empty_blocks = (int*) malloc( sizeof(int)*req_blocks );


    //Find Empty Blocks

    struct FatBlock* fb;
    fb = (struct FatBlock*)malloc( sizeof( struct FatBlock ) );
    int done = 0;

    for( i = 0 ; i < FATBLOCK_COUNT && !done ; i++ ){
      read_block( fb , i + 1 + ROOT_BLOCKS  );

      for( j = 0 ; j < BLOCKSIZE / sizeof( struct FatEntry ) && !done ; ++j ){
          
        if( ! fb->array[j].is_full  ){

          empty_blocks[ n_empty ] = i * ( BLOCKSIZE / sizeof( struct FatEntry ) ) + j;
          n_empty++;

          if( n_empty == req_blocks ){
            done = 1;
          }
        }     
      }
    }
    free( fb );

    //Fill last block
    read_block( content, cur_block );
    strncpy( content + offset , (char*)buf , BLOCKSIZE - offset );
    
    buf = (char*)buf + ( BLOCKSIZE - offset );
    size -= BLOCKSIZE - offset;

    write_block( content , cur_block );

    //update fat entry
    get_fat_entry( cur_block , fe );
    fe->value =  empty_blocks[0];
    fe->is_full = 1;
    write_fat_entry( cur_block , fe );

    for( i = 0; i < n_empty ; i++ ){

      //get_fat_entry( cur_block , fe );
      //set_fat_entry( fe , (unsigned int) empty_blocks[i] );

      cur_block = empty_blocks[i];
      
      //Update content block
      read_block( content, cur_block );
      strncpy( content , (char*)buf , BLOCKSIZE > size ? size : BLOCKSIZE );
      buf = (char*)buf + ( BLOCKSIZE > size ? size : BLOCKSIZE );
      size -= ( BLOCKSIZE > size ? size : BLOCKSIZE );
      write_block( content , cur_block );

      //update fat entry
      get_fat_entry( cur_block , fe );
      
      if( i == n_empty - 1 )
        fe->value =  -1;
      else{
        fe->value = empty_blocks[i+1];
      }

      fe->is_full = 1;
      write_fat_entry( cur_block , fe );

    }
    free( empty_blocks );
  }

  free( fe );
  free( content );
}

#endif



