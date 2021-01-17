/*
  Process id and shared memory adress mappings
*/
#ifndef __MAPPING__
#define __MAPPING__

#include <stdio.h>
#define NUM_PROCESSES 10


/*

int   mapping_size;
int   pids[ NUM_PROCESSES ];
char* ptrs[ NUM_PROCESSES ];

void init_mappings(){
  int i;

  for( i = 0 ; i < NUM_PROCESSES ; i++ ){
    pids[i] = -1;
  }
}


int add_mapping( char* ptr, int pid ){
  
  int i;
  i = 0;
  while( i < NUM_PROCESSES && pids[i] != -1 )i++;

  if( i == NUM_PROCESSES )return -1;

  pids[i] = pid;
  ptrs[i] = ptr;
  return 0;
}

char* get_mapping( int pid ){
  int i;
  int index;
  index = -1;
  for( i = 0; i < NUM_PROCESSES ; i++){
    if( pids[i] == pid )index = i;
  }
  
  if( index == -1 )return NULL;
  
  return ptrs[index];
}

char* get_mapping_last( int pid ){
  int i;
  int index;
  index = -1;
  for( i = 0; i < NUM_PROCESSES ; i++){
    if( pids[i] == pid )index = i;
  }
  
  if( index == -1 )return NULL;
  
  return mapping_size + ptrs[index];
}

int remove_mapping( int pid ){

  int i;
  int index;
  index = -1;
  for( i = 0; i < NUM_PROCESSES ; i++){
    if( pids[i] == pid )index = i;
  }
  
  if( index == -1 )return -1;
  
  pids[index] = -1;
  ptrs[index] = NULL; //For safety purpose

  return 0;
}
*/

#endif
