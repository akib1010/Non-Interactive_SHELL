//-------------------------------------
//Name: Farhan Akib Rahman
//Student Number:7854163
//Course: Comp3439
//Assignment: 2 , Question:1
//
//Remarks:Implement a non-interactive shell that can use redirection and pipes
//-------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <fcntl.h>

#define MAX_CHAR 100//Maximum characters of a single command line
int firstProcess=1;
int lastProcess=0;
int* allFd[2];
//Keeps track of how many pipes are in the command
int pipeNum=0;
//This function process a line which does not have pipes
int processLine(char** args,int size)
{
    int ret=0;//return value
    //dynamically allocate the arguments
    char *myArgs[size];
    int i;
    for(i=0;i<size;i++)
    {
        myArgs[i]=strdup(args[i]);
    }
    //Add NULL to the end of the line
    myArgs[size]=NULL;
    //Start processing
    int rc=fork();
    if(rc<0)
    {
        printf("Fork Failed!");
        ret=1;
    }
    //Child process
    else if(rc==0)
    {
        //transform the process
        execvp(myArgs[0],myArgs);
        //This lines of code should not be executed if exec() is successful
        printf("\nexecvp() failed\n");
        ret=1;
        
    }
    //Parent process
    else
    {
        wait(NULL);
    }
    return ret;
}

//This function runs a segment of a piped command
void pipeProcess(char** args,int size)
{
    //Dynamically allocate the argumets into an array
    char* myArgs[size];
    int i;
    for(i=0;i<size;i++)
    {
        myArgs[i]=strdup(args[i]);
    }
    //Add NULL to the end of the line
    myArgs[size]=NULL;
    
    int fd[2];
    int rc;
    if(pipe(fd)<0)
    {
        printf("\nPipe Error\n");
    }
    //Create a new process
    rc=fork();
    //If fork() fails
    if(rc<0)
    {
        printf("Fork Failed!");
    }
    //Child process
    else if(rc==0)
    {
        //If it is the first process
        if(firstProcess)
        {
            allFd[0][1]=fd[1];
            allFd[0][0]=fd[0];
            firstProcess=0;
            //close read end
            close(fd[0]);
            if(fd[1]!=STDOUT_FILENO)
            {
                //swap the file descriptor with Standard output
                dup2(fd[1],STDOUT_FILENO);
                close(fd[1]);
            }
            //execute the process
            execvp(myArgs[0],myArgs);
            //This lines of code should not be executed if exec() is successful
            printf("\nexecvp() failed\n");
        }
        //if it is the last process
        if(lastProcess)
        {
            allFd[pipeNum][1]=fd[1];
            allFd[pipeNum][0]=fd[0];
            //close the pipe
            close(fd[1]);
            close(fd[0]);
            if(allFd[pipeNum-1][0]!=STDIN_FILENO)
            {
                dup2(allFd[pipeNum-1][0],STDIN_FILENO);
            }
            //Execute the process
            execvp(myArgs[0],myArgs);
            //This lines of code should not be executed if exec() is successful
            printf("\nexecvp() failed\n");
        }
        allFd[pipeNum-1][1]=fd[1];
        allFd[pipeNum-1][0]=fd[0];
        //If the process is in the middle
        //swap file descriptors for input and output
        dup2(allFd[pipeNum-2][0],STDIN_FILENO);
        dup2(fd[1],STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        //Execute the process
        execvp(myArgs[0],myArgs);
        //This lines of code should not be executed if exec() is successful
        printf("\nexecvp() failed\n");
    }
    //parent process
    else
    {
        wait(NULL);
    }
    
}


//This function parses the command line string and passes the commands to be processed
void parseLine(char* line)
{
    char* args[MAX_CHAR];//The command line argument split by whitespace
    char* token;
    char* temp;
    int fd;
    int count=0;
    //Split by whitespace
    token=strtok(line," ");
    while(token!=NULL)
    {
        //If the command is an input redirection
        if(strcmp(token,"<")==0)
        {
            temp=strtok(NULL," ");
            //If the last character of the word is \n
            if(temp[strlen(temp)-1]=='\n')
            {
                temp[strlen(temp)-1]='\0';
            }
            fd=open(temp,O_RDONLY|O_CLOEXEC);
            dup2(fd,STDIN_FILENO);
        }
        //If the command is an output redirection
        else if(strcmp(token,">")==0)
        {
            temp=strtok(NULL," ");
            //If the last character of the word is \n
            if(temp[strlen(temp)-1]=='\n')
            {
                temp[strlen(temp)-1]='\0';
            }
            fd=open(temp,O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC);
            dup2(fd,STDOUT_FILENO);
        }
        //If the command is a pipe
        else if(strcmp(token,"|")==0)
        {
            //Indicate that this command has pipes and run the process that has been read so far
            pipeNum++;
            pipeProcess(args,count);
            //Reinitialize count so that the array can be reset
            count=0;
        }
        else
        {
            args[count]=token;
            count++;
        }
        token=strtok(NULL," ");
    }
    //Trim the last word of the argument
    if(args[count-1][strlen(args[count-1])-1]=='\n')
    {
        args[count-1][strlen(args[count-1])-1]='\0';
    }
    if(pipeNum==0)
    {
        //Process the line normally, if it fails give an error message
        if(processLine(args,count)==1)
        {
            printf("\nProcessing line %s ... %s Failed\n",args[0],args[count-1]);
        }
    }
    else
    {
        //This is the lastprocess of a piped command
        lastProcess=1;
        pipeProcess(args,count);
    }
}

int main(int argc, char* argv[])
{
    assert(argc>0);
    FILE *fp=fopen(argv[1],"r");//open the file
    char line[MAX_CHAR];
    //Check if the file exists
    if(fp==NULL)
    {
        printf("\nFile %s does not exist\n",argv[1]);
        exit(EXIT_FAILURE);
    }
    while(fgets(line,MAX_CHAR,fp)!=NULL)
    {
        parseLine(line);
    }
    fclose(fp);//close the file
    
    
    
    return EXIT_SUCCESS;
}
