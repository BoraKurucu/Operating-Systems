#ifndef __OTP__
#define __OTP__

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "vsimplefs.h"
#include "superblock.h"
#include "fat.h"
#include "root_block.h"

struct DirEntry otp[ MAX_FILES ];
int    modes[ MAX_FILES ];
int    valid_otp[ MAX_FILES ];
int    last_pos [ MAX_FILES ];
int    entry_pos[ MAX_FILES ]; 

int add_otp_file( struct DirEntry* fcb , int mode, int ep ){

  int i;
  int flag = 0;
  int fp;
  for( i = 0 ; i < MAX_FILES && !flag ; i++ ){
    if( valid_otp[i] == 0 ) {
        flag = 1;
        fp = i;
      }
  } 
  if( flag == 0 )return -1;

  copy_dir_entry( otp + fp , fcb );
 
  modes[ fp ] = mode;
  valid_otp[ fp ] = 1;
  entry_pos[ fp ] = ep;
  
  return fp;
}

void remove_otp_file( int fd ){

  if( fd >= MAX_FILES || fd < 0 ) return;
  valid_otp[ fd ] = 0;

}

int check_possible_read( int fd, int n ){
  // returns 1 if it is possible to read n bytes from file fd
  if(  otp[ fd ].file_size >= last_pos[ fd ] + n ) return 1;
  
  return 0;
}

void advance_last_pos( int fd , int n ){
  if( check_possible_read( fd, n) )
    last_pos[ fd ] += n;
}

#endif