#ifndef __MEMHEADER__
#define __MEMHEADER__

#include <stdio.h>
#include "mapping.h"

/*
Header of the memory space
*/

struct Header{
  int is_allocated;
  int size;
  int next;
  int prev;
};

struct Header* get_next( struct Header temp ){
  struct Header* cur_next = (struct Header)( (char)temp + (temp->next) );
}

struct Header* get_prev( struct Header temp ){
  struct Header* cur_next = (struct Header)( (char)temp - (temp->next) );
}


void display_header( struct Header* h ){
  printf("pid = %d , {is_allocated : %d, size : %d , next : %p , prev : %p }\n" , getpid() , h->is_allocated, h->size , h->get_next()  , h->get_prev() );
}


int is_allocatable( struct Header* hole, int allocation_size ){
  if( hole == NULL )return 0;

  int real_allocation_size;
  real_allocation_size = allocation_size + sizeof( struct Header );
  
  printf( "real allocation size = %d pid = %d\n" , real_allocation_size , getpid() );
  
  printf( "is allocatable hole , pid= %d\n" , getpid() );
  printf(" hole ptr : %p \n", hole );
  display_header( hole );

  if( ( hole->size + sizeof(struct Header) >= real_allocation_size ) && ( hole->is_allocated == 0 ) )return 1;
  return 0;
}

struct Header* allocate_hole( struct Header* hole_ptr , int allocation_size ){
  if( !is_allocatable( hole_ptr, allocation_size ) )return NULL;

  int real_allocation_size;
  real_allocation_size = allocation_size + sizeof( struct Header );
  
  if( real_allocation_size == hole_ptr->size + sizeof( struct Header ) ){
    hole_ptr->is_allocated = 1;
  }

  else{
    struct Header* new_hole_ptr;
    char* new_hole_char_ptr;

    new_hole_char_ptr = (char*)hole_ptr;
    new_hole_char_ptr += sizeof( struct Header ) + allocation_size;
    new_hole_ptr = (struct Header*) new_hole_char_ptr;
    //new_hole_ptr = hole_ptr + sizeof( struct Header ) + allocation_size;
    
    new_hole_ptr->size = hole_ptr->size - allocation_size - sizeof( struct Header );
    hole_ptr->size = allocation_size;
    new_hole_ptr->is_allocated = 0; 

    if( hole_ptr->get_next() != NULL )hole_ptr-> get_next() -> get_prev() = new_hole_ptr;
    new_hole_ptr->get_next() = hole_ptr->get_next();
    hole_ptr->get_next() = new_hole_ptr;
    new_hole_ptr->get_prev() = hole_ptr;

    hole_ptr->is_allocated = 1;
  }

  return hole_ptr;
}

int deallocate( struct Header* alloc_ptr ){
  if( alloc_ptr->is_allocated == 0 )return -1;
    
  //If No Merge
  if( alloc_ptr-> get_prev() == NULL && alloc_ptr-> get_next() == NULL ){
    alloc_ptr->is_allocated = 0;
  }
  else if( alloc_ptr->get_prev() == NULL && alloc_ptr->get_next() ->is_allocated == 1 ){
    alloc_ptr->is_allocated = 0;
  }
  else if( alloc_ptr->get_next() == NULL && alloc_ptr->get_prev()->is_allocated == 1 ){
    alloc_ptr->is_allocated = 0;
  }
  else if(  ( alloc_ptr->get_next() != NULL )  &&
            ( alloc_ptr->get_prev() != NULL ) &&
            ( alloc_ptr->get_prev()->is_allocated == 1 ) &&
            ( alloc_ptr->get_next()->is_allocated == 1 ) ){
                alloc_ptr->is_allocated = 0;
  }
  //Merge head with right side
  else if( alloc_ptr->get_prev() == NULL ){
    alloc_ptr->size += alloc_ptr->get_next()->size + sizeof( struct Header );
    alloc_ptr->get_next() = alloc_ptr->get_next()->get_next();
    if( alloc_ptr->get_next() != NULL )alloc_ptr->get_next()->get_prev() = alloc_ptr;
    alloc_ptr->is_allocated = 0;
  }
  //Merge tail with left side
  else if( alloc_ptr-> get_next() == NULL ){
    struct Header* new_hole;
    new_hole = alloc_ptr -> get_prev();
    new_hole->size += alloc_ptr->size + sizeof( struct Header );
    new_hole->get_next() = NULL;
    alloc_ptr->is_allocated = 0; //For safety purpose
  }
  //Merge with right side
  else if( alloc_ptr->get_prev()->is_allocated == 1 ){
    alloc_ptr->size += alloc_ptr->get_next()->size + sizeof( struct Header );
    alloc_ptr->get_next() = alloc_ptr->get_next()->get_next();
    alloc_ptr -> get_next() -> get_prev() = alloc_ptr;
    alloc_ptr -> is_allocated = 0;
  }
  //Merge with left side
  else if( alloc_ptr -> get_next() -> is_allocated == 1 ){
    struct Header* new_hole;
    new_hole = alloc_ptr -> get_prev();
    new_hole->size += alloc_ptr->size + sizeof( struct Header );
    new_hole->get_next() = alloc_ptr->get_next();
    alloc_ptr->get_next()->get_prev() = new_hole;
    alloc_ptr->is_allocated = 0; //For safety purpose
  }
  //Merge with both sides
  else if ( alloc_ptr->get_next()->is_allocated == 0 && alloc_ptr->get_prev()->is_allocated == 0 )
  {
    struct Header* new_hole;
    new_hole = alloc_ptr->get_prev();
    new_hole->size += alloc_ptr->size + sizeof( struct Header );
    new_hole->size += alloc_ptr->get_next()->size + sizeof( struct Header );
    new_hole->get_next() = alloc_ptr->get_next()->get_next();
    if( new_hole->get_next() != NULL ){
      new_hole->get_next()->get_prev() = new_hole;
    }
    alloc_ptr->is_allocated = 0; //For safety purpose
  }
  else{
    return -1;
  }
  return 0;
}

#endif