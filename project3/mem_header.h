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

struct Header* get_next( struct Header* temp ){
  if( ( temp->next ) == 0 )return NULL;
  struct Header* cur_next = (struct Header*)( (char*)temp + (temp->next) );
  return cur_next;
}

struct Header* get_prev( struct Header* temp ){
  if( ( temp->prev ) == 0 )return NULL;
  struct Header* cur_next = (struct Header*)( (char*)temp - (temp->next) );
  return cur_next;
}

void display_header( struct Header* h ){
  //printf("pid = %d , {is_allocated : %d, size : %d , next : %p , prev : %p }\n" , getpid() , h->is_allocated, h->size , h->next  , h->prev );
}

int is_allocatable( struct Header* hole, int allocation_size ){
  if( hole == NULL )return 0;

  int real_allocation_size;
  real_allocation_size = allocation_size + sizeof( struct Header );

  if( ( hole->size + sizeof(struct Header) > real_allocation_size ) && ( hole->is_allocated == 0 ) )return 1;
  return 0;
}

struct Header* allocate_hole( struct Header* hole_ptr , int allocation_size ){
  if( !is_allocatable( hole_ptr, allocation_size ) )return NULL;

  int real_allocation_size;
  real_allocation_size = allocation_size + sizeof( struct Header );
  
  if( real_allocation_size == hole_ptr->size + sizeof( struct Header ) ){
    hole_ptr->is_allocated = 1;
  }

  else if( real_allocation_size < ( hole_ptr->size + sizeof( struct Header ) ) ){
    struct Header* new_hole_ptr;
    char* new_hole_char_ptr;

    new_hole_char_ptr = (char*)hole_ptr;

    new_hole_char_ptr += sizeof( struct Header ) + allocation_size;
    new_hole_ptr = (struct Header*) new_hole_char_ptr;
    //new_hole_ptr = hole_ptr + sizeof( struct Header ) + allocation_size;
    new_hole_ptr->size = hole_ptr->size - allocation_size - sizeof( struct Header );
    hole_ptr->size = allocation_size;
    new_hole_ptr->is_allocated = 0; 
    //hole_ptr->next->prev = new_hole_ptr

    if( hole_ptr->next != 0 ) {
      get_next( hole_ptr )-> prev = (int)( (char*)get_next( hole_ptr ) - (char*)new_hole_ptr );
      new_hole_ptr -> next = (int)( (char*)get_next( hole_ptr ) - (char*)new_hole_ptr );
    }
    else{
      new_hole_ptr -> next = 0;
    }
    //if( hole_ptr->next != 0 ) get_next( hole_ptr )-> prev = (int)( (char*)get_next( hole_ptr ) - (char*)new_hole_ptr );
    //new_hole_ptr -> next = (int)( (char*)get_next( hole_ptr ) - (char*)new_hole_ptr );
    hole_ptr -> next = (int)( (char*)new_hole_ptr - (char*)hole_ptr );
    new_hole_ptr -> prev = (int)( (char*)new_hole_ptr - (char*)hole_ptr );
    
    /*
    if( get_next(hole_ptr) != NULL ) *get_prev( get_next(hole_ptr) ) = new_hole_ptr;
    *get_next( new_hole_ptr ) = get_next( hole_ptr );
    *get_next( hole_ptr ) = new_hole_ptr;
    *get_prev( new_hole_ptr ) = hole_ptr;
    */

    hole_ptr->is_allocated = 1;
  }
  else{
    return NULL;
  }

  return hole_ptr;
}

int deallocate( struct Header* alloc_ptr ){
  if( alloc_ptr->is_allocated == 0 )return -1;
    
  //If No Merge
  if( alloc_ptr->prev == 0 && alloc_ptr->next == 0 ){
    alloc_ptr->is_allocated = 0;
  }
  else if( alloc_ptr->prev == 0 && alloc_ptr->next != 0 && get_next( alloc_ptr )->is_allocated == 1 ){
    alloc_ptr->is_allocated = 0;
  }
  else if( alloc_ptr->next == 0 &&  get_prev( alloc_ptr )->is_allocated == 1 ){
    alloc_ptr->is_allocated = 0;
  }

  /*
  if( get_prev( alloc_ptr ) == NULL && get_next( alloc_ptr ) == NULL ){
    alloc_ptr->is_allocated = 0;
  }
  else if( get_prev( alloc_ptr ) == NULL && get_next( alloc_ptr ) ->is_allocated == 1 ){
    alloc_ptr->is_allocated = 0;
  }
  else if( get_next( alloc_ptr ) == NULL && get_prev( alloc_ptr )->is_allocated == 1 ){
    alloc_ptr->is_allocated = 0;
  }
  */
  else if( ( alloc_ptr -> next != 0 ) &&
           ( alloc_ptr -> prev != 0 ) &&
           ( get_prev( alloc_ptr )->is_allocated == 1 ) &&
           ( get_next( alloc_ptr )->is_allocated == 1 ) ){
    alloc_ptr->is_allocated = 0;
  }
  /*
  else if(  ( get_next( alloc_ptr ) != NULL )  &&
            ( get_prev( alloc_ptr ) != NULL ) &&
            ( get_prev( alloc_ptr )->is_allocated == 1 ) &&
            ( get_next( alloc_ptr )->is_allocated == 1 ) ){
                alloc_ptr->is_allocated = 0;
  }
  */
  //Merge head with right side
  else if( alloc_ptr->prev == 0 ){
    alloc_ptr->size += get_next( alloc_ptr )->size + sizeof( struct Header );

    if( alloc_ptr->next != 0 &&  get_next( alloc_ptr )->next != 0 )
      alloc_ptr -> next = (int)( (char*)get_next( get_next( alloc_ptr ) ) - (char*)alloc_ptr );
    else
      alloc_ptr -> next = 0;
    
    if( alloc_ptr->next != 0 ) get_next( alloc_ptr ) -> prev = (int)( (char*)get_next( alloc_ptr ) - (char*)alloc_ptr );

    /*
    alloc_ptr->size += get_next( alloc_ptr )->size + sizeof( struct Header );
    *get_next( alloc_ptr ) = get_next( get_next( alloc_ptr ) );
    if( get_next( alloc_ptr ) != NULL ) *get_prev( get_next( alloc_ptr ) ) = alloc_ptr;
    */
    alloc_ptr->is_allocated = 0;
  }
  //Merge tail with left side
  else if( alloc_ptr->next == 0 ){
    struct Header* new_hole;
    new_hole =  get_prev( alloc_ptr );
    new_hole->size += alloc_ptr->size + sizeof( struct Header );
    new_hole->next = 0;
    alloc_ptr->is_allocated = 0; //For safety purpose
  }
  //Merge with right side
  else if( get_prev( alloc_ptr )->is_allocated == 1 ){
    alloc_ptr->size += get_next( alloc_ptr )->size + sizeof( struct Header );
    
    if( get_next( alloc_ptr )->next == 0 )
      alloc_ptr -> next = 0;
    else
      alloc_ptr -> next = (int)( (char*)get_next( get_next( alloc_ptr ) ) - (char*)alloc_ptr );

    if( alloc_ptr->next != 0 )
      get_next(  alloc_ptr ) -> prev =   (int)( (char*)get_next(  alloc_ptr ) - (char*)alloc_ptr ); 


    //*get_next( alloc_ptr ) = get_next( get_next( alloc_ptr ) );
    //*get_prev( get_next(  alloc_ptr ) ) = alloc_ptr;
    alloc_ptr -> is_allocated = 0;
  }
  //Merge with left side
  else if( get_next( alloc_ptr ) -> is_allocated == 1 ){
    struct Header* new_hole;
    new_hole =  get_prev( alloc_ptr );
    new_hole->size += alloc_ptr->size + sizeof( struct Header );
    
    new_hole -> next = (int)( (char*)get_next( alloc_ptr ) - (char*)new_hole );
    get_next( alloc_ptr ) -> prev = (int)( (char*)get_next( alloc_ptr ) - (char*)new_hole );
    
    //*get_next( new_hole ) = get_next( alloc_ptr );
    //*get_prev( get_next( alloc_ptr ) ) = new_hole;
    alloc_ptr->is_allocated = 0; //For safety purpose
  }
  //Merge with both sides
  else if ( get_next( alloc_ptr )->is_allocated == 0 && get_prev( alloc_ptr )->is_allocated == 0 )
  {
    struct Header* new_hole;
    new_hole = get_prev( alloc_ptr );
    new_hole->size += alloc_ptr->size + sizeof( struct Header );
    new_hole->size += get_next( alloc_ptr )->size + sizeof( struct Header );

    if( get_next( alloc_ptr )->next != 0 )
      new_hole -> next = ( (char*)get_next( get_next( alloc_ptr ) ) - (char*)new_hole );
    else
      new_hole->next = 0;

    //*get_next( new_hole ) = get_next( get_next( alloc_ptr ) );
    if( new_hole->next != 0 ){
      get_next( new_hole )->prev = (int)( (char*)get_next( new_hole ) - (char* )new_hole );

      //*get_prev( get_next( new_hole ) ) = new_hole;
    }
    alloc_ptr->is_allocated = 0; //For safety purpose
  }
  else{
    return -1;
  }
  return 0;
}

#endif