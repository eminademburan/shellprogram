#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
  
#define BUFFER_SIZE 25
#define READ_END  0
#define WRITE_END  1
#define STDOUT_FILENO 1
#define STDIN_FILENO 0
        
void splitArgs( char* args[] , char str[] )
{
   
   int indexes = 0;
   
//program name to the first index
   args[indexes] = strtok(str, " ");
   
   
   while( args[indexes] != NULL)
   {
      indexes++;
      
//argumets will go to the other indexes
      args[indexes] = strtok(NULL, " ");
   }
   
}


int main(int argc, char *argv[] )
{  
   
   if (argc != 3)
   {     
      fprintf(stderr, "usage: isp.out <BYTE NUMBER> <PROGRAM MODE>");
      return -1;    
   }
   
   else if((argc == 3) && (atoi(argv[1]) <= 4096) && (atoi(argv[1]) > 0)){
      // this is normal communication mode
      if( atoi(argv[2]) == 1){
         
         while(1)
         {
            
            char str[100];
            scanf("%[^\n]%*c", str);
            char pipeChar = '|';
            char *contain = strchr( str, pipeChar);
                                   
            if( contain == NULL )
            {
            
            	 
               pid_t pid;
               
               //printf("command 1 %s \n" , str);
               
               char* args[30];
               
//split command into its arguments
               splitArgs(args, str);
                            
               /*struct timeval current_time;   
               struct timeval last_time;       
							 gettimeofday(&current_time, NULL);*/
               
               pid = fork();
               
               if( pid < 0)
               {
                  fprintf( stderr, "Fork failed");
                  exit(-1);
               }
               
               if( pid == 0 )
               {
                  execvp(args[0], args);
               }
               
               if( pid > 0)
               {
                  wait(NULL);
//printf("child completed \n");
									/*gettimeofday(&last_time, NULL);
									printf("%ld microseconds \n",  last_time.tv_usec - current_time.tv_usec);*/
                  
               }
               
               
               
            }
            
            else
            {
               char str2[100];
               
               strcpy( str2, str);
               
//take the first command
               char *token = strtok(str, "|");
               
               char *com1;
               com1 = token;
               
               char *com2;
               
               char *token2 = strtok(str2, "|");
               
               while( token2 != NULL )
               {
//take the second command
                  com2 = token2;
                  token2 = strtok( NULL, "|");
               }
               
               /*
                printf("command 1 %s \n" , com1);
                printf("command 2 %s \n" , com2);*/
               
//split commands into arguments
               char* args1[30];
               splitArgs( args1, com1);
               
               char* args2[30];
               splitArgs( args2, com2);
               
               
               /*struct timeval current_time;   
               struct timeval last_time;      
							 gettimeofday(&current_time, NULL);*/
							 
               /*for(int i = 0; i < 3; i++)
                {
                printf("args 1 %s \n", args1[i]);
                printf("args 2 %s \n", args2[i]);
                }*/
               
               
               pid_t pid;
               
//create file descriptor array
               int fd[2];
               
//create pipe for communication
               if(pipe(fd) == -1){ fprintf(stderr, "Piper Failed"); return 1;}
               pid = fork();
               
               if(pid < 0)
               {
                  fprintf( stderr, "Fork failed");
                  exit(-1);
               }
               
//first child will execute first program
               else if(pid==0)
               {
                  dup2(fd[WRITE_END], STDOUT_FILENO);
                  //close unused sides of pipes
                  close(fd[READ_END]);
                  execvp(args1[0], args1);
                  exit(1);
               }
               
               else
               {
                  pid=fork();
                  
                  if(pid < 0)
                  {
                     fprintf( stderr, "Fork failed");
                     exit(-1);
                  }
                  
//second child will execute second program
                  if(pid==0)
                  {
                     dup2(fd[READ_END], STDIN_FILENO);
//close unused side of pipe
                     close(fd[WRITE_END]);
                     execvp(args2[0], args2);
                     exit(1);
                  }
                  else
                  {
                     //close unused sides of pipe
                     close(fd[READ_END]);
                     close(fd[WRITE_END]);
                     
                     for(int i = 0 ; i < 2; i++)
                        wait(NULL);
                        
                     /*gettimeofday(&last_time, NULL);
                     printf("%ld microseconds \n",  last_time.tv_usec - current_time.tv_usec);*/
											//printf("child completed \n");
                  }
               }
            }
         }
         
      }
      
      
      //this is tapped communication mode
      else if( atoi(argv[2]) == 2){
         
         while(1)
         {
            						
            int readCount = 0;
            int writeCount = 0;
            int charCount = 0;
            int charSum = 0;
            
            //get the number of byte
            int numOfByte = atoi(argv[1]);
            char buffer[numOfByte];
            
            
            char str[100];
            scanf("%[^\n]%*c", str);
            
            char pipeChar = '|';
            char *contain = strchr( str, pipeChar);
            
            
            //if pipe symbol does not exist take new input
            if( contain == NULL )
            {
               printf("Second mode is only for compaund statemtns \n");
            }
            
            //if
            else
            {
               
               
               char str2[50];
               
               strcpy( str2, str);
               
//take the first command
               char *token = strtok(str, "|");
               
               char *com1;
               com1 = token;
               
               char *com2;
               
//take the second command
               char *token2 = strtok(str2, "|");
               
               while( token2 != NULL )
               {
                  com2 = token2;
                  token2 = strtok( NULL, "|");
               }
               
               /*printf("command 1 %s \n" , com1);
                printf("command 2 %s \n" , com2);*/
               
               
//split commands to their arguments
               char* args1[30];
               splitArgs( args1, com1);
               
               char* args2[30];
               splitArgs(args2, com2);
               
               /*struct timeval current_time;     
               struct timeval last_time;       
							 gettimeofday(&current_time, NULL);*/
               
               pid_t pid;
               
               int fd1[2];
               int fd2[2];
               
//create two pipes for main and two processes
               if(pipe(fd1) == -1){ fprintf(stderr, "Piper Failed"); return 1;}
               if(pipe(fd2) == -1){ fprintf(stderr, "Piper Failed"); return 1;}
               
               
               
               pid = fork();
               
               if(pid < 0)
               {
                  fprintf( stderr, "Fork failed");
                  exit(-1);
               }
               
//first child will execute first program
               else if(pid==0)
               {
                  dup2(fd1[WRITE_END], STDOUT_FILENO);
//close unused sides of pipes
                  close(fd1[READ_END]);
                  close(fd2[WRITE_END]);
                  close(fd2[READ_END]);
                  execvp(args1[0], args1);
                  exit(1);
               }
               else
               {
                  
                  pid=fork();
                  
                  if(pid < 0)
                  {
                     fprintf( stderr, "Fork failed");
                     exit(-1);
                  }
                  
//second child will execute second program
                  else if(pid==0)
                  {
                     dup2(fd2[READ_END], STDIN_FILENO);
//close unused sides of pipes
                     close(fd2[WRITE_END]);
                     close(fd1[WRITE_END]);
                     close(fd1[READ_END]);
                     execvp(args2[0], args2);
                     exit(1);
                  }
                  else
                  {
                     
//close unused sides of pipes
                     close(fd1[WRITE_END]);
                     close(fd2[READ_END]);
                     
//read the first pipe's read end and write to the seconds pipes write end
                     while( (charCount = read(fd1[READ_END],buffer,numOfByte)) > 0)
                     {
                        readCount++;
                        charSum = charSum + charCount;
//printf("%s \n", readBuffer);
                        write( fd2[WRITE_END], buffer, numOfByte);
                        writeCount++;
//printf("%s \n",writeBuffer);
                     }
                     
                     readCount++;
                     
//close unused sides of pipes
                     close(fd2[WRITE_END]);
                     close(fd1[READ_END]);
                     
                     for(int i = 0 ; i < 2; i++)
                        wait(NULL);
                        
                     /*gettimeofday(&last_time, NULL);
                     printf("%ld microseconds \n", last_time.tv_usec - current_time.tv_usec);*/
                     
//print results
                     printf("character-count %d \n", charSum);
                     printf("read-call-count %d \n", readCount);
                     printf("write-call-count %d \n", writeCount);
                     
                  }
               }              
            }            
         }     
      }
      return 0;
   }
   
}
