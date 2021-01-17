#ifndef __ALLOCALGORITHMS__
#define __ALLOCALGORITHMS__

#include <stdio.h>
#include "mem_header.h"
/*
Allocation Algorithms
*/

struct Header* find_first_fit( struct Header* head, int allocation_size ){
  struct Header* cur;
  

  for( cur = head; cur != NULL ; cur = get_next( cur ) ){

    if( is_allocatable( cur, allocation_size ) == 1 ) {
      display_header( cur );
      return cur;
    }

  }

  return NULL;
}

struct Header* find_worst_fit( struct Header* head, int allocation_size ){
  int worst_fit_size = 0;
  struct Header* cur;
  struct Header* worst_fit_ptr = NULL;

  for( cur = head; cur != NULL ; cur = get_next( cur ) ){
    if( is_allocatable( cur, allocation_size ) ){
      if( cur->size > worst_fit_size ) {
        worst_fit_size = cur->size;
        worst_fit_ptr = cur;
      }
    }
  }

  return worst_fit_ptr;
}

#endif
