/*
 * Name: Cam Nguyen
 * Programming Assignment 1
 * Description: Mavs Shell a simple shell program that
 * takes in 5 arguments and has a few custom functions
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

//Struct for our Queue since we want a FIFO data structure
//to take care of our pidlist data mangement
typedef struct
{
	int *queue;// where the pids will be stored
	int head;// top of the queue
	int tail;// end of our queue
	int size;//current size of the queue
}Queue;

//Defines the size of the queue
#define MAX_LIST_SIZE  11
//Defines the max of the command line
#define MAX_COMMAND_SIZE 255
//Defines the max size of arugments that the shell supports
#define MAX_NUM_ARG 6

//Defined the names of the commands I will write for the shell
#define QUIT  "quit"
#define EXIT  "exit"
#define WHITESPACE  " "
#define SHOWPID  "showpid"
#define CHANGEDIR  "cd"
#define NEWLINE "\n"

void init_queue(Queue *pid_list);

void update_queue(Queue *pid_list, int pid);

void show_pid(Queue *pid_list);
/*funct: handle_signal
 *param: int constant value of the signal
 *description: checks if the signal int value is 
 *one of the predefined ones we are trying to catch 
 *and then catches it doing nothing to it 
 */
static void handle_signal( int sig);
/*funct: execute_commands
 *param: pointer to the command, pointer to the aruments, 
 *the pointer to the queue of pid ids, the pointer to the input string, a integer
 * holding the pid status
 *descript: compares the command to the preset commands that are defined at the top of the code
 *returns 1 if they choose to quit and 0 otherwise so we can break out of the main loop and quit 
 *the shell. We pass the inputstring if the user just puts in a newline to call for input again
 * we use the pointer to the queue list so we can store and print off the pid list
 */
int execute_commands(char *command,char **arugments,Queue *pid_list, int status );
/*funct:run_process
 *param: pointer to the command, pointer to the list of arguments, the status,
 *pointer to the pid list
 *descript: does the processes for the program it takes in the arugments and the command
 *and passes them to the 4 locations of exec to have them run the command's prebuilt function
 *it also saves the pids into the pid list 
 */
void run_process(char *command, char **arguments, int status, Queue *pid_list);

int main(void) 
{	
	//Initializes quit to zero will be used as a flag to exit the loop
	//when it is set to 1
	int quit = 0;	
	//Allocates the command line string to the max size 
	char *command_str = (char*) malloc(MAX_COMMAND_SIZE);
	int status = 0;
	//Queue structure to handle the pid list and its updating and management 	
	Queue pid_list;
 
	//Creates a pointer to the 2d array with the size of the command line buffer for each cell
	char *tokens[MAX_NUM_ARG];
        //holds the number of inputs from the command line
	int argc = 0;
	//pointer to point to the token
        char *arg_ptr;
	
	struct sigaction act;

	memset(&act, '\0', sizeof(act));	
	act.sa_handler = &handle_signal;
	init_queue( &pid_list);
		
	while(!quit)
	{	
		printf("msh>");				

		if(sigaction(SIGINT, &act, NULL) < 0)
                {
                        perror("sigaction:");
                        return 1;
                }

	        else if(sigaction(SIGTSTP, &act, NULL) < 0)
        	{
        		perror("signaction:");
                	return 1;
        	}
	 	//Reads the command from the command line.
	 	//The max command that will  be read is MAX_COMMAND_SIZE
	 	//This while will wait until the user inputs something since
	 	//fgets returns NULL when there is no input
		while(!fgets(command_str,MAX_COMMAND_SIZE,stdin));		
		
		argc = 0;
	
		//compares the input to newline and if it isn't equal it runs because
		//strcmp returns 0 if the strings are equilavent
		if(strcmp(command_str,NEWLINE) )
		{
			
			//declares the working str to hold a copy of the command_str without
			//the new line return. The newline return is removed by allocate 
			//enough memory to hold the command string but subtracting 1 to
			//skip over copying the new line return 	
			char *working_str;
       		 	working_str = (char*) malloc(strlen(command_str)-1);
       			strncpy(working_str,command_str,strlen(command_str) - 1);
		
			//assigns the working_str to the working root
			//to track its original value to free the memeory
			//at the end
			char *working_root;
			working_root = (char*) malloc(strlen(working_str));
			strncpy(working_root,working_root,strlen(working_str));
		
			//Tokenizes the input strings with whitespace used as a delimiter
			
			while(((arg_ptr = strsep(&working_str,WHITESPACE)) != NULL) && (argc < MAX_NUM_ARG))
			{

				tokens[argc] = strndup(arg_ptr, MAX_COMMAND_SIZE);
				if( strlen( tokens[argc] ) == 0)
				{
					tokens[argc] =  NULL;
				}	
				argc++;
			}	
			
			//makes the last index of tokens a null so the list will be formatted for
			//the exec function			
			tokens[argc] = (char*)NULL;			
				                
			//Assigns execute commands return to quit
			//if the user entered quit assigns 1 to break out of the loop
			//else it just assigned 0 to continue the loop
			quit = execute_commands(tokens[0],tokens, &pid_list,status);
	
			//frees working root string if it was not null
			if(working_root)	
			{
				free(working_root);
			}
		}			
	}
	free(command_str);
	free(pid_list.queue);
	return 0;
}

void init_queue(Queue *pid_list)
{
	pid_list->queue = malloc( MAX_LIST_SIZE * sizeof(int*));
	// the queue is empty this will keep track of the number of elements as we add them
	pid_list->size = 0;
	//start the head at the top of the queue 
	pid_list->head = 0;
	//start the tail out of the queue
	pid_list->tail = -1;
}

void update_queue(Queue *pid_list, int pid)
{
	//checks if the queue is full
	if( pid_list->size != MAX_LIST_SIZE )

	{
		//sets it to minus 1 so we can reuse indices when full
		if(pid_list->tail == MAX_LIST_SIZE -1)
		{
			pid_list->tail = -1;
		}
		//increments the tails position so we can 
		// add the pid to the next area on the queue and 
		// increases the size
		pid_list->tail = pid_list->tail + 1;
		pid_list->queue[pid_list->tail] = pid;
		pid_list->size = pid_list->size + 1;	
	}
	//the queue was full so we increase the heads position
	//and we increase the tail to insert the pid size to insert
	else
	{	 
		pid_list->head = pid_list->head + 1;
		pid_list->tail = pid_list->tail + 1;
		pid_list->queue[pid_list->tail] = pid;
		pid_list->size = 10;
			
	}
}

void show_pid(Queue *pid_list)
{
	int i;
	//checks if the queue is empty
	if( pid_list->size == 0)

	{
		printf("No pids\n");
	}
	// the queue was not so we start from the head to the tail and print the pids
	else
	{
		for( i = pid_list->head ; i != pid_list->tail ; i++)
		{
			printf("%d\n",pid_list->queue[i]);
		}	
	}
}

static void handle_signal(int sig)
{
	switch(sig)
	{
		case SIGINT:
	
		break;
		case SIGTSTP:

		break;
		default:
			printf("Unable to determine the signal\n");
		break;
	}
}

int execute_commands(char *command, char **arugments, Queue *pid_list, int status)
{
	if(!strcmp(QUIT,command) || !strcmp(EXIT,command))
        {	
        	return 1;
	}

       else if(!strcmp(NEWLINE, command))
        {	
		return 0;       
	}

        else if(!strcmp(SHOWPID,command))        
	{     
		show_pid(pid_list);
		return 0;
	}

        else if(!strcmp(CHANGEDIR,command))
        {
		if(chdir(arugments[1]) == -1)
                {
			perror("");
                }
		return 0;
        }
	else if (strcmp("",command))
	{
		run_process(command,arugments,status, pid_list);
	}
	return 0;
}

void run_process(char *command, char **arguments, int status, Queue *pid_list)
{ 
	int i = 0;
	//stores the paths in which we will search for the command in the order
	//we will execute them
	char path_list[4][255] = {"/","/usr/local/bin/","/usr/bin/","/bin/" };
        
	int result[4] = { 0,0,0,0};
	//concatenates the path with the command so we can pass into the execv function
	for (i = 0; i < 4; i++)
        {
                strcat(path_list[i],command);
        }
	
	//creates a new process
        pid_t pid = fork();

	//in case the case that the fork fails
        if( pid == -1)
        {
        	perror("fork failed: ");
                exit( EXIT_FAILURE);
        }
	 
        else if( pid == 0)
	{         
		//runs execv through the path list starting from the current directory to bin to 
		//find the program we are trying to run and if it failed stores a -1 in the result
		//array if call the values in result are -1 tells the user that the command does not 
		//exist 
		for(i = 0; i < 4; i++)
		{
			if(execv(path_list[i], arguments) == -1)	
			{
				//Checks if the errno number wasn't that 
				//there is no such file or directory to supress
				//the output when the execv is looking in the wrong
				//directory for the program but it does exist and is 
				//correct in another area of the program
				if(errno != ENOENT)
				{
					perror("");
				}
				result[i] = -1;
			}		
		}
		
		if(result[0] == -1 && result[1] == -1 && result[2] == -1 && result[3] == -1)
		{
			printf("%s : Command not found.\n",command);
		}	                
        	
		fflush(NULL);
                exit(EXIT_SUCCESS);
        }
	//in the parent waits for the child to finish then flushes
        else
        {
		waitpid(pid , &status,0);	
		fflush(NULL);
        }
	
	//sets a flag for both the possible pids	
	int flag = 1;
	int flag1 = 1;	
	//checks if they exit already in the queue 
	//sets the flags to 0 
	for ( i =0 ; i < MAX_LIST_SIZE; i++)
	{
		if(pid_list->queue[i] == getpid())
		{
			flag = 0;
		}
		else if( pid_list->queue[i] == pid)
		{
			flag = 0;
		}
	
	}
	// adds to the queue if the they do not already exist in the queue
	if(flag == 1)
	{
		update_queue(pid_list,getpid());
	}
	if(flag1 == 1)
	{
        	update_queue(pid_list,pid); 	
	}
}	

