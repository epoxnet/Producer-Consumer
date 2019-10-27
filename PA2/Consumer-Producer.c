//Sean Dehghani - OS - PA2

/************************************************************************/
#include	<sys/types.h>
#include	<sys/ipc.h>
#include	<sys/shm.h>
#include	<errno.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<pthread.h>
#define SIZE 1024 //used Define to simplifiy Size changes 

typedef struct buffer{
	pthread_mutex_t lock;
	pthread_cond_t shout;
	int front;
	int rear;
	int count;
	int endOfFile;
	char bytes[SIZE];
} buffer;


/******************************************Main**************************************************/
int main(int argc, char const *argv[])
 {
	if (argc != 2)
	{
		printf("Wrong Input! Please enter in a file \n");
		exit(0);
	}
	int Pid;//Pid=Process ID
	Pid=fork(); //use fork for auto process creation

/**************************************Producer*********************************************************/
	if (Pid > 0)//parent recive the Pid of child;(Pid > 0)=>Parent
	{	
		printf("producer=>parent process id:%d\n",Pid);
		//do producer
		FILE *file = fopen(argv[1], "r");
		int shmid;
		key_t key = getpid();//key = process id for producer 
		buffer* newBuff;
		if ((shmid = shmget(key, sizeof(newBuff), 0666 | IPC_CREAT | IPC_EXCL)) != -1)
		 {
			newBuff = (buffer*) shmat(shmid, 0, 0);
			printf("successful creation\n");


			newBuff->front = 0;
			newBuff->count = 0;
			newBuff->endOfFile=0;
			pthread_mutexattr_t attr;
			pthread_condattr_t condAttr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
			pthread_mutex_init(&newBuff->lock, &attr);
			pthread_condattr_init(&condAttr);
			pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
			pthread_cond_init(&newBuff->shout, &condAttr);
		} //creating shared memory

		else if ((shmid = shmget(key, 0, 0)) != -1)
		{
			printf("successful attachment\n" );
			newBuff = (buffer*) shmat(shmid, 0, 0);
		}
		else
		{
			printf("Error . Exiting\n");
			exit(0);
		}

		pthread_mutex_lock(&newBuff->lock);
		

		while (fscanf(file, "%c", &newBuff->bytes[newBuff->rear]) != EOF) //read in file
		{
			newBuff->count++;
			while (newBuff->count == SIZE)
			{ //when buffer is full
				pthread_cond_signal(&newBuff->shout);
				pthread_cond_wait(&newBuff->shout, &newBuff->lock);
			}
		newBuff->rear = ((newBuff->rear + 1)%sizeof(newBuff->bytes));
		}
		newBuff->endOfFile = 1;
		pthread_cond_signal(&newBuff->shout);
		pthread_mutex_unlock(&newBuff->lock);

		shmdt(&newBuff);

		return 0;


	}
/******************************************* consumer***************************************************/
	else
		
	{
		printf("consumer=>child process id:%d\n",Pid);
		//do consumer
		FILE *file = fopen(argv[1], "w");//*file1
		int shmid;
		key_t key = getppid();//key = parent process id for consumer => process id of producer
		buffer* newBuff;
		//creating shared memory along with intialization
		if ((shmid = shmget(key, sizeof(newBuff), 0666 | IPC_CREAT | IPC_EXCL)) != -1) 
		{
			newBuff = (buffer*) shmat(shmid, 0, 0);
			printf("successful creation\n");
			newBuff->front = 0;
			newBuff->count = 0;
			newBuff->endOfFile = 0;
			pthread_mutexattr_t attr;
			pthread_condattr_t condAttr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
			pthread_mutex_init(&newBuff->lock, &attr);
			pthread_condattr_init(&condAttr);
			pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
			pthread_cond_init(&newBuff->shout, &condAttr);
		}
		//finding shared memory that is already created
		else if ((shmid = shmget(key, 0, 0)) != -1)
		{
			printf("successful attachment\n" );
			newBuff = (buffer*) shmat(shmid, 0, 0);
		}
		else{ //can't find shared memory
			printf("shared memory is unavailable!\n");
			exit(0);
		}
		pthread_mutex_lock(&newBuff->lock);
		//do this while the producer hasn't reached EOF and the buffer is empty
		while ((newBuff->endOfFile == 0) || (newBuff->count != 0))
		{

			//while buff is empty
			while (newBuff->count == 0)
			{
				pthread_cond_signal(&newBuff->shout);
				pthread_cond_wait(&newBuff->shout, &newBuff->lock);
			}
			//take two bytes at a time and input them in reverse order
			fprintf(file, "%c%c", newBuff->bytes[newBuff->front + 1], newBuff->bytes[newBuff->front]);
			newBuff->front = ((newBuff->front + 2)%sizeof(newBuff->bytes));
			newBuff->count -= 2;
			//if the byte size is uneven just put last byte in normally
			if (newBuff->count == 1)
			{
				fprintf(file, "%c", newBuff->bytes[newBuff->front]);//file1
				newBuff->front = ((newBuff->front + 1)%sizeof(newBuff->bytes));
				newBuff->count --;
			}
		}
		pthread_mutex_unlock(&newBuff->lock);
		shmdt(&newBuff);

		return 0;
	}
	return 0;//success

}


