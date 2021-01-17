#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
 #include <sys/wait.h>
#include <time.h>          
#include "shareddefs.h"
#include<stdio.h> 
#include<stdlib.h> 

//The link below demonstrates the test of my both programs with various inputs,and shows that both programs are correct.
//https://drive.google.com/drive/folders/1oHNImAoXUwN-yvEJYOeBjztwAlKwSSE8?usp=sharing
struct Node 
{ 
    char * word; 
    struct Node* next; 
    int count;
};
void printList(struct Node *heads) 
{ 
    struct Node *temp = heads; 
    while(temp != NULL) 
    { 
	printf("element is %s count is %d \n",temp->word,temp->count);
        temp = temp->next; 
    } 
} 
void push(struct Node** head_ref, char * new_word,int count) 
{ 


    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node)); 
  

    new_node->word  = new_word; 
  
    new_node->count = count;

    new_node->next = (*head_ref); 
  

    (*head_ref)    = new_node; 
}
void sort(struct Node **h)
{
    char * a;
    int r;

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

		r = temp1->count;
		temp1->count = temp2->count;
		temp2->count = r;

              }
           }
       }
}
struct Node *head = NULL;
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
void doJob(int indexer,char * fileName)
{

	mqd_t mq;  struct item item; int n;
	if(indexer == 0)	
        mq = mq_open(MQNAME0, O_RDWR);
	if(indexer == 1)	
        mq = mq_open(MQNAME1, O_RDWR);
	if(indexer == 2)	
        mq = mq_open(MQNAME2, O_RDWR);
	if(indexer == 3)	
        mq = mq_open(MQNAME3, O_RDWR);
	if(indexer == 4)	
        mq = mq_open(MQNAME4, O_RDWR);
	
	
	
	if (mq == -1) { perror("mq_open failed\n"); exit(1); }
	FILE *fp;
	char line[128];
	struct Node *current;
	head = current = NULL;
	fp = fopen(fileName, "r");

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
	printList(head);
	printf("Hello there \n");
    	for(current = head; current ; current=current->next){
	 item.id = hash[numbers(current->word)];
	 strcpy(item.astr, current->word);
	 	 printf("sending  %s \n",item.astr);
	 n = mq_send(mq, (char *) &item, sizeof(struct item), 0);
		 	 printf("sent  %s \n",item.astr);
	 if (n == -1) {perror("mq_send failed\n"); exit(1); }

    	}


	mq_close(mq);
	
	

}

char * receiveWord(int indexer,int * count)
{
mqd_t mq;   struct mq_attr mq_attr;
	struct item *itemptr;
	int n, buflen;  char *bufptr;

	if(indexer == 0)
	{
	mq = mq_open(MQNAME0, O_RDWR | O_CREAT, 0666, NULL);
	}
	else if(indexer == 1)
	{
	mq = mq_open(MQNAME1, O_RDWR | O_CREAT, 0666, NULL);
	}
	else if(indexer == 2)
	{
	mq = mq_open(MQNAME2, O_RDWR | O_CREAT, 0666, NULL);
	}
	else if(indexer == 3)
	{
	mq = mq_open(MQNAME3, O_RDWR | O_CREAT, 0666, NULL);
	}
	else if(indexer == 4)
	{
	mq = mq_open(MQNAME4, O_RDWR | O_CREAT, 0666, NULL);
	}
	if (mq == -1) { perror("can not create msg queue\n"); exit(1); }
	mq_getattr(mq, &mq_attr);
	buflen = mq_attr.mq_msgsize;
	bufptr = (char *) malloc(buflen);
	n = mq_receive(mq, (char *) bufptr, buflen, NULL);
	if (n == -1) { perror("mq_receive failed\n"); exit(1); }
        itemptr = (struct item *) bufptr;
	free(bufptr);		
	mq_close(mq);
	char * word = (char *) itemptr->astr;
	*count = itemptr->id;
	return word;


}
void writer(char * fileName,char * word,int count)
{



   FILE *fp;
   fp = fopen(fileName, "a+");
   fprintf(fp,"%s %d \n",word,count);
   fprintf(fp,"\n");
   fclose(fp);





}
int main(int argc, char *argv[])
{  


 if(argc <= 2)
      return 0;
	  int totalFile;
          totalFile =(int)(atoi(argv[1]));

	int n;
	if(argc >=4 )
	{
		mq_open(MQNAME0, O_RDWR | O_CREAT, 0666, NULL);
		n = fork();
		if(n == 0)
		{
			doJob(0,argv[2]);
			exit(0);
		}
		else
		{
		      wait(NULL);
			
		}
	}

	if(argc >=5 )
	{
		mq_open(MQNAME1, O_RDWR | O_CREAT, 0666, NULL);
		n = fork();
		if(n == 0)
		{
			doJob(1,argv[3]);
			exit(0);
		}
		else
		{
		      wait(NULL);
			
		}

	}


	if(argc >=6 )
	{
		mq_open(MQNAME2, O_RDWR | O_CREAT, 0666, NULL);
		n = fork();
		if(n == 0)
		{
			doJob(2,argv[4]);
			exit(0);
		}
		else
		{
		      wait(NULL);
			
		}

	}
	
	if(argc >=7 )
	{
		mq_open(MQNAME3, O_RDWR | O_CREAT, 0666, NULL);
		n = fork();
		if(n == 0)
		{
			doJob(3,argv[5]);
			exit(0);
		}
		else
		{
		      wait(NULL);
			
		}

	}
	if(argc >=8)
	{
		mq_open(MQNAME4, O_RDWR | O_CREAT, 0666, NULL);
		n = fork();
		if(n == 0)
		{
			doJob(4,argv[6]);
			exit(0);
		}
		else
		{
		      wait(NULL);
			
		}

	}

	
	int finished[5] = {0,0,0,0,0};
	int filesFinished = 0;
	struct Node * head = NULL;

	
	int count;

	int index = 0;
	//strcpy(word,receiveWord(0,&count));
	//printf("word is %s \n",word);
	//push(&head,word,count);		
	while(filesFinished < totalFile)
	{
	   for(int i = 0; i < totalFile;i++)
		{
			if(finished[i] == 0)
			{
			  char word[64];
			  strcpy(word,receiveWord(i,&count));
			  index++;
			  if(strcmp(word,"zzzzz") == 0)
				{
				printf("Files finish incremented \n");
				finished[i] = 1;
				filesFinished++;
				}
			   else
				{
				 char * hey = strdup(word);
				 printf("pushing %s \n",hey);
				   struct Node *temp = head; 
					int exist = 0;
				    while(temp != NULL) 
				    { 
					if(strcmp(temp->word,hey) == 0)
					{ 	
						exist = 1;
						temp->count = temp->count + count;
						printf("I found %s in the list %d, prev count %d new count %d \n",temp->word,i,count,temp->count);
					}

					temp = temp->next; 
				    } 
				    if(exist == 0)
  				      push(&head,hey,count);
				}
				
			}
		}  

	}
	
	



	 printf("PRINTING THE LIST \n");
	 sort(&head);
	 printList(head);
	 
	struct Node *temp = head; 
       while(temp != NULL) 
	{ 
	writer(argc[argv-1],temp->word,temp->count);	
	temp = temp->next; 
	}  

	
	
	return 0;
}

