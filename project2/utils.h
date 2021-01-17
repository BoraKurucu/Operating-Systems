#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <mqueue.h>
#include <sys/time.h>

#define MAX_WORD_SIZE 1024

pthread_mutex_t queue_lock;
pthread_mutex_t array_lock;


struct burst_node{
  struct burst_node* next;
  int burst_length;
  int process_id;
  struct timeval process_time;
  struct timeval enqueu_time;
  int is_first_response;
};

struct t_param{
  char* filename;
  int  id;
};

int** burst_lengths;

int ii;

int* file_index;

void init_array(){
  burst_lengths = malloc( sizeof(int*)*5 );
  file_index = malloc( sizeof(int) * 5 );
} 


void set_array( FILE *fp , int n ){
  pthread_mutex_lock(&array_lock);
  
  burst_lengths[n] = malloc( sizeof(int)*10 );

  char temp[ MAX_WORD_SIZE ];
  int j = 0;

  while( fscanf( fp, "%1023s" , temp ) != -1 ){
    fscanf( fp, "%1023s" , temp );
    //printf( "file read : %s\n", temp );
    burst_lengths[n][j] = atoi( temp );
    j++;
  }
  
  burst_lengths[n][j] = -1;

  /*
  printf("array set %d: \n", n);
  for( j = 0 ; j <10 ;j++){
    printf( "%d ", burst_lengths[n][j]); 
  }
  */


  pthread_mutex_unlock(&array_lock);
}

/*
void init_array( FILE** fp, int n ){
  char temp[ MAX_WORD_SIZE ];

  for(i = 0; i < n ; i++ ){
    int j = 0;  
    
    while( fscanf( fp[i], "%1023s" , temp ) != -1 ){
      fscanf( fp[i], "%1023s" , temp );
      burst_lengths[i][j] = atoi( temp );
      j++;
    }
    burst_lengths[i][j] = -1;
  }
}
*/

int get_burst_length( FILE *fp, int index ){
  pthread_mutex_lock(&array_lock);
  
  if( index < 0 ){
    pthread_mutex_unlock(&array_lock);
    return -1;
  }

  /*
  printf("\narray%d: \n", index);
  int j;
  for( j = 0 ; j <10 ;j++){
    printf( "%d ", burst_lengths[index][j]); 
  }
  */

  file_index[index] += 1;
  //printf("get burst length : %d index = %d, fileindex = %d\n" , burst_lengths[index][ file_index[index] - 1  ], index , file_index[index] - 1 );
  pthread_mutex_unlock(&array_lock);
  return burst_lengths[index][ file_index[index] - 1  ];
}

void enqueue( struct burst_node** head ,struct  burst_node* new_node ){
  
  pthread_mutex_lock(&queue_lock);
  
  struct burst_node* cur;

  gettimeofday( &new_node->enqueu_time, NULL);

  if( (*head) == NULL ){
    *head = new_node;
    new_node->next = NULL; 
    pthread_mutex_unlock(&queue_lock); 
    return;
  }
  else{
    cur = *head;
    while( cur->next != NULL ) cur = cur->next;

    cur->next = new_node;
    new_node->next = NULL;
  }
  
  pthread_mutex_unlock(&queue_lock); 
  return;
  
}

int all_terminated( int* cond_var , int n){
  
  /*
    If all cond_vars are -1, return 1
    else return 0
  */

  int flag = 1;
  int i;

  for( i = 0 ; i < n ; i++ ){
    if( cond_var[i] != -1 )flag = 0;
  }

  return flag;
}

/*
void enqueue(struct  burst_node* head , int burst_length ){
  struct  burst_node* cur;
  struct  burst_node* new_node = malloc( sizeof(struct  burst_node* ) );
  new_node->burst_length = burst_length;

  if( head == NULL ){
    head = new_node;
    new_node->next = NULL; 
    return;
  }
  else{
    cur = head;
    while( cur->next != NULL ) cur = cur->next;

    cur->next = new_node;
    new_node->next = NULL;
  }
  return;

}
*/

struct burst_node* dequeue( struct  burst_node** head , struct  burst_node* remove_ptr ){

  pthread_mutex_lock(&queue_lock);

  gettimeofday(&remove_ptr->process_time, NULL);
  
  if( *head == NULL ) return NULL;
  struct  burst_node* cur = *head;
  struct  burst_node* prev = *head;

  if( *head == remove_ptr ){
    *head =  ( *head )->next;
    pthread_mutex_unlock(&queue_lock); 
    return remove_ptr;
  }

  cur = cur->next;

  while(  cur != NULL ){
    
    if( cur == remove_ptr ){
      prev -> next = cur -> next;
      
      //burst_length = remove_ptr-> burst_length;
      //free( remove_ptr );
      pthread_mutex_unlock(&queue_lock); 
      return remove_ptr;
    }

    prev = cur;
    cur = cur->next;

  }
  pthread_mutex_unlock(&queue_lock); 
  return NULL;
}

void display_list( struct burst_node* head ){
  if( head == NULL )printf( "INVALID HEAD \n " );
  
  while( head != NULL ){
    printf("(pid = %d , l = %d ) -> " , head->process_id , head->burst_length );
    head = head->next;
  }
  printf(" NULL \n");

}
