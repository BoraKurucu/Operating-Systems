#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "smemlib.h"

#define N 512   //32 64 128 256 512
#define M 16  //32 64 128 256 512

int first = 0;
int first2 = 0;

void writer(char *fileName, char* word )
{

  FILE *fp;
  if (first == 0)
  {
    fp = fopen(fileName, "w+");
    first = 1;
  }
  else
  {
    fp = fopen(fileName, "a+");
  }

  fprintf(fp, "%s\n", word);
  fclose(fp);
}

void writer2(char *fileName, char* word )
{

  FILE *fp;
  if (first2 == 0)
  {
    fp = fopen(fileName, "w+");
    first2 = 1;
  }
  else
  {
    fp = fopen(fileName, "a+");
  }

  fprintf(fp, "%s\n", word);
  fclose(fp);
}

int main(int argc, char *argv[])
{
  srand ( time(NULL) );
  
  int i;
  char c[N];
  int  allocs[N];

  for( i = 0 ; i < N ; i++ )
    c[i] = 'A' + (random() % 26);


  char filename[10];
  char alloc_size[10];
  snprintf( filename , 10, "%d", getpid() );
  strcat( filename , ".txt" );

  int ret;
  ret = smem_open();
  if (ret == -1)
  {
    printf("smem_open failed");
    exit(1);
  }

  char* p[N];

  int j;
  for( i = 0 ; i < N ; i++ ){
    allocs[i] = 16;
    p[i] = smem_alloc( allocs[i] );

    if( p[i] != NULL ){
      snprintf( alloc_size , 10, "%d", allocs[i] );
      writer2("allocs.txt", alloc_size );
      for( j = 0 ; j < allocs[i] ; j++ ){
        p[i][j] = c[i];
      }
        
    }
    else{
      allocs[i] = 0;
    }

  }

  /**************************
   * 
   */
  int valid = 1;
  for( i = 0; i < N ; i++ ){
    valid = 1;

    if( p[i] != NULL){
      for( j = 0 ; j < allocs[i] ; j++ ){
        if(p[i][j] != c[i] )valid =0;
      }

      if( valid == 1 ){
        writer( filename , "True" );
      }else{
        writer( filename , "True" );
      }
    }
  }
  

  for( i = 0 ; i < N ; i++ ){
    snprintf( alloc_size , 10, "%d", -allocs[i] );
    writer2("allocs.txt", alloc_size );
    smem_free( p[i] );
  }

  return (0);
}
