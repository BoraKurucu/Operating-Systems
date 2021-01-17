#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
 #include <sys/wait.h>
#include <time.h>          
#include<stdio.h> 
#include<stdlib.h> 
#include <pthread.h>

//The link below demonstrates the test of my both programs with various inputs,and shows that both programs are correct.
//https://drive.google.com/drive/folders/1oHNImAoXUwN-yvEJYOeBjztwAlKwSSE8?usp=sharing
typedef struct Node 
{ 
    char * word; 
    struct Node* next; 
    int count;
}Node;
void printList(struct Node *heads) 
{ 
    struct Node *temp = heads; 
    while(temp != NULL) 
    { 
	printf("element is %s \n",temp->word);
        temp = temp->next; 
    } 
} 
void sort(struct Node **h)
{
    char * a;



    struct Node *temp1;
    struct Node *temp2;

    for(temp1=*h;temp1!=NULL;temp1=temp1->next)
      {
        for(temp2=temp1->next;temp2!=NULL;temp2=temp2->next)
          { 
            if( strcmp(temp1->word,temp2->word) > 0)
              {
                a = temp1->word;
                temp1->word = temp2->word;
                temp2->word = a;
              }
           }
       }
}
struct Node * head1 = NULL; 
struct Node * head2 = NULL; 
struct Node * head3 = NULL; 
struct Node * head4 = NULL; 
struct Node * head5 = NULL; 
int fileNumber = 0;
int hash[5381];
int numbers( char *str)
{
    int numbers = 5381;
    int index;

    while (index = *str++)
        numbers = ((numbers << 5) + numbers) + index; 

    int returner = numbers%5381;
    while(returner < 0)
	{
	returner = returner + 5381;
	}
	return returner;
}
void *runner (void *param)
{
FILE *fp;
char line[128];
struct Node *current;
struct Node *head;
head = current = NULL;
fp = fopen(param, "r");

    while (fscanf(fp, " %1023s", line) == 1)
  {
        struct Node *node = malloc(sizeof(struct Node));
        node->word = strdup(line);
	node->count = 0;
        node->next =NULL;
	int index = numbers(node->word);
	printf("node is %s index is %d \n",node->word,index);
	if(hash[index] == 0)
	{
		 hash[index]++;
		if(head == NULL){
		    current = head = node;
		} else {
		    current = current->next = node;
		}
	}
	else
	{
	  hash[index]++;
	}
	
    }
    fclose(fp);
       struct Node *node = malloc(sizeof(struct Node));
       	node->word = "zzzzz";
        node->next =NULL;
	node->count = 0;
	if(head == NULL){
		    current = head = node;
		} else {
		    current = current->next = node;
		}
    	sort(&head);
    
   	if(fileNumber == 1)
	{
	 head1 = head;
	}
   	else if(fileNumber == 2)
	{
	 head2 = head;
	}
	else if(fileNumber == 3)
	{
	 head3 = head;
	}
	else if(fileNumber == 4)
	{
	 head4 = head;
	}
	else if(fileNumber == 5)
	{
	 head5 = head;
	}
	printf("Hi \n");
	pthread_exit(0); 
	
	
}
void writer(char * fileName,char * word,int count)
{

   FILE *fp;
   fp = fopen(fileName, "a+");
   fprintf(fp,"%s %d \n",word,count);
   fprintf(fp,"\n");
   fclose(fp);

}
int  main(int argc, char *argv[]){
	pthread_t tid; /* id of the created thread */
	pthread_attr_t attr;  /* set of thread attributes */
	
	if(argc <= 3)
      	return 0;

	int totalFile;
          totalFile =(int)(atoi(argv[1]));

	if(argc >=4 )
	{
	fileNumber = 1;
	pthread_attr_init (&attr); 	
	pthread_create (&tid, &attr, runner, argv[2]); 
	pthread_join (tid, NULL);
		printf("Hi3 \n");
	}
	if(argc >=5 )
	{
		printf("Hi2 \n");
	fileNumber = 2;
	pthread_attr_init (&attr); 	
	pthread_create (&tid, &attr, runner, argv[3]); 
	pthread_join (tid, NULL);
	}
	if(argc >=6 )
	{

	fileNumber = 3;
	pthread_attr_init (&attr); 	
	pthread_create (&tid, &attr, runner, argv[4]); 
	pthread_join (tid, NULL);
	}
	if(argc >=7 )
	{

	fileNumber = 4;
	pthread_attr_init (&attr); 	
	pthread_create (&tid, &attr, runner, argv[5]); 
	pthread_join (tid, NULL);
	}
	if(argc >=8 )
	{
	fileNumber = 5;
	pthread_attr_init (&attr); 	
	pthread_create (&tid, &attr, runner, argv[6]); 
	pthread_join (tid, NULL);
	}
	
	int totalFinished = 0;
	int finished[5] = {0,0,0,0,0};
	int minIndex = 0;

	Node * myHeads[5] ; 
        for (int i=0; i<5; i++) 
	{
	  myHeads[i] = NULL;
	}
	myHeads[0] = head1;
	myHeads[1] = head2;
	myHeads[2] = head3;
	myHeads[3] = head4;
	myHeads[4] = head5;



	 char curMin[1023];
	 strcpy(curMin,"zzzzz");
	printf("Hello \n");
	while(totalFinished < totalFile)
	{
		strcpy(curMin,"zzzzz");
	 	for(int i = 0; i < totalFile;i++)
		{
			 if(finished[i] == 0)
			{
		          char word[1023];
			  strcpy(word,myHeads[i]->word);
			  printf("Word is %s \n",word);
				
			  if(strcmp(word,"zzzzz") == 0)
			  {
  			    totalFinished++;
			    finished[i] = 1;
				
			  }
			  else if(strcmp(curMin,word) > 0)
			  {
			    minIndex = i;
			    strcpy(curMin,word);
			    printf("MinWord is %s \n",curMin);
			  }
			 
			}
		
	        }
		 if(strcmp(curMin,"zzzzz") != 0)
		{
		int index = numbers(myHeads[minIndex]->word);
		writer(argv[argc-1],curMin,hash[index]);
		myHeads[minIndex] = myHeads[minIndex]->next;
		printf("At the end minword is %s \n",curMin);
		}
		
		
	}
			    


		

	
	
	return 0;
}

