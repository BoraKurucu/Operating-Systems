#ifndef __ROOT_BLOCK__
#define __ROOT_BLOCK__

#include "vsimplefs.h"
#include "superblock.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <stdint.h>
#include <string.h>

#define MAX_FILENAME 64
#define ROOT_BLOCKS 7

struct DirEntry{
  char name[ MAX_FILENAME ];
  int  file_size;
  int  first_block;
  
  char dummy[ 256 - sizeof(int)*2 - sizeof(char)*MAX_FILENAME ];
};

void copy_dir_entry( struct DirEntry* dest,  struct DirEntry* source ){
  
  dest->file_size = source->file_size;
  dest->first_block = source->first_block;

  strcpy( dest->name , source->name );

}


struct DirBlock{
  struct DirEntry entries[ BLOCKSIZE / sizeof( struct DirEntry ) ];
};

void init_dir_block()
{
 struct DirBlock* cur_dir_block;

  cur_dir_block = (struct DirBlock*)malloc( sizeof( struct DirBlock ) );
  int i,j;

  for( j = 0 ; j < BLOCKSIZE/sizeof( struct DirEntry ) ; ++j ){
    cur_dir_block->entries[j].name[0] = '\0';  
    cur_dir_block->entries[j].file_size = 0;
    cur_dir_block->entries[j].first_block = 0;
  }

  for( i = 0; i < ROOT_BLOCKS ; i++ ){
    write_block ( cur_dir_block, i + 1 );
  }

  free( cur_dir_block );
}


void get_dir_entry( int n, struct DirEntry* entry ){
  int dir_block_num = n / ( BLOCKSIZE / sizeof( struct DirBlock) );
  int offset = n - dir_block_num * ( BLOCKSIZE/ sizeof( struct DirBlock) );

  struct DirBlock* cur_dir_block;
  cur_dir_block = (struct DirBlock*)malloc( sizeof( struct DirBlock ) );

  read_block( cur_dir_block , dir_block_num + 1 );
    
  copy_dir_entry( entry, cur_dir_block->entries + offset );

  free( cur_dir_block );

  //read_block ( cur_fat_block, fat_block_num )
}

void set_dir_entry( int n , struct DirEntry* entry ){
  int dir_block_num = n / ( BLOCKSIZE/ sizeof( struct DirEntry) );
  int offset = n - dir_block_num * ( BLOCKSIZE/ sizeof( struct DirEntry) );

  struct DirBlock* cur_dir_block;
  cur_dir_block = (struct DirBlock*)malloc( sizeof( struct DirBlock ) );

  read_block( cur_dir_block , dir_block_num + 1 );
    
  copy_dir_entry( cur_dir_block->entries + offset, entry );

  write_block( cur_dir_block , dir_block_num + 1 );

  free( cur_dir_block );

  //read_block ( cur_fat_block, fat_block_num )
}

int find_dir_entry( char* name, struct DirEntry* ret_entry ){
  struct Superblock* sb;
  sb = (struct Superblock*)malloc( sizeof( struct Superblock ) );

  read_block ( sb, 0 );
  
  int i,j;  
  struct DirBlock* cur_dir_block;
  cur_dir_block = (struct DirBlock*)malloc( sizeof( struct DirBlock ) );
  for( i = 0 ; i < ROOT_BLOCKS ; i++ ){
      read_block( cur_dir_block , i + 1 );
      for( j = 0 ; j < BLOCKSIZE / sizeof( struct DirEntry) ; ++j ){
        if( sb->free_fcb[ i * ( BLOCKSIZE / sizeof( struct DirEntry ) ) + j ] == 1 && ( strcmp( name, cur_dir_block->entries[j].name ) == 0 ) ){
          copy_dir_entry( ret_entry , ( cur_dir_block->entries  + j ) );
          free( sb );
          free( cur_dir_block );
          return i * ( BLOCKSIZE / sizeof( struct DirEntry ) ) + j;
        }     
      }
  }
  free( sb );
  free( cur_dir_block );
  return -1;
}

#endif


