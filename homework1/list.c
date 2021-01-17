
#include <stdio.h> 
#include <stdlib.h> 
#include <time.h> 
#include <sys/time.h>

struct Element
{
   int value;
   struct Element * next;

};
int main()
{
    long int before;
    long int next;
    long int before2;
    long int next2;
    struct timeval cur_time;
    struct Element * head = NULL;
    srand(time(0)); 
    gettimeofday(&cur_time, NULL);
    printf("Time before the insertion is \n seconds : %ld\nmicro seconds : %ld \n",before2 = cur_time.tv_sec, before = cur_time.tv_usec);
for(int i = 0; i < 10000;i++)
{
	if(i == 0)
	{
	   head = (struct Element*)malloc(sizeof(struct Element)); 
	   head->value = rand()%10000;
	}
	else
	{
	   struct Element * element = (struct Element*)malloc(sizeof(struct Element)); 
	   element->value = rand()%10000;  
	   element->next = head;
	   head = element;
	}
}
   gettimeofday(&cur_time, NULL);
   printf("Time after the insertion is \n seconds : %ld\nmicro seconds : %ld",next2 = cur_time.tv_sec, next = cur_time.tv_usec);
   printf("\nTime elapsed to make insertion is \n");
   printf("%ld seconds and %ld miliseconds \n",next2-before2,next-before);  
   return 0;
}



