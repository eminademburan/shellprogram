#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#define MAXCHAR 1000
#define MAXTHREADNUM 10

//define w thread arguments
struct wThreadArg
{
    int t_index;
    char fileName[MAXCHAR];
};

//define s thread arguments
struct sThreadArg
{
    int t_index;
    char *algo;
    //number of total burst
    int noOfIte;
};

//define a burst struct which will be used in runqueue
struct burst{
  int threadInd;
  int burstInd;
  double burstLengt;
  struct timeval generation_time;
  struct burst *next;
};

//hold pointer to head of runqueu
struct burst *headToRq = NULL;

//hold global array for virtual runtimes and average waiting times
double vrunTimes[MAXTHREADNUM];
double avgWaitTimes[MAXTHREADNUM];
int burstCounts[MAXTHREADNUM];
int THREAD_NUM;
int BURST_NUM;
int MIN_A;
int MIN_B;
double AVG_A;
double AVG_B;
char *algoName;

//declare condition variable and mutex variable
pthread_mutex_t rqLock;
pthread_cond_t cv;

//this function takes a string which holds inter arrival time and burst time then returns these two double variables
void splitLine( char *line, double *arrTime, double *burstTime)
{
    char *str;
    str = strtok (line," ");
    *arrTime = atof(str);
    str = strtok (NULL," ");
    *burstTime = atof(str);
}

//a function to insert new created burst
void insertLast(struct burst *newBurst ) {

   if( headToRq == NULL )
   {
   	headToRq = newBurst;
   	return;
   }
   else
   {
   	struct burst *current = headToRq;
   
   	while( current->next != NULL)
   		current = current->next;
   
   	current->next = newBurst;
   } 
}

//a function to create random exponential number, lamb is equal to 1 / mean
double ran_expo_Gen(double lamb){
    double u;
    
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / lamb;
}

//this function select the burst which should be executed according to alogorithm from runqueu
double findBurstToExecute(int selectAlgo, int *threadID, struct timeval *gene_time)
{
    struct burst *current = headToRq;
    int index = 0;
    int i = 0;
    
    //find the initial min values
    double minTime = current->generation_time.tv_sec ;
    double minTime2 = current->generation_time.tv_usec;
    double minBurst = current->burstLengt;
    int maxPriority = current->threadInd;
    double minVirtualRuntime = vrunTimes[ (current->threadInd)-1];
    *gene_time = current->generation_time;
    *threadID = current->threadInd;
    int hasVisited[THREAD_NUM];
    int b;
    for(b = 0; b < THREAD_NUM;b++)
    {	
    	hasVisited[b] = 0;
    }

    //find the index of burst which will be executed next by iterating over runqueue
    while( current != NULL )
    {
    	//find the earliest arrival time when algorith is FCFS
    	
        if( minTime > current->generation_time.tv_sec  && selectAlgo == 0)
        {
            minTime2 = current->generation_time.tv_usec;
            minTime = current->generation_time.tv_sec;
            *gene_time = current->generation_time;
            *threadID = current->threadInd; 
            index = i;
        }
        else if( minTime == current->generation_time.tv_sec && minTime2 > current->generation_time.tv_usec && selectAlgo == 0 )
        {
            minTime2 = current->generation_time.tv_usec;
            minTime = current->generation_time.tv_sec;
            *gene_time = current->generation_time;
            *threadID = current->threadInd; 
            index = i;
        }

	//find the shortest burst time when algorith is SJF
	if( (hasVisited[current->threadInd - 1] == 0) && (selectAlgo == 1))
	{
		hasVisited[current->threadInd - 1] = 1;
        	if( minBurst > current->burstLengt )
        	{
        		*gene_time = current->generation_time;
        		minBurst = current->burstLengt;
            		index = i;     
            		*threadID = current->threadInd; 			
        	}
        	
        }

	//find the maximum priority time when algorith is PRIO
        if( maxPriority > current->threadInd && selectAlgo == 2 )
        {
            *gene_time = current->generation_time;
            index = i;       
            maxPriority = current->threadInd;
            *threadID = current->threadInd; 
        }

	//find the minimum virtual run time when algorith is VRUNTIME
	if( (hasVisited[current->threadInd - 1] == 0) && (selectAlgo == 3) )
	{
		hasVisited[current->threadInd - 1] = 1;
        	if( minVirtualRuntime > vrunTimes[ (current->threadInd) - 1] )
        	{
            		index = i;
            		*threadID = current->threadInd;        
            		minVirtualRuntime = vrunTimes[ (current->threadInd) - 1];   
            		*gene_time = current->generation_time;     
        	}
        }
        current = current->next;
        i++;
    }

    current = headToRq;

    int j = 0;
    struct burst *previous = current;

    //remove the burst which is selected in the previous while loop
    while( current != NULL )
    {
        if( index == j )
        {    
            printf("Executed in thread: %d  burst index: %d with burst time sec: %.2f arr time sec %.2f and arr time usec: %.2f\n", current->threadInd, current->burstInd, current->burstLengt,  (double)(current->generation_time.tv_sec), (double)(current->generation_time.tv_usec) );
            //remove the burst from runqueue if queue has one burst
            if( j == 0 )
            {
                headToRq = current->next;
                double burstTime = current->burstLengt;
                free(current);
                return burstTime;
            }
            //remove the burst from runqueue if queue has many elements
            else
            {
                previous->next = current->next;
                double burstTime = current->burstLengt;
                free(current);
                return burstTime;
            }
            
        }

            previous = current;
            j++;
            current = current->next;
    }
}

//this W thread function will generate burst with random exponential numbers
void *burst_generate(void *arg_ptr)
{	

	for( int i = 0; i < BURST_NUM; i++)
	{
	    
	    
	    double arr_time;
	    //generate random inter arrivel time which is greater than MIN_A
	    while( (arr_time = ran_expo_Gen(AVG_A)) <= MIN_A);
	    arr_time *= 1000;

	    double burst_time;
	    //generate random burst time which is greater than MIN_A
	    while( (burst_time = ran_expo_Gen(AVG_B)) <= MIN_B);

	
	    //sleep as much as inter arrival time
	    usleep( (useconds_t)arr_time );
	    
	    

	//accessing runqueue is critical section, lock variable is acquired
	    pthread_mutex_lock(&rqLock);
	    
	    struct burst *newBurst = (struct burst*) malloc(sizeof(struct burst));
	    
	    //create burst struct with generated parameters
	    newBurst->threadInd = ((struct wThreadArg *) arg_ptr)->t_index;
	    newBurst->burstInd = i+1;
	    newBurst->burstLengt = burst_time;
	    gettimeofday(&(newBurst->generation_time), NULL);
	    newBurst->next = NULL;  

	    int haveBurst;

		printf("Generated in thread: %d burst index : %d with burst length: %.2f and arrTime sec: %.2f and arrTime usec:%.2f\n", newBurst->threadInd, newBurst->burstInd,newBurst->burstLengt,   (double)newBurst->generation_time.tv_sec,  (double)newBurst->generation_time.tv_usec);
	
        	if( headToRq == NULL)
        	{
            		haveBurst = 0;
        	}
        	else
        	{
            		haveBurst = 1;
        	}
        
        	//insert the burst
	    	insertLast(newBurst);

		//if runque is empty at first signal the S thread to start execute
	      	if( haveBurst == 0)
	      	{
	        	pthread_cond_signal(&cv);
	      	}

		//critical section ends, release lock variable
        	pthread_mutex_unlock(&rqLock);
	}
	pthread_exit(NULL);	
}


//this W thread function will generate burst when burst times and inter arrival times is given in the file
void *burst_generate_from_file(void *arg_ptr)
{
		

    char *fileNameThread = ((struct wThreadArg *) arg_ptr)->fileName;
    FILE * fp;
    char str[MAXCHAR];
    fp = fopen(fileNameThread , "r");

    if (fp == NULL)
    {
   	printf("dosya bulunamadý");
        exit(EXIT_FAILURE);
    }
   
    //printf("%s\n",((struct wThreadArg *) arg_ptr)->fileName);
    int i = 0;
    
    //iterate in the file for each line(burst)
    while( fgets( str, MAXCHAR, fp ) != NULL )
    {
	    double arr_time = 0;

	    double burst_time = 0;

	    //determine inter arrival time and burst time from file 
	    splitLine( str, &arr_time, &burst_time);
	    arr_time *= 1000;
		
	    //sleep as much as inter arrival time
	    usleep( (useconds_t)arr_time );
	    
	    //create burst struct with generated parameters
	    
	    
	    

	    

	//accessing runqueue is critical section, lock variable is acquired
	    pthread_mutex_lock(&rqLock);
	    
	    struct burst *newBurst = (struct burst*) malloc(sizeof(struct burst));

	    newBurst->threadInd = ((struct wThreadArg *) arg_ptr)->t_index;
	    newBurst->burstInd = i+1;
	    newBurst->burstLengt = burst_time;
	    gettimeofday(&(newBurst->generation_time), NULL);
	    newBurst->next = NULL;
	    
	   printf("Generated in thread: %d burst index: %d with burst length: %.2f and  and arrTime sec: %.2f and arrTime usec: %.2f\n", newBurst->threadInd, newBurst->burstInd, newBurst->burstLengt,   (double)newBurst->generation_time.tv_sec,  (double)newBurst->generation_time.tv_usec);
	    
	    int haveBurst;

            if( headToRq == NULL)
            {
                 haveBurst = 0;
            }
            else
            {
                 haveBurst = 1;
            }
            //insert the burst
	    insertLast(newBurst);

	    //if runque is empty at first signal the S thread to start execute
	    if( haveBurst == 0)
	    {
            	pthread_cond_signal(&cv);
            }

	    //critical section ends, release lock variable
            pthread_mutex_unlock(&rqLock);
            i++;
     }
	    pthread_exit(NULL);
}



//this function takes the algorithm name and assign them to a integer value
int determineAlgo( )
{
     if( strcmp("FCFS", algoName) == 0 )
        return 0;
     else if( strcmp("SJF", algoName) == 0 )
        return 1;
     else if( strcmp("PRIO", algoName) == 0)
        return 2;
     else if( strcmp("VRUNTIME", algoName) == 0)
        return 3;
}

//this S thread function will schedule and execute burst 
void *schedule_burst(void *arg_ptr)
{
    //determine the algorithm as integer value
    int algoNumber = determineAlgo();
        
    //iterate for total number of burst generated	
    for( int i = 0; i < ((struct sThreadArg*) arg_ptr)->noOfIte; i++){
        double time = 0;
        int threadID = -3;
        
        //access to runqueue is critical section, acquire the lock variable
        pthread_mutex_lock(&rqLock);

	//if runqueue is empty wait on a condition variable
        while( headToRq == NULL )
            pthread_cond_wait(&cv, &rqLock);
            
        struct timeval gene_time;
        //find the thread ID and generation time of the of burst which will be executed and find the burst time of that thread
        time = findBurstToExecute( algoNumber, &threadID, &gene_time );

	//if algorithm is VRUNTIME update the virtual run time values of the thread whose burst was executed
	if( algoNumber == 3)
	{	   
            vrunTimes[threadID-1] += time * ( 0.7 + 0.3 * ( threadID ));
        }
        
        struct timeval execution_time;
        gettimeofday(&execution_time, NULL);
        
        
        double secWait = execution_time.tv_sec - gene_time.tv_sec;
        double usecWait = execution_time.tv_usec - gene_time.tv_usec;
        
        secWait *= 1000000;  
        
        double totalWaitingInMicroSec = (secWait + usecWait) / 1000;
        
        //update total waiting times of the thread 
        avgWaitTimes[threadID-1] += totalWaitingInMicroSec;
        
        time*= 1000;
        
	//end of the critical section
        pthread_mutex_unlock(&rqLock);


	//execute the burst during its burst time by sleeping
        usleep( (useconds_t)time );
    }
    
    pthread_exit(NULL);
}



int main( int argc, char *argv[])
{


    //use different seeds for random numebr generation  
    srand ( time(NULL) );
    
    //initialize the mutex lock variable
    if( pthread_mutex_init(&rqLock, NULL)!= 0)
    {
        printf("mutex variable cannot be initialized");
    }

    //initialize the condition variable
    if( pthread_cond_init(&cv, NULL) != 0)
    {
        printf("condition variable cannot be initialized");
    }
           
    //if burst are generated randomly, take parameters of the program accordingly
    if (argc == 8) {
        THREAD_NUM = atoi(argv[1]);
        BURST_NUM = atoi(argv[2]);
        MIN_A = atoi(argv[5]);
        MIN_B = atoi(argv[3]);
        AVG_A = 1 / atof(argv[6]);
        AVG_B = 1 / atof(argv[4]);
     }
     //if burst are generated from a file, take parameters of the program accordingly
     else if ( argc == 5)
        THREAD_NUM = atoi(argv[1]);

    //initialize pthread ids
    pthread_t tids[THREAD_NUM + 1];
    //initialzie w thread arguments array
    struct wThreadArg t_args[THREAD_NUM];

	int ret;
	//hold variable for total number of bursts
	int numOfBurst = 0;
	for ( int i = 0; i < THREAD_NUM + 1; ++i) {

				
        if( i != THREAD_NUM)
        {
            //assign thread ids fow w threads
            t_args[i].t_index = i +1;
	    
	    //if bursts will be generated randomly then use burst_generate thread function
            if( argc == 8 )
            {
                strcpy(t_args[i].fileName, "");
                ret = pthread_create(&(tids[i]),
				     NULL, burst_generate, (void *) &(t_args[i]));
	         burstCounts[i] = BURST_NUM;
            }
            //if bursts will be generated from a file then use burst_generate_from_file thread function
            else if( argc == 5 )
            {
                char *inPrefix = argv[4];
                char extension[] = ".txt";
                char achar[] = "-";
                int threadInd = i + 1;
                char fileName[100];
                //find the name of each file for each thread and pass file name to w thread function
                snprintf(fileName, 100, "%s%s%d%s", inPrefix, achar, threadInd, extension);
                char str1[MAXCHAR];
                FILE * fp1;
                fp1 = fopen(fileName, "r");
                int lineNum = 0;
                //find the number of burst for each thread
                while( fgets( str1, MAXCHAR, fp1 ) != NULL )
		{
			lineNum = lineNum + 1;
		}
		
		burstCounts[i] = lineNum;
		//update the total number of burst in the program
		numOfBurst = numOfBurst + lineNum; 	
                strcpy(t_args[i].fileName, fileName );
                
                //initalize w thread while reading burst from file
                ret = pthread_create(&(tids[i]),
				     NULL, burst_generate_from_file, (void *) &(t_args[i]));
            }
		
	    //initalize virtual run times to zero
            vrunTimes[i] = 0;
            avgWaitTimes[i] = 0;
        }

        else
        {
            struct sThreadArg s1;
		
	    //pass the total number of burst and algorithm to S thread 
            if( argc == 8)
            {
                s1.noOfIte = THREAD_NUM * BURST_NUM;
                algoName =  argv[7];
            }
            else
            {
                algoName = argv[2];
                s1.noOfIte = numOfBurst;
            }
            s1.t_index = i + 1;
            ret = pthread_create(&(tids[i]),
				     NULL, schedule_burst, (void *) &(s1));
        }

		//give an error when thread creation failse
		if (ret != 0) {
			printf("thread create failed \n");
			exit(1);
		}
	}
	
	//wait for end of each thread
	for( int i = 0; i < THREAD_NUM + 1; ++i){
		 ret = pthread_join( tids[i], NULL);		 
	}
	for( int b = 0; b < THREAD_NUM; b++ )
	{
		printf("Total waiting time for thread %d (ms): %.2f \n", b+1, avgWaitTimes[b] / burstCounts[b] );
	} 
	printf("main: all threads terminated\n");
	return 0;
}
