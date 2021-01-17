#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <time.h>
#include <sys/time.h>

#include "utils.h"

#define USECS_IN_SEC 1000000

int process_simulator(void *p);
struct burst_node *fcfs();
struct burst_node *sjf();

struct burst_node *rq = NULL;

int *process_status; // -1 : Terminated , 0: Ready State , 1: Running state
pthread_cond_t* cond_var;
pthread_mutex_t* lock;

int i;

int min_cpu, max_cpu, min_io, max_io, m, N, quantum;
char *outfile;
char *algorithm;
char *infile;
int *burst_cnt;

FILE *ofptr;

int main(int argc, char *argv[])
{
  /*
  Input format : 
  ./schedule N minCPU maxCPU minIO maxIO outfile duration algorithm quantum infileprefix

  Durattion( M ): each process will produce M cpu burst
  quantum( q ) : non-zere value if algorithm is RR 

  Example:
  schedule 5 100 300 500 8000 out.txt 50 RR 100 no-infile

  */

  //SET ARGS TO ASSOCIATED VARIABLES
  infile = malloc(sizeof(char) * MAX_WORD_SIZE);
  outfile = malloc(sizeof(char) * MAX_WORD_SIZE);
  algorithm = malloc(sizeof(char) * MAX_WORD_SIZE);

  N = atoi(argv[1]);

  min_cpu = atoi(argv[2]);
  max_cpu = atoi(argv[3]);

  min_io = atoi(argv[4]);
  max_io = atoi(argv[5]);

  strcpy(outfile, argv[6]);

  m = atoi(argv[7]);

  strcpy(algorithm, argv[8]);

  quantum = atoi(argv[9]);

  //infile    = argv[9];
  strcpy(infile, argv[10]);

  //printf("params are taken\n");
  //printf("output file name : %s\n" , outfile );
  ofptr = fopen(outfile, "w");

  //take initial time
  struct timeval initial_time;
  gettimeofday(&initial_time, NULL);

  long int *total_waiting_time;
  total_waiting_time = malloc(sizeof(long int) * N);

  int *response_time;
  response_time = malloc(sizeof(long int) * N);

  burst_cnt = malloc(sizeof(long int) * N);

  //Preperation for Simulation

  //declare variables
  pthread_attr_t attr;
  pthread_t *tid;

  tid = malloc(sizeof(pthread_t) * N);
  
  process_status = malloc(sizeof(int *) * N);
  cond_var =  malloc( sizeof( pthread_cond_t ) * N );
  lock = malloc( sizeof( pthread_mutex_t ) * N ); 
  
  /*
  for( i = 0 ; i < N ; i++){
    cond_var[i] = PTHREAD_COND_INITIALIZER;
    lock[i] = PTHREAD_MUTEX_INITIALIZER;
  }
  */
    

  struct t_param *param;

  char process_id_str[MAX_WORD_SIZE];

  if (strcmp(infile, "no-infile") != 0)
  {
    init_array();
  }

  //printf("before for, N = %d\n", N);
  for (i = 0; i < N; i++)
  {

    //printf("inside for 1, %d\n", i);
    pthread_attr_init(&attr);

    param = malloc(sizeof(struct t_param));
    param->id = i;
    //printf("inside for 2 , %d\n", i);

    //printf( " infile = %s\n" , infile );

    if (strcmp(infile, "no-infile") == 0)
    {
      param->filename = NULL;
      //printf("processes are created %d\n", i);
    }
    else
    {
      //printf("in else %d\n", i);
      param->filename = malloc(sizeof(char) * MAX_WORD_SIZE);
      strcpy(param->filename, infile); //TODO

      sprintf(process_id_str, "%d", i + 1);

      strcat(param->filename, process_id_str);
      strcat(param->filename, ".txt");
      //printf("file name prepeared : %s\n" , param->filename );
    }

    //printf("vefore pthresad\n");
    pthread_create((tid + i), &attr, process_simulator, param);
    //printf("after pthresad\n");
  }

  //printf("processes are created\n");

  //SE simulation
  struct burst_node *burst_ptr;
  while (1)
  {

    //TODO check scheduling algorithm

    //Chose algorithm

    if (strcmp(algorithm, "FCFS") == 0)
    {
      burst_ptr = fcfs();
    }
    else if (strcmp(algorithm, "SJF") == 0)
    {
      burst_ptr = sjf();
    }
    else
    {
      burst_ptr = fcfs();
    }

    if (burst_ptr != NULL)
    {

      //take burst from the queue
      //printf("before deueue, burst_ptr = %d\n" , burst_ptr );

      burst_ptr = dequeue(&rq, burst_ptr);

      //If algorithm is qq, split the current burst
      int original_burst_time = -1;
      if (strcmp(algorithm, "RR") == 0 && burst_ptr->burst_length > quantum)
      {
        original_burst_time = burst_ptr->burst_length;
        burst_ptr->burst_length = quantum;
      }

      //("after deueue, burst_ptr = %d\n" , burst_ptr );

      //printf("Burst taken from the queue:\n");
      //printf("(pid = %d , l = %d )\n" , burst_ptr->process_id , burst_ptr->burst_length );

      //printf("current status of the queue:\n");
      //display_list( rq );

      //Take Current System Time
      struct timeval current_time;
      long int current_u_time;
      gettimeofday(&current_time, NULL);

      current_u_time = ( (USECS_IN_SEC * current_time.tv_sec) + current_time.tv_usec) - ((USECS_IN_SEC * initial_time.tv_sec) + initial_time.tv_usec);

      //** SAVE BURST START TO OUTPUT FILE
      //printf("------------printed-------------\n");
      fprintf(ofptr, "%010lu %d %d\n", current_u_time, burst_ptr->burst_length, burst_ptr->process_id);
      //**

      //update total waitin time
      total_waiting_time[burst_ptr->process_id] += ((USECS_IN_SEC * current_time.tv_sec) + current_time.tv_usec) - ((USECS_IN_SEC * initial_time.tv_sec) + initial_time.tv_usec);

      if (burst_ptr->is_first_response)
      {
        response_time[burst_ptr->process_id] += ((USECS_IN_SEC * current_time.tv_sec) + current_time.tv_usec) - ((USECS_IN_SEC * initial_time.tv_sec) + initial_time.tv_usec);
      }

      //burst_ptr->enqueue_time;

      //simulate burst taken from queue
      usleep(burst_ptr->burst_length); //TODO change to usleep

      if (original_burst_time != -1)
      {
        struct burst_node *new_burst = malloc(sizeof(struct burst_node));

        new_burst->burst_length = original_burst_time - quantum;
        new_burst->process_id = burst_ptr->process_id;
        new_burst->next = NULL;
        new_burst->is_first_response = 0;
        enqueue(&rq, new_burst);
      }

      //printf(" after cpu burst sleep, pid = %d , process_status = %d \n" , burst_ptr->process_id , process_status[ burst_ptr -> process_id ] );
      if (process_status[burst_ptr->process_id] == 0){
        //printf("signaled...\n");
        
        process_status[burst_ptr->process_id] = 1;
        //pthread_cond_signal( ); 
        pthread_mutex_lock( &( lock[ param->id ] ) );
        pthread_cond_signal( &( cond_var[ burst_ptr->process_id ] )  );
        pthread_mutex_unlock( &( lock[ param->id ] ) );
        /* do something that might make condition true */ 
      }
        

      //while( process_status[ burst_ptr -> process_id ] ); //WAIT FOR OTHER THREAD
    }

    if (burst_ptr != NULL)
    {
      free(burst_ptr);
      burst_ptr = NULL;
    }
    //printf("current status of the queue:\n");
    //display_list( rq );

    if (rq == NULL && all_terminated(process_status, N) )
    {
      printf("-------------END OF PROCESSES---------------\n");
      break;
    }
    else
    {
      int i;
      //printf("not terminated : \n");
      for (i = 0; i < N; i++)
      {
        //printf( "cond var %d = %d\n" , i , process_status[i] );
      }
    }
  }

  for (i = 0; i < N; i++)
  {
    pthread_join(tid[i], NULL);
  }
  printf("FINAL REPORT : \n");
  printf("Total waiting times:\n");
  printf("%10s %20s\n", "PID", "Total Waiting Time");
  for (i = 0; i < N; i++)
    printf("%10d %20lu\n", i, total_waiting_time[i]);

  printf("\nAverage Response Times:\n");
  printf("%10s %20s\n", "PID", "Avg Response Time");
  for (i = 0; i < N; i++)
    printf("%10d %20f\n", i, response_time[i] * (1.0) / burst_cnt[i]);

  printf("\nNumber of bursts produced:\n");
  printf("%10s %20s\n", "PID", "Total Bursts");
  for (i = 0; i < N; i++)
    printf("%10d %20lu\n", i, burst_cnt[i]);

  fclose(ofptr);

  return 0;
}

/*

    SCHEDULING ALGORITHMS

*/

struct burst_node *fcfs()
{
  return rq;
}

struct burst_node *sjf()
{

  //printf("sjf called\n");

  if (rq == NULL)
  {
    //printf( "sjf return NULL true = %d\n" , rq == NULL );
    return NULL;
  }

  int min_length = rq->burst_length;

  struct burst_node *min_burst = rq;
  struct burst_node *cur = rq;

  while (cur != NULL)
  {
    if (cur->burst_length < min_length)
    {
      min_burst = cur;
      min_length = cur->burst_length;
    }
    cur = cur->next;
  }
  //printf( "sjf return NULL = %d\n" , min_burst == NULL );
  return min_burst;
}

struct burst_node *rr()
{
  struct timeval cur_time;
  gettimeofday(&cur_time, NULL);
}

int process_simulator(void *p)
{

  struct t_param *param = (struct t_param *)p;

  //srand(time(NULL));
  int burst_length;
  int io_burst_length;

  if (param->filename != NULL)
  { //READ FROM FILE
    printf("reading file...\n");
    FILE *fp;
    fp = fopen(param->filename, "r");

    set_array(fp, param->id);

    if (fp == NULL)
    {
      printf("can not open %s \n", (char *)param->filename);
      exit(0);
    }

    burst_length = get_burst_length(fp, param->id);

    //printf("burst burst okundu\n");

    struct burst_node *new_burst = malloc(sizeof(struct burst_node));

    new_burst->burst_length = burst_length;
    new_burst->process_id = param->id;
    new_burst->next = NULL;
    new_burst->is_first_response = 1;
    gettimeofday(&(new_burst->enqueu_time), NULL);

    burst_cnt[param->id] += 1;
    enqueue(&rq, new_burst);

    process_status[param->id] = 0;

    //printf("First burst added to queue:\n");
    //display_list( rq );

    while (1)
    {

      //if ( process_status[param->id] )
      //{
      pthread_mutex_lock( &( lock[ param->id ] ) );
      while ( process_status[param->id] == 0 )
        pthread_cond_wait( &( cond_var[ param->id ] ), &( lock[ param->id ] ) );
    
    
      
      //pthread_cond_wait( &( cond_var[ param->id ] ), &( lock[ param->id ] ) ); 

        //create random io time
        io_burst_length = get_burst_length(fp, param->id);
        //printf("burst burst okundu\n");
        //printf("inside if io burst length = %d\n", io_burst_length);
        if (io_burst_length == -1){
          process_status[param->id] = -1;
          return;
        }
          

        //printf( "Before io burst\n");
        //wiat for io
        usleep(io_burst_length); // TODO change to usleep

        //printf("after io burst\n");

        burst_length = get_burst_length(fp, param->id);

        if (burst_length == -1){
          process_status[param->id] = -1;
          return;
        }
          

        new_burst = malloc(sizeof(struct burst_node));

        new_burst->burst_length = burst_length;
        new_burst->process_id = param->id;
        new_burst->next = NULL;
        gettimeofday(&(new_burst->enqueu_time), NULL);

        burst_cnt[param->id] += 1;
        //put it into queue
        enqueue(&rq, new_burst);

        //printf("New burst added to the queue:\n");
        //display_list(rq);

        process_status[param->id] = 0;
      //}
      pthread_mutex_unlock( &( lock[ param->id ] ) );
      
    }
    process_status[param->id] = -1;
    return 0;
  }
  else
  { //CREATE BURST WITH RANDOM LENGTH

    burst_length = random() % (max_cpu - min_cpu) + min_cpu;

    struct burst_node *new_burst = malloc(sizeof(struct burst_node));

    new_burst->burst_length = burst_length;
    new_burst->process_id = param->id;
    new_burst->next = NULL;
    new_burst->is_first_response = 1;
    //new_burst->enqueu_time = NULL;
    //new_burst->process_time = NULL;
    gettimeofday(&(new_burst->enqueu_time), NULL);

    burst_cnt[param->id] += 1;

    enqueue(&rq, new_burst);

    process_status[param->id] = 0;

    //printf("First burst added to queue:\n");
    //display_list( rq );

    int remain = m - 1;

    while (remain > 0)
    {

      //if (process_status[param->id] == 1)
      //{
      pthread_mutex_lock( &( lock[ param->id ] ) );
      while ( process_status[param->id] == 0 )
        pthread_cond_wait( &( cond_var[ param->id ] ), &( lock[ param->id ] ) );

      //pthread_cond_wait( &( cond_var[ param->id ] ), &( lock[ param->id ] ) ); 
      
      //printf("-----\n");
        //create random io time
        io_burst_length = random() % (max_io - min_io) + min_io;

        //printf( "Before io burst pid = %d\n" , param->id );
        //wiat for io
        usleep(io_burst_length); // TODO change to usleep
        //printf("after io burst pid = %d \n" , param->id );

        burst_length = random() % (max_cpu - min_cpu) + min_cpu;

        new_burst = malloc(sizeof(struct burst_node));

        new_burst->burst_length = burst_length;
        new_burst->process_id = param->id;
        new_burst->next = NULL;
        //new_burst->enqueu_time = NULL;
        //new_burst->process_time = NULL;
        gettimeofday(&(new_burst->enqueu_time), NULL);

        burst_cnt[param->id] += 1;
        //put it into queue
        process_status[param->id] = 0;
        remain--;

        enqueue(&rq, new_burst);

        //printf("New burst added to the queue:\n");
        //display_list( rq );

        //printf( "cond var of %d is set to 0.\n" , param->id );
      //}
      //printf("remain = %d , pid = %d  \n" , remain, param->id );
      pthread_mutex_unlock( &( lock[ param->id ] ) );
    }
    process_status[param->id] = -1;
    //printf("----------------End of process--------------%d \n" , param->id );
    return 0;
  }
  return 0;
}